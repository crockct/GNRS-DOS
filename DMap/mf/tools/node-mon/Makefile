PROGRAM = mf_node_mon

BINDIR = /usr/local/bin/

OBJS = node_mon.o node_stats.o 
CC = g++
INCLUDEDIRS = 
#CFLAGS = -O0 -Wall -c $(INCLUDEDIRS) -DDEBUG=1 -g 
CFLAGS = -O2 -Wall -c $(INCLUDEDIRS)
LIBS = -loml2 -locomm #-lpopt
LDFLAGS = -Wall  $(LIBS)

all: build
build: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

node_mon.o: node_mon.cpp node_stats.h config.h
	$(CC) $(CFLAGS) node_mon.cpp

node_stats.o: node_stats.cpp node_stats.h
	$(CC) $(CFLAGS) node_stats.cpp

install: $(PROGRAM)
	install -m 755 $(PROGRAM) $(BINDIR)

clean:
	rm -f a.out *.o *~ mf_node_mon
