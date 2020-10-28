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
    application.file = myfile;
    
}

//create file
void createFile(char *filename){
    FILE * myfile;
    myfile = fopen(filename, "w+");
    application.file = myfile;
}

void sendControlPacket(int controlCamp, char* filename, int fd){
    unsigned char controlPacket[MAX_CONTROL_SIZE + 5] = "";
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
    //sprintf((controlPacket + 2), "%c", (unsigned char) L1);


    //int file_size_2 = application.fileSize;
    //printf("File size: %d", file_size_2);
    //printf("File size 1st byte: %d", (file_size_2 & 0xFF));
    //printf("File size 2nd byte: %d", ((file_size_2 / 256) & 0xFF));

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

/*
    unsigned char a = (unsigned char) file_size_2 & 0xFF;
    controlPacket[2 + temp] = (unsigned char) file_size_2 & 0xFF;
    printf("This should be 216 ->: %d", (int) a);
    sprintf(controlPacket + 3, "%c", (unsigned char) file_size_2 & 0xFF);
    int temp = 1;
    for(temp = 1; temp < L1; temp++){
        file_size_2 /= 256;
        printf("Current file size: %d", file_size_2);
        if(file_size_2 == 0){
            break;
        }
        controlPacket[2 + temp] =  (unsigned char) file_size_2 & 0xFF;
        //sprintf(controlPacket + 2 + temp, "%c", (unsigned char) file_size_2 & 0xFF);
    }
    printf("temp = %d\n", temp);*/


    

    /*
    sprintf(controlPacket + 3, "%c", (unsigned char)(file_size >> 24) & 0xFF);
    sprintf(controlPacket + 4, "%c", (unsigned char)(file_size >> 16) & 0xFF);
    sprintf(controlPacket + 5, "%c", (unsigned char)(file_size >> 8) & 0xFF);
    sprintf(controlPacket + 6, "%c", (unsigned char) file_size & 0xFF);
    */

    sprintf(controlPacket + 3 + num_bytes, "%c", T2);
    //sprintf(controlPacket + 4 + num_bytes, "%c", (unsigned char) L2);
    controlPacket[4 + num_bytes] = (unsigned char) L2;
    //strcat(controlPacket, V2);
    int j = 0;
    for (int i = 0; i < L2; i++){
        controlPacket[5 + num_bytes + j] = filename[i];
        //sprintf(controlPacket + 5 + num_bytes + j, "%c", filename[i]);
        j++;
    }

    for(int l = 0; l < 5 + num_bytes + j; l++){
        printf("%d ", (int) controlPacket[l]);   
    }

    //printf("size of packet: %d\n", 5+num_bytes+j);
    llwrite(fd, controlPacket, 5+num_bytes+j);
    //llwrite(fd, controlPacket, 5+L1+L2);
    printf("Packet control sent.\n");


}



void readControlPacket(int fd, char* string){ //  receiver port filedes
    //char string[256];
    //llread(fd, string);
    
    int sizeArg0 = (int) string[2];
    int sizeArg1 = (int) string[2 + 2 + sizeArg0];

    printf("Size argument 0: %d\n", sizeArg0);
    printf("Size argument 1: %d\n", sizeArg1);

    for(int i = 0; i < sizeArg0 + sizeArg1 + 5; i++){
        //printf("%d ", (unsigned int) string[i] & 0xFF);
    }

    int file_size = 0;
    int a = 1;
    for(int i = 0; i < sizeArg0; i++){
        a *= 256;
        file_size += (((unsigned int) string[2 + sizeArg0 - i]) & 0xFF) * a / 256;
    }

    printf("File size: %d\n", file_size);

    char name[256];
    for(int i = 2 + 2 + sizeArg0 + 1; i < 2 + 2 + sizeArg0 + 1 + sizeArg1; i++){
        sprintf(name - (2 + 2 + sizeArg0 + 1) + i, "%c",  string[i]);
    }
    printf("Filename: %s\n", name);
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
    return (frame[4] == (char) CONTROL_START || frame[4] == (char) CONTROL_END);
}

