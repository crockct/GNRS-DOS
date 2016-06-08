#General variables
#DESDIRTEMP=/root/mfclient-0.1/debian/mfclient
BIN_DST_DIR = $(DESTDIR)/usr/bin/
#BASE_FOLDER = $(PWD)
VERSION = 0.1
#BASE_FOLDER = /home/wontoniii/Francesco/Rutgers/mobilityfirst/mobilityfirst
BASE_FOLDER = $(PWD)
MF_FOLDER = $(BASE_FOLDER)/../
CPP = g++
CC = gcc
LFLAGS=-lrt -lpthread
CFLAGS = -O2 -Wall 
MF_COMMON_INCLUDE = -I$(BASE_FOLDER)/../common/
#Basic way to find where the java folder is. Might need to be improved
#JAVA_HOME = /usr/lib/jvm/java-1.7.0-openjdk-i386
JAVA_HOME = $(shell find /usr/lib/jvm/ -name 'java-1.7.0-openjdk-*')
ifndef JAVA_HOME
	JAVA_HOME = $(shell find /usr/local/java/ -name 'jdk1.7.*')
endif
JAVAFLAGS=-I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux/

#Stack variables
STACK_CFLAGS = -D__OML__
STACK_LIBS = -lpcap -lpthread -lcrypto -loml2 -locomm -lpopt
STACK_FOLDER = $(BASE_FOLDER)/hoststack/src
STACK_OML_FOLDER = $(STACK_FOLDER)/oml
STACK_SRCS_LIST = $(wildcard $(STACK_FOLDER)/*.cpp)
		
STACK_OML_SRCS_LIST = $(wildcard $(STACK_OML_FOLDER)/*.cpp)

STACK_SRCS_ALL = $(STACK_SRCS_LIST) $(STACK_OML_SRCS_LIST)

STACK_OML_HDRS = $(wildcard $(STACK_OML_FOLDER)/*.h)

STACK_PROGRAM = $(STACK_FOLDER)/mfstack
STACK_DST_CONFDIR = $(DESTDIR)/etc/mfstack/

#OML variables
SCAFFOLD = /usr/bin/oml2-scaffold

STACK_OBJS = $(STACK_SRCS_ALL:%.cpp=%.o)
STACK_OML_OBJS = $(STACK_OML_SRCS_ALL:%.cpp=%.o)

#Libmfapi variables
NETAPI_CFLAGS = -c -Wall -fPIC -DLINUX
NETAPI_NAME = libmfapi.so
NETAPI_JNI_NAME = libmfapi-jni.so
NETAPI_VERSION = 0
NETAPI_SUBVERSION = 1.1
NETAPI_LDFLAGS = -shared -Wl,-soname,$(NETAPI_NAME)
NETAPI_LIBS = -lpthread
NETAPI_FOLDER = $(BASE_FOLDER)/netapi/c/src
NETAPI_JNI_FOLDER = $(NETAPI_FOLDER)/jni
NETAPI_SOURCES = $(NETAPI_FOLDER)/mfapi.c $(NETAPI_FOLDER)/log.c
NETAPI_JNI_SOURCES = $(NETAPI_JNI_FOLDER)/edu_rutgers_winlab_jmfapi_JMFAPI.c
NETAPI_OBJECTS = $(NETAPI_FOLDER)/mfapi.o $(NETAPI_FOLDER)/log.o
NETAPI_JNI_OBJECTS = $(NETAPI_JNI_FOLDER)/edu_rutgers_winlab_jmfapi_JMFAPI.o
NETAPI_SHAREDLIB = $(NETAPI_NAME).$(NETAPI_VERSION).$(NETAPI_SUBVERSION)
NETAPI_JNI_SHAREDLIB = $(NETAPI_JNI_NAME).$(NETAPI_VERSION).$(NETAPI_SUBVERSION)
DST_NETAPI_LIB = $(NETAPI_FOLDER)/$(NETAPI_SHAREDLIB)
DST_NETAPI_JNI_LIB = $(NETAPI_JNI_FOLDER)/$(NETAPI_JNI_SHAREDLIB)
NETAPI_HEADERS = $(NETAPI_FOLDER)/mfapi.h $(MF_FOLDER)/common/include/mfflags.h 

#JAVA netapi variables
NETAPI_JAVA_FOLDER = $(BASE_FOLDER)/netapi/java
MAVEN_BIN = /usr/bin/mvn
NETAPI_JAVA_JAR = $(NETAPI_JAVA_FOLDER)/target/jmfapi-1.0-SNAPSHOT.jar
NETAPI_JAVA_SRCS_FOLDER = $(NETAPI_JAVA_FOLDER)/src/main/java/edu/rutgers/winlab/jmfapi/

#Libgnrs variables
GNRS_API_NAME=libgnrs.so
GNRS_API_JNI_NAME=libgnrs-jni.so
GNRS_API_VERSION=0
GNRS_API_SONAME=$(GNRS_API_NAME)
GNRS_API_SUBVERSION=1.1
GNRS_API_FOLDER=$(BASE_FOLDER)/gnrsapi/cpp
GNRS_API_JNI_FOLDER=$(BASE_FOLDER)/gnrsapi/jni
GNRS_API_DST=$(GNRS_API_FOLDER)/$(GNRS_API_NAME).$(GNRS_API_VERSION).$(GNRS_API_SUBVERSION)
GNRS_API_JNI_DST=$(GNRS_API_JNI_FOLDER)/$(GNRS_API_JNI_NAME).$(GNRS_API_VERSION).$(GNRS_API_SUBVERSION)
GNRS_API_SOURCES=$(wildcard $(GNRS_API_FOLDER)/*.cpp)
GNRS_API_OBJECTS=$(GNRS_API_SOURCES:.cpp=.o)
GNRS_API_JNI_SOURCES=$(wildcard $(GNRS_API_JNI_FOLDER)/*.cpp)
GNRS_API_JNI_OBJECTS=$(GNRS_API_JNI_SOURCES:.cpp=.o)
GNRS_API_HEADERS=$(GNRS_API_FOLDER)/gnrs_cxx.h $(GNRS_API_FOLDER)/guid.h $(GNRS_API_FOLDER)/net_addr.h
GNRS_API_CFLAGS=-fPIC -Wall -c -g
GNRS_API_LFLAGS=-Wall

#JAVA gnrs variables
GNRS_JAVA_FOLDER = $(BASE_FOLDER)/gnrsapi/java
GNRS_JAVA_JAR = $(GNRS_JAVA_FOLDER)/target/jgnrs-1.0-SNAPSHOT.jar

all: build

build: $(STACK_PROGRAM) $(DST_NETAPI_LIB) $(DST_NETAPI_JNI_LIB) $(NETAPI_JAVA_JAR) $(GNRS_JAVA_JAR)  $(GNRS_API_DST) $(GNRS_API_JNI_DST)

.PHONY: install_stack install_netapi clean_stack clean_netapi clean_netapi_java install_netapi_java install_gnrs_java clean_gnrs_java install_gnrsapi clean_gnrsapi

install: install_stack install_netapi install_netapi_java install_gnrs_java install_gnrsapi

clean: clean_stack clean_netapi clean_netapi_java clean_gnrs_java  clean_gnrsapi

#Stack compilation
$(STACK_PROGRAM): $(STACK_OML_FOLDER)/mfstack_oml.h $(STACK_OML_FOLDER)/mfstack_popt.h $(STACK_OBJS)
	$(info "Generating executable")
	$(CPP) -o $@ $^ $(LDFLAGS) $(STACK_LIBS)

$(STACK_OML_FOLDER)/mfstack_popt.h: $(STACK_OML_FOLDER)/mfandroidstack.rb
	$(info "Generating OML headers")
	cd $(STACK_OML_FOLDER); \
	$(SCAFFOLD) --opts $<; \
	cd $(BASE_FOLDER);

$(STACK_OML_FOLDER)/mfstack_oml.h: $(STACK_OML_FOLDER)/mfandroidstack.rb
	$(info "Generating OML headers")
	cd $(STACK_OML_FOLDER); \
	$(SCAFFOLD) --oml $<; \
	cd $(BASE_FOLDER);

$(STACK_FOLDER)/%.o: $(STACK_FOLDER)/%.cpp
	$(info "Generating .o files")
	$(CPP) $(CFLAGS) $(STACK_CFLAGS) $(MF_COMMON_INCLUDE) -c $< -o $@

install_stack: $(STACK_PROGRAM)
	install -d $(BIN_DST_DIR) $(DESTDIR)/usr $(DESTDIR)/etc $(STACK_DST_CONFDIR)
	install -m 755 $(STACK_PROGRAM) $(BIN_DST_DIR)
	install -m 644 hoststack/conf/* $(STACK_DST_CONFDIR)

#Clean stack
clean_stack: 
	rm -f $(STACK_FOLDER)/*.o $(STACK_OML_FOLDER)/*.o $(STACK_PROGRAM) $(STACK_OML_FOLDER)/mfstack_oml.h $(STACK_OML_FOLDER)/mfstack_popt.h

#Compile libmfapi
$(DST_NETAPI_LIB): $(NETAPI_OBJECTS)
	$(CC) $(NETAPI_LDFLAGS) -o $(DST_NETAPI_LIB) $(NETAPI_OBJECTS) $(NETAPI_LIBS)

$(NETAPI_FOLDER)/%.o: $(NETAPI_FOLDER)/%.c
	$(CC) $(JAVAFLAGS) $(MF_COMMON_INCLUDE) $(NETAPI_CFLAGS) $< -o $@

$(DST_NETAPI_JNI_LIB): $(DST_NETAPI_LIB) $(NETAPI_JNI_OBJECTS)
	$(CC) $(NETAPI_LDFLAGS) -o $(DST_NETAPI_JNI_LIB) $(NETAPI_JNI_OBJECTS) $(NETAPI_LIBS)

$(NETAPI_JNI_FOLDER)/%.o: $(NETAPI_JNI_FOLDER)/%.c
	$(CC) $(JAVAFLAGS) $(MF_COMMON_INCLUDE) $(NETAPI_CFLAGS) $< -o $@

install_netapi: $(DST_NETAPI_LIB)
	install -d $(DESTDIR)/usr $(DESTDIR)/usr/lib $(DESTDIR)/usr/include $(DESTDIR)/usr/include/mobilityfirst
	install -m 755 $(DST_NETAPI_LIB) $(DESTDIR)/usr/lib/
	install -m 755 $(DST_NETAPI_JNI_LIB) $(DESTDIR)/usr/lib/
	install -m 644 $(NETAPI_HEADERS) $(DESTDIR)/usr/include/mobilityfirst

clean_netapi:
	rm -f $(NETAPI_FOLDER)/*.o $(NETAPI_JNI_FOLDER)/*.o $(DST_NETAPI_LIB) $(DST_NETAPI_JNI_LIB)

#Compile java mfapi
$(NETAPI_JAVA_JAR):
	cd $(NETAPI_JAVA_FOLDER); \
	mvn clean package; \
	cd $(BASE_FOLDER);

#Review where to install jar libs
install_netapi_java: $(NETAPI_JAVA_JAR)
	install -d $(DESTDIR)/usr $(DESTDIR)/usr/lib $(DESTDIR)/usr/lib/mfjava
	install -m 755 $(NETAPI_JAVA_JAR) $(DESTDIR)/usr/lib/mfjava/

clean_netapi_java:
	cd $(NETAPI_JAVA_FOLDER); \
	mvn clean; \
	cd $(BASE_FOLDER);


#Compile GNRS api
$(GNRS_API_DST): $(GNRS_API_OBJECTS)
	$(CPP) -shared -Wl,-soname,$(GNRS_API_SONAME) -o $(GNRS_API_DST) $^ $(GNRS_API_LFLAGS)

$(GNRS_API_FOLDER)/%.o: $(GNRS_API_FOLDER)/%.cpp
	$(CPP) $(JAVAFLAGS) $(MF_COMMON_INCLUDE) $(GNRS_API_CFLAGS) $< -o $@ 

$(GNRS_API_JNI_DST): $(GNRS_API_JNI_OBJECTS) $(GNRS_API_DST) 
	$(CPP) -shared -Wl,-soname,$(GNRS_API_JNI_SONAME) -o $(GNRS_API_JNI_DST) $^ $(GNRS_API_LFLAGS)

$(GNRS_API_JNI_FOLDER)/%.o: $(GNRS_API_JNI_FOLDER)/%.cpp
	$(CPP) $(JAVAFLAGS) $(MF_COMMON_INCLUDE) $(GNRS_API_CFLAGS) $< -o $@

install_gnrsapi: $(GNRS_API_DST)
	install -d $(DESTDIR)/usr $(DESTDIR)/usr/lib $(DESTDIR)/usr/include
	install -m 755 $(GNRS_API_DST) $(DESTDIR)/usr/lib/
	install -m 755 $(GNRS_API_JNI_DST) $(DESTDIR)/usr/lib/
	install -m 644 $(GNRS_API_HEADERS) $(DESTDIR)/usr/include/

clean_gnrsapi:
	rm -f $(GNRS_API_FOLDER)/*.o $(GNRS_API_JNI_FOLDER)/*.o $(GNRS_API_DST) $(GNRS_API_JNI_DST)

#Compile GNRS Java API
$(GNRS_JAVA_JAR):
	cd $(GNRS_JAVA_FOLDER); \
	mvn clean package; \
	cd $(BASE_FOLDER);

#Review where to install jar libs
install_gnrs_java: $(GNRS_JAVA_JAR)
	install -d $(DESTDIR)/usr $(DESTDIR)/lib $(DESTDIR)/usr/lib/mfjava
	install -m 755 $(GNRS_JAVA_JAR) $(DESTDIR)/usr/lib/mfjava/

clean_gnrs_java:
	cd $(GNRS_JAVA_FOLDER); \
	mvn clean; \
	cd $(BASE_FOLDER);

