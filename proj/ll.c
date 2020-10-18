#include "ll.h"


volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int flag_rewrite_SET = 1; //In the first input loop, dictates wether SET should be rewritten 
int tries = 0;
int res;

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


void alarmHandler(){
  printf("alarm was called.\n");
  tries++;
  flag_rewrite_SET = 0;
  printf("%d\n", tries);
}


int SETstateMachine(char* setmsg){
    int n = 0;
    while (STOP==FALSE) {       /* loop for input */
      if(n == 0){
        if((int) setmsg[n] != (char) FLAG){ //If the first character sent isn't FLAG, then it's invalid and should be sent again 
          SUCESS = FALSE; //Não teve sucesso
        }
      }
      else if(SUCESS){
        if(n == 1){
          if(setmsg[n] != (char) (A_SEND)){
            SUCESS = FALSE;
          }
          printf("1\n");
        }
        else if(n == 2){
          if(setmsg[n] != ((char) (C_SET))){
            SUCESS = FALSE;
          }
          printf("2\n");
        }
        else if(n == 3){
          if(setmsg[n] != (char) (BCC1)){
            SUCESS = FALSE;
          } 
          printf("3\n");
        }
        else if(n == 4){
          if(setmsg[n] != (char) (FLAG)){
            SUCESS = FALSE;
          }
          printf("4\n");
        }
        else if(n == 5){
            STOP = TRUE;
            printf("5\n");
        }
      }
      else if(n == 5){
        n = -1;
        printf("Failure.");
        return FALSE;
      }
      printf("%c\n", setmsg[n]);
      n++;
    }
    return SUCESS;
}

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



void llopen(int fd, flag flag){
    struct termios oldtio,newtio;

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
    
    
    switch(flag){
        case TRANSMITTER:
        {
        
            struct sigaction action;
            memset(&action,0,sizeof action);
            action.sa_handler = alarmHandler;
            action.sa_flags = 0;
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

                
            char set[6];
            BuildSet(set);
            char ua[6] = "";

            while(tries < NUM_TRIES){
              if(flag_rewrite_SET){
                flag_rewrite_SET = 0;


                res = write(fd,set,6);
                printf("SET message sent\n");
                alarm(3);
                if(res != 6){
                  printf("Did not write 6 characters. Exiting\n");
                  exit(0);
                }
              }
              res = read(fd, ua, 6);
              if (res) printf("%s\n", ua);
              
              if(res == -1){
                printf("Faild to read UA. Trying again.\n");
              }
              else if(res != 6){
                printf("UA doesn't have the correct length, It should be 6, it is %d.\n", res);
              }
              else{
                if(ReadUA(ua, res) != res){
                  printf("Error when receiving UA. The program will not reset to sending SET.\n");
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

            break;
        }
        case RECEIVER:
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
            printf("Read string.\n");
            //receber e verificar o SET
            if (SETstateMachine(str) == FALSE){
              printf("Error in SET message\n");
              exit(-1);
            }
            res = write(STDOUT_FILENO,str,6);  

            char UAsend[255];

            //SendUA(UAsend);
            sprintf((UAsend + 0) , "%c", (char) FLAG);
            sprintf((UAsend + 1) , "%c", (char) A_Recieve);
            sprintf((UAsend + 2) , "%c", (char) C_UA);
            sprintf((UAsend + 3) , "%c", (char) BCC2);
            sprintf((UAsend + 4) , "%c", (char) FLAG);
            UAsend[strlen(UAsend)] = 0;

            
            printf("Sending UA.\n");
            res = write(fd, UAsend, 6);
            printf("%s\n", UAsend);
            printf("UA sent.\n");

            break;
    }
}

