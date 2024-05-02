#include <stdint.h>
#include <time.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MainCall.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

//------------------------2.feladat------------------------//
//------------------------Ez a függvény létrehoz egy időben változó mérési adatsort, ahol az értékek egyaránt növekedhetnek és csökkenhetnek véletlenszerűen.------------------------//
int Measurement(int **Values)
{
    srand(time(NULL));
    int szam = time(NULL) % 3600;
    int tmp_mins = (szam / 60) % 15;
    int tmp_secs = szam % 60;
    int seconds = tmp_mins * 60 + tmp_secs;

    if (seconds < 100)
    {
        seconds = 100;
    }

    float tmp_rand;
    int *Numbers = (int *)malloc(seconds * sizeof(int));
    int nums = 0;

    Numbers[0] = nums;

    for (int i = 1; i < seconds; i++)
    {
        tmp_rand = rand() / (float)RAND_MAX;

        if (tmp_rand < 0.428571)
        {
            nums++;
        }
        
        else if (tmp_rand > 1.0 - (11.0 / 31.0))
        {
            nums--;
        }

        Numbers[i] = nums;
    }

    *Values = Numbers;

    return seconds;
}

//------------------------3.feladat------------------------//
//------------------------Ez a függvény létrehoz egy BMP formátumú képfájlt a kapott adatok alapján.------------------------//
void BMPcreator(int *Values, int NumValues)
{
    unsigned long int fsize = (62 + (NumValues * NumValues));
    unsigned int pad = 0;
    int file = open("chart.bmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH);
    unsigned char **tomb = (unsigned char **)calloc(NumValues, sizeof(unsigned char *));

    unsigned char f_headder[] = 
    {
        0x42,
        0x4d,
        0x00,
        0x00,
        0x00,
        0x00,
        0x3e,
        0x00,
        0x00,
        0x00,
        0x28,
        0x00,
        0x00,
        0x00,
        0x01,
        0x00,
        0x01,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x61,
        0x0f,
        0x00,
        0x00,
        0x61,
        0x0f,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x11,
        0x00, // Fekete
        0x00, // Fekete
        0x00, // Fekete
        0xFF, // Fehér
        0xFF, // Fehér
        0xFF, // Fehér
        0x00, // Kék
        0x00  // Kék
    };

    for (int i = 0; i < NumValues; ++i)
    {
        tomb[i] = (unsigned char *)calloc(NumValues, sizeof(unsigned char));
    }

    for (int i = 0; i < NumValues; ++i)
    {
        int elem = (NumValues / 2) + Values[i];
        if (elem > NumValues)
        {
            elem = NumValues - 1;
        }

        else if (elem < -(NumValues / 2))
        {
            elem = 0;
        }

        tomb[elem][i] = 0x01;
    }

    write(file, f_headder, sizeof(unsigned char) * 2);
    write(file, &fsize, sizeof(int));
    write(file, &f_headder[2], sizeof(unsigned char) * 12);
    write(file, &NumValues, sizeof(int));
    write(file, &NumValues, sizeof(int));
    write(file, &f_headder[14], sizeof(unsigned char) * 36);

    for (int i = 0; i < NumValues; i++)
    {
        int x = 7;

        for (int j = 0; j < NumValues; ++j)
        {
            if (tomb[i][j] == 0x01)
            {
                int ps = 1 << x;
                pad = pad + ps;
            }

            if (x == 24)
            {
                write(file, &pad, sizeof(int));
                pad = 0;
                x = 7;
                continue;
            }

            if (x % 8 == 0)
            {
                x = (j % 32) + 9;
            }

            --x;
        }

        write(file, &pad, sizeof(int));
        pad = 0;
    }

    free(tomb);
    close(file);
}

//------------------------4.feladat------------------------//
//------------------------Ez a függvény a futó chart elnevezésű folyamat PID-jét keresi a /proc könyvtárban lévő fájlok alapján.------------------------//
int FindPID()
{
    DIR *dir = opendir("/proc");

    if (dir == NULL)
    {
        printf("Hiba a könyvtar megnyitasakor");
        exit(11);
    }

    struct dirent *entry;
    pid_t ownpid = getpid();
    char filename[256];
    char line[256];

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
            {
                continue;
            }

        strncpy(filename, "/proc/", 256);
        strncat(filename, entry->d_name, 256 - strlen(filename));
        strncat(filename, "/status", 256 - strlen(filename));

        FILE *fp = fopen(filename, "r");

        if (fp == NULL)
        {
            printf("Hiba a fajl megnyitasakor!");
            exit(12);
        }

        while (fgets(line, 256, fp))
        { 
            if (strncmp(line, "Name:", 5) == 0 && strstr(line, "\tchart\n") != NULL)
            { 
                while (fgets(line, 256, fp))
                { 
                    if (strncmp(line, "Pid:", 4) == 0)
                    {
                        int pid = atoi(strtok(line + 4, " \t\n"));
                        
                        if (pid != ownpid)
                        { 
                            fclose(fp);
                            closedir(dir);
                            return pid;
                        }
                    }
                }
            }
        }

        fclose(fp);
    }

    closedir(dir);
    return -1;
}