void sendDataPackets(int fd, char* filename){
    char dataPacket[CHUNK_LEN+4];
    int n_sequence = 0;    
    int bytesRead = 0;
    unsigned char buf[CHUNK_LEN];
    //int test = open("test.txt", O_WRONLY | O_APPEND);
    //printf("teste.\n");
    while (bytesRead < application.fileSize){
        int bytes = read(application.fileDescriptor, &buf, CHUNK_LEN);
        bytesRead += bytes;
        //printf("nbytes: %d\n", bytes);
        sprintf(dataPacket, "%c", (char) 0x01); //valor: 1 ---> dados
        sprintf(dataPacket + 1, "%c", (char)(n_sequence % 255));
        int L2 = bytes / 256;
        int L1 = bytes % 256;
        //printf("L2: %d ; L1: %d\n", L2, L1);
        sprintf(dataPacket + 2, "%c", (char) L2);
        sprintf(dataPacket + 3, "%c", (char) L1);
        for (int i = 0; i < 256*L2 + L1; i++){
            //sprintf(dataPacket + 4 + i, "%c", buf[i]);
            dataPacket[4 + i] = buf[i];
        }
        //printf("L2: %d ; L1: %d\n", dataPacket[2], dataPacket[3]);
        int f_size = min(CHUNK_LEN + 4, application.fileSize);
        f_size = min(f_size, 256 * L2 + L1 + 4);
        llwrite(fd, dataPacket, f_size);        
        n_sequence++;
    }
    sendControlPacket(CONTROL_END, filename, fd);
    printf("CCCC\n");
}

void readPackets(int fd, char* filename){
    printf("Start of readPackets\n");
    int dest = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (dest == -1){
        perror("dest\n");
    }
    char buf[MAX_TRAMA_SIZE];
    while(1){
        //printf("%lu\n", sizeof(buf));
        llread(fd, buf);
        printf("Buf char: %d", (int) buf[0]);
        if (buf[0] == (char) 0x01) {
            //receive data packet
            //char bytes[CHUNK_LEN];
            int len = getDataLen(buf);
            printf("Gets through getDataLen\n");
            fwrite(buf+4, 1, len, application.file);
            printf("Length: %d\n\n\n\n\n\n\n", len);
            for(int i = 0; i < len; i++){
                //printf("%d ", ((int) buf[i + 4]) & 0xFF);
            }
        }
        
        else if (buf[0] == 0x02) {
            readControlPacket(fd, buf);
            createFile("dest/pinguim.gif");
        }

        else if (buf[0] == 0x03) {
            readControlPacket(fd, buf);
            break;
        }
    }
}

/*
void receiveDataPackets(int fd){
    unsigned char buf[1028];
    int dest = open("dest/pinguim.gif", O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (dest == -1){
        perror("dest\n");
    }

    while (1){
        printf("\nReading.\n");
        llread(fd, buf);
        //write(STDOUT_FILENO, buf, 1028);
        //printf("")
        char bytes[CHUNK_LEN]; 
        if (buf[4] == CONTROL_END) break;
        getData(buf);
        printf("N: %d\n",(unsigned int) buf[5]);
        write(dest, bytes, CHUNK_LEN);
    }
    readControlPacket(fd);
}
*/

int getDataLen(char* frame){ //read data packets -> returns array with bytes to be added to the destination file
    //printf("Char: %d\n", frame[0]);
    //printf("Char: %d\n", frame[1]);
    if (frame[0] != (char) 0x01) return 0;
    //printf("N: %d\n", (int) frame[5]);
    //printf("C: %d\n", (int) frame[4]);
    int L2 = ((int) frame[2]) & 0xFF;
    int L1 = ((int) frame[3]) & 0xFF;
    printf("L2: %d ; L1: %d\n", L2, L1);
    //int j = 0;
    /*for (int i = 0; i < 256*L2 + L1; i++){
        sprintf(result+j, "%c", frame[8+i]);
        j++;
    }*/

    return 256*L2+L1;
}