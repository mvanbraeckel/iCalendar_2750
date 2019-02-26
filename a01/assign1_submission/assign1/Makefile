# @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
# @course CIS*2750: Software Systems Integration and Development - A1
# @version 05/02/2018
# @file Makefile
# @brief compiles everything for assign1

CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -g -Wall -std=c11
LDFLAGS = -L~/bin

all: list parser

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/LinkedListAPI.c -o bin/liblist.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/liblist.o -o bin/liblist.so -shared

parser: list src/CalendarParser.c include/CalendarParser.h src/ParserHelper.c include/ParserHelper.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/CalendarParser.c -o bin/libcal.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/ParserHelper.c -o bin/libphelp.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/libcal.o bin/libphelp.o -o bin/libcal.so -shared

clean:
	rm -rf all list parser *.o *.so */*.o */*.so core