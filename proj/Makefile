CC = gcc
CFLAGS = -Wall
DEPS = protocol.h
OBJ = write.o read.o
TARGETS = noncanonicaltrama writenoncanonicaltrama protocol

all: U2 Q2

%.o: %.c $(DEPS)
	@$(CC) $(CFLAGS) -c -o $@ $<
	@echo $@

U2: $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm
	@echo $@

Q2: $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $@.c $(OBJ) -lm
	@echo $@

clean:
	@rm *.o $(TARGETS)
