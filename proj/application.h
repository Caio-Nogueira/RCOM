#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include "ll.h"

#define CONTROL_START 2
#define CONTROL_END 3
#define T_FILE_SIZE 0
#define T_FILE_NAME 1
#define MAX_CONTROL_SIZE 1024
#define CHUNK_LEN 1024


typedef struct{
    char file_name[256];
    int fileDescriptor;
    int fileSize;
    FILE* file;
    flag status;
}applicationLayer;

const char * get_extension(const char * name);

int readBytes(char * bytes, int num_bytes, int num_members, FILE * file);

int min(const int a, const int b);

void readFile(char *filename);

void sendControlPacket(int controlCamp, char* filename, int fd);

void readControlPacket(int fd);

unsigned verifyControlPacket(char* frame);

void sendDataPackets(int fd, char* filename);

void receiveDataPackets(int fd);

void getData(char* frame, char* result);
