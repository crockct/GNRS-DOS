CC = g++
LFLAGS=-lrt
CFLAGS = -g -Wall -I$(MF_HOME)/common/ -I../
LIBS = -lpthread

SRCS= ../mftimeoutevent.cpp \
		../mftimeoutmanager.cpp \
		testtimeout.cpp

OBJS = $(SRCS:%.cpp=%.o)

all: check-env testtimeout

check-env:
ifndef MF_HOME
	$(info MF_HOME is undefined)
	$(error MF_HOME has to be the folder where the common header files are)
else
	$(info MF_HOME is $(MF_HOME))
endif

testtimeout: $(OBJS)
	$(info 'Generating executable')
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(info "Generating .o files")
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r *.o ../*.o testtimeout
