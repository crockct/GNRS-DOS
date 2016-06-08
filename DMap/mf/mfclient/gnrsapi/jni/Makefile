SHAREDLIB=libgnrs_jni.so

SHAREDLIBDIR = /usr/lib
EXTOBJECTS=$(SHAREDLIBDIR)/libgnrs.so

CC=g++
JAVAFLAGS=-I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux/
LOCALFLAGS=-I.
CFLAGS=-c -Wall -fPIC -DLINUX
LDFLAGS=-shared -Wl,-soname,libgnrs_jni.so 
CSOURCES=edu_rutgers_winlab_jgnrs_JGNRS.cpp log.cpp
OBJECTS=edu_rutgers_winlab_jgnrs_JGNRS.o log.o 
HDRS=edu_rutgers_winlab_jgnrs_JGNRS.h log.h ../cpp/gnrs_cxx.h ../cpp/guid.h ../cpp/net_addr.h

all: build

build: check-env $(SHAREDLIB)

check-env:
ifndef JAVA_HOME
	$(error JAVA_HOME is undefined)
else
	$(info JAVA_HOME is $(JAVA_HOME))
endif

$(SHAREDLIB): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(SHAREDLIB) $(OBJECTS) $(EXTOBJECTS)

$(OBJECTS): $(CSOURCES) $(HDRS)
	$(CC) $(JAVAFLAGS) $(LOCALFLAGS) $(CFLAGS) $(CSOURCES)

install: $(SHAREDLIB)
	install -m 755 $(SHAREDLIB) $(SHAREDLIBDIR)

clean:
	rm -f *.o $(SHAREDLIB) 