//------------------------5/2.feladat------------------------//
//------------------------Ez a függvény adatokat küld egy fájlba, majd elküld egy SIGUSR1 szignált egy másik folyamatnak (a chart nevű folyamatnak), hogy jelezze neki, hogy elkészült az adatküldés.------------------------//
void SendViaFile(int *Values, int NumValues)
{
    FILE *fp = fopen("Measurement.txt", "w");

    if (fp == NULL)
    {
        printf("Hiba a fajl megnyitasakor!\n");
        free(Values);
        exit(12);
    }

    for (int i = 0; i < NumValues; i++)
    {
        fprintf(fp, "%d\n", Values[i]);
    }
    fclose(fp);

    int pid = FindPID();

    if (pid == -1)
    {
        printf("Nem talalhato fogadasi folyamat.\n");
        free(Values);
        exit(14);
    }

    kill(pid, SIGUSR1);
}

//------------------------5/3.feladat------------------------//
//------------------------Ez a függvény fogadja az adatokat egy fájlból, majd ezeket az adatokat továbbítja egy másik függvénynek a további feldolgozás érdekében.------------------------//
void ReceiveViaFile(int sig)
{
    FILE *fp = fopen("Measurement.txt", "r");

    if (fp == NULL)
    {
        printf("Hiba a fajl megnyitasakor!.\n");
        exit(12);
    }

    int *Values = NULL;
    int NumValues = 0;
    char line[256];

    while (fgets(line, 256, fp))
    {
        NumValues++;
        int *temp = (int *)realloc(Values, NumValues * sizeof(int));

        if (temp == NULL)
        {
            printf("Hiba a memoria lefoglalasa soran.\n");
            free(Values);
            exit(16);
        }

        Values = temp;
        Values[NumValues - 1] = atoi(line);

    }

    fclose(fp);
    BMPcreator(Values, NumValues);
    free(Values);
}

//------------------------7.feladat------------------------//
//------------------------Ez a függvény kezeli a különböző jeleket, amelyeket a program kap.(pl: CTRL+C)------------------------//
void SignalHandler(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nSzerver leallitva, Szia!\n");
        exit(17);
    }

    else if (sig == SIGUSR1)
    {
        printf("\nHiba: A fajlon keresztüli küldesi szolgaltatas nem elerheto!\n");
        exit(13);
    }

    else if (sig == SIGALRM)
    {
        printf("\nHiba: A szerver nem valaszol idokereten belül.\n");
        exit(18);
    }
}

