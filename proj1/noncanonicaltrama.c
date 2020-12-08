#include "application.h"
#include <sys/time.h>
int main(int argc, char** argv)
{
    int fd,c, res;
    struct timeval start, end;
    double time_used;    
    if ( (argc < 3) || 
  	     ((strcmp("/dev/ttyS", argv[1])!=0))) {
      printf("Usage:\tncan SerialPort Baudrate\n\tex: ncan /dev/ttyS1 B38400\n");
      exit(1);
    }
    long baud = 0;
    baud = strtol(argv[2],NULL, 16);
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
    char str[256];
    llopen(fd, RECEIVER, baud);
    gettimeofday(&start, NULL);
    readPackets(fd);    
    llclose(fd);
    gettimeofday(&end, NULL);
    time_used = (end.tv_sec + end.tv_usec / 1e6) - (start.tv_sec - start.tv_usec / 1e6);
    printf("Seconds elapsed: %f\n", time_used);
    return 0;
}
