#include "application.h"

applicationLayer application;
//extern int fd;

const char * get_extension(const char * name){//Thanks to stackoverflow.com/questions/5309471/getting-file-extensions-in-c
    const char *dot = strrchr(name, '.');
    if(!dot || dot == name) return "";
    return dot + 1;
}

int readBytes(char * bytes, int num_bytes, int num_members, FILE * file){
    return (int) fread(bytes, num_bytes, num_members, file);
}

int min(const int a, const int b){
    return a < b ? a : b;
}


//parse file information
void readFile(char *filename){
    FILE * myfile;
    myfile = fopen(filename, "r");
    int fd = fileno(myfile);
    //Get file size    
    fseek(myfile, 0L, SEEK_END);
    int file_size = ftell(myfile);
    rewind(myfile);
    memcpy(application.file_name, filename, 256);
    application.fileSize = file_size; //size (in bytes)
    application.fileDescriptor = fd;
    
}


void sendControlPacket(int controlCamp, char* filename, int fd){
    unsigned char controlPacket[MAX_CONTROL_SIZE] = "";
    sprintf(controlPacket, "%c", (unsigned char) controlCamp);
    printf("Sending packet control.\n");
    readFile(filename);
    //send file size
    char T1 = (unsigned char) 0;
    int L1 = sizeof(application.fileSize);
    int file_size = application.fileSize;

    

    //send file name
    char T2 = 0x01;
    int L2;
    char V2[256];
    sprintf(V2, "%s", application.file_name);
    L2 = strlen(V2);

    sprintf((controlPacket + 1), "%c", T1);
    sprintf((controlPacket + 2), "%c", (unsigned char) L1);
    sprintf(controlPacket + 3, "%c", (unsigned char)(file_size >> 24) & 0xFF);
    sprintf(controlPacket + 4, "%c", (unsigned char)(file_size >> 16) & 0xFF);
    sprintf(controlPacket + 5, "%c", (unsigned char)(file_size >> 8) & 0xFF);
    sprintf(controlPacket + 6, "%c", (unsigned char) file_size & 0xFF);


    sprintf(controlPacket + 3 + L1, "%c", T2);
    sprintf(controlPacket + 4 + L1, "%c", (unsigned char) L2);
    //strcat(controlPacket, V2);
    int j = 0;
    for (int i = 0; i < L2; i++){
        sprintf(controlPacket + 5 + L1 + j, "%c", filename[i]);
        j++;
    }


    llwrite(fd, controlPacket, 5+L1+L2);
    printf("Packet control sent.\n");


}



void readControlPacket(int fd){ //  receiver port filedes
    char str[256];
    llread(fd, str);
    char* buffer = str+4; //skip FLAG, A, C, and BCC1
    write(STDOUT_FILENO, buffer, 50);
    fflush(stdout);
    int L1, L2;
    char* filename;
    char controlCamp = buffer[0];
    if (buffer[1] == (char) 0x00){         //file_size
        printf("\nReading file size\n");

        L1 = (int) buffer[2];
        printf("L1: %d\n", L1);
        char* V1 = malloc(L1*sizeof(char));
        int j = 0;
        for (int i = 3; i < 3+L1; i++){
            V1[j++] = buffer[i];
        }
    }
    
    printf("\nT2: %d\n", (int) buffer[3+L1]);
    if (buffer[3+L1] == (char) 0x01){      //file_name
        printf("Reading file name\n");
        L2 = (int) buffer[4+L1];
        filename = malloc(L2*sizeof(char));
        int j = 0;
        for (int i = 5+L1; i < 5+L1+L2; i++){
            filename[j++] = buffer[i];
            printf("%c\n", buffer[i]);
            
        }
    }
    printf("Reading control packet.\n");
    //printf("%s\n", filename);
}


unsigned verifyControlPacket(char* frame){ //verifies if the current frame contains a control packet
    char buffer[256] = frame+4;
    return (buffer[0] == (char) CONTROL_START || buffer[0] == (char) CONTROL_END);
}

void sendDataPacket(){
    //TODO
}