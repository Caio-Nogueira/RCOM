# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o download clientTCP.c utils.c -lm

clean:	
	@rm -f download

download: clean main
	@./download "ftp://anonymous:xd@ftp.up.pt/debian/README.mirrors.txt"
