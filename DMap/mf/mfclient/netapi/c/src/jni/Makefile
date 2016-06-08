#This makefile allows the generation of the libmfapi.so library that includes also the jni interface
CC=gcc
JAVAFLAGS=-I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux/
LOCALFLAGS=-I$(MF_HOME)/common/
CFLAGS=-c -Wall -fPIC -DLINUX
LDFLAGS=-shared -Wl,-soname,libmfapi_jni.so -lmfapi
CSOURCES=edu_rutgers_winlab_jmfapi_JMFAPI.c
OBJECTS=edu_rutgers_winlab_jmfapi_JMFAPI.o
SHAREDLIB=libmfapi_jni.so
SHAREDLIBDIR=$(DESTDIR)/usr/lib/

all: check-env $(SHAREDLIB)

$(SHAREDLIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(SHAREDLIB) $(OBJECTS)

$(OBJECTS): $(CSOURCES) edu_rutgers_winlab_jmfapi_JMFAPI.h
	$(CC) $(JAVAFLAGS) $(LOCALFLAGS) $(CFLAGS) $(CSOURCES)

check-env:
ifndef JAVA_HOME
	$(error JAVA_HOME is undefined)
else
	$(info JAVA_HOME is $(JAVA_HOME))
endif
ifndef MF_HOME
	$(error MF_HOME is undefined)
endif

install: $(SHAREDLIB)
	install -m 755 $(SHAREDLIB) $(SHAREDLIBDIR)

clean:
	rm -rf *.o $(SHAREDLIB) 
