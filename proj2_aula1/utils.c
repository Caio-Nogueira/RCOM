#include "utils.h"
#include <ctype.h>
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

    int usernameLength = getField(string, parsingPoint, ":",tcpInfo->user);
    int passwordLength = 0;
    if(usernameLength == 0){
        int usernameLength = getField(string, parsingPoint, "@",tcpInfo->user);
        strncpy(tcpInfo->password, "\0", 1);
        parsingPoint += usernameLength;
        if(usernameLength == 0){
            parsingPoint--;
        }
    }
    else{
        parsingPoint += usernameLength;
        parsingPoint++; //The ':' char
        passwordLength = getField(string, parsingPoint, "@",tcpInfo->password);
        strncpy(tcpInfo->password, string + parsingPoint, passwordLength);
    }
    parsingPoint += passwordLength;
    parsingPoint++; //The '@' char

    if(parsingPoint == length){
        printf("No hostname was inserted.\n");
        return 122;
    }
    //Parse hostname
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

int verifyFields(struct fields *tcpInfo){
    if(tcpInfo->password[0] == '\0' && strcmp(tcpInfo->user, "anonymous") && tcpInfo->user[0] != '\0'){
		printf("Insert your password: ");
		scanf("%s", tcpInfo->password);
		printf("%s\n", tcpInfo->password);
	}
	else if (tcpInfo->user[0] == '\0') {
		strcpy(tcpInfo->user, "anonymous");
		printf("%s\n", tcpInfo->user);
		strcpy(tcpInfo->password, "ftp");
	}
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
            return 1;
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

int get_port(char * buf, int num_bytes){
    int port = 0;
    char * lastComma = strrchr(buf, ',');
    int i = lastComma - buf + 1;
    int port2 = 0;
    while(i < num_bytes){
        if(isdigit(buf[i])){
            port2 *= 10;
            port2 += (unsigned char) buf[i] - 48;
        }
        else{
            break;
        }
        i++;
    }


    char * result = strchr(buf, ',');
    result++;
    result = strchr(result, ',');
    result++;
    result = strchr(result, ',');
    result++;
    result = strchr(result, ',');
    result++;
    
    i = result - buf;
    while(i < num_bytes){
        if(isdigit(buf[i])){
            port *= 10;
            port += (unsigned char) buf[i] - 48;
        }
        else{
            break;
        }
        i++;
    }
    port *= 256;
    port += port2;
    return port;
}

int getField(const char* string, int startPoint, char * delims, char* field){
    int numchars = strcspn(string + startPoint, delims);
    if(numchars == strlen(string) - startPoint){
        field[0] = '\0';
        return 0;
    }
    field = strncpy(field, string + startPoint, numchars);
    field[numchars] = '\0';
    return numchars;
}

char* getFilenameFromPath(char* url){
    int fileSize = strlen(url);
    //printf("%d\n", fileSize);
    int i = 1;
    int index = 0;
    while (i < fileSize) {
        if (url[i] == '/') {
            index = i + 1;
        }
        i++;
        //printf("i: %d ; buf[i]: %c\n",i, url[i]);
    }

    //printf("url: %s\n", url);
    return url + index;
}

int writeAndReadFields(char* buf, int sockfd, struct fields tcpInfo){
	//Write username
	int length = strlen(tcpInfo.user);
	write(sockfd, "user ", 5);
	if(tcpInfo.user[0] == '\0'){
		write(sockfd, "anonymous", 9);
	}else{
		write(sockfd, tcpInfo.user, length);
	}
	write(sockfd, "\n", 1);

	// Read response
	//int bytes = read(sockfd, buf, 256);
    readSocketResponse(sockfd, buf);
	printf("Response to username: %s\n", buf);

	//Verify response
	if(buf[0] == '5' && buf[1] == '3' && buf[2] == '0'){
		//Invalid username. Username should be "anonymous"
		close(sockfd);
		return 0;
	}
	else if(buf[0] != '3' && strcmp(tcpInfo.user, "anonymous")){
		//Server error.
		close(sockfd);
		return 0;
	}

	//Write username
	char * password = (char *) malloc(strlen(tcpInfo.password) + 7);
	password[0] = 'p';
	password[1] = 'a';
	password[2] = 's';
	password[3] = 's';
	password[4] = ' ';
	password[5] = '\0'; //Important! strcat will "break" otherwise
	strcat(password, tcpInfo.user);
	length = strlen(password);
	password[length] = '\n';
	password[length + 1] = '\0';
	write(sockfd, password, strlen(password));

	free(password);
	readSocketResponse(sockfd, buf);
	//buf[bytes] = '\0';
	printf("Response to password: %s\n", buf);

	//Verify response
	if(buf[0] == '5' && buf[1] == '3' && buf[2] == '0'){
		//Invalid password. Password should be "pass"
		close(sockfd);
		return 0;
	}
	else if(buf[0] != '2' && buf[0] != '3'){
		//Server error.
		close(sockfd);
		return 0;
	}

	//Setting up passive mode
    
	write(sockfd, "pasv\n", 5);
    
	readSocketResponse(sockfd, buf);
	//buf[bytes] = '\0';
	printf("Response to pasv: %s\n", buf);
	int port = get_port(buf, strlen(buf));
    return port;
}


int getFileSizeOnMessage(char* response){
    int i = 0; 
    while( response[i] != '(')
        response++;
    
    int result;
    sscanf(response+1, "%d", &result);
    return result;
}

int readSocketResponse(int socket_fd, char* str) {
    FILE* fp = fdopen(socket_fd, "r");

	do {
		memset(str, 0, 1024);
		str = fgets(str, 1024, fp);
		printf("%s", str);

	} while (!('1' <= str[0] && str[0] <= '5') || str[3] != ' ');

	return 0;
}
