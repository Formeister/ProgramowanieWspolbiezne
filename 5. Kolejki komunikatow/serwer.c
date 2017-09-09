#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define WORD_SIZE 64
#define DB_SIZE 8


void the_end();

typedef struct{
    long type;
    char word[WORD_SIZE];
}message;

typedef struct{
	char * word_pl;
	char * word_en;
}translation;


//Funkcja inicjująca bazę słów
void DbInit(void * db){

    ((translation *) db)[0].word_pl = "cześć";
    ((translation *) db)[0].word_en = "hello";

    ((translation *) db)[1].word_pl = "zegar";
    ((translation *) db)[1].word_en = "clock";

    ((translation *) db)[2].word_pl = "komputer";
    ((translation *) db)[2].word_en = "computer";

    ((translation *) db)[3].word_pl = "czas";
    ((translation *) db)[3].word_en = "time";

    ((translation *) db)[4].word_pl = "programowanie";
    ((translation *) db)[4].word_en = "programming";

    ((translation *) db)[5].word_pl = "kot";
    ((translation *) db)[5].word_en = "cat";

    ((translation *) db)[6].word_pl = "pies";
    ((translation *) db)[6].word_en = "dog";

    ((translation *) db)[7].word_pl = "dziewczyna";
    ((translation *) db)[7].word_en = "girl";

    printf("Inicjacja serwera zakończona powodzeniem.\n");
}

//Wyszukiwarka słowa
int search(void * db, int length, const char * word)
{
    int i;
    char foo[WORD_SIZE];
    strcpy(foo, word);
    foo[strlen(word)-1] = '\0';

    for(i = 0; i < length; i++){
        if(strcmp(((translation *) db)[i].word_pl, foo) == 0){
            //printf("Znaleziono: %s", ((translation *) db)[i].word_en);
	    return i;
	}
    }
    return -1;
}


int req_id, res_id;


int main(){

    message req;
    message res;
    
    translation db[DB_SIZE];

    DbInit(db);

    key_t key_req=1111;
    key_t key_res=2222;
    int search_result = 0;

    signal(SIGINT, the_end);

    //Tworzenie kolejki komunikatu zapytania
    req_id = msgget(key_req, 0666 | IPC_CREAT);
    if (req_id == -1) {
        fprintf(stderr, "Błąd podczas tworzenia komunikatu zapytania: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Tworzenie kolejki komunikatu odpowiedzi
    res_id = msgget(key_res, 0666 | IPC_CREAT);
    if (res_id == -1){
        fprintf(stderr, "Błąd podczas tworzenia komunikatu odpowiedzi: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Pobieranie komunikatów od klientów
    for(;;){
        if (msgrcv(req_id, &req, WORD_SIZE, 0, 0) == -1){
            fprintf(stderr, "Pobieranie komunikatu od klienta zakończone niepowodzeniem: %d\n", errno);
            exit(EXIT_FAILURE);
        }

    	res.type = getpid();

        search_result = search(db, DB_SIZE, req.word); 

        if (search_result != -1){
            strcpy(res.word, db[search_result].word_en);   
        }
        else{
            strcpy(res.word, "Nie znam takiego słowa");
        }

        //Wysyłanie odpowiedzi do klienta
        if (msgsnd(res_id, &res, WORD_SIZE, 0) == -1){
            fprintf(stderr, "Wysyłanie komunikatu zakończone niepowodzeniem.\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}

//Usuwanie kolejek
void the_end(){
    printf("\nUsuwanie kolejek komunikatów...");
    if (msgctl(req_id, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Usuwanie kolejki zapytania zakończone niepowodzeniem.\n");
        exit(EXIT_FAILURE);
    }
    if (msgctl(res_id, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Usuwanie kolejki odpowiedzi zakończone niepowodzeniem.\n");
        exit(EXIT_FAILURE);
    }
    printf(" SUKCES\n");

    exit(EXIT_SUCCESS);
}

