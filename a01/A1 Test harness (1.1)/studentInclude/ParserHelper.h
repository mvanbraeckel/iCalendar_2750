/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A1
 * @version 28/01/2019
 * @file ParserHelper.h
 * @brief Header file for ParserHelper.c
 */

#ifndef PARSERHELPER_H
#define PARSERHELPER_H

// ============================== INCLUDES ==============================
#include <ctype.h>

//#include "LinkedListAPI.h"
#include "CalendarParser.h"

// ============================== DEFINES ===============================
#define HTAB                     9
#define LF                      10
#define CR                      13
#define DQUOTE                  22
#define SPACE                   32
#define PLUS_SIGN               43
#define COMMA                   44
#define HYPHEN_MINUS            45
#define PERIOD                  46
#define SOLIDUS                 47
#define COLON                   58
#define SEMICOLON               59
#define LATIN_CAPITAL_LETTER_N  78
#define LATIN_CAPITAL_LETTER_T  84 //same as regular 'T'
#define LATIN_CAPITAL_LETTER_X  88
#define LATIN_CAPITAL_LETTER_Z  90
#define BACKSLASH               92
#define LATIN_SMALL_LETTER_N   110

// ======================================================================

void freeStringArray(char** lines, int numLines);
void freeParserTemp(Event* ev, Alarm* alm, char** lines, int numLines, Calendar *myCal, bool currEventAdded, bool currAlarmAdded);

ICalErrorCode parseDT(bool duplicate, char* input, DateTime *dt);
bool isValidDT(DateTime *dt);

Event* createEvent();
Alarm* createAlarm();
Property* createProperty(char* line, int delimIndex);

int strcmpic(const char *p1, const char *p2);
int strncmpic(const char *s1, const char *s2, size_t n);

void writePropertyList(FILE *fp, List* propList);
void writeEventList(FILE *fp, List* evList);
void writeAlarmList(FILE *fp, List* almList);

ICalErrorCode validateICal(const Calendar *obj);
ICalErrorCode validateEvent(const Event *ev);
ICalErrorCode validateAlarm(const Alarm *alm);
bool validateDT(const DateTime dt);

bool searchPropertyList(const void* first, const void* second);

int ishex(char c);

#endif