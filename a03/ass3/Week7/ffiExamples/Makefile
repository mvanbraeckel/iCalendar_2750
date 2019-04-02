CC=gcc
CFLAGS= -std=c11

#UNAME Shell Variable
UNAME_S := $(shell uname -s)

# OS X Dependant Variables
ifeq ($(UNAME_S), Darwin)
    LIB := sharedLib.dylib
# GNU/Linux Depedant Variables
else ifeq ($(UNAME_S), Linux)
    LIB := sharedLib.so
endif

$(LIB): sharedLib.o
	$(CC) -shared -o $(LIB) sharedLib.o

sharedLib.o: sharedLib.c
	$(CC) $(CFLAGS) -c -fpic sharedLib.c

clean:
	rm $(LIB) *.o