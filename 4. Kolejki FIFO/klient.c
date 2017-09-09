#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define SERVERFIFO "serwerfifo"
#define CLIENTFIFO "klientfifo"

//Funkcja pomocnicza do walidacji odczytu id
int clean_stdin()
{
    while (getchar()!='\n');
    return 1;
}

int main(int argc, char *argv[]){

  char *home = getenv("HOME");
  int fd;
  //char user_path[100] = "";
  int id = 0;
  char *user_path;
  char message[150] = "";
  char client_fifo[100] = "";
  char server_response[200] = "";
  int response_length;
  char response[100] = "";

  //Sprawdzanie argumentów
  if(argc != 2){
    printf("Błędna liczba argumentów (%d), spodziewany argument: użytkownik\n", argc-1);	
    exit(1);
  }

  //Sprawdzanie długości łańcucha $HOME klienta
  int user_path_length = strlen(argv[1]) + strlen(home);
  
  //Deklaracja długości łańcucha $HOME klienta
  user_path=(char *) malloc(user_path_length*sizeof(char));  
  
  //Przypisanie $HOME klienta
  strcat(user_path, argv[1]);
  strcat(user_path, home);

  //Pobieranie id z walidacją
  char c;
  int counter = 0;

  do {
    if (counter == 0) printf("Proszę podać numer id wyszukiwanego elementu:\n");
    else printf("Błędna wartość.\nProszę ponownie podać numer id wyszukiwanego elementu:\n");
    counter++;
  } while (((scanf("%d%c", &id, &c)!=2 || c!='\n') && clean_stdin()) || id<1);

  //Tworzenie komunikatu
  message[0] = user_path_length+1;
  message[1] = id;
  strcat(message, user_path);

  //strcat(client_fifo, user_path);
  strcat(client_fifo, CLIENTFIFO);

  mkfifo(client_fifo, S_IRWXU);

  printf("Oczekiwanie na serwer...\n");
  //Otwieranie kolejki serwerfifo
  fd = open(SERVERFIFO, O_WRONLY, 0);
  if (fd < 0){
    printf("Błąd podczas odczytu kolejki.\n");
    return(-2);
  }

  write(fd, message, strlen(message));
  close(fd);
  printf("Zapytanie zostało wysłane.\n");


  printf("Oczekiwanie na odpowiedź serwera...\n");
  fd = open(client_fifo, O_RDONLY, S_IRWXU);
  read(fd, server_response, 1);
  response_length = server_response[0];
  read(fd, response, response_length);

  printf("Odpowiedź odesłana przez serwer:\n%s\n", response);
  close(fd);

  unlink(client_fifo);

  return 0;

}
