#include "ll.h"


volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int flag_rewrite_SET = 1; //In the first input loop, dictates wether SET should be rewritten 
int tries = 0;
int res;
int odd = 0;


char result[127] = ""; //Contains the trama that will be sent next/is being sent

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
  flag_rewrite_SET = 1;
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
            sigaction(SIGALRM, &action, NULL);

            newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
            newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



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
              printf("Start of loop.\n");
              printf("%d.\n", flag_rewrite_SET);
              if(flag_rewrite_SET){
              printf("Before write.\n");
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
            newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



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

/*Writer function
Adds trama to array to be used in writter
*/
char * buildwritearray(int odd, char * message, size_t * size){//Aditional arguments will need to be char * original_message, int size_of_original_message
  //char message[121] = "123\0a456"; //This shouldn't be the ,maximum size of the final message
  //size_t size = 8;
  int real_size = (int) *size;
  printf("%d\n", real_size);
  sprintf(result, "%c", (char) FLAG);
  sprintf(result + 1, "%c", (char) A_SEND);
  char current_C = (char) (C_SET | ((odd) * EVENIC));
  sprintf(result + 2, "%c", current_C);
  sprintf(result + 3, "%c", (char) (A_SEND | current_C));
  //sprintf(result + 4, "%s", message); //Check if this works with '\0' later
  //sprintf(result + 4 + size, "%c", );

  int bcc2 = 0;
  for(int i = 0; i < real_size; i++){
    if(message[i] == '\0'){
      printf("\ni: %d, Null character found!\n", i);
      break;
    }
    else{
      write(STDOUT_FILENO, message + i, 1);
    }
    //if (message[i] != '\0')
    sprintf(result + 4 + i, "%c", *(message + i));
    //result[3 + size] = message[i];
    bcc2 = bcc2 ^ message[i];
  }
  //printf("\n");
  /*
  for(int i = 0; i < size; i++){
    if(result[i + 4] == '\0'){
      write(STDOUT_FILENO, "A", 1);
    }
    else{
    write(STDOUT_FILENO, result + i + 4, 1);
    }
  }*/

  //printf("\n");
  sprintf(result + 4 + (real_size), "%c", (char) bcc2);
  sprintf(result + 5 + (real_size), "%c", (char) FLAG);
  //sprintf(result + 6 + (real_size), "%c", (char) 0);
  printf("\n");
  write(STDOUT_FILENO, result, real_size + 6);
  printf("\n");
  return result;
}


int readInformationFrame(int fd, char* buffer){
  //read I frame
  int len = 0;
  char byte;
  while (read(fd, &byte, 1) > 0){
    if (byte == '\0') break;
    buffer[len++] = byte;
    //printf("aaa\n");
  }
  return len;
}


void DataFrameStateMachine(InformationFrameState *state, char byte){
  switch (*state)
  {
  
  case START:
    if (byte == FLAG) *state = FLAG_RCVD;
    break;
  
  case FLAG_RCVD:
    if (byte == A_SEND) *state = A_RCVD;
    else if (byte == FLAG){
      *state = FLAG_RCVD;
    }
    break;
  
  case A_RCVD:
    if (byte == EVENIC || byte == BASEIC){
      *state = C_RCVD;
    }
    else if (byte == FLAG){
      *state = FLAG_RCVD;
    }
    break;

  case C_RCVD:
    if (byte == (A_SEND ^ BASEIC) || byte == (A_SEND ^ EVENIC)){
      *state = BCC1_RCVD;
    }
    else if (byte == FLAG){
      *state = FLAG_RCVD;
    }
    break;

  case BCC1_RCVD:
    if (byte != FLAG) *state = DATA_RCVD;
    else if (byte == FLAG){
      *state = FLAG_RCVD;
    }
    break;
  case DATA_RCVD:
    if (byte == FLAG) *state = END; //sucess
    break;

  case END: break;

  default:
    break;
  }
}


/*Writer function
Ver pags 14 e 15 do guião
*/
int llwrite(int fd, char * buffer, int length){
  int odd = 0;
  char* frame = buildwritearray(odd, buffer, (size_t *) &length);
  //frame[length] = '\0';
  odd = (odd+1) % 2;
  //write(STDOUT_FILENO, result, length+6);
  write(fd, frame, length + 6);
  printf("%d\n", length);
  return length;
  //TODO: implement alarm to protect I frames


}


/*Reader function
Reads the buffer, (eventually removes stuffing), interprets the content and sends back trama de supervisão
Ver pags 14 e 15 do guião
*/
void llread(int fd, char * buffer){
  int frame_length = readInformationFrame(fd, buffer);
  printf("%s\n", buffer);
  //TODO: implement stuffing/destuffing

}
