/*Non-Canonical Input Processing*/


#include "application.h"

#define MODEMDEVICE "/dev/ttyS11"



//char str[255]; //SET array
//char stri[255]; //UA array
//char UA[255]; //UA array
int UA_read = FALSE;

int res, fd;

//int flag_rewrite_SET = TRUE; //In the first input loop, dictates wether SET should be rewritten 


int main(int argc, char** argv)
{
    int c;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {
      perror(argv[1]); exit(-1); 
    }
    
    llopen(fd, TRANSMITTER);
    sendControlPacket(CONTROL_START, "pinguim.gif", fd);
    sendDataPackets(fd, "pinguim.gif");
    printf("BCSAC\n");
    //sendControlPacket(CONTROL_END, "pinguim.gif", fd);

    
    /*int i = 0;
    while(i < 5){
    llwrite(fd, message, size);
    i++;
    }*/


    llclose(fd);

    close(fd);
    return 0;
}
