#include "ll.h"

volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int flag_rewrite_SET = 1; //In the first input loop, dictates wether SET should be rewritten 
int flag_rewrite_frame = 1;
int tries = 0;
int res;
int odd = 0;


linkLayer ll = {0xB38400, 0};

char result[MAX_TRAMA_SIZE] = ""; //Contains the trama that will be sent next/is being sent


struct termios oldtio;


speed_t checkBaudrate(long br){
    switch (br){
        case 0xB0:
            return B0;
        case 0xB50:
            return B50;
        case 0xB75:
            return B75;
        case 0xB110:
            return B110;
        case 0xB134:
            return B134;
        case 0xB150:
            return B150;
        case 0xB200:
            return B200;
        case 0xB300:
            return B300;
        case 0xB600:
            return B600;
        case 0xB1200:
            return B1200;
        case 0xB1800:
            return B1800;
        case 0xB2400:
            return B2400;
        case 0xB4800:
            return B4800;
        case 0xB9600:
            return B9600;
        case 0xB19200:
            return B19200;
        case 0xB38400:
            return B38400;
        case 0xB57600:
            return B57600;
        case 0xB115200:
            return B115200;
        case 0xB230400:
            return B230400;
        default:
            printf("Bad baudrate value. Using default (B38400)");
            return B38400;
    }
}

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
  printf("alarm was called. Cleaning input buffer\n");
  tries++;
  flag_rewrite_SET = 1;
  flag_rewrite_frame = 1;
  printf("n_tries: %d\n", tries);
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
  return 5;
}



void llopen(int fd, flag flag, long baud){
    struct termios newtio;

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    ll.baudrate = checkBaudrate(baud);

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = ll.baudrate | CS8 | CLOCAL | CREAD;
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

            newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
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
              if(flag_rewrite_SET){
              //printf("Before write.\n");
                flag_rewrite_SET = 0;


                res = write(fd,set,5);
                printf("SET message sent\n");
                alarm(3);
                if(res != 5){
                  printf("Did not write 5 characters. Exiting\n");

                  exit(0);
                }
              }
              //res = read(fd, ua, 6);
              int num_times = 0;
              while(num_times < 5){
                res = read(fd, ua + num_times, 1);
                if(res != -1){
                  num_times++;
                  res = num_times;
                  printf("byte: %d", *(ua+num_times));
                }
                else{
                  break;
                }
              }
              //if (res) printf("%s\n", ua);
              
              if(res == -1){
                printf("Faild to read UA.\n"); //Trying again.\n");
              }
              else if(res != 5){
                printf("UA doesn't have the correct length, It should be 6, it is %d.\n", res);
              }
              else{
                if(ReadUA(ua, res) != res){
                  printf("Error when receiving UA. The program will not reset to sending SET.\n");
                  printf("State machine not implemented yet. Exiting instead\n");
                  //exit(0);
                }
                else{
                  alarm(0); //Clear every alarm
                  break;
                }
              }
              
            }      
            if(tries >= NUM_TRIES){
              printf("Number of tries exceeded. Exiting.\n");
              exit(0);
            }      
            break;
        }
        case RECEIVER:
            newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
            newtio.c_cc[VMIN]     = 1;


            struct sigaction action;
            memset(&action,0,sizeof action);
            action.sa_handler = alarmHandler;
            action.sa_flags = 0;
            sigaction(SIGALRM, &action, NULL);


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
            
            int num_times = 0;
            while(num_times < 5){
              res = read(fd, str + num_times, 1);
              if(res != -1){
                num_times++;
              }
            }
            for(int i = 0; i < 5; i++){
              printf("%d", (int) str[i] & 0xFF);
            }
            //res = read(fd,str,6);   /* returns after 6 chars have been input */
            printf("Read string.\n");
            //receber e verificar o SET
            if (SETstateMachine(str) == FALSE){
              printf("Error in SET message\n");
              exit(-1);
            }
            res = write(STDOUT_FILENO,str,5);  

            char UAsend[255];

            //SendUA(UAsend);
            sprintf((UAsend + 0) , "%c", (char) FLAG);
            sprintf((UAsend + 1) , "%c", (char) A_Recieve);
            sprintf((UAsend + 2) , "%c", (char) C_UA);
            sprintf((UAsend + 3) , "%c", (char) BCC2);
            sprintf((UAsend + 4) , "%c", (char) FLAG);
            //UAsend[strlen(UAsend)] = 0;

            
            printf("Sending UA.\n");
            res = write(fd, UAsend, 5);
            printf("%s\n", UAsend);
            printf("UA sent.\n");

            break;
    }
    ll.sequenceNumber = 0;
}

