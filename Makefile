DEBUGFLAGS = -g -W -Wall
BUILDFLAGS = $(DEBUGFLAGS) -D_REENTRANT
CC = gcc

all: 3jsbin

3jsbin: 3jsbin.o
	$(CC) -g -o 3jsbin 3jsbin.o

3jsbin.o: 3jsbin.c
	$(CC) $(BUILDFLAGS) -c -o 3jsbin.o 3jsbin.c

clean:
	rm -f *.o
	rm -f 3jsbin
