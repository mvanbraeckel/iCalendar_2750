CC = gcc
CPPFLAGS=
CFLAGS = -Wall -std=c11 -g
LDFLAGS= -L.

all: StructListDemo
	
StructListDemo: StructListDemo.o liblist.so
	$(CC) $(CFLAGS) $(LDFLAGS) -o StructListDemo StructListDemo.o  -llist
	
StructListDemo.o: StructListDemo.c
	$(CC) $(CFLAGS) -c StructListDemo.c 
	
liblist.so: LinkedListAPI.o
	$(CC) -shared -o liblist.so LinkedListAPI.o

LinkedListAPI.o: LinkedListAPI.c LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic LinkedListAPI.c

clean:
	rm -rf StructListDemo *.o *.so
