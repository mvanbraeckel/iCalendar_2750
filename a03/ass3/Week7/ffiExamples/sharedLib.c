#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//void printFunc(void)
void printFunc(void){
    printf("sharedLib.so: called printFunc(). Woot!\n\n");
}

int addTwo(int n){
    return n+2;
}

void putDesc(char* str){
    printf("sharedLib.so: putDesc() received the string %s\n\n", str);
}

char* getDesc(void){
    char str[] = "[[{\"givenName\":\"Al\",\"surname\":\"Foo\"},{\"givenName\":\"Bill\",\"surname\":\"Foo\"}],[{\"givenName\":\"Jill\",\"surname\":\"Foo\"},{\"givenName\":\"June\",\"surname\":\"Foo\"}]]";
    char* retStr = malloc(strlen(str)+1);
    strcpy(retStr, str);
    
    return retStr;
}