/*Writer function
Adds trama to array to be used in writter
*/
void buildwritearray(int odd, char * message, size_t * size){//Aditional arguments will need to be char * original_message, int size_of_original_message
  //char message[121] = "123\0a456"; //This shouldn't be the ,maximum size of the final message
  //size_t size = 8;

  int bcc2 = 0;
  int real_size = (int) *size;
  for(int i = 0; i < real_size; i++){
    bcc2 = bcc2 ^ message[i];
  }
  //printf("real size: %d\n", real_size);
  sprintf(result, "%c", (unsigned char) FLAG);
  sprintf(result + 1, "%c", (unsigned char) A_SEND);
  char current_C = (char) (C_SET | ((ll.sequenceNumber) * EVENIC));
  sprintf(result + 2, "%c", current_C ^ A_SEND);
  sprintf(result + 3, "%c", (char) (A_SEND | current_C));
  //sprintf(result + 4, "%s", message); //Check if this works with '\0' later
  //sprintf(result + 4 + size, "%c", );

  int j = 0;
  for(int i = 0; i < real_size; i++){
    /*if(message[i] == '\0'){
      write(STDOUT_FILENO, "A", 1);
    }
    else{
    write(STDOUT_FILENO, message + i, 1);
    }*/
    //write(STDOUT_FILENO, message + i, 1);
    //printf("Message %d: %d\n", i, (int) message[i]);
    //printf("Result 4 + i + j: %d\n", (int) result[4 + i + j]);
    if(message[i] == (char) FLAG){      
      //printf("C\n");
      result[4 + i + j] = (char) STUFFLAG1;
      
      //sprintf(result + 4 + i + j, "%c", (char) STUFFLAG1);
      //bcc2 = bcc2 ^ message[i];//result[ 4 + i + j];
      j++;
      
      result[4 + i + j] = (char) STUFFLAG2;
      //sprintf(result + 4 + i + j, "%c", (char) STUFFLAG2);
      //bcc2 = bcc2 ^ result[ 4 + i + j];
    }else if(message[i] == (char) REPLACETRAMA2){
      //printf("B\n");
      //sprintf(result + 4 + i + j, "%c", (char) STUF7D1);
      result[4 + i + j] = (char) STUF7D1;
      //bcc2 = bcc2 ^ message[i];//result[ 4 + i + j];      
      j++;
      result[4 + i + j] = (char) STUF7D2;
      //sprintf(result + 4 + i + j, "%c", (char) STUF7D2);
      //bcc2 = bcc2 ^ result[ 4 + i + j];
    }
    else{      
      //printf("A\n");
      result[4 + i + j] = message[i];
      //printf("?\n");
      //sprintf(result + 4 + i + j, message + i, 1);
      //bcc2 = bcc2 ^ result[4 + i + j];
      
    }

    //printf("%d\n", 4 + i + j);
    /*printf("i: %d", i+j);
    printf(" ; char: %c\n", result[ 4 + i + j]);*/
    //result[3 + size] = message[i];
    //bcc2 = bcc2 ^ message[i];
  }

  //printf("End of a loop.\n");
  //printf("%s\n", result);
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
  //printf("bcc2: %d, flag, %d", (int) bcc2, FLAG);
  //printf("true?: %d, %d", bcc2 == (char) FLAG, 1 == 1);
  if(bcc2 == (char) FLAG){
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUFFLAG1);
    j++;
    //sprintf(result + 4 + (real_size) + j, "%c", (char) STUFFLAG2);
    result[4 + (real_size) + j] =  (char) STUFFLAG2;
  }else if(bcc2 == (char) REPLACETRAMA2){
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUF7D1);
    j++;
    //sprintf(result + 4 + (real_size) + j, "%c", (char) STUF7D2);
    result[4 + (real_size) + j] =  (char) STUF7D2;
  }
  else{    
    //sprintf(result + 4 + (real_size) + j, "%c", (char) bcc2); 
    result[4 + (real_size) + j] = (char) bcc2;
  }
  //printf("bcc: %c", *(result + 4 + (real_size) + j));
  result[5 + (real_size) + j] = (char) FLAG;
  //sprintf(result + 5 + (real_size) + j, "%c", (char) FLAG);
  (*size) += 6 + j;
  real_size = (*size);
  //Print final message
  /*for(int i = 0; i < (*size) + j; i++){
    write(STDOUT_FILENO, result + i, 1);
  }*/
  //write(STDOUT_FILENO, result, 6 + (real_size) + j);
  //printf("Size: %d\n", real_size);
  memcpy(message, result, (real_size));
  
}

