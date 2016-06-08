CC=g++
LDFLAGS=-lpthread -lmfapi -ldl
CFLAGS=-c -g -Wall -DNO_SSL -DNO_CGI
SERVER_OBJECTS=mf-video-server/task.o mf-video-server/worker.o mf-video-server/workerlist.o mf-video-server/videoserver.o mf-video-server/mfserver.o
PROXY_SOURCES=$(wildcard mfproxy/*.cpp) $(wildcard mfproxy/*.c)
PROXY_OBJECTS_T=$(PROXY_SOURCES:.cpp=.o)
PROXY_OBJECTS=$(PROXY_OBJECTS_T:.c=.o)
RES_OBJECTS=name-resolver/nameresolver.o name-resolver/parser.o

#all: mfproxy mfserver
all: mfproxy

mfproxy: $(PROXY_OBJECTS) $(RES_OBJECTS)
	$(CC) -o mfproxy/mfproxy $(PROXY_OBJECTS) $(RES_OBJECTS) $(LDFLAGS)

mfserver: resolver server
	$(CC) -o mf-video-server/mfserver $(SERVER_OBJECTS) $(RES_OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm mfproxy/mfproxy mfproxy/*.o mf-video-server/mfserver mf-video-server/*.o name-resolver/*.o
