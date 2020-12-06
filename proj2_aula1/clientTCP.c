/*      (C)2000 FEUP  */

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

#include "utils.h"

#define SERVER_PORT 21

int create_socket(char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;

    // Server address handling
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip); /* 32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port); /* server TCP port must be network byte ordered */

    // Open an TCP socket 

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(0);
    }

    /* Connect to the server         */
    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(0);
    }

    return sockfd;
}


int readAndPrintEverything(char * buf, int length, int sockfd){
	int bytes = 0;
	int lines_read = 0; //Ou times_read, quando/se fizermos o TODO
	
	int lastBuffer = FALSE;
	int endOfLine = TRUE;
	int endOfBuf = FALSE;
	int startBytes = 0;
	int endBytes = 0;
	
	//int get_line(char* buf, int bytes, int* lastBuf, int* endOfLine, int* endOfBuf, int* startBytes, int* endBytes);
	while(lastBuffer == FALSE){
		//Set start and end bytes to 0
		startBytes = 0;
		endBytes = 0;
		endOfBuf = FALSE;


		bytes = read(sockfd, buf, 255);
		if(bytes <= 0){
			perror("a");
			continue;
		}
		buf[bytes] = '\0';
		

		while(endOfBuf != TRUE){
			int invalid_response = get_line(buf, bytes, &lastBuffer, &endOfLine, &endOfBuf, &startBytes, &endBytes);
			if(invalid_response){
				return -1;
			}
			if(endOfLine == TRUE){
				printf("%c", buf[startBytes - 4]);
				printf("%c", buf[startBytes - 3]);
				printf("%c", buf[startBytes - 2]);
				printf("%c", buf[startBytes - 1]);
			}
			for(int i = startBytes; i<endBytes; i++){
				printf("%c", buf[i]);
			}
			if(endOfLine == TRUE){
				printf("\n");
			}
			endBytes++;
		}

		//finalLine = find_lines(buf, bytes, &startLine);
		
		//printf("%s\n", buf);
		/*
		for(int i = 0; i < bytes; i++){
			printf("%x ", buf[i]);
		}
		printf("\n");
		printf("Bytes %d\n", bytes);*/
		lines_read++;
		//TODO: Add part where, if the line isn't over yet, it keeps reading
		
	}
	return lines_read;
}

int main(int argc, char** argv){

	//Get the IP adress of the host whose hostname is the corresponding part in argv
	struct fields tcpInfo;
    if(argc == 1){
        printf("Missing URL field.\n");
        return 1;
    }
    int parseRes = parseURL(argv[1], strlen(argv[1]), &tcpInfo);
    if(parseRes){
        return parseRes;
    }
	printf("Hostname: %s\n", tcpInfo.hostname);

    struct hostent *h;
    if ((h=gethostbyname(tcpInfo.hostname)) == NULL) {  
            herror("gethostbyname");
            exit(1);
    }
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));


	int	sockfd = create_socket(inet_ntoa(*((struct in_addr *)h->h_addr)), SERVER_PORT);
	
	char buf[256];
	printf("Reading\n");

	readAndPrintEverything(buf, 256, sockfd);
	
	//Write username
	int length = strlen(tcpInfo.user);

	write(sockfd, "user ", 5);
	write(sockfd, tcpInfo.user, length);
	write(sockfd, "\n", 1);

	// Read response
	int bytes = read(sockfd, buf, 256);
	buf[bytes] = '\0';
	printf("Response to username: %s\n", buf);

	//Verify response
	if(buf[0] == '5' && buf[1] == '3' && buf[2] == '0'){
		//Invalid username. Username should be "anonymous"
		close(sockfd);
		return 0;
	}
	else if(buf[0] != '3'){
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
	bytes = read(sockfd, buf, 256);
	buf[bytes] = '\0';
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
	bytes = read(sockfd, buf, 256);
	buf[bytes] = '\0';
	printf("Response to pasv: %s\n", buf);

	int port = get_port(buf, bytes);
	printf("Port: %d\n", port);

	//Data socket creation
    int datafd = create_socket(inet_ntoa(*((struct in_addr *)h->h_addr)), port);


	char command[512];

    sprintf(command, "telnet %s %d\n", tcpInfo.hostname, port);

    if (write(datafd, command, strlen(command)) < 0) {
        perror("Failed to send command.\n");
        exit(1);
    }

	
	printf("writting the retrieve command\n");
	write(sockfd, "retr pipe.txt\n", strlen("retr pipe.txt\n"));

	printf("reading the retrieve command\n");
	//readAndPrintEverything(new_buf, 256, datafd);
	bytes = read(datafd, buf, 256);
	buf[bytes] = '\0';
	printf("buf: %s", buf);


	close(datafd);
	close(sockfd);
	//exit(EXIT_SUCCESS);
	return 0;
}