int destuffing(int isOdd, char * message, int * size){
  char resu[CHUNK_SIZE + 5] = "";
  int real_size = (*size);
  
  

  int bcc2_received = (int) (message[(*size)-2]);
   int bcc1_received = 0;
  int endchars; //Última carater da trama
  if(message[(*size) - 2] == (char) STUF7D2){
	  bcc1_received = REPLACETRAMA2;
    endchars = (*size) - 2;
   // printf("bcc with stuffing.\n");
    bcc1_received = (int) message[(*size)-2];
   if(message[(*size) - 3] == STUF7D1){
     bcc1_received = (int) message[(*size)-3];
     endchars--;
   }
  }

  else if(message[(*size) - 2] == (char) STUFFLAG2){
	bcc1_received = FLAG;
    endchars = (*size) - 2;
   // printf("bcc with stuffing.\n");
    bcc1_received = (int) message[(*size)-2];
    if(message[(*size) - 3] == STUFFLAG1){
      bcc1_received = (int) message[(*size)-3];
      endchars--;
    }
    
    //endchars--;
  }
  else{
    endchars = (*size) - 2;
	  bcc1_received = (unsigned int) (message[(*size)-2] & 0xff);
    //bcc2_received = (unsigned int) (message[(*size)-3] & 0xff);
  }

  int bcc2 = 0;
  int j = 0;
  int countnext = 1;
  //printf("Went past first part.\n");

  for(int i = 4; i < endchars; i++){
    if(message[i + j] == (char) STUFFLAG1 || message[i + j] == (char) STUF7D1){
      //printf("first if, i = %d\n", i-4);

      //bcc2 = bcc2 ^ message[i + j];
      if(message[i + j] == (char) STUFFLAG1){
        bcc2 = bcc2 ^ (char) FLAG;
        countnext = 0;
      }
      else if(message[i + j] == (char) STUF7D1){
        bcc2 = bcc2 ^ (char) REPLACETRAMA2;
        countnext = 0;
      }

      if(message[i + 1 + j] == (char) STUFFLAG2){
      //printf("second if.\n");
        resu[i-4] = (char) FLAG;
      }
      else if(message[i + 1 + j] == (char) STUF7D2){
      //printf("third if.\n");
        resu[i-4] = (char) REPLACETRAMA2;
      }
      else{
      //printf("else.\n");
      }
	    //real_size--;
      endchars--;
      j++;
    }
    else{
      if(countnext == 1){
        bcc2 = bcc2 ^ message[i + j];
      }
      else if(countnext == 0){
        countnext = 1;
      }
      resu[i-4] = message[i + j];
	  //bcc2 = bcc2 ^ resu[i-4];
    }
  }

	/*bcc2 = 0;
  for (int i = 0; i < real_size; i++){
		bcc2 = bcc2 ^  resu[i];
  }*/
  
  (*size) -= j + 4 + 2;
  real_size = (*size);
  //printf("reached memcpy.\n");
 // printf("%d\n", endchars);
 // printf("size: %d\n",real_size);
  //int sub_bcc_count = 0;
  /*if(endchars - j - 6 == (*size) - 3){
    printf("A\n");
    sub_bcc_count = 1;
    //real_size--;
  }*/
  bcc2 = 0;
  for (int i = 0; i < (*size); i++){
		bcc2 = bcc2 ^  resu[i];
  }

  
  
  memcpy(message, resu, real_size);

  bcc2 = 0;
  for (int i = 0; i < (*size); i++){
		bcc2 = bcc2 ^  message[i];
  }
  
  if(endchars - 6 == (*size) - 3){
    (*size)--;
    return 0;
  }
  else if((unsigned int) (bcc1_received & 0xff) == (unsigned int) (bcc2 & 0xff)){
    return 0;
  }

  else if ((unsigned int) (bcc2_received & 0xff) == (unsigned int) (bcc2 & 0xff)){
    return 0;
  }

  printf("Destuffing failed: bcc_received: %d, real_bcc: %d\n", (unsigned int) (bcc1_received & 0xff),(unsigned int) (bcc2 & 0xff));
  return 1;
}

