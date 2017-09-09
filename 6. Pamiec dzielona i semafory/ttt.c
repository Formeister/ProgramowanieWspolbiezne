#include <stdio.h>
#include <stdlib.h>


#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>

#define TRUE (1==1)
#define FALSE (1==0)
#define BOARD_SIZE 3
#define SHM_KEY 4321 //Klucz pamięci współdzielonej
#define SEM_KEY 5432 //Klucz tablicy semaforów


//char board[BOARD_SIZE][BOARD_SIZE];
struct shared_use_st {
    char board[BOARD_SIZE][BOARD_SIZE];
};


int sem_id;
int shm_id;
char *sh_mem;
int the_end = FALSE;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};


//Nadawanie wartości pojedynczemu semaforowi
int set_sem(int sem_id, int sem_num, int value) {
    union semun sem_union;
    sem_union.val = value;
    if (semctl(sem_id, sem_num, SETVAL, sem_union) == -1) return 0;
    return 1;
}

//Operacja na semaforze
void op_sem(int sem_num, int sem_op) {
    struct sembuf sem;
    sem.sem_num = sem_num;
    sem.sem_op = sem_op;
    sem.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem, 1) == -1) {
        //fprintf(stderr, "Błąd podczas operacji na semaforze.\n");
        exit(EXIT_FAILURE);
    }
}

//Usuwanie tablicy semaforów
void del_sem(int sem_id){
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1){
        fprintf(stderr, "Błąd podczas usuwania tablicy semaforów.\n");
        exit(EXIT_FAILURE);
    }
}

//Odłączanie pamięcie współdzielonej
void dt_shm(){
    if (shmdt(sh_mem) == -1){
        fprintf(stderr, "Błąd podczas odłączania pamięci współdzielonej.\n");
        exit(EXIT_FAILURE);
    }
}

//Usuwanie pamięcie współdzielonej
void del_shm(int shm_id){
    if (shmctl(shm_id, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Błąd podczas usuwania pamięci współdzielonej.\n");
        exit(EXIT_FAILURE);
    }
}

//Usuwanie semaforów oraz odłączanie pamięci współdzielonej
void clean() {
    printf("Usuwanie tablicy semaforów... ");
    del_sem(sem_id);
    printf("OK\n");
    printf("Odłączanie pamięci współdzielonej... ");
    dt_shm(sh_mem);
    printf("OK\n");    
    printf("Usuwanie pamięci współdzielonej... ");
    del_shm(shm_id);
    printf("OK\n");

    exit(EXIT_SUCCESS);    
}


//Rysowanie planszy
void draw_board(char board[BOARD_SIZE][BOARD_SIZE]){

    int i, j;
    int k = 0;

    printf("\n");
    for (i = 0; i < BOARD_SIZE; i++){
        for(j = 0; j < BOARD_SIZE; j++){
            printf(" %c", board[i][j]);
            if(j==0 || j==1) printf(" |");
            k++;
        }
        if(i==0 || i==1) printf("\n-----------\n");
    }
    printf("\n\n");
}

//Tworzenie tablicy semaforów, rywalizacja o pierwszy ruch
int init_sems() {

    int player_id;

    if ((sem_id = semget(SEM_KEY, 2, 0777 | IPC_CREAT | IPC_EXCL)) != -1){
        player_id = 1;
    }
    else{
        sem_id = semget(SEM_KEY, 2, 0777 | IPC_CREAT);
        player_id = 2;
    }

    if (!set_sem(sem_id, 0, 0)){
        fprintf(stderr, "Błąd podczas tworzenia tablicy semaforów.\n");
        exit(EXIT_FAILURE);
    }
    if (!set_sem(sem_id, 1, 1)){
        fprintf(stderr, "Błąd podczas tworzenia tablicy semaforów.\n");
        exit(EXIT_FAILURE);
    }

    return player_id;
}

//Tworzenie pamięci współdzielonej
void init_sh_mem(){
    shm_id = shmget(SHM_KEY, sizeof(struct shared_use_st), 0777 | IPC_CREAT);
    if (shm_id == -1){
        fprintf(stderr, "Błąd podczas tworzenia pamięci współdzielonej.\n");
        exit(EXIT_FAILURE);
    }
    sh_mem = shmat(shm_id, 0, 0);
    if (*sh_mem == -1){
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }


    //Wypełnianie planszy
    struct shared_use_st* shared_variables;
    shared_variables = (struct shared_use_st*)sh_mem;
    int i, j;
    int c = 1;
    for (i = 0; i < BOARD_SIZE; i++){
        for (j = 0; j < BOARD_SIZE; j++){
            (shared_variables->board)[i][j] = c + '0';
            c++;
        }
    }
}

//Pobieranie znaku gracza
char get_player_mark(int player_id) {
    return player_id == 1 ? 'X' : 'O';
}

//Sprawdzanie czy plansza jest w pełni zapełniona
int is_board_complete(char board[BOARD_SIZE][BOARD_SIZE]) {
    int i, j;
    int c = 1;
    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == c + '0') {
                return FALSE;
            }
	    c++;
        }
    }
    return TRUE;
}

