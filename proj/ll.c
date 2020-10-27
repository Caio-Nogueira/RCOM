#include "ll.h"


volatile int STOP=FALSE;
volatile int SUCESS=TRUE;

int flag_rewrite_SET = 1; //In the first input loop, dictates wether SET should be rewritten 
int flag_rewrite_frame = 1;
int tries = 0;
int res;
int odd = 0;


char result[255] = ""; //Contains the trama that will be sent next/is being sent

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
  flag_rewrite_frame = 1;
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
              if(flag_rewrite_SET){
              //printf("Before write.\n");
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
              //if (res) printf("%s\n", ua);
              
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
void buildwritearray(int odd, char * message, size_t * size){//Aditional arguments will need to be char * original_message, int size_of_original_message
  //char message[121] = "123\0a456"; //This shouldn't be the ,maximum size of the final message
  //size_t size = 8;
  int real_size = (int) *size;
  printf("real_size: %d\n", real_size);
  sprintf(result, "%c", (char) FLAG);
  sprintf(result + 1, "%c", (char) A_SEND);
  char current_C = (char) (C_SET | ((odd) * EVENIC));
  sprintf(result + 2, "%c", current_C ^ A_SEND);
  sprintf(result + 3, "%c", (char) (A_SEND | current_C));
  //sprintf(result + 4, "%s", message); //Check if this works with '\0' later
  //sprintf(result + 4 + size, "%c", );

  int bcc2 = 0;
  int j = 0;
  for(int i = 0; i < real_size; i++){
    /*if(message[i] == '\0'){
      write(STDOUT_FILENO, "A", 1);
    }
    else{
    write(STDOUT_FILENO, message + i, 1);
    }*/
    //write(STDOUT_FILENO, message + i, 1);
    if(message[i] == (char) FLAG){      
      sprintf(result + 4 + i + j, "%c", (char) STUFFLAG1);
      bcc2 = bcc2 ^ result[ 4 + i + j];
      j++;      
      sprintf(result + 4 + i + j, "%c", (char) STUFFLAG2);
      bcc2 = bcc2 ^ result[ 4 + i + j];
    }else if(message[i] == (char) REPLACETRAMA2){
      sprintf(result + 4 + i + j, "%c", (char) STUF7D1);
      bcc2 = bcc2 ^ result[ 4 + i + j];      
      j++;
      sprintf(result + 4 + i + j, "%c", (char) STUF7D2);
      bcc2 = bcc2 ^ result[ 4 + i + j];
    }
    else{      
      sprintf(result + 4 + i + j, message + i, 1);
      bcc2 = bcc2 ^ result[ 4 + i + j];
      
    }

    /*printf("i: %d", i+j);
    printf(" ; char: %c\n", result[ 4 + i + j]);*/
    //result[3 + size] = message[i];
    //bcc2 = bcc2 ^ message[i];
  }
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
  if(bcc2 == (char) FLAG){
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUFFLAG1);
    j++;
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUFFLAG2);
  }else if(bcc2 == (char) REPLACETRAMA2){
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUF7D1);
    j++;
    sprintf(result + 4 + (real_size) + j, "%c", (char) STUF7D2);
  }
  else{    
    sprintf(result + 4 + (real_size) + j, "%c", (char) bcc2); 
  }
  //printf("bcc: %c", *(result + 4 + (real_size) + j));
  sprintf(result + 5 + (real_size) + j, "%c", (char) FLAG);
  (*size) += 6 + j;
  real_size = *size;
  //Print final message
  /*for(int i = 0; i < (*size) + j; i++){
    write(STDOUT_FILENO, result + i, 1);
  }*/
  printf("result: %s\n", result);
  memcpy(message, result, 255);
  return; //result;
}

int destuffing(int isOdd, char * message, int * size){
  printf("\n");
  char resu[255] = "";
  if(message[0] != (char) FLAG){
    return 1;
  }
  if(message[1] != (char) A_SEND){
    return 1;
  }
  if(message[2] != (char) (BASEIC | ((odd) * EVENIC))){
    return 1;
  }
  if(message[3] != (char) ((C_SET | ((odd) * EVENIC) ^ A_SEND))){
    return 1;
  }

  if(message[(*size) - 1] != (char) FLAG){
    return 1;
  }

  int endchars; //Última carater da trama
  if(message[(*size) - 2] == (char) STUF7D2  || message[(*size) - 2] == (char) STUFFLAG2){
    endchars = (*size) - 3;
  }
  else{
    endchars = (*size) - 2;
  }

  int bcc2;
  int j = 0;

  for(int i = 4; i < endchars; i++){
    if(message[i + j] == (char) STUFFLAG1 || message[i + j] == (char) STUF7D1){
        bcc2 = bcc2 ^ message[i + j];
      if(message[i + 1 + j] == (char) STUFFLAG2){
        sprintf(resu + i - 4, "%c", FLAG);
      }
      else if(message[i + 1 + j] == (char) STUF7D2){
      }
      else{
        return 1;
      }
      j++;
    }
    else{
      sprintf(resu + i - 4, "%c", message[i + j]);
    }
  }
  (*size) -= j + 4 + 2;
  memcpy(message, resu, (*size));
  if(endchars == (*size) - 3){
    return 0;
  }
  else if(message[(*size) - 2] == bcc2){
    return 0;
  }

}


int readInformationFrame(int fd, char* buffer, int* success){
  //read I frame
  int len = 0;
  char byte;
  InformationFrameState state = START;
    printf("Start\n");
  while (read(fd, &byte, 1) > 0){
    fflush(stdout);
    //call destuffing function
    DataFrameStateMachine(&state, byte);
    buffer[len++] = byte;
    if (state == END) {
      *success = ACK;
      printf("ACK achieved!\n");
      return len;
    }
    else if (state == DISCONNECT){
      printf("DISC achieved!\n");
      return -1;
    }
    else if (state == ERROR){
      break;
    }
  }
  printf("\n");
  *success = NACK;
  return 0;
}


