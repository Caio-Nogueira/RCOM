#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define USERNAME_LENGTH 255
#define PASSWORD_LENGTH 255
#define HOST_LENGTH 255
#define URL_LENGTH 1023

struct fields {
    char user[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char hostname[HOST_LENGTH];
    char urlPath[URL_LENGTH];
};

int parseURL(char* string, int length, struct fields *tcpInfo);