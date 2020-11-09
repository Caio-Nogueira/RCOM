#include "application.h"

applicationLayer application;
//extern int fd;
extern int llread_success;

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
    application.file = myfile;
    
}

//create file
void createFile(char *filename){
    FILE * myfile;
    myfile = fopen(filename, "w+");
    application.file = myfile;
}

void sendCtrlPacket(int controlCamp, char* filename, int fd, int num_bytes_message){
    int file_size_3 = application.fileSize;


    //calculating the number of bytes of file name
    int num_bytes = 1;

    int L1 = sizeof(application.fileSize);
    for(num_bytes; num_bytes < L1; num_bytes++){
        file_size_3 /= 256;
        if(file_size_3 == 0){
            break;
        }
    }
    file_size_3 = application.fileSize;

    num_bytes += 3; //First part of control packet
    num_bytes += 2; //Second part of control packet
    num_bytes += strlen(filename);
    

    sendControlPacket(controlCamp, filename, fd, num_bytes);

}

void sendControlPacket(int controlCamp, char* filename, int fd, int num_bytes_message){
    unsigned char controlPacket[MAX_CONTROL_SIZE + 5] = "";
    if(num_bytes_message > MAX_CONTROL_SIZE){
        printf("Message name is unreasonably long.");
        fflush(stdout);
        exit(0);
    }
    //controlPacket = (unsigned char *) malloc(num_bytes_message + 5);
    sprintf(controlPacket, "%c", (unsigned char) controlCamp);
    //printf("Sending packet control.\n");
    readFile(filename);
    //send file size
    char T1 = (unsigned char) 0;
    int L1 = sizeof(application.fileSize);
    printf("L1 = %d\n", L1);
    int file_size = application.fileSize;

    

    //send file name
    char T2 = 0x01;
    int L2;
    char V2[256];
    sprintf(V2, "%s", application.file_name);
    L2 = strlen(V2);

    sprintf((controlPacket + 1), "%c", T1);
    
    int file_size_3 = application.fileSize;

    int num_bytes = 1;
    for(num_bytes; num_bytes < L1; num_bytes++){
        file_size_3 /= 256;
        if(file_size_3 == 0){
            break;
        }
    }
    file_size_3 = application.fileSize;


    int num_bytes_2 = num_bytes;
    for(num_bytes_2; num_bytes_2 > 0; num_bytes_2--){
        file_size_3 /= 256;
        controlPacket[3 + num_bytes - num_bytes_2] = (unsigned char) file_size_3 & 0xFF;
    }

    controlPacket[2] = (unsigned char) num_bytes;
    file_size_3 = application.fileSize;
    //printf("Last byte: %d\n", file_size_3 & 0xFF);
    controlPacket[2 + num_bytes] = (unsigned char) file_size_3 & 0xFF;    

    sprintf(controlPacket + 3 + num_bytes, "%c", T2);
    controlPacket[4 + num_bytes] = (unsigned char) L2;
    int j = 0;
    for (int i = 0; i < L2; i++){
        controlPacket[5 + num_bytes + j] = filename[i];
        j++;
    }

    int bytes_written = llwrite(fd, controlPacket, 5+num_bytes+j);



}



