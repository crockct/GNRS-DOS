#General variables
#General variables
#DESDIRTEMP=/root/mfclient-0.1/debian/mfclient
BIN_DST_DIR = $(DESTDIR)/usr/bin/
#BASE_FOLDER = $(PWD)
#BASE_FOLDER = /home/wontoniii/Francesco/Rutgers/mobilityfirst/mobilityfirst
BASE_FOLDER = $(PWD)
CPP = g++
CC = gcc
LFLAGS=-lrt
CFLAGS = -O2 -Wall 
MF_COMMON_INCLUDE = -I$(BASE_FOLDER)/../common/
#Make this x64 compatible
JAVA_HOME = /usr/lib/jvm/java-1.7.0-openjdk-i386
JAVAFLAGS=-I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux/

#JAVA gnrs variables
GNRS_FOLDER = $(BASE_FOLDER)/jserver
GNRS_JAR = $(GNRS_FOLDER)/target/gnrs-server-1.0.0-SNAPSHOT-jar-with-dependencies.jar

all: build

build: $(GNRS_JAR)

.PHONY: clean_gnrs install_gnrs 

install: install_gnrs 

clean: clean_gnrs 

#Compile GNRS
$(GNRS_JAR):
	cd $(GNRS_FOLDER); \
        mvn clean package -Dfile=lib/org.ardverk.patricia-trie_0.6.jar -DgroupId=org.ardverk -DartifactId=patricia-trie -Dversion=0.6 -Dpackaging=jar -DskipTests=true; \
        cd $(BASE_FOLDER);

#Review where to install jar libs
install_gnrs: $(GNRS_JAR)
	install -d $(DESTDIR)/usr $(DESTDIR)/usr/lib $(DESTDIR)/usr/lib/mfjava
	install -m 755 $(GNRS_JAR) $(DESTDIR)/usr/lib/mfjava/

clean_gnrs:
	cd $(GNRS_FOLDER); \
        mvn clean; \
        cd $(BASE_FOLDER);

