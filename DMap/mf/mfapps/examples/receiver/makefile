CC=gcc
CFLAGS=-c -g -Wall -I$(MF_HOME)/common -I$(MF_HOME)/netapi/c/ -DLINUX
LDFLAGS=-lmfapi -lpthread
SOURCES=receiver.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=receiver

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
