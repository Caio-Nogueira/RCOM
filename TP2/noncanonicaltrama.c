/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x73
#define A_SEND 0x03
#define C_SET 0x03
#define BCC1 (A_SEND ^ C_SET)

#define A_Recieve 0x01
#define C_UA 0x07
#define BCCRec (A_SEND ^ C_SET)

volatile int STOP=FALSE;

volatile int SUCESS=TRUE;

/*
int SendUA(char* UAsend){

  return 0;
}*/

int verifyUA(char *UAresponse){//char str[]){
    printf("Attempt\n");
  //char strUA[255];
  int n = 0;
  int sucess = FALSE;

  while (STOP==FALSE) {       /* loop for input */
      if(n == 0){
        if((int) UAresponse[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          sucess = FALSE; //Não teve sucesso
        }
      }
      else if(sucess){
        if(n == 1){
          if(UAresponse[n] != (char) (A_SEND)){
            sucess = FALSE;
          }
          //printf("1\n");
        }
        else if(n == 2){
          if(UAresponse[n] != ((char) (C_SET))){
            sucess = FALSE;
          }
          //printf("2\n");
        }
        else if(n == 3){
          if(UAresponse[n] != (char) (BCC1)){
            sucess = FALSE;
          } 
          //printf("3\n");
        }
        else if(n == 4){
          if(UAresponse[n] != (char) (FLAG)){
            sucess = FALSE;
          }
          //printf("4\n");
        }
        else if(n == 5){
            sucess = TRUE;
            STOP = TRUE;
            //printf("5\n");
        }
      }
      else if(n == 5){
        n = -1;
        printf("Failure.");
        exit(0);
      }
      //printf("%c\n", str[n]);
      n++;
    }
    printf("Sucess\n");
    return sucess;
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];

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
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }


      //printf("AAAAAAAAAAAAAAAAAAAAA\n");
    printf("New termios structure set\n");

    int n = 0;
    char str[255];
    

      res = read(fd,str,6);   /* returns after 6 chars have been input */
    //receber e verificar o SET
    while (STOP==FALSE) {       /* loop for input */
      if(n == 0){
        if((int) str[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          SUCESS = FALSE; //Não teve sucesso
        }
      }
      else if(SUCESS){
        if(n == 1){
          if(str[n] != (char) (A_SEND)){
            SUCESS = FALSE;
          }
          //printf("1\n");
        }
        else if(n == 2){
          if(str[n] != ((char) (C_SET))){
            SUCESS = FALSE;
          }
          //printf("2\n");
        }
        else if(n == 3){
          if(str[n] != (char) (BCC1)){
            SUCESS = FALSE;
          } 
          //printf("3\n");
        }
        else if(n == 4){
          if(str[n] != (char) (FLAG)){
            SUCESS = FALSE;
          }
          //printf("4\n");
        }
        else if(n == 5){
            STOP = TRUE;
            //printf("5\n");
        }
      }
      else if(n == 5){
        n = -1;
        //printf("Failure.");
        exit(0);
      }
      //printf("%c\n", str[n]);
      n++;
    }

    res = write(STDOUT_FILENO,str,6);  

    
    res = write(fd,str,6);   
    //printf("%d bytes written\n", res);
    
    char UAsend[255];

    //SendUA(UAsend);
    sprintf((UAsend + 0) , "%c", (char) FLAG);
    sprintf((UAsend + 1) , "%c", (char) A_Recieve);
    sprintf((UAsend + 2) , "%c", (char) C_UA);
    sprintf((UAsend + 3) , "%c", (char) BCCRec);
    sprintf((UAsend + 4) , "%c", (char) FLAG);
    UAsend[strlen(UAsend)] = 0;

    
    printf("Sending UA.\n");
    res = write(fd, UAsend, 6);

    printf("UA sent.\n");
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



    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
