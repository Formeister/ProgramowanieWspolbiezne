#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SERVERFIFO "serwerfifo"
#define CLIENTFIFO "klientfifo"
#define DB_SIZE 20

typedef struct {
    int ID;
    char* nazwisko;
} record;


int main(int argc, char *argv[]){

  int i, fd, id;
  char user_message[150];
  char user_path[100];
  int user_path_length;
  int found;
  char *result;
  char server_response[200] = {};
  char client_fifo[100] = "";
  
  //Inicjacja bazy
  record database[DB_SIZE] = {};

  for(i = 1; i <= 14; i++) database[i].ID = i;

  database[0].nazwisko = "Fabianski";
  database[1].nazwisko = "Pazdan";
  database[2].nazwisko = "Pazdan";
  database[3].nazwisko = "Piszczek";
  database[4].nazwisko = "Formela";
  database[5].nazwisko = "Lewandowski";
  database[6].nazwisko = "Milik";
  database[7].nazwisko = "Zielinski";
  database[8].nazwisko = "Teodorczyk";
  database[9].nazwisko = "Krychowiak";
  database[10].nazwisko = "Blaszczykowski";
  database[11].nazwisko = "Glik";
  database[12].nazwisko = "Grosicki";
  database[13].nazwisko = "Peszko";
  database[14].nazwisko = "Szczesny";
 
  mkfifo(SERVERFIFO, S_IRWXU);  

  printf("Inicjacja serwera zakończona.\n");

  while (1){
    printf("Oczekiwanie na zapytania klientów...\n");
    //Odczyt kolejki serwerfifo
    fd = open(SERVERFIFO, O_RDONLY, S_IRWXU);
    read(fd, user_message, 2);
    if (strlen(user_message) != 0){
      //printf("user_message %s\n", user_message);

    //Pobieranie długości $HOME klienta  
    user_path_length = user_message[0];  

    //printf("user_path_length %d\n", user_path_length);

    //Pobieranie wartości id od klienta
    id = user_message[1];
    //printf("id %d\n", id);

    //Inicjacja zmiennej dla $HOME klienta
    //user_path = (char *) malloc(user_path_length*sizeof(char)); 
    
    //Zapisywanie $HOME klienta
    read(fd, user_path, user_path_length-1);
    //printf("user_path %s\n", user_path);

    //Przeszukiwanie bazy
    printf("Zapytanie o rekord z numerem id = %d\n", id);
    found = 0;   
    for (i = 0; i < DB_SIZE; i++) {
      if (database[i].ID == id) {
        result = malloc(strlen(database[i].nazwisko)*sizeof(char)+1);
        strcpy(result,database[i].nazwisko);
	found = 1;
	break;
      }
    }

    if (found == 0) {
      result = malloc(6*sizeof(char)+1);
      strcpy(result,"Nie ma");
    }

    //Czyszczenie
    int j;
    for (j=0;j<200;j++) server_response[j] = '\0';
    
    server_response[0] = strlen(result);
    strcat(server_response, result);

    //Definiowanie ścieżki do klientfifo
    strcpy(client_fifo,"");
    //strcat(client_fifo, user_path);
    strcat(client_fifo, CLIENTFIFO);

    printf("Próba zapisu do kolejki: %s\n", client_fifo);

    // write response to response buffer
    fd = open(client_fifo, O_WRONLY, 0);
    if (fd < 0) {
      printf("Błąd podczas odczytu kolejki.\n");
      continue;
    }
    
    int foo;
    foo = write(fd, server_response, strlen(server_response));
    if (foo > 0) printf("Zapis do kolejki zakończony powodzeniem. Rezultat: %s\n", result);
    close(fd);
    free(result);
    }
  }
  return 0;
}
