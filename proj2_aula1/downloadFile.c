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

    struct hostent *h;

    if ((h=gethostbyname(tcpInfo.hostname)) == NULL) {  
            herror("gethostbyname");
            exit(1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

    


    int	controlSocketfd;
	struct	sockaddr_in server_addr;
	char	buf[9999] = ""; //= "Mensagem de teste na travessia da pilha TCP/IP\n";  
	int	bytes;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr)); //Zero out the struct's variables
	server_addr.sin_family = AF_INET; //Do not change
	server_addr.sin_addr.s_addr = inet_addr(h->h_addr);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(21); //Fazer 2 sockets com 2 sockaddr_in		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((controlSocketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(0);
	}
	/*connect to the server*/
	if(connect(controlSocketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
	exit(0);
	}
    	/*send a string to the server*/
	bytes = read(controlSocketfd, buf, strlen(buf));
	buf[bytes] = '\0';
	printf("Numero de bytes lidos %d\n", bytes);
	printf("Bytes lidos %s\n", buf);

	close(controlSocketfd);
	exit(0);    


    return 0;
}
