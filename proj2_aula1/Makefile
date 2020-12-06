# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o download clientTCP.c utils.c -lm

clean:	
	@rm -f download

download: clean main
	@./download "ftp://rcom:rcom@netlab1.fe.up.pt/pipe.txt"
