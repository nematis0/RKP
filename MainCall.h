
#ifndef MAINCALL_H
#define MAINCALL_H

#define BUFSIZE 1024 // Max length of buffer
#define PORT_NO 3333

//------------------------2.feladat------------------------//
int Measurement(int **Values);

//------------------------3.feladat------------------------//
void BMPcreator(int *Values, int NumValues);

//------------------------4.feladat------------------------//
int FindPID();

//------------------------5/2.feladat------------------------//
void SendViaFile(int *Values, int NumValues);

//------------------------5/3.feladat------------------------//
void ReceiveViaFile(int sig);

//------------------------6/1.feladat------------------------//
void SendViaSocket(int *Values, int NumValues);

//------------------------6/2.feladat------------------------//
void ReceiveViaSocket();

//------------------------7.feladat------------------------//
void SignalHandler(int sig);

#endif
