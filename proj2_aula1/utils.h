
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define USERNAME_LENGTH 255
#define PASSWORD_LENGTH 255
#define HOST_LENGTH 255
#define URL_LENGTH 1023

#define TRUE 0
#define FALSE 1

struct fields {
    char user[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char hostname[HOST_LENGTH];
    char urlPath[URL_LENGTH];
};

int parseURL(char* string, int length, struct fields *tcpInfo);

//Buf if the char buffer, bytes is it's size, lastBuf is a bool to check when to stop reading from the server, endOfLine is a bool to check if it read a full line, endofbuf is to check if it's the last buffer
int get_line(char* buf, int bytes, int* lastBuf, int* endOfLine, int* endOfBuf, int* startBytes, int* endBytes);

int get_port(char* buf, int num_bytes);

char* getFilenameFromPath(char* url);

int getField(const char* string, int startPoint, char * delims, char* field);

int writeAndReadFields(char* buf, int sockfd, struct fields tcpInfo);

int getFileSizeOnMessage(char* response);
