#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>


#define _POSIX_SOURCE 1 /* POSIX compliant source */


#define FLAG 0x7E
#define A_SEND 0x03
#define C_SET 0x03
#define BCC1 (A_SEND ^ C_SET)

#define A_Recieve 0x01
#define C_UA 0x07
#define BCC2 (A_Recieve ^ C_UA)
#define BAUDRATE B38400

#define NUM_TRIES 3
#define TIME_PER_TRY 1

#define FALSE 0
#define TRUE 1


//------Information trama-------
//flag = FLAG
// A = A_SEND
#define ACK 0
#define NACK 1

#define BASEIC 0x00
#define EVENIC 0x40
//BCC1 is A ^ C
//BCC2 is an XOR between every data byte

#define STUFFLAG1 0x7D
#define STUFFLAG2 0x5E

#define REPLACETRAMA2 0x7D
#define STUF7D1 0x7D
#define STUF7D2 0x5D



typedef enum {TRANSMITTER, RECEIVER} flag;

typedef enum {START, FLAG_RCVD, A_RCVD, C_RCVD, BCC1_RCVD, DATA_RCVD, END, ERROR} InformationFrameState;


void llopen(int fd, flag flag);


int SETstateMachine(char* setmsg);

void buildwritearray(int odd, char * message, size_t * size);

int destuffing(int odd, char * message, int * size);

int llwrite(int fd, char * buffer, int length);

int readInformationFrame(int fd, char* buffer, int* success);

void DataFrameStateMachine(InformationFrameState *state, char byte);

void llread(int fd, char * buffer);

void buildRresponse(char* buffer, int *N_r, int success);


int readResponse(char* buffer);

void llclose(int fd);