//------------------------6/1.feladat------------------------//
//------------------------Szerver/Kliens kozti kommunikacio------------------------//
//------------------------Forras: https://irh.inf.unideb.hu/~vargai/download/sysprog/UDP_server_Linux.c ------------------------//
//------------------------Forras: https://irh.inf.unideb.hu/~vargai/download/sysprog/UDP_client_Linux.c ------------------------//
void SendViaSocket(int *Values, int NumValues)
{
    /*********************** Declarations **********************/
    int s;                     // socket ID
    int bytes;                 // received/sent bytes
    int flag;                  // transmission flag
    char on;                   // sockopt option
    unsigned int server_size;  // length of the sockaddr_in server
    struct sockaddr_in server; // address of server

    /************************ Initialization ********************/
    on = 1;
    flag = 0;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT_NO);
    server_size = sizeof server;

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        printf("Socket letrehozasa nem sikerult");
        free(Values);
        exit(19);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Sending NumValues *****************/
    bytes = sendto(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        printf("Nem sikerult NumValues kuldese");
        free(Values);
        close(s);
        exit(20);
    }
    printf(" %i byte lett kuldve a szerverre.\n", bytes);
    signal(SIGALRM, SignalHandler);
    alarm(1); 

    /****************** Receive server's response ***************/
    int server_response;
    bytes = recvfrom(s, &server_response, sizeof(server_response), flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0)
    {
        printf("Fogadasi hiba");
        free(Values);
        close(s);
        exit(21);
    }
    signal(SIGALRM, SIG_IGN); 
    alarm(0);                 

    if (server_response != NumValues)
    {
        fprintf(stderr, "NumValues elteres: elvart %d, kapott %d\n", NumValues, server_response);
        close(s);
        exit(22);
    }

    /************************ Sending data **********************/
    bytes = sendto(s, Values, NumValues * sizeof(int), flag, (struct sockaddr *)&server, server_size);
    if (bytes <= 0)
    {
        printf("Adatkuldesi hiba");
        close(s);
        exit(23);
    }
    printf(" %i byte lett kuldve a szerverre.\n", bytes);

    /****************** Receive server's response ***************/
    int received_bytes;
    bytes = recvfrom(s, &received_bytes, sizeof(received_bytes), flag, (struct sockaddr *)&server, &server_size);
    if (bytes < 0)
    {
        printf("Fogadasi hiba");
        close(s);
        exit(21);
    }

    if (received_bytes != NumValues * sizeof(int))
    {
        fprintf(stderr, "Adatmeret elteres: elvart %ld byte, kapott %d byte\n", NumValues * sizeof(long unsigned int), received_bytes);
        close(s);
        exit(25);
    }

    /************************ Closing ***************************/
    close(s);
}

//------------------------6/2.feladat------------------------//
void ReceiveViaSocket()
{
    /*********************** Declarations ***********************/
    int s;                             // socket ID
    int bytes;                         // received/sent bytes
    char on;                           // sockopt option
    unsigned int client_size;          // length of the sockaddr_in client
    struct sockaddr_in server, client; // address of server and client

    /*********************** Initialization ********************/
    on = 1;
    client_size = sizeof client;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT_NO);

    /************************ Creating socket *******************/
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        printf("Socket letrehozasa nem sikerult");
        exit(19);
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
    setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

    /************************ Binding ***************************/
    if (bind(s, (struct sockaddr *)&server, sizeof server) < 0)
    {
        printf("Rogzito hiba");
        exit(27);
    }
    while (1)
    {
        /************************ Receiving NumValues ****************/
        int NumValues;
        bytes = recvfrom(s, &NumValues, sizeof(NumValues), 0, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            printf("Fogadasi hiba");
            close(s);
            exit(21);
        }

        printf("%d ertekeket küld az %s IP-cimu kliens\n", NumValues, inet_ntoa(client.sin_addr));

        /***************** Sending server response *******************/
        bytes = sendto(s, &NumValues, sizeof(NumValues), 0, (struct sockaddr *)&client, client_size);
        if (bytes <= 0)
        {
            printf("Hiba valasz kuldeskor");
            close(s);
            exit(29);
        }

        /************************ Receiving data **********************/
        int *Values = malloc(NumValues * sizeof(int));
        bytes = recvfrom(s, Values, NumValues * sizeof(int), 0, (struct sockaddr *)&client, &client_size);
        if (bytes < 0)
        {
            printf("Fogadasi hiba");
            free(Values);
            close(s);
            exit(21);
        }

        /***************** Sending server response *******************/
        int data_size = NumValues * sizeof(int);
        bytes = sendto(s, &data_size, sizeof(data_size), 0, (struct sockaddr *)&client, client_size);
        if (bytes <= 0)
        {
            printf("Hiba valasz kuldeskor");
            free(Values);
            close(s);
            exit(29);
        }
        BMPcreator(Values, NumValues);
        free(Values);
    }

    /************************ Closing ***************************/
    close(s);
}