//Sprawdzanie 
int is_line_complete(char mark, char board[BOARD_SIZE][BOARD_SIZE]) {
    int i, j;
    int result;

    //Linie poziome
    for (i = 0; i < BOARD_SIZE; i++) {
        result = TRUE;
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] != mark) {
                result = FALSE;
            }
        }
        if (result) return result;
    }

    //Linie pionowe
    for (i = 0; i < BOARD_SIZE; i++) {
        result = TRUE;
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[j][i] != mark) {
                result = FALSE;
            }
        }
        if (result) return result;
    }
    //Skos - lewa góra, prawy dół
    result = TRUE;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i][i] != mark) {
            result = FALSE;
        }
    }
    if (result) return result;

    //Skos - prawa góra, lewy dół
    result = TRUE;
    for (i = 0; i < BOARD_SIZE; i++) {
        if (board[i][BOARD_SIZE - 1 - i] != mark) {
            result = FALSE;
        }
    }
    return result;
}

void win_check(char board[BOARD_SIZE][BOARD_SIZE], int player_id) {
    int winner;

    if (is_line_complete(get_player_mark(1), board)) {
        winner = 1;
    }
    else if (is_line_complete(get_player_mark(2), board)){
        winner = 2;
    }
    else if (is_board_complete(board)){
        winner = 0;
    }
    else{
        winner = -1;
    }
    if (winner > 0){
        if (player_id == winner){
            printf("WYGRANA! : )\n\n");
            // zwalniamy przegranego, żeby mógł posprzątać
            op_sem(1 - (player_id - 1), 1);
            the_end = TRUE;
        }
        else{
            printf("PRZEGRANA : (\n\n");
            clean(); // przegrany musi posprzątać
        }
    }
    else if (winner == 0){
        printf("Rozgrywka zakończona remisem.\n\n");
        if (player_id == 1){
            // zwalniamy gracza 2, żeby mógł posprzątać
            op_sem(1 - (player_id - 1), -1);
            the_end = TRUE;
        }
        else{
            clean(); // w razie remisu gracz 2 sprząta
        }
    }
}


void move(int row, int column, int player_id, char board[BOARD_SIZE][BOARD_SIZE]){
    printf("row: %d, column: %d\n", row, column);
    board[row][column] = get_player_mark(player_id);
}

int is_move_correct(int choice, int *row, int *column, char board[BOARD_SIZE][BOARD_SIZE]){

    int i, j;
    int result = FALSE;

    for (i = 0; i < BOARD_SIZE; i++) {
        for (j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == choice + '0') {
                result = TRUE;
                *row = i;
		*column = j;
                goto end;
            }
        }
    }    
    end:
    printf("Choice: %d\n", choice);
    return choice >= 1 && choice <= (BOARD_SIZE*BOARD_SIZE) && result;
}

void get_move(int *choice, int *row, int *column, char board[BOARD_SIZE][BOARD_SIZE]){
    int first_iteration = TRUE;
    do {
        if (!first_iteration){
            printf("Nieprawidłowy ruch! Wybierz inną pozycję: ");
        }
        scanf("%d", choice);
        first_iteration = FALSE;
    } while (!is_move_correct(*choice, row, column, board));
}

void gameplay(int player_id) {
    struct shared_use_st* shared_variables;
    shared_variables = (struct shared_use_st*)sh_mem;
    int other_player_id = 3 - player_id;
    if (player_id == 1) {
        system("clear");
        draw_board(shared_variables->board);
        printf("Oczekiwanie na gracza %d...\n", other_player_id);
    }
    if (player_id == 2) {
        op_sem(other_player_id - 1, 1);
        op_sem(player_id - 1, -1);
        system("clear");
        draw_board(shared_variables->board);
        printf("Oczekiwanie na ruch gracza %d...\n", other_player_id);

    }
    while (TRUE) {
        op_sem(player_id - 1, -1);
        system("clear");
        draw_board(shared_variables->board);
        win_check(shared_variables->board, player_id);
        printf("Gracz %d: ", player_id);
        printf("wybierz pozycję: ");
        int choice, row, column;
        get_move(&choice, &row, &column, shared_variables->board);
        move(row, column, player_id, shared_variables->board);
        system("clear");
        draw_board(shared_variables->board);
        win_check(shared_variables->board, player_id);
        if (the_end != TRUE) printf("Oczekiwanie na ruch gracza %d...\n", other_player_id);
        op_sem(other_player_id - 1, 1);
    }
}


int main(){

    signal(SIGINT, clean);
    init_sh_mem();
    // graczem 1 zostaje ten, kto pierwszy utworzył semafory
    int player_id = init_sems();

    gameplay(player_id);

    return EXIT_SUCCESS;

}


