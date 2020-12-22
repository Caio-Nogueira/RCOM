/*      (C)2000 FEUP  */

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
	//Get the url's fields (username, password, hostname and file url)
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
	//Verify if the url is correct; ask for password if only it is missing and assume username anonymous if both are missing
	verifyFields(&tcpInfo);

    struct hostent *h;
	//Get the IP
    if ((h=gethostbyname(tcpInfo.hostname)) == NULL) {  
            herror("gethostbyname");
            exit(1);
    }
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
	//Create socket from IP
	int	sockfd = create_socket(inet_ntoa(*((struct in_addr *)h->h_addr)), SERVER_PORT);
	
	char buf[10000];
	printf("Reading\n");

	readAndPrintEverything(buf, 10000, sockfd);
	
	//writes username, password and "pasv" commands, gets the port if the responses were correct
	int port = writeAndReadFields(buf, sockfd, tcpInfo);
	if(port == 0){
		return 0;
	}

	printf("Port: %d\n", port);

	//Data socket creation
    int datafd = create_socket(inet_ntoa(*((struct in_addr *)h->h_addr)), port);
	
	printf("writting the retrieve command\n");
	char* retr_command = malloc(strlen(tcpInfo.urlPath) + 6);
	sprintf(retr_command, "retr %s\n", tcpInfo.urlPath);
	printf("command: %s\n", retr_command);
	write(sockfd, retr_command, strlen(retr_command));



	printf("reading the retrieve command\n");

	int responseCode = {0};
	char response[1024];

	FILE* file = fdopen(sockfd, "r");

	fgets(response, 1024, file);
	printf("%s\n", response);
	sscanf(response, "%d", &responseCode);
	//Get the file's size
	int size = getFileSizeOnMessage(response);
	printf("File size = %d bytes\n", size);

	if (responseCode != 150){
		printf("Error\n");
		return 1;
	}

	else {
		printf("Success: code = %d\n", responseCode);
	}

	char filename[256];

	strcpy(filename, getFilenameFromPath(tcpInfo.urlPath));

	for (int i = 0; i < strlen(filename); i++){
		if (filename[i] == '/') strcpy(filename, getFilenameFromPath(tcpInfo.urlPath));
	}

	int fd = open(filename, O_CREAT | O_WRONLY , 0666);
	printf("filename: %s\n", filename);
	char bytes[1024];
	int bytesRead;
	float progress = 0.0;
	int totalBytes = 0;

	while ((totalBytes != size)) {
		bytesRead = read(datafd, bytes, 1024);
		if (bytesRead < 0){
			perror("read()\n");
			exit(1);
		}
		totalBytes += bytesRead;
		progress = (totalBytes*100.0/size) ;
		printf("Current progress: %f\n", progress);
		write(fd, bytes, bytesRead);
	}
	
	close(datafd);
	close(sockfd);
	return 0;
}


