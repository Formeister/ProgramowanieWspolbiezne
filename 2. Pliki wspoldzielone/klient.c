#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Podaj nazwe serwera jako parametr. np ./klient jdermont\n");
        exit(0);
    }
    char *serwer = argv[1];
    char lockfile[64] = "";
    strcat(lockfile,"/home/studinf/");
    strcat(lockfile,serwer);
    strcat(lockfile,"/lockfile");

    printf("Nazwa serwera: %s\n",serwer);
    while (open(lockfile, O_CREAT|O_EXCL, 0) == -1) {
        printf("Serwer zajety lub niedostepny, prosze czekac...\n");
        sleep(3);
    }

    char wiadomosc[256];
    printf("Wpisz wiadomosc. ESC i Enter konczy.\n");
    int z = 0;
    unsigned char c;
    do {
        c = getchar();
        wiadomosc[z++] = c;
    } while (c != 27);
    wiadomosc[z] = '\0';

    char bufor_klienta[64] = "";
    strcat(bufor_klienta,"/home/studinf/");
    strcat(bufor_klienta,serwer);
    strcat(bufor_klienta,"/dane");
    char bufor_serwera[64] = "";
    strcat(bufor_serwera,"/home/studinf/");
    strcat(bufor_serwera,serwer);
    strcat(bufor_serwera,"/wyniki");
    int file;
    file = open(bufor_klienta,O_WRONLY|O_CREAT|O_EXCL|O_APPEND, S_IRWXU);
    if (file == -1) {
        printf("Wystapil blad, nie mozna otworzyc pliku %s.\n",bufor_klienta);
        exit(1);
    }
    write(file, "jdermont", strlen("jdermont"));
    write(file, "\n", 1);
    write(file, wiadomosc, z);
    close(file);
    open(lockfile, O_CREAT|O_EXCL, 0);

    printf("Czekam na odpowiedz serwera...\n");
    while ((file = open(bufor_serwera, O_RDONLY, S_IRWXU)) == -1);
    
    char odpowiedz[256] = "";
    read(file, odpowiedz, 256);
    int i = 0;
    while ((c = odpowiedz[i]) != 27) i++;
    odpowiedz[i] = '\0';
    printf("Odpowiedz serwera:\n%s\n", odpowiedz);
    close(file);
    unlink(bufor_serwera);

    return 0;
}
