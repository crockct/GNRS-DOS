CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=bitrate.c, bitratetest.c, bitrateserver.c
OBJECTS=$(SOURCES:.c=.o)

all: bitratetest bitrateserver

bitratetest: bitrate.o bitratetest.o
	$(CC) $(LDFLAGS) bitrate.o bitratetest.o -o $@
bitrateserver: bitrate.o bitrateserver.o
	$(CC) $(LDFLAGS) bitrate.o bitrateserver.o -o $@
.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o bitratetest bitrateserver
install:
	cp bitratetest bitrateserver /usr/local/bin/
