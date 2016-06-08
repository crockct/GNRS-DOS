#General variables
#DESDIRTEMP=/root/mfclient-0.1/debian/mfclient
BIN_DST_DIR = $(DESTDIR)/usr/bin/
#BASE_FOLDER = $(PWD)
VERSION = 0.1
#BASE_FOLDER = /home/wontoniii/Francesco/Rutgers/mobilityfirst/mobilityfirst/mfapps
BASE_FOLDER = $(PWD)
CPP = g++
CC = gcc
LFLAGS=-lrt
CFLAGS = -c -O2 -Wall 
MF_COMMON_INCLUDE = -I$(BASE_FOLDER)/../common/

#mfping variables
MFPING_LDFLAGS=-lmfapi -lpthread -lm
MFPING_FOLDER = $(BASE_FOLDER)/mfping/c
MFPING_SOURCES=$(MFPING_FOLDER)/ping.c
MFPING_OBJECTS=$(MFPING_SOURCES:.c=.o)
MFPING_PROGRAM=$(MFPING_FOLDER)/mfping

#mfping variables
MFRECEIVER_LDFLAGS=-lmfapi 
MFRECEIVER_FOLDER = $(BASE_FOLDER)/examples/receiver
MFRECEIVER_SOURCES=$(MFRECEIVER_FOLDER)/receiver.c
MFRECEIVER_OBJECTS=$(MFRECEIVER_SOURCES:.c=.o)
MFRECEIVER_PROGRAM=$(MFRECEIVER_FOLDER)/mfreceiver

#mfping variables
MFSENDER_LDFLAGS=-lmfapi
MFSENDER_FOLDER = $(BASE_FOLDER)/examples/sender
MFSENDER_SOURCES=$(MFSENDER_FOLDER)/sender.c
MFSENDER_OBJECTS=$(MFSENDER_SOURCES:.c=.o)
MFSENDER_PROGRAM=$(MFSENDER_FOLDER)/mfsender

all: build

build: $(MFPING_PROGRAM) $(MFRECEIVER_PROGRAM) $(MFSENDER_PROGRAM)

.PHONY: install_mfping clean_mfping install_mfsender install_mfreceiver clean_mfsender clean_mfreceiver

install: install_mfping install_mfsender install_mfreceiver 

clean: clean_mfping clean_mfsender clean_mfreceiver

#Stack compilation
$(MFPING_PROGRAM): $(MFPING_OBJECTS) 
	$(CC) $(MFPING_OBJECTS) -o $@ $(MFPING_LDFLAGS)

$(MFPING_OBJECTS): $(MFPING_SOURCES)
	$(CC) $(CFLAGS) $(MF_COMMON_INCLUDE) $(MFPING_CFLAGS) $< -o $@

install_mfping: $(MFPING_PROGRAM)
	install -d  $(DESTDIR)/usr $(DESTDIR)/usr/bin 
	install -m 755 $(MFPING_PROGRAM) $(DESTDIR)/usr/bin

#Clean stack
clean_mfping: 
	rm -rf $(MFPING_PROGRAM) $(MFPING_OBJECTS)


#Stack compilation
$(MFRECEIVER_PROGRAM): $(MFRECEIVER_OBJECTS)
	$(CC) $(MFRECEIVER_OBJECTS) -o $@ $(MFRECEIVER_LDFLAGS)

$(MFRECEIVER_OBJECTS): $(MFRECEIVER_SOURCES)
	$(CC) $(CFLAGS) $(MF_COMMON_INCLUDE) $(MFRECEIVER_CFLAGS) $< -o $@

install_mfreceiver: $(MFRECEIVER_PROGRAM)
	install -d  $(DESTDIR)/usr $(DESTDIR)/usr/bin 
	install -m 755 $(MFRECEIVER_PROGRAM) $(DESTDIR)/usr/bin

#Clean stack
clean_mfreceiver:
	rm -rf $(MFRECEIVER_PROGRAM) $(MFRECEIVER_OBJECTS)

#Stack compilation
$(MFSENDER_PROGRAM): $(MFSENDER_OBJECTS)
	$(CC) $(MFSENDER_OBJECTS) -o $@ $(MFSENDER_LDFLAGS)

$(MFSENDER_OBJECTS): $(MFSENDER_SOURCES)
	$(CC) $(CFLAGS) $(MF_COMMON_INCLUDE) $(MFSENDER_CFLAGS) $< -o $@

install_mfsender: $(MFSENDER_PROGRAM)
	install -d  $(DESTDIR)/usr $(DESTDIR)/usr/bin 
	install -m 755 $(MFSENDER_PROGRAM) $(DESTDIR)/usr/bin

#Clean stack
clean_mfsender:
	rm -rf $(MFSENDER_PROGRAM) $(MFSENDER_OBJECTS)
