PROGRAMS = add update lookup robustness_test

CC = g++
INCLUDEDIRS = -I ../ -I$(MF_HOME)/common/
CFLAGS = -Wall -c $(INCLUDEDIRS)
LDFLAGS = -Wall
LIBDIRS = -L ../
LIBS = -lgnrs

all: build

build: $(PROGRAMS)

add: add_mappings.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(LIBDIRS)

update: update_mappings.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(LIBDIRS)

lookup: lookup.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(LIBDIRS)

robustness_test: robustness_test.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(LIBDIRS)

add_mappings.o: add_mappings.cpp
	$(CC) $(CFLAGS) add_mappings.cpp

update_mappings.o: update_mappings.cpp
	 $(CC) $(CFLAGS) update_mappings.cpp

lookup.o: lookup.cpp
	$(CC) $(CFLAGS) lookup.cpp

robustness_test.o: robustness_test.cpp
	$(CC) $(CFLAGS) robustness_test.cpp

clean:
	rm -f *.o *~ $(PROGRAMS)
