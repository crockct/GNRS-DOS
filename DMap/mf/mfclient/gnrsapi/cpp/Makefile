SHAREDLIB = libgnrs.so

SHAREDLIBDIR = /usr/lib/

OBJS = gnrs_cxx.o udpipv4_endpoint.o
CC = g++
DEBUG = #-DDEBUG=1 #-g 
INCLUDEDIRS = -I$(MF_HOME)/common/ 
CFLAGS = -fPIC -Wall -c $(INCLUDEDIRS) $(DEBUG)
LDFLAGS = -Wall $(DEBUG) 
LIBS = 

all: build

build: $(SHAREDLIB)

$(SHAREDLIB): $(OBJS)
	$(CC) -shared -Wl,-soname,$@ -o $@ $^ $(LIBS)

gnrs_cxx.o: gnrs_cxx.cpp gnrs_cxx.h net_addr.h udpipv4_endpoint.h exceptions.h common.h constants.h
	$(CC) $(CFLAGS) gnrs_cxx.cpp 

udpipv4_endpoint.o: udpipv4_endpoint.cpp net_addr.h udpipv4_endpoint.h exceptions.h common.h constants.h
	$(CC) $(CFLAGS) udpipv4_endpoint.cpp

install: $(SHAREDLIB)
	install -m 755 $(SHAREDLIB) $(SHAREDLIBDIR)

clean:
	rm -f *.o $(SHAREDLIB)