void DataFrameStateMachine(InformationFrameState *state, char byte){
  switch (*state)
  {
  
  case START:
    //printf("START\n");
    if (byte == FLAG) *state = FLAG_RCVD;
    break;
  
  case FLAG_RCVD:
    //printf("FLAG_RCVD\n");
    if (byte == A_SEND) *state = A_RCVD;
    else if(byte == A_Recieve){
      *state = DISCA;
      printf("Disconnect possibly detected.\n");
    }
    else {
      *state = ERROR;
      return;
    }
    break;
  
  case A_RCVD:
    //printf("A_RCVD\n");
    if (byte == EVENIC || byte == BASEIC){
      *state = C_RCVD;
    }
    else {
      *state = ERROR;
      return;
    }
    break;

  case C_RCVD:
    //printf("C_RCVD\n");
    if (byte == (A_SEND ^ BASEIC) || byte == (A_SEND ^ EVENIC)){
      *state = BCC1_RCVD;
    }
    else {
      *state = ERROR;
      return;
    }
    break;

  case BCC1_RCVD:
    //printf("BCC1_RCVD\n");
    if (byte != FLAG) *state = DATA_RCVD;
    else {
      *state = ERROR;
      return;
    }
    break;
  case DATA_RCVD:
    //printf("DATA_RCVD\n");
    if (byte == FLAG){ *state = END; //sucess
      printf("End.\n");
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
  int odd = 0;
  //buildwritearray(odd, buffer, (size_t *) &length);
  tries = 0;
  flag_rewrite_frame = TRUE;
  while (tries < NUM_TRIES){
    if (flag_rewrite_frame){
      flag_rewrite_frame = 0;
      buildwritearray(odd, buffer, (size_t *) &length);
      write(fd, buffer, length);
      alarm(3);
    }
    char response[6];
    int res = read(fd, response, 6);
    if (res == -1) perror("fd");
    else if (res == 6){
      printf("Reading response!\n");
      if (readResponse(response) == ACK){
        alarm(0); //clear alarms
        odd = (odd + 1) % 2;
        break;
      }
      else{
        printf("Invalid response!\n");
      }
    }
  } 
  return length;
}


/*Reader function
Reads the buffer, (eventually removes stuffing), interprets the content and sends back trama de supervisão
Ver pags 14 e 15 do guião
*/
void llread(int fd, char * buffer){
  int verify = -1;
  int frame_length = readInformationFrame(fd, buffer, &verify);
  if(frame_length >= 0){
    //write(STDOUT_FILENO, buffer, frame_length);
    //fflush(stdout);
    char response[6] = "";
    int current_N = 0; //numero de serie por parte do recetor
      switch(verify){
        case ACK:
          buildRresponse(response, &current_N, ACK);
          destuffing(odd, buffer, &frame_length); //TODO: Adicionar um case ao switch DUP com buildrespmas semonse de ACK 
          break;
        case NACK:
          buildRresponse(response, &current_N, NACK);
          break;
        case DISCONNECT:
          printf("Case disconnect.\n");
          sprintf(response, "%c", (char) FLAG);
          sprintf(response + 1, "%c", (char) A_Recieve);
          sprintf(response + 2, "%c", (char) DISC);
          sprintf(response + 3, "%c", (char) (A_Recieve ^ DISC));
          sprintf(response + 4, "%c", (char) FLAG);
          break;
        default: 
          printf("Default case.\n");
          break;
      }
      write(fd, response, 6);
    }
  else if(frame_length == -1){
      char disc[6] = "";
      sprintf(disc, "%c", (char) FLAG);
      sprintf(disc + 1, "%c", (char) A_Recieve);
      sprintf(disc + 2, "%c", (char) DISC);
      sprintf(disc + 3, "%c", (char) (A_Recieve ^ DISC));
      sprintf(disc +4, "%c", (char) FLAG);
      write(fd, disc, 5);
      alarm(3);
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
        printf("Thing recieved isn't an UA. Exiting anyways, since the most likely option by far is errors in the UA sent\n");
      }
      exit(0);
  }
}


void buildRresponse(char* buffer, int *N_r, int success){
  sprintf((buffer + 0), "%c", FLAG);
  sprintf((buffer + 1), "%c", A_SEND);
  char control_byte;
  switch (success)
  {
  case ACK:
    *N_r = (*N_r + 1) % 2;
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
  sprintf((buffer + 2), "%c", (unsigned char)control_byte);
  sprintf((buffer + 3), "%c", (char) bcc);
  sprintf((buffer + 4), "%c", FLAG);
  sprintf((buffer + 5), "%c", '\0');
}


int readResponse(char* buffer){
  int success = -1;
  for(int i = 0; i < 6; i++){
    switch(i){
      case 0:
        if(buffer[i] != FLAG){
          printf("FLAG error!\n");
          return 0;
        }
        break;
      case 1:
        if(buffer[i] != A_SEND){
          printf("A_SEND error!\n");
          return 1;
        }
        break;
      case 2:
      {
        unsigned char control = (unsigned char) buffer[i];
        if(control == 0x85 || control == 0x05){
          success = ACK;
        }
        else if (control == 0x81 || control == 0x01){
          success = NACK;
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
      printf("llclose\n");
  //TO IMPLEMENT

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
    char response[6];
    printf("Reading\n");
    int res = read(fd, response, 5);
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
        return 6;
      }
    }
  }
  return -1;
}
