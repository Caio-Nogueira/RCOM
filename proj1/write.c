/*Non-Canonical Input Processing*/
#include "application.h"
#define MODEMDEVICE "/dev/ttyS11"
int UA_read = FALSE;
int res, fd;
//int flag_rewrite_SET = TRUE; //In the first input loop, dictates wether SET should be rewritten 
void printInvalidArgumentMessage(){
  printf("Usage:\twncan SerialPort number_of_bytes_in_trama Baudrate Filename\n\tex: ncan /dev/ttyS1 500 B38400 pinguim.gif\n");
}
int main(int argc, char** argv)
{
    int c;
    int number_bytes_trama;
    //Parse arguments
    if ( (argc < 5) || 
  	     (strcmp("/dev/ttyS", argv[1])!=0)) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
    if(argc < 3){
      printInvalidArgumentMessage();
      exit(1);
    }
    else{
      for(int i = 0; i < strlen(argv[2]); i++){
        if(!isdigit(argv[2][i])){
          printf("%s is not a number", argv[2]);
          printInvalidArgumentMessage();
          exit(1);
        }
      }
      number_bytes_trama = atoi(argv[2]);
      if(number_bytes_trama < 12 && number_bytes_trama > 131085){
        printInvalidArgumentMessage();
      }
    }
    int number_bytes_message = (number_bytes_trama - 10) / 2;

	long baud = 0;

	baud = strtol(argv[3], NULL,16);


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {
      perror(argv[1]); exit(-1); 
    }    
    llopen(fd, TRANSMITTER, baud);
    printf("%d\n", number_bytes_message);
    sendControlPacket(CONTROL_START, argv[4], fd, number_bytes_message);
    sendDataPackets(fd, argv[4], number_bytes_message);
    printf("BCSAC\n");

    llclose(fd);
    return 0;
}
