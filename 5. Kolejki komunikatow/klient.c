#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>


#define WORD_SIZE 64

typedef struct{
    long type;
    char word[WORD_SIZE];
}message;


int main(){

    message req;
    message res;

    int req_id, res_id;
    key_t key_req=1111;
    key_t key_res=2222;
    char word_buf[WORD_SIZE]; 

    
    //Tworzenie kolejki komunikatu zapytania
    req_id = msgget(key_req, 0666 | IPC_CREAT);
    if (req_id == -1) {
        fprintf(stderr, "Błąd podczas tworzenia komunikatu zapytania: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Tworzenie kolejki komunikatu odpowiedzi
    res_id = msgget(key_res, 0666 | IPC_CREAT);
    if (res_id == -1) {
        fprintf(stderr, "Błąd podczas tworzenia komunikatu odpowiedzi: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    //Pobieranie danych do zapytania
    printf("Proszę wprowadzić słowo do przetłumaczenia na język angielski:\n");
    fgets(word_buf, WORD_SIZE, stdin);
    req.type = getpid();
    strcpy(req.word, word_buf);
  
    //Wysyłanie komunikatu na koniec kolejki
    if (msgsnd(req_id, &req, WORD_SIZE, 0) == -1){
        fprintf(stderr, "Wysyłanie komunikatu zakończone niepowodzeniem.\n");
        exit(EXIT_FAILURE);
    }

    //Pobieranie komunikatu od serwera
    //printf("Słowo do przetłumaczenia zostało wysłane.\nOczekiwanie na odpowiedź ze strony serwera...\n");
    if (msgrcv(res_id, &res, WORD_SIZE, 0, 0) == -1){
        fprintf(stderr, "Pobieranie komunikatu od serwera zakończone niepowodzeniem: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("Odpowiedź: %s\n", res.word);
    return 0;
}

