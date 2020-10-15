/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS11"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define FLAG 0x7E
#define A_SEND 0x03
#define C_SET 0x03
#define BCC1 (A_SEND ^ C_SET)

#define A_Recieve 0x01
#define C_UA 0x07
#define BCC2 (A_Recieve ^ C_UA)

#define NUM_TRIES 3

//int UA_received = FALSE;
volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int tries = 0;


char str[255]; //SET array
char stri[255]; //UA array
//char UA[255]; //UA array
int fd;
int UA_read = FALSE;

int res;

void alarmHandler(){
  printf("UA Was not sent. Retrying.\n");
  printf("TODO: implement reset function\n");
  exit(0);
}


int main(int argc, char** argv)
{
    int c;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    signal(SIGALRM, alarmHandler);
    
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

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



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
    
    sprintf((str + 0) , "%c", (char) FLAG);
    sprintf((str + 1) , "%c", (char) A_SEND);
    sprintf((str + 2) , "%c", (char) C_SET);
    sprintf((str + 3) , "%c", (char) BCC1);
    sprintf((str + 4) , "%c", (char) FLAG);
    //str[strlen(str)] = 0;

    /*printf("Sending SET.\n");
    res = write(fd,str,6);
    printf("SET sent.\n");
    strcpy(trama, str);*/

    
    int n = 0;
    
    int a = 0;


    alarm(5); //set 3 seconds alarm
    //End of "Sending Set"

    while(TRUE){
      //Sending SET
      printf("Sending SET.\n");
      res = write(fd,str,6);

      printf("SET sent.\n");
      res = read(fd,stri,6);   /* returns after 5 chars have been input */
      printf("Waiting.\n");
      if(res == 0){
        sleep(1);
      }
      else{
        break;
      }
    }
    //res = read(fd,stri,6);   /* returns after 5 chars have been input */
    printf("%d\n", res);
    printf("UA is: \n");
    write(STDOUT_FILENO, stri, 6);
    printf("\n");

    UA_read = TRUE;
 while (STOP==FALSE) {       /* loop for input */
      if(n == 0){
        if((int) stri[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          SUCESS = FALSE; //Não teve sucesso
            printf("UA failed at char 0 (from 0)\n");
        }
          printf("n=0\n");
          printf("%d\n", SUCESS);
      }
      else if(SUCESS){
        if(n == 1){
          if(stri[n] != (char) (A_Recieve)){
            SUCESS = FALSE;
            STOP = TRUE;
            printf("UA failed at char 1 (from 0)\n");
          }
          printf("n=1\n");
          printf("%d\n", SUCESS);
        }
        else if(n == 2){
          if(stri[n] != ((char) (C_UA))){
            SUCESS = FALSE;
            STOP = TRUE;
            printf("UA failed at char 2 (from 0)\n");
          }
          printf("n=2\n");
          printf("%d\n", SUCESS);
        }
        else if(n == 3){
          if(stri[n] != (char) (BCC2)){
            SUCESS = FALSE;
            STOP = TRUE;
            printf("%d\n", (int) stri[n]);
            printf("UA failed at char 3 (from 0)\n");
          } 
          printf("n=3\n");
          printf("%d\n", SUCESS);
        }
        else if(n == 4){
          if(stri[n] != (char) (FLAG)){
            printf("UA failed at char 4 (from 0)\n");
            SUCESS = FALSE;
            STOP = TRUE;
          }
          printf("n=4\n");
          printf("%d\n", SUCESS);
        }
        else if(n == 5){
            STOP = TRUE;
          printf("n=5\n");
          printf("%d\n", SUCESS);
        }
      }
      else if(n == 5){
        //n = -1;
        STOP = TRUE;
        SUCESS = FALSE;
        printf("Failed, string does not contain end char at the end\n");
      }
      n++;
    }
    printf("UA response interpreted as correct.\n");
    printf("UA response is: \n");
    write(STDOUT_FILENO, stri, 6);
    printf("\n");


    close(fd);
    return 0;
}
