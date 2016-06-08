PROGRAM = mfstack

BINDIR = $(DESTDIR)/usr/bin/

CC = g++
LFLAGS=-lrt
CFLAGS = -O0 -g -Wall -I$(MF_HOME)/common/
#CFLAGS = -g -O2 -Wall -I$(MF_HOME)/common/
LIBS = -lpcap -lpthread -lcrypto

SCAFFOLD = /usr/bin/oml2-scaffold

SRCS= $(wildcard *.cpp)

#Compile using "make OML=1 to build in OML monitoring"
ifdef OML
	CFLAGS += -D__OML__
	LIBS += -loml2 -locomm -lpopt
	OML_HDRS = oml/mfstack_oml.h
	OML_SRCS = oml/mfomlmessage.cpp \
			oml/mfoml.cpp 
	SRCS += $(OML_SRCS)
endif

OBJS = $(SRCS:%.cpp=%.o)

all: check-env build

build: $(PROGRAM)

check-env:
ifndef MF_HOME
	$(info MF_HOME is undefined)
	$(error MF_HOME has to be the folder where the common header files are)
else
	$(info MF_HOME is $(MF_HOME))
endif

$(PROGRAM): $(OML_HDRS) $(OBJS)
	$(info "Generating executable")
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

%.o: %.cpp
	$(info "Generating .o files")
	$(CC) $(CFLAGS) -c $< -o $@

oml/mfstack_oml.h:
	$(MAKE) -C oml

install: $(PROGRAM)
	install -d $(BINDIR) $(DESTDIR)/usr
	install -m 755 $(PROGRAM) $(BINDIR)

clean: 
	rm -f *.o $(PROGRAM)

ifdef OML
	$(MAKE) -C oml clean
endif
