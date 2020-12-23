#include "parsers.h"

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