int readInformationFrame(int fd, char* buffer, int* success, int *duplicate){
  //read I frame
  int len = 0;
  char byte;
  InformationFrameState state = START;
  
  alarm(20);

  
  while (read(fd, &byte, 1) > 0){
    DataFrameStateMachine(&state, byte, duplicate);
    if(state == START){
      len = 0;
    }
    buffer[len++] = byte;
    if (state == END) {
      *success = ACK;
      if(duplicate){
        printf("ACK achieved!\n");
      }
      else{
        printf("Duplicate packet recieved.\n");
      }
      return len;
    }
    else if (state == DISCONNECT){
      printf("DISC achieved!\n");
      return -1;
    }
    else if (state == ERROR){
      printf("ERROR.\n");
      break;
    }
  }
  alarm(0);
  *success = NACK;
  return 0;
}


void DataFrameStateMachine(InformationFrameState *state, char byte, int *duplicate){
  switch (*state)
  {
  
  case START:
    //printf("START\n");
    //printf("byte: %d\n", (int) byte);
    if (byte == FLAG) *state = FLAG_RCVD;
    else
        *state = START;
    break;
  
  case FLAG_RCVD:
    //printf("FLAG_RCVD\n");
    if (byte == A_SEND) *state = A_RCVD;
    else if(byte == A_Recieve){
      *state = DISCA;
      printf("Disconnect possibly detected.\n");
    }
    else if(byte == FLAG){
        *state = FLAG_RCVD;
    }
    else {
      printf("Error byte: %d\n", (int) byte);
      *state = START;
      return;
    }
    break;
  
  case A_RCVD:
    //printf("A_RCVD\n");
    if ((byte == EVENIC && ll.sequenceNumber == 1) || (byte == BASEIC && ll.sequenceNumber == 0)){
      *state = C_RCVD;
    }
    else if(byte == EVENIC || byte == BASEIC){
      printf("Duplicate packet found.\n");
      (*duplicate) = TRUE;
      *state = C_RCVD;
    }
    else {
      *state = START;
      return;
    }
    break;

  case C_RCVD:
    //printf("C_RCVD\n");
    if (byte == (A_SEND ^ BASEIC) || byte == (A_SEND ^ EVENIC)){
      *state = BCC1_RCVD;
    }
    else {
      *state = START;
      return;
    }
    break;

  case BCC1_RCVD:
    //printf("BCC1_RCVD\n");
    if (byte != FLAG) *state = DATA_RCVD;
    else {
      *state = START;
      return;
    }
    break;
  case DATA_RCVD:
    //printf("DATA_RCVD\n");
    if (byte == FLAG){ *state = END; //sucess
      //printf("End.\n");
      }
    break;

  case END: 
    //printf("END\n");
    break;

  case ERROR:
    break;
  case DISCA:
    if(byte == DISC){
      printf("Disconnected C.\n");
      *state = DISCC;
    }
    else{
      *state = ERROR;
    }
  case DISCC:
    if(byte == DISC || A_SEND){
      printf("Disconnect BCC.\n");
      *state = DISCBCC;
    }
    else{
      *state = ERROR;
    }
    break;  
  case DISCBCC:
    if(byte == FLAG){
      printf("FLAG disconnect\n");
      *state = DISCONNECT;
    }
    printf("\n");
  default:
    break;
  }
}


