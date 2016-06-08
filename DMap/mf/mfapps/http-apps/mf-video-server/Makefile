CC = g++
CFLAGS = -c -g -Wall

all: mfserver.o videoserver.o worker.o workerlist.o task.o

mfserver.o: mfserver.cpp
	$(CC) $(CFLAGS) mfserver.cpp

videoserver.o: videoserver.cpp
	$(CC) $(CFLAGS) videoserver.cpp
	
worker.o: worker.cpp
	$(CC) $(CFLAGS) worker.cpp
	
workerlist.o: workerlist.cpp
	$(CC) $(CFLAGS) workerlist.cpp
	
task.o: task.cpp
	$(CC) $(CFLAGS) task.cpp

clean:
	rm *.o
