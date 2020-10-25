#include "application.h"

applicationLayer application;
extern int fd;

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


void sendControlPacket(int controlCamp, char* filename){
    unsigned char controlPacket[MAX_CONTROL_SIZE] = "";
    sprintf(controlPacket, "%c", (unsigned char) controlCamp);
    readFile(filename);
    //send file size
    char T1 = (char) 0;
    int L1 = sizeof(application.fileSize);
    int file_size = application.fileSize;

    
    //printf("size: %d\n", L1);

    //send file name
    char T2 = 0x01;
    int L2;
    char V2[256];
    sprintf(V2, "%s", application.file_name);
    L2 = strlen(V2);

    sprintf((controlPacket + 1), "%c", T1);
    sprintf((controlPacket + 2), "%c", (char) L1);
    sprintf(controlPacket + 3, "%c", (unsigned char)(file_size >> 24) & 0xFF);
    sprintf(controlPacket + 4, "%c", (unsigned char)(file_size >> 16) & 0xFF);
    sprintf(controlPacket + 5, "%c", (unsigned char)(file_size >> 8) & 0xFF);
    sprintf(controlPacket + 6, "%c", (unsigned char) file_size & 0xFF);


    sprintf(controlPacket + 3 + L1, "%c", T2);
    sprintf(controlPacket + 4 + L1, "%c", (unsigned char) L2);
    strcat(controlPacket, V2);


    llwrite(fd, controlPacket, 4+L1+L2);
    write(STDOUT_FILENO, controlPacket, 256);


}

void sendDataPacket(){
    //TODO
}