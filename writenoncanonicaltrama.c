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
#define TIME_PER_TRY 1


volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int tries = 0;


//char str[255]; //SET array
//char stri[255]; //UA array
//char UA[255]; //UA array
int fd;
int UA_read = FALSE;

int res;

int flag_rewrite_SET = TRUE; //In the first input loop, dictates wether SET should be rewritten 


void BuildSet(char *set){
    
    sprintf((set + 0) , "%c", (char) FLAG);
    sprintf((set + 1) , "%c", (char) A_SEND);
    sprintf((set + 2) , "%c", (char) C_SET);
    sprintf((set + 3) , "%c", (char) BCC1);
    sprintf((set + 4) , "%c", (char) FLAG);

}

int ReadUA(char * ua, int numChars){
  for(int i = 0; i < numChars; i++){
    switch(i){
      case 0:
        if(ua[i] != FLAG){
          return 0;
        }
        break;
      case 1:
        if(ua[i] != A_Recieve){
          return 1;
        }
        break;
      case 2:
        if(ua[i] != C_UA){
          return 2;
        }
        break;
      case 3:
        if(ua[i] != BCC2){
          return 3;
        }
        break;
      case 4:
        if(ua[i] != FLAG){
          return 4;
        }
    }
  }
  return 6;
}

void alarmHandler(){
  printf("alarm was called.\n");
  tries++;
  flag_rewrite_SET = TRUE;
  printf("%d\n", tries);
  /*if(tries == NUM_TRIES){
    printf("Maximum number of tries happened. Resetting to start.");
  printf("TODO: implement reset function *part of state machine.\n");
  }
  else if(tries > NUM_TRIES){
    printf("Number of tries greater than maximum number of tries. This should not happen.");
  }
  else{
    printf("tries++\n");
  }
  return;*/
  return;
}


int main(int argc, char** argv)
{
    int c;
    struct termios oldtio,newtio;
    char buf[255];
    int i, sum = 0, speed = 0;
    //signal(SIGALRM, alarmHandler);
    struct sigaction action;
    action.sa_handler = alarmHandler;
    action.sa_flags = 0;

    sigaction(SIGALRM, &action, NULL);


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




    
    char set[6];
    BuildSet(set);
    char ua[6];

    while(tries < NUM_TRIES){
      if(flag_rewrite_SET){
        flag_rewrite_SET = FALSE;


        res = write(fd,set,6);
        alarm(3);
        if(res != 6){
          printf("Did not write 6 characters. Exiting\n");
          exit(0);
        }
      }
      res = read(fd, ua, 6);
      
      
      if(res == -1){
        printf("Faild to read UA. Trying again.\n");
      }
      else if(res != 6){
        printf("UA doesn't have the correct length, It should be 6, it is %d.\n", res);
      }
      else{
        if(ReadUA(ua, res) != res){
          printf("Error when recieving UA. The program will not reset to sending SET.\n");
          printf("State machine not implemented yet. Exiting instead\n");
          exit(0);
        }
        else{
          alarm(0); //Clear every alarm
          break;
        }
      }
      
    }
    printf("%s\n", ua);
    /*
    
    int n = 0;
    
    int a = 0;

    
    alarm(TIME_PER_TRY); //set alarm
    //End of "Sending Set"

    while(TRUE){
      //Sending SET
      printf("Sending SET.\n");
      res = write(fd,str,6);

      printf("SET sent.\n");
      res = read(fd,stri,6);   // returns after 5 chars have been input 
      printf("Waiting.\n");
      if(res == 0){
        sleep(1);
      }
      else{
        break;
      }
    }
    //res = read(fd,stri,6);   // returns after 5 chars have been input 
    printf("%d\n", res);
    printf("UA is: \n");
    write(STDOUT_FILENO, stri, 6);
    printf("\n");

    UA_read = TRUE;
 while (STOP==FALSE) {       // loop for input 
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


    close(fd);*/
    return 0;
}
