CC=gcc
CFLAGS=-c -g -Wall -I$(MF_HOME)/common -I$(MF_HOME)/netapi/c/ -DLINUX
LDFLAGS=-lmfapi -lpthread
SOURCES=sender.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sender

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS) 

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
