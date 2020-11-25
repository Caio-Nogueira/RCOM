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
#define SERVER_ADDR "193.137.29.15"

int readAndPrintEverything(char * buf, int length, int sockfd){
	int bytes = 0;
	int lines_read = 0; //Ou times_read, quando/se fizermos o TODO
	while(1){
		bytes = read(sockfd, buf, 255);
		if(bytes <= 0){
			perror("a");
			continue;
		}
		buf[bytes] = '\0';
		printf("%s\n", buf);
		lines_read++;
		//TODO: Add part where, if the line isn't over yet, it keeps reading
		if(buf[0] != '2' && buf[0] != '3'){
			return -1;
		}
		if(buf[3] != '-')
			break;
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
    struct hostent *h;
    if ((h=gethostbyname(tcpInfo.hostname)) == NULL) {  
            herror("gethostbyname");
            exit(1);
    }
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));




	int	sockfd;
	struct	sockaddr_in server_addr;
	char buf[257] = ""; //= "Mensagem de teste na travessia da pilha TCP/IP\n";  
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT); //Fazer 2 sockets com 2 sockaddr_in		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(0);
	}
	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
	exit(0);
	}

	printf("Abc\n");
	readAndPrintEverything(buf, 256, sockfd);
	printf("Finished\n");

	printf("Writting.\n");
	
	write(sockfd, "user anonymous", sizeof("user anonymous"));
	
	printf("Write successful.\n");

	int bytes = read(sockfd, buf, 256);
	buf[bytes] = '\0';
	printf("Response to username: %s\n", buf);
	


	/*send a string to the server*/
	//bytes = read(sockfd, buf, 999);
	//buf[bytes] = '\0';

	close(sockfd);
	//exit(EXIT_SUCCESS);
	return 0;
}


