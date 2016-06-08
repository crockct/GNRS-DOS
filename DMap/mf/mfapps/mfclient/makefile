CC=gcc
CFLAGS=-c -Wall -I$(MF_HOME)/common -I$(MF_HOME)/netapi/c/ -DLINUX
LDFLAGS=-lmfapi -lpthread
SOURCES=mfclient.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mfclient

all: $(SOURCES) $(EXECUTABLE)
        
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *.o $(EXECUTABLE)
install:
	cp $(EXECUTABLE) /usr/local/bin/
