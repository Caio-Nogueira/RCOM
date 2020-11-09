/*Non-Canonical Input Processing*/
#include "application.h"
#include <time.h>


int main(int argc, char** argv)
{
    int fd,c, res;

    clock_t start, end;
    double cpu_time_used;    


    if ( (argc < 3) || 
  	     ((strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) &&
          (strcmp("/dev/ttyS0", argv[1])!=0))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    start = clock();

    long baud = 0;

    baud = strtol(argv[2],NULL, 16);

    


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
    char str[256];

    llopen(fd, RECEIVER, baud);
    readPackets(fd, "dest/imagem.jpg");

    //llread(fd, str);
    //readControlPacket(fd);


    /*char buffer[255] = "";
    while(TRUE){
    llread(fd,buffer);
    }*/
/*
    char UAresponse[255];


    printf("Awaiting UA response.\n");
    res = read(fd, UAresponse, 6);
    printf("UA response read.\n");
    printf("UA response is: \n");
    write(STDOUT_FILENO, UAresponse, 6);
    printf("\n");

    printf("Veryfing UA response.\n");
    int UA = verifyUA(UAresponse);
    printf("UA response confirmed.\n");
    printf("UA response is: \n");
    write(STDOUT_FILENO, UAresponse, 6);
    printf("\n");
*/

    
    //printf("New termios structure set\n");
      //printf("AAAAAAAAAAAAAAAAAAAAA");


    //while (STOP==FALSE) {       /* loop for input */
    //  res = read(fd,buf,255);   /* returns after 5 chars have been input */
    //  buf[res]=0;               /* so we can printf... */
    //  printf(":%s:%d\n", buf, res);
    //  if (buf[0]=='z') STOP=TRUE;
    //}



  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */



    //tcsetattr(fd,TCSANOW,&oldtio);
    llclose(fd);
    end = clock();

    cpu_time_used = ((double)(end-start)) / CLOCKS_PER_SEC;

    printf("Seconds elapsed: %f\n", cpu_time_used*10);
    return 0;
}