/*Writer function
Ver pags 14 e 15 do guião
*/
int llwrite(int fd, char * buffer, int length){
  tries = 0;
  flag_rewrite_frame = TRUE;

  buildwritearray(odd, buffer, (size_t *) &length);
  //int temp = buffer[length - 2];
  while (tries < NUM_TRIES && flag_rewrite_frame){
    if (flag_rewrite_frame){
      flag_rewrite_frame = 0;

      //Uncomment and uncomment temp declaration to induce errors in certain bits
      /*if(rand() % 15 == 0){
        buffer[length - 2] = 3;
      }
      else{
        buffer[length - 2] = temp;
      }*/

      printf("Writting to buffer now.");
      write(fd, buffer, length);
      alarm(20);
    }
    printf("Reading.\n");
    char response[6];

    
    int num_times = 0;
    /*while(num_times < 5 && !flag_rewrite_frame){
      res = read(fd, response + num_times, 1);//+ num_times, 1);
      printf("response i: %x\n", response[num_times]);
      if(res != -1){
        num_times++;
        res = num_times;
      }
    }*/
    char temp[6] = "";
    int res2 = read(fd, temp, 5);
    if(res2 != 0){
        printf("Res2 != 0, temp[0] = %d", (unsigned char) temp[0]);
    }
    if (res2 == -1){
      perror("fd");

      sleep(1);
    }
    else if (res2 == 5){
      printf("Reading response!\n");
      int response = readResponse(temp);
      if (response == ACK ||response == DUP){
        alarm(0); //clear alarms
        if(response == ACK){
          printf("RR recieved.\n");
        }
        flag_rewrite_frame = 0;
        //odd = (odd + 1) % 2;
        ll.sequenceNumber++;
        ll.sequenceNumber %= 2;
        break;
      }
      else{
        printf("REJ or invalid response!\n");
        alarm(0);
        flag_rewrite_frame = 1;
        //break;
      }
    }
  }
  return length;
}


/*Reader function
Reads the buffer, (eventually removes stuffing), interprets the content and sends back trama de supervisão
Ver pags 14 e 15 do guião
*/
int llread(int fd, char * buffer){
  int verify = -1;
  int duplicate = 1;
  int frame_length = readInformationFrame(fd, buffer, &verify, &duplicate);

  if(frame_length >= 0){
    //write(STDOUT_FILENO, buffer, frame_length);
    //fflush(stdout);
    char response[6] = "";
    int current_N = 0; //numero de serie por parte do recetor
    //printf("Verify %d\n", verify);
      switch(verify){
        case ACK:
            if(!duplicate){
                buildRresponse(response, &ll.sequenceNumber, ACK);
                write(fd, response, 5);
              return 0;
            }
            if(destuffing(ll.sequenceNumber, buffer, &frame_length) == 0){
                ll.sequenceNumber++;
                ll.sequenceNumber %= 2;
                buildRresponse(response, &ll.sequenceNumber, ACK);
                break; 
            }
            else{
                printf("Destuffing error\n");
                buildRresponse(response, &ll.sequenceNumber, NACK);
                write(fd, response, 5);
                return -1;
            break;
            } 
          //printf("Frame length: %d\n", frame_length);
          //printf("C\n");
          break;
        case NACK:
          printf("NACK read.\n");
          buildRresponse(response, &ll.sequenceNumber, NACK);
          write(fd, response, 5);
          return -1;
          break;
        case DISCONNECT:
          printf("Case disconnect.\n");
          response[0] = FLAG;
          response[1] = A_Recieve;
          response[2] = DISC;
          response[3] = (A_Recieve ^ DISC);
          response[4] = FLAG;
          break;
        default: 
          printf("Default case.\n");
          break;
      }
      write(fd, response, 5);
      return frame_length;
    }
  /*else if(frame_length == -1){
      char disc[5];
      disc[0] = FLAG;
      disc[1] = A_Recieve;
      disc[2] = DISC;
      disc[3] = (A_Recieve ^ DISC);
      disc[4] = FLAG;
      write(fd, disc, 5);
      //alarm(3);
      char UA[5];
      int response = read(fd, UA, 5);
      if(UA[0] == (char) FLAG && UA[1] == (char) A_Recieve && UA[2] == (char) C_UA && UA[3] == (char) (A_Recieve ^ C_UA) && UA[4] == (char) FLAG){
        printf("UA detected, Ending now.\n");
      }
      else{
        printf("%d\n", response);
        for(int i = 0; i < 5; i++){
          printf("%x", UA[i]);
        }
        printf("Thing received isn't an UA. Exiting anyways, since the most likely option by far is errors in the UA sent\n");
      }
      exit(0);
  }*/
  return 0;
}


void buildRresponse(char* buffer, int *N_r, int success){
  sprintf((buffer + 0), "%c", FLAG);
  sprintf((buffer + 1), "%c", A_SEND);
  char control_byte;
  switch (success)
  {
  case ACK:
    //*N_r = (*N_r + 1) % 2;
    if (*N_r == 1) {
      control_byte = 0x85;
    }
    else if (*N_r == 0){
      control_byte = 0x05;
    }
    else return;
    break;
  
  case NACK:
    if (*N_r == 0) {
      control_byte = 0x01;
    }
    else if (*N_r == 1){
      control_byte = 0x81;
    }
    else return;
    break;

  default:
    return;
  }

  int bcc = (int) A_SEND ^ (int) control_byte;
  buffer[2] = control_byte;
  buffer[3] = bcc;
  buffer[4] = (char) FLAG;
}


int readResponse(char* buffer){
  int success = -1;
  for(int i = 0; i < 6; i++){
    switch(i){
      case 0:
        if(buffer[i] != FLAG){
          printf("FLAG error!\n");
          return NACK;
        }
        break;
      case 1:
        if(buffer[i] != A_SEND){
          printf("A_SEND error!\n");
          return 2;
        }
        break;
      case 2:
      {
        unsigned char control = (unsigned char) buffer[i];
        if((control == 0x85 && ll.sequenceNumber == 0) || (control == 0x05 && ll.sequenceNumber == 1)){
          success = ACK;
        }
        else if ((control == 0x81 & ll.sequenceNumber == 0) || (control == 0x01 & ll.sequenceNumber == 1)){
          success = NACK;
        }
        else if(control == 0x85 || control == 0x05 || control == 0x81 || control == 0x01){
          printf("Duplicate packet found.\n");
          success = DUP;
        }
        else
        {
          printf("CONTROL error!\n");
          return 2;
        }
        
        break;
      }
      case 3:
      {
        char bcc;
        bcc = buffer[1] ^ buffer[2];
        if(buffer[i] != (char) bcc){
          printf("Wrong bcc.\n");
          return 3;
        }
        break;
      }
      case 4:
        if(buffer[i] != FLAG){
          return 4;
        }
    }
  }
  return success;
}

int checkdisc(char * str){
  return(str[0] == (char) FLAG && str[1] == (char) A_Recieve && str[2] == (char) DISC && str[3] == (char) (A_Recieve ^ DISC) && str[4] == FLAG);
}

int llclose(int fd){
      //printf("llclose\n");

  char disc[6] = "";
  sprintf(disc, "%c", (char) FLAG);
  sprintf(disc + 1, "%c", (char) A_Recieve);
  sprintf(disc + 2, "%c", (char) DISC);
  sprintf(disc + 3, "%c", (char) (A_Recieve ^ DISC));
  sprintf(disc + 4, "%c", (char) FLAG);
  char UA[6];
  sprintf(UA, "%c", (char) FLAG);
  sprintf(UA + 1, "%c", (char) A_Recieve);
  sprintf(UA + 2, "%c", (char) C_UA);
  sprintf(UA + 3, "%c", (char) (A_Recieve ^ C_UA));
  sprintf(UA + 4, "%c", (char) FLAG);


  tries = 0;
  flag_rewrite_frame = TRUE;
      printf("llclose\n");
  while (tries < NUM_TRIES){
    if (flag_rewrite_frame){
      flag_rewrite_frame = 0;
      write(fd, disc, 5);
      alarm(3);
      printf("Write successful\n");
    }
    char response[6] = "";
    printf("Reading\n");
    //int res = read(fd, response, 5);
    int num_times = 0;
    while(num_times < 5){
      res = read(fd, response + num_times, 1);
      if(res != -1){
        num_times++;
        res = num_times;
        printf("byte: %d", *(response+num_times));
      }
      else{
        break;
      }
    }

    for(int i = 0; i < 5; i++){
      printf("%x ", (int) response[i]);
    }
    printf("Length: %d\n", res);
    printf("Read successful\n");
    if (res == -1) perror("fd");
    else if (res == 5){
      if(checkdisc(response)){
        printf("Disconnect has been recieved back. Sending UA now.\n");
        write(fd, UA, 5);
        printf("Success\n");
        return 5;
      }
    }
  }

  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
  perror("tcsetattr");
  exit(-1);
  }
  return -1;
}
