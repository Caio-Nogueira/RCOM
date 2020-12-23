#include "utils.h"
int readAndPrintEverything(char * buf, int length, int sockfd);
int parseURL(char* string, int length, struct fields *tcpInfo);
//Buf if the char buffer, bytes is it's size, lastBuf is a bool to check when to stop reading from the server, endOfLine is a bool to check if it read a full line, endofbuf is to check if it's the last buffer
int get_line(char* buf, int bytes, int* lastBuf, int* endOfLine, int* endOfBuf, int* startBytes, int* endBytes);
