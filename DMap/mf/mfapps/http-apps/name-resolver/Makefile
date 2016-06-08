CC = g++
LFLAGS=-lrt
CFLAGS = -c -g -Wall

all: parser.o nameresolver.o

test: test.o nameresolver.o parser.o
	$(CC) $(LFLAGS) -o test test.cpp nameresolver.o parser.o

nameresolver.o: parser.o nameresolver.cpp
	$(CC) $(CFLAGS) nameresolver.cpp
	
parser.o: parser.cpp
	$(CC) $(CFLAGS) parser.cpp

clean:
	rm *.o 
