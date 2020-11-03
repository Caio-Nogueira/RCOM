#include "application.h"

applicationLayer application;
//extern int fd;

int llread_size_packet = 0;

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
    

    if(num_bytes_message >= num_bytes){
        sendControlPacket(controlCamp, filename, fd, num_bytes);
    }
    else{

        printf(">:[");
        exit(0);
    }

}

void sendControlPacket(int controlCamp, char* filename, int fd, int num_bytes_message){
    
    unsigned char *controlPacket;//[MAX_CONTROL_SIZE + 5] = "";
    if(num_bytes_message > MAX_CONTROL_SIZE){
        printf("Message name is unreasonably long.");
        fflush(stdout);
        exit(0);
    }
    controlPacket = (unsigned char *) malloc(num_bytes_message + 5);
    sprintf(controlPacket, "%c", (unsigned char) controlCamp);
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
    controlPacket[2 + num_bytes] = (unsigned char) file_size_3 & 0xFF;

    sprintf(controlPacket + 3 + num_bytes, "%c", T2);
    //Adding filename's length in bytes to control packet
    controlPacket[4 + num_bytes] = (unsigned char) L2;
    int j = 0;
    //Adding filename to control packet
    for (int i = 0; i < L2; i++){
        controlPacket[5 + num_bytes + j] = filename[i];
        j++;
    }

    printf("Message:");
    for(int i = 0; i < 5 + num_bytes + j; i++){
        printf("%d ", controlPacket[i]);
    }
    printf("\n");
    int bytes_written = 0;
    int bytes_to_write;
    char * arr;
    arr = (char *) malloc(num_bytes_message);
    while(bytes_written < 5 + num_bytes + j){
        bytes_to_write = min(num_bytes_message, 5+num_bytes+j - bytes_written);
        
        memcpy(arr, controlPacket + bytes_written, bytes_to_write);
        for(int i = 0; i < 7; i++){
            printf("%d ", (int) arr[i]);
        }
        printf("\n");
        printf("%d\n", bytes_to_write);
        llwrite(fd, arr, bytes_to_write);//controlPacket + bytes_written, bytes_to_write);
        bytes_written += bytes_to_write;
    }
    printf("\n");
}



int readControlPacket(int fd, char* string, int num_bytes_read, char * tempArr){ //  receiver port filedes
    int num_bytes_size;
    int num_bytes_filename;
    int ready = 0;

    /*printf("String: ");
    for(int i = 0; i < 7; i++){
        printf("%d ", (int) string[i]& 0xFF);
    }

    printf("\nTemp array: ");
    for(int i = 0; i < 7; i++){
        printf("%d ", tempArr[i]& 0xFF);
    }
    printf("\n");*/
    if(num_bytes_read != llread_size_packet){
        memcpy(string + num_bytes_read - llread_size_packet, tempArr , llread_size_packet);
    }
    printf("%d\n", num_bytes_read);
        if(num_bytes_read >= 3){
            num_bytes_size = string[2];
            if(num_bytes_read >= 3 + num_bytes_size + 2){
                num_bytes_filename = string[2 + num_bytes_size + 2];
                //printf("%d %d %d\n", num_bytes_size, num_bytes_filename, 5 + num_bytes_size + num_bytes_filename);
                for(int i = 0; i < 15; i++){
                    printf("%d ", string[i]);
                }
                if(num_bytes_read == 3 + num_bytes_size + 2 + num_bytes_filename){
                    ready = 1;
                }
                else if(num_bytes_read > 3 + num_bytes_size + 2 + num_bytes_filename){
                    printf("Too many bytes (%d) read. Exiting now", num_bytes_read);
                }
            }
        }/*
    printf("Start\n");
    for(int i = 0; i < num_bytes_read; i++){
        printf("%d\n", (int) (string[i] & 0xFF));
    }
    printf("End\n\n");*/
    if(ready){
        int sizeArg0 = (int) string[2];
        int sizeArg1 = (int) string[2 + 2 + sizeArg0];

        int file_size = 0;
        int a = 1;
        for(int i = 0; i < sizeArg0; i++){
            a *= 256;
            file_size += (((unsigned int) string[2 + sizeArg0 - i]) & 0xFF) * a / 256;
        }

        char name[256];
        for(int i = 2 + 2 + sizeArg0 + 1; i < 2 + 2 + sizeArg0 + 1 + sizeArg1; i++){
            sprintf(name - (2 + 2 + sizeArg0 + 1) + i, "%c",  string[i]);
        }
        return 0;
    }
    return 1;
}


