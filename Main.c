#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "MainCall.h"

#define name "Borsos Attila Máté"
#define version 1.09
#define date "2024.04.30"

void help()
{
    printf("Lehetseges parancsok:\n");
    printf("--help\n");
    printf("--version   |verzio lekerdezes\n");

    printf("\nProgram uzemmodjai:\n");
    printf("-send       |kuldo uzemmod\n");
    printf("-receive    |fogado uzemmod\n");

    printf("\nKommunikacio uzemmodjai:\n");
    printf("-file       |file uzemmod\n");
    printf("-socket     |socket uzemmod\n");
}

void versionn()
{
    char *attributes[] = {"Developer", "Version", "Date"};
    char *values[] = {"Borsos Attila Máté", "1.09", "2024.04.30"};

    //------------------------Sorrend keverese------------------------//
    srand(time(NULL));
    int order[3] = {0, 1, 2};
    for (int i = 2; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int temp = order[i];
        order[i] = order[j];
        order[j] = temp;
    }

    //------------------------Kevert kiiratasa------------------------//
    for (int i = 0; i < 3; ++i)
    {
        printf("%s: %s\n", attributes[order[i]], values[order[i]]);
    }
}

//------------------------1.feladat------------------------//
int main(int argc, char *argv[])
{
    signal(SIGINT, SignalHandler);
    signal(SIGUSR1, SignalHandler);

    if (strcmp(argv[0], "./chart") != 0)
    {
        printf("A fajl nevenek a kovetkezonek kell lennie!: \"chart\"!");
        exit(1);
    }

    //------------------------Ha csak a "./chart" parancs van megadva, akkor küldő üzemmódban indul a program------------------------//
    if (argc == 1)
    {
        int *Values;
        int NumValues = Measurement(&Values);
        SendViaFile(Values, NumValues);
        free(Values);
        exit(2);
    }

    //------------------------Ellenőrizze, hogy van-e fogadó üzemmód------------------------//
    int receiverMode = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-receive") == 0)
        {
            receiverMode = 1;
            break;
        }
    }

    if (argc == 1 || argc > 3)
    {
        help();
        exit(3);
    }

    if (argc == 2)
    {
        if (strcmp(argv[1], "--version") == 0)
        {
            versionn();
        }

        else if (strcmp(argv[1], "--help") == 0)
        {
            help();
        }
        //------------------Program uzemmodjai------------------//
        else if (strcmp(argv[1], "-receive") == 0)
        {
            signal(SIGUSR1, ReceiveViaFile);
            while (1)
            {
                pause();
                printf("Sikeresen elkeszult(BMP)!\n");
            }
        }

        else if (strcmp(argv[1], "-send") == 0)
        {
            int *Values;
            int NumValues = Measurement(&Values);
            SendViaFile(Values, NumValues);
            free(Values);
            exit(4);
        }
        //------------------Kommunikacio uzemmodjai------------------//
        else if (strcmp(argv[1], "-socket") == 0)
        {
            int *Values;
            int NumValues = Measurement(&Values);
            SendViaSocket(Values, NumValues);
            free(Values);
            exit(5);
        }

        else if (strcmp(argv[1], "-file") == 0)
        {
            int *Values;
            int NumValues = Measurement(&Values);
            SendViaFile(Values, NumValues);
            free(Values);
            exit(6);
        }

        else
        {
            help();
            exit(7);
        }
    }

    //------------------Kuldes/Fogadasi arg kezeles------------------//
    if (argc == 3)
    {
        if ((strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-send") == 0) || (strcmp(argv[2], "-file") == 0 && strcmp(argv[1], "-send") == 0))
        {
            int *Values;
            int NumValues = Measurement(&Values);
            SendViaFile(Values, NumValues);
            free(Values);
            exit(8);
        }

        else if ((strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-send") == 0) || (strcmp(argv[2], "-socket") == 0 && strcmp(argv[1], "-send") == 0))
        {
            int *Values;
            int NumValues = Measurement(&Values);
            SendViaSocket(Values, NumValues);
            free(Values);
            exit(9);
        }

        else if ((strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-receive") == 0) || (strcmp(argv[2], "-file") == 0 && strcmp(argv[1], "-receive") == 0))
        {
            signal(SIGUSR1, ReceiveViaFile);
            while (1)
            {
                pause();
                printf("Sikeresen elkeszult(BMP)!\n");
            }
        }

        else if ((strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-receive") == 0) || (strcmp(argv[2], "-socket") == 0 && strcmp(argv[1], "-receive") == 0))
        {
            ReceiveViaSocket();
        }

        else
        {
            help();
            exit(10);
        }
    }
    return 0;
}
