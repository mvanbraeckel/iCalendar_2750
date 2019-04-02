# @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
# @course CIS*2750: Software Systems Integration and Development - A3
# @version 21/03/2019
# @file Makefile
# @brief compiles everything for assign3

# (compiler) (compiler flags) (-c, if compile to .o) (includes) (libraries) -o executableName
CC = gcc
CPPFLAGS = -Iinclude
CFLAGS = -g -Wall -std=c11 -pedantic
# -c goes on command line (compile to .o file / don't link)
LDFLAGS = -L~/bin
# these go in command line (-o executable -o caltest) ("-shared" for shared object lib)

#UNAME Shell Variable
#UNAME_S := $(shell uname -s)

# OS X Dependant Variables
#ifeq ($(UNAME_S), Darwin)
#    LIB_EXT := sharedLib.dylib
# GNU/Linux Depedant Variables
#else ifeq ($(UNAME_S), Linux)
#    LIB_EXT := sharedLib.so
#endif

all: list parser sharedLib tester

list: src/LinkedListAPI.c include/LinkedListAPI.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/LinkedListAPI.c -o bin/liblist.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/liblist.o -o bin/liblist.so -shared
#	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/liblist.so -o bin/liblist

parser: list src/CalendarParser.c include/CalendarParser.h src/ParserHelper.c include/ParserHelper.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/CalendarParser.c -o bin/libcal.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/ParserHelper.c -o bin/libphelp.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/libcal.o bin/libphelp.o -o bin/libcal.so -shared
#	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/libcal.so -o bin/libcal

sharedLib: list parser 
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/libcal.o bin/libphelp.o bin/liblist.o -o ../sharedLib.so -shared

tester: src/TestHarness.c list parser sharedLib
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -c src/TestHarness.c -o bin/tester.o -fpic
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/*.o -o bin/tester.so -shared
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) bin/*.so -o bin/tester

run:
	./bin/tester

val:
	valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./bin/tester

clean:
	rm -rf all list parser *.o *.so */*.o */*.so ../sharedLib.so