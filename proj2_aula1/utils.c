#include "utils.h"
#include <ctype.h>
//Struct that contains the username, password, host and url path of the URL typed

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
