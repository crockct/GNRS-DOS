CC=gcc
CFLAGS=-c -Wall -I$(MF_HOME)/common -I$(MF_HOME)/netapi/c/  -DLINUX
LDFLAGS=-lmfapi -lpthread
SOURCES=mfftp.c mfftpcmd.c mfftpreceiving.c mfftptypes.c mfftpoptmode.c mfftputils.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mfftp
	
all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE)
