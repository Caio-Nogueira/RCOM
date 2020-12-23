# Compiler and linker
CC = gcc

# Flags
CFLAGS = -Wall -g

main: 
	@gcc ${CFLAGS} -o download clientTCP.c utils.c parsers.c -lm

clean:	
	@rm -f download

download: clean main
	@./download "ftp://ftp.up.pt/pub/CPAN/RECENT-6h.json"
