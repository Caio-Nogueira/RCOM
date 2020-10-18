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




typedef enum {TRANSMITTER, RECEIVER} flag;


void llopen(int fd, flag flag);


int SETstateMachine(char* setmsg);



