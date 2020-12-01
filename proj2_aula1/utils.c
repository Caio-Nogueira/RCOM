#include "utils.h"

//Struct that contains the username, password, host and url path of the URL typed

int parseURL(char* string, int length, struct fields *tcpInfo){

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
    //Copy username onto struct element
    strncpy(tcpInfo->hostname, string + parsingPoint, hostnameLength);
    tcpInfo->hostname[hostnameLength] = '\0';
    parsingPoint += hostnameLength;
    parsingPoint++; //The '@' char

    strcpy(tcpInfo->urlPath, string + parsingPoint);
    tcpInfo->urlPath[length - parsingPoint] = '\0';

    return 0;
}

int get_line(char* buf, int bytes, int* lastBuf, int* endOfLine, int* endOfBuf, int* startBytes, int* endBytes){
    (*startBytes) = (*endBytes);
    
    //Last time this function was used, it finished on a newline character
    if((*endOfLine) == TRUE){
        if(buf[(*startBytes)] == '2' || buf[(*startBytes)] == '3'){
            //Start of a new line 
            if(buf[(*startBytes) + 3] == '-'){
                //More lines should be read from the server
            }
            else{
                //Nothing else should be read from the server for now
                (*lastBuf) = TRUE;
            }
            (*startBytes) += 4;
        }
        else{
            printf("Invalid response.\n");
        }
    }
    char* endSpot = strchr(buf + (*startBytes), '\n');
    if(endSpot == NULL){
        //Didn't finish on a newline
        (*endOfLine) = FALSE;
        (*endOfBuf) = TRUE;
        (*endBytes) = bytes;
        return 0;
    }
    else{
        //Finished on a newline
        (*endOfLine) = TRUE;
    }
    (*endBytes) = endSpot - buf;
    if((*endBytes) == bytes - 1){
        (*endOfBuf) = TRUE;
    }
    return 0;
}