void readControlPacket(int fd, char* string){ //  receiver port filedes
    //char string[256];
    //llread(fd, string);
    
    int sizeArg0 = (int) string[2];
    int sizeArg1 = (int) string[2 + 2 + sizeArg0];

    //printf("Size argument 0: %d\n", sizeArg0);
    //printf("Size argument 1: %d\n", sizeArg1);

    for(int i = 0; i < sizeArg0 + sizeArg1 + 5; i++){
        //printf("%d ", (unsigned int) string[i] & 0xFF);
    }

    int file_size = 0;
    int a = 1;
    for(int i = 0; i < sizeArg0; i++){
        a *= 256;
        file_size += (((unsigned int) string[2 + sizeArg0 - i]) & 0xFF) * a / 256;
    }

    //printf("File size: %d\n", file_size);

    char name[256];
    for(int i = 2 + 2 + sizeArg0 + 1; i < 2 + 2 + sizeArg0 + 1 + sizeArg1; i++){
        sprintf(name - (2 + 2 + sizeArg0 + 1) + i, "%c",  string[i]);
    }
    //printf("Filename: %s\n", name);
/*
    char str[256];
    llread(fd, str);
    char* buffer = str;//+4; //skip FLAG, A, C, and BCC1
    write(STDOUT_FILENO, buffer, 50);
    fflush(stdout);
    int L1, L2;
    char* filename;
    char controlCamp = buffer[0];
    if (buffer[1] == (char) 0x00){         //file_size
        printf("\nReading file size\n");

        L1 = (int) buffer[2];
        //printf("L1: %d\n", L1);
        char* V1 = malloc(L1*sizeof(char));
        int j = 0;
        for (int i = 3; i < 3+L1; i++){
            V1[j++] = buffer[i];
        }
    }
    
    //printf("\nT2: %d\n", (int) buffer[3+L1]);
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
    printf("Control packet read.\n");
    //printf("%s\n", filename);*/
}


unsigned verifyControlPacket(char* frame){ //verifies if the current frame contains a control packet
    //char buffer[256] ;
    //buffer = frame+4;
    return (frame[0] == (char) CONTROL_START || frame[0] == (char) CONTROL_END);
}

void sendDataPackets(int fd, char* filename, int num_bytes_message){
    char dataPacket[CHUNK_LEN+5];
    int n_sequence = 0;    
    int bytesRead = 0;

    int length = 1;
    unsigned char buf[CHUNK_LEN+1];
    while (bytesRead < application.fileSize){
        int bytes = read(application.fileDescriptor, &buf, num_bytes_message);
        bytesRead += bytes;
        dataPacket[0] = (char) 0x01;
        dataPacket[1] = (char) (n_sequence % 255);
        int L2 = bytes / 256;
        int L1 = bytes % 256;
        dataPacket[2] = (unsigned char) L2;
        dataPacket[3] = (unsigned char) L1;
        for (int i = 0; i < bytes; i++){
            dataPacket[4 + i] = (char) buf[i];
        }
        int f_size = min(bytes + 4, application.fileSize + 4);
        f_size = min(f_size, 256 * L2 + L1 + 4);
        int length = llwrite(fd, dataPacket, f_size);
        n_sequence++;
        n_sequence %= 255;
    }
    sendControlPacket(CONTROL_END, filename, fd, num_bytes_message);
}

void readPackets(int fd, char* filename){
    printf("Start of readPackets\n");
    char buf[MAX_TRAMA_SIZE];
    unsigned int current = -1;
    int total = 0;
    while(1){
        int llread_res = llread(fd, buf);
        if(llread_res > 0){
            if (buf[0] == (char) 0x01) {
                //receive data packet
                
                int len = getDataLen(buf);
                total += len;
                fwrite(buf+4, 1, len, application.file);
            }
        
            else if (buf[0] == 0x02) {
                readControlPacket(fd, buf);
                createFile(filename);
            }

            else if (buf[0] == 0x03) {
                readControlPacket(fd, buf);
                break;
            }
            else{
                printf("Application layer: ");
                exit(1);
            }

        }

        /*else if (buf[0] == FLAG){
            printf("ERROR: buf[4] = %d\n", (int) buf[4]);
            printf("data len: %d\n", getDataLen(buf+4));
        }*/
    }
}


int getDataLen(char* frame){ //read data packets -> returns array with bytes to be added to the destination file
    if (frame[0] != (char) 0x01) return 0;
    int L2 = ((int) frame[2]) & 0xFF;
    int L1 = ((int) frame[3]) & 0xFF;
    //printf("L2: %d ; L1: %d\n", L2, L1);
    return 256*L2+L1;
}