#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFOR_KLIENTA "/home/studinf/jdermont/dane"
#define BUFOR_SERWERA "/home/studinf/jdermont/wyniki"
#define LOCKFILE "/home/studinf/jdermont/lockfile"

int main() {
    char wiadomosc[256] = "";
    char odpowiedz[256] = "";
    int file;
    char c;
    int i;

    unlink(BUFOR_SERWERA);
    unlink(LOCKFILE);
    printf("Serwer nasluchuje...\n");
    while (1 == 1) {
        while ((file = open(BUFOR_KLIENTA, O_RDONLY, S_IRWXU)) == -1);
        read(file, wiadomosc, 256);
        printf("Wiadomosc od uzytkownika ");
        i = 0;
        while((c = wiadomosc[i++]) != '\n') putchar(c);
        printf(":\n");
        while((c = wiadomosc[i++]) != 27) putchar(c);
        printf("\n");
        close(file);

        printf("\nWyslij odpowiedz (ESC i Enter koncza):\n");
        i = 0;
        do {
            c = getchar();
            odpowiedz[i++] = (char)c;
        } while (c != 27);
        odpowiedz[i] = '\0';
        file = open(BUFOR_SERWERA,O_WRONLY|O_CREAT|O_EXCL|O_TRUNC, S_IRWXU);
        if (file == -1) {
            printf("Nie mozna otworzyc pliku: %s\n",BUFOR_SERWERA);
            exit(1);
        }
        write(file, odpowiedz, strlen(odpowiedz));
        close(file);
        printf("Wiadomosc wyslana.\n");
        unlink(BUFOR_KLIENTA);
        unlink(LOCKFILE);
    }
    return 0;
}

