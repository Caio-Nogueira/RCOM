/*Non-Canonical Input Processing*/


#include "ll.h"

#define MODEMDEVICE "/dev/ttyS11"



//char str[255]; //SET array
//char stri[255]; //UA array
//char UA[255]; //UA array
int UA_read = FALSE;

int res;

//int flag_rewrite_SET = TRUE; //In the first input loop, dictates wether SET should be rewritten 


int main(int argc, char** argv)
{
    int c, fd;

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

    llopen(fd, TRANSMITTER);

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


    */
    close(fd);
    return 0;
}
