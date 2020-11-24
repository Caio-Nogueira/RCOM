#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define USERNAME_LENGTH 255
#define PASSWORD_LENGTH 255
#define HOST_LENGTH 255
#define URL_LENGTH 1023


//Struct that contains the username, password, host and url path of the URL typed
struct fields {
    char user[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char hostname[HOST_LENGTH];
    char urlPath[URL_LENGTH];
};


int parseURL(char* string, int length, struct fields *tcpInfo){
    printf("Function was called\n");

    //Number of characters already parsed.
    int parsingPoint = 0;


    //Parse ftp://
    const int num_chars_url_before_user = 6;
    if(strncmp(string, "ftp://", num_chars_url_before_user)){
        printf("Invalid URL string. Does not start with ftp://\n");
        return 1;
    }
    parsingPoint += num_chars_url_before_user;

    //Parse username
    int usernameLength = strcspn(string + parsingPoint, ":");
    if(usernameLength + 1 > USERNAME_LENGTH){
        printf("Username is too long");
        return 111;
    }
    if(parsingPoint + usernameLength == length){
        printf("No ':' after username field.\n");
        return 11;
    }
    //Copy username onto struct element
    strncpy(tcpInfo->user, string + parsingPoint, usernameLength);
    tcpInfo->user[usernameLength] = '\0';
    parsingPoint += usernameLength;
    parsingPoint++; //The ':' char


    //Parse password
    int passwordLength = strcspn(string + parsingPoint, "@");
    if(passwordLength + 1 > PASSWORD_LENGTH){
        printf("Password is too long");
        return 112;
    }
    if(parsingPoint + passwordLength == length){
        printf("No '@' after password field.\n");
        return 12;
    }
    //Copy password onto struct element
    strncpy(tcpInfo->password, string + parsingPoint, passwordLength);
    tcpInfo->password[passwordLength] = '\0';
    parsingPoint += passwordLength;
    parsingPoint++; //The '@' char
    
    //Parse password
    int hostnameLength = strcspn(string + parsingPoint, "/");
    if(hostnameLength + 1 > PASSWORD_LENGTH){
        printf("Hostname is too long");
        return 112;
    }
    if(parsingPoint + hostnameLength == length){
        printf("No '/' after hostname field.\n");
        return 12;
    }
    printf("Hostname length: %d\n", hostnameLength);
    //Copy username onto struct element
    strncpy(tcpInfo->hostname, string + parsingPoint, hostnameLength);
    tcpInfo->hostname[hostnameLength] = '\0';
    parsingPoint += hostnameLength;
    parsingPoint++; //The '@' char

    strcpy(tcpInfo->urlPath, string + parsingPoint);
    tcpInfo->urlPath[length - parsingPoint];

    return 0;
}

int main(int argc, char** argv){
    struct fields tcpInfo;
    if(argc == 1){
        printf("Missing URL field.\n");
        return 1;
    }
    int parseRes = parseURL(argv[1], strlen(argv[1]), &tcpInfo);
    if(parseRes){
        return parseRes;
    }
    printf("Username: %s\n", tcpInfo.user);
    printf("Password: %s\n", tcpInfo.password);
    printf("Hostname: %s\n", tcpInfo.hostname);
    printf("Path: %s\n", tcpInfo.urlPath);
    return 0;
}

