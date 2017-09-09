#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>


#define PORT 5000
#define SERVER_IP "153.19.1.202"
#define FAMILY AF_INET


int main(int argc, char *argv[]){

    int request_number, buffer;
    struct sockaddr_in address;
    unsigned int alen;
    int sock;

    //Sprawdzanie argumentów
    if(argc != 2){
        printf("Błędna liczba argumentów (%d), spodziewany argument: liczba\n", argc-1);	
        exit(1);
    }

    //Pobieranie argumentu
    request_number = atoi(argv[1]);

    //Tworzenie socketu
    if ((sock = socket(FAMILY, SOCK_DGRAM, 0)) == -1 ){
        fprintf(stderr, "Błąd podczas tworzenia socketu.");
        exit(1);
    }

    //Definiowanie adresu
    address.sin_family = FAMILY;
    address.sin_port = htons(PORT);
    if ((inet_pton(FAMILY, SERVER_IP, &address.sin_addr)) == 0 ){
        fprintf(stderr, "Błąd podczas konwersji IP: %s\n", strerror(errno));
        exit(1);
    }
    alen = sizeof(struct sockaddr_in);

    buffer = htonl(request_number);

    printf("Wysyłanie zapytania...\n");
    int send = sendto(sock, (char *)&buffer, sizeof(int), 0, (struct sockaddr *) &address, alen);
    if (send == -1){
        fprintf(stderr, "Błąd podczas wysyłania zapytania: %s\n", strerror(errno));
        exit(1);
    }

    printf("Pobieranie odpowiedzi...\n");
    int receive = recvfrom(sock, (char *)&buffer, sizeof(int), 0, (struct sockaddr *) &address, &alen);
    if (receive == -1){
        fprintf(stderr, "Błąd podczas pobierania odpowiedzi: %s\n", strerror(errno));
        exit(1);
    }   
    buffer = ntohl(buffer);    

    printf("Dla zapytania \"%d\" odebrano odpowiedź: \"%d\"\n", request_number, buffer);

    return 0;
}