unsigned verifyControlPacket(char* frame){ //verifies if the current frame contains a control packet
    //char buffer[256] ;
    //buffer = frame+4;
    return (frame[0] == (char) CONTROL_START || frame[0] == (char) CONTROL_END);
}

void sendDataPackets(int fd, char* filename, int num_bytes_message){
    //int number_bytes = 65535;
    char *dataPacket;//[CHUNK_LEN+5];
    dataPacket = (char *) malloc(num_bytes_message + 5);
    int n_sequence = 0;    
    int bytesRead = 0;
    unsigned char buf[CHUNK_LEN+1];
    printf("Reading packets now: %d\n", application.fileSize);
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
            //printf("i: %d ; buf[i]: %d\n", bytesRead + i, buf[i]);
        }
        //printf("L2: %d ; L1: %d\n", dataPacket[2], dataPacket[3]);
        int f_size = min(bytes + 4, application.fileSize + 4);
        f_size = min(f_size, 256 * L2 + L1 + 4);
        printf("f_size %d: \n", f_size);
        int length = llwrite(fd, dataPacket, f_size);        
        printf("len: %d\n", length);
        n_sequence++;
        n_sequence %= 255;
    }
    sendControlPacket(CONTROL_END, filename, fd, num_bytes_message);
    printf("CCCC\n");
}

void readPackets(int fd, char* filename){
    printf("Start of readPackets\n");
    char buf[MAX_TRAMA_SIZE];
    int current = -1;
    int total = 0;

    char *temp;
    int inicialized = 0;
    int num_bytes_ctrl = 0;

    while(1){
        llread(fd, buf);
        printf("Buf char: %d", (int) buf[0]);
        if (buf[0] == (char) 0x01) {
            //receive data packet
            //if ((unsigned int) buf[1] == current) continue;
            int last = current;
            current = (int) buf[1];
            if (current <= last && last != 255) continue;
            
            printf("Current N: %d\n", current);
            int len = getDataLen(buf);
            total += len;
            printf("Gets through getDataLen: len = %d\n\n", len);
            fwrite(buf+4, 1, len, application.file);
            printf("Total: %d\n\n\n\n\n\n\n", total);
            //fflush(fd);
            for(int i = 0; i < len; i++){
                //printf("i: %d ; buf[i]: %d\n", total+i,  buf[4+i]);
            }
        }
        
        else if (buf[0] == 0x02) {
            printf("Number of bytes in start control packet: %d\n", llread_size_packet);
            if(!inicialized){
                printf("AAAAAAAAAA\n");
                temp = (char *) malloc(llread_size_packet * 2 + 13);
            }
            int read_result = 1;
            while(read_result == 1){
                num_bytes_ctrl += llread_size_packet;
                for(int i = 0; i < 7; i++){
                    printf("%d ", (int) temp[i]& 0xFF);
                }
                printf("Num bytes ctrl: %d\n", num_bytes_ctrl);
                read_result = readControlPacket(fd, buf, num_bytes_ctrl, temp);
                printf("%d\n", read_result);
                if(read_result == 1){
                    llread(fd, temp);
                }
            }
            num_bytes_ctrl = 0;
            createFile(filename);
        }

        else if (buf[0] == 0x03) {
            printf("Number of bytes in start control packet: %d\n", llread_size_packet);
            num_bytes_ctrl += llread_size_packet;
            int read_result = 1;
            while(read_result == 1){
                read_result = readControlPacket(fd, buf, num_bytes_ctrl, temp);
                if(read_result == 1){
                    llread(fd, temp);
                }
            }
            break;
        }

        else if (buf[0] == FLAG){
            printf("ERROR: buf[4] = %d\n", (int) buf[4]);
            printf("data len: %d\n", getDataLen(buf+4));
        }
    }
}


int getDataLen(char* frame){ //read data packets -> returns array with bytes to be added to the destination file
    if (frame[0] != (char) 0x01) return 0;
    int L2 = ((int) frame[2]) & 0xFF;
    int L1 = ((int) frame[3]) & 0xFF;
    printf("L2: %d ; L1: %d\n", L2, L1);
    return 256*L2+L1;
}