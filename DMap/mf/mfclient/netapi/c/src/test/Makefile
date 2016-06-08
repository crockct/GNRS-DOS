CC = g++
LFLAGS=-lmfapi  -lpthread
CFLAGS = -g -Wall -I$(MF_HOME)/common/ -I../
#LIBS = -lpthread

SRCS= testapi.c

OBJS = $(SRCS:%.c=%.o)

all: check-env testapi

check-env:
ifndef MF_HOME
	$(info MF_HOME is undefined)
	$(error MF_HOME has to be the folder where the common header files are)
else
	$(info MF_HOME is $(MF_HOME))
endif

testapi: $(OBJS)
	$(info 'Generating executable')
	$(CC) -o $@ $(OBJS) $(LFLAGS)

%.o: %.cpp
	$(info "Generating .o files")
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r *.o testapi
