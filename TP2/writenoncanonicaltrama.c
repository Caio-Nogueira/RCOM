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
#define MODEMDEVICE "/dev/ttyS11"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x73
#define A_SEND 0x03
#define C_SET 0x03
#define BCC1 (A_SEND ^ C_SET)

#define A_Recieve 0x01
#define C_UA 0x07

volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    
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

    printf("New termios structure set\n");

    /*testing*/
    
    char str[255];
    sprintf((str + 0) , "%c", (char) FLAG);
    sprintf((str + 1) , "%c", (char) A_SEND);
    sprintf((str + 2) , "%c", (char) C_SET);
    sprintf((str + 3) , "%c", (char) BCC1);
    sprintf((str + 4) , "%c", (char) FLAG);
    str[strlen(str)] = 0;

    printf("Sending SET.\n");
    res = write(fd,str,6);
    printf("SET sent.\n");
 

    
    int n = 0;
    char stri[255] = "";
    char strbuf[255] = "";
    
    int a = 0;

    printf("Reading SET response.\n");
      res = read(fd,stri,6);   /* returns after 5 chars have been input */

    printf("SET response read.\n");
 while (STOP==FALSE) {       /* loop for input */
      if(n == 0){
        if((int) stri[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          SUCESS = FALSE; //Não teve sucesso
        }
      }
      else if(SUCESS){
        if(n == 1){
          if(stri[n] != (char) (A_SEND)){
            SUCESS = FALSE;
          }
        }
        else if(n == 2){
          if(stri[n] != ((char) (C_SET))){
            SUCESS = FALSE;
          }
        }
        else if(n == 3){
          if(stri[n] != (char) (BCC1)){
            SUCESS = FALSE;
          } 
        }
        else if(n == 4){
          if(stri[n] != (char) (FLAG)){
            SUCESS = FALSE;
          }
        }
        else if(n == 5){
            STOP = TRUE;
        }
      }
      else if(n == 5){
        n = -1;
      }
      n++;
    }
    printf("SET response interpreted as correct.\n");
    printf("Writting back SET response.\n SET is: ");
    res = write(STDOUT_FILENO,stri,6);
    printf("\n");
    printf("SET response written.\n");
  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

 
    printf("Atemp.\n");
    char UA[255] = "";
    printf("Awaiting UA.\n");
    res = read(fd,UA,6);   
    printf("Checking UA.\n");
    while (STOP==FALSE) {       /* loop for input */  
      if(n == 0){
        if((int) UA[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          SUCESS = FALSE; //Não teve sucesso
        }
      }
      else if(SUCESS){
        if(n == 1){
          if(UA[n] != (char) (A_Recieve)){
            SUCESS = FALSE;
          }
        }
        else if(n == 2){
          if(UA[n] != ((char) (C_UA))){
            SUCESS = FALSE;
          }
        }
        else if(n == 3){
          if(UA[n] != (char) (BCC1)){
            SUCESS = FALSE;
          } 
        }
        else if(n == 4){
          if(UA[n] != (char) (FLAG)){
            SUCESS = FALSE;
          }
        }
        else if(n == 5){
            STOP = TRUE;
        }
      }
      else if(n == 5){
        n = -1;
      }
      n++;
    }
    printf("UA is correct.\n");

    printf("Sending UA response.\n");
    res = write(fd, UA, 6);
    printf("UA response sent.\n");


   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
