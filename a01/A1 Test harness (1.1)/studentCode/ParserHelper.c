/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A1
 * @version 05/02/2019
 * @file ParserHelper.c
 * @brief Helper functions for CalendarParser.c
 */

// ============================== INCLUDES ==============================
#include "CalendarParser.h"
#include "ParserHelper.h"

// ========================= MY HELPER FUNCTIONS ========================
// ============================ FREE HELPERS ============================ 

/**
 * Frees a char** given the its number of lines
 * @param lines -the string array
 * @param numLines -the number of lines in the string array
 */
void freeStringArray(char** lines, int numLines) {
	for(int i = 0; i <  numLines; i++) {
		free(lines[i]);
	}
	free(lines);
}

/**
 * Frees all the temporary memory while creating a calendar and returning an error early
 * @param currEvent -the event currently being parsed (MAY be in a list)
 * @param currAlarm -the alarm currently being parsed (MAY be in a list)
 * @param lines -the string array being used for parsing
 * @param numLines -the number of lines of the string array passed
 * @param myCal -the calendar object being created
 * @param currEventAdded -false if the currEvent being parsed hasn't been added to an Event list yet
 * @param currAlarmAdded -false if the currAlarm being parsed hasn't been added to an Alarm list yet
 */
void freeParserTemp(Event* currEvent, Alarm* currAlarm, char** lines, int numLines, Calendar *myCal, bool currEventAdded, bool currAlarmAdded) {
	if(!currAlarmAdded && currAlarm != NULL) {
		deleteAlarm(currAlarm);
	}
	if(!currEventAdded && currEvent != NULL) {
		deleteEvent(currEvent);
	}
	freeStringArray(lines, numLines);
	deleteCalendar(myCal);
}

// ========================= DATE-TIME HELPERS ========================= 

/**
 * Parses a line for DateTime
 * @param duplicate -true if it has already been encountered in the current component
 * @param input -the current line being parsed
 * @param dt -pointer to the DateTime to be loaded into
 * @return INV_DT if an error occurs, otherwise OK
 */
ICalErrorCode parseDT(bool duplicate, char* input, DateTime *dt) {
	// check if duplicate, size is proper (15 or 16) (NOTE: accounts for empty string case), T is at index 8
	if(duplicate) {
		return INV_EVENT;
	} else if( (strlen(input) != 15 && strlen(input) != 16) || input[8] != 'T' ) {
		return INV_DT;
	}

	// get date and time (make sure all are integers)
	for(int j = 0; j < 15; j++) {
		if(!isdigit(input[j]) && j != 8) {
			return INV_DT;
		}
		// since index 8 is 'T'
		if(j < 8) {
			(*dt).date[j] = input[j];
		} else if(j > 8) {
			(*dt).time[j-9] = input[j];
		}
	}
	(*dt).date[8] = '\0';
	(*dt).time[6] = '\0';
	
	// check if UTC
	if(toupper(input[strlen(input)-1]) == 'Z') {
		(*dt).UTC = true;
	}

	// if YYYYMMDD or HHMMSS format is invalid, free everything and return INV_DT error
	bool validDT = isValidDT(dt);
	if(!validDT) {
		return INV_DT;
	} else { // dt is valid
		return OK;
	}
}

/**
 * Checks if a DateTime contains legal/valid values
 * @param -pointer to the DateTime being validated
 * @return true if it's valid
 */
bool isValidDT(DateTime *dt) {
	bool validDT = true;
	char part[5] = "0000\0";
	// check valid year, month, day
	// year
	strncpy(part, (*dt).date, 4);
	part[4] = '\0';
	// month
	strncpy(part, (*dt).date+4, 2);
	part[2] = '\0';
	int month = atoi(part);
	if(month == 0 || month > 12) {
		validDT = false;
	}
	// day (not 0 and doesn't exceed max day of its month)
	strncpy(part, (*dt).date+6, 2);
	part[2] = '\0';
	int day = atoi(part);
	if( day == 0 || (month == 2 && day > 29) || (((month < 8 && month % 2 == 1) || (month > 7 && month % 2 == 0)) && day > 31) ||
			(((month < 7 && month % 2 == 0) || (month > 6 && month % 2 == 0)) && day > 30) ) {
		validDT = false;
	}

	// check valid hours, minutes, seconds
	// hour
	strncpy(part, (*dt).time, 2);
	part[2] = '\0';
	if(atoi(part) > 23) {
		validDT = false;
	}
	// minute
	strncpy(part, (*dt).time+2, 2);
	part[2] = '\0';
	if(atoi(part) > 59) {
		validDT = false;
	}
	// second
	strncpy(part, (*dt).time+4, 2);
	part[2] = '\0';
	if(atoi(part) > 60) {
		validDT = false;
	} else if(atoi(part) == 60) { // set to 59 seconds if 60 seconds (not supported)
		(*dt).time[4] = '5';
		(*dt).time[5] = '9';
	}

	return validDT;
}

// ========================== CREATION HELPERS ==========================

/**
 * Creates an Event pointer, initializing all values
 * @return an allocated and fully initialized Event pointer
 */
Event* createEvent() {
    Event *ev = NULL;
	ev = malloc(sizeof(Event));
	strcpy(ev->UID, "");
	strcpy(ev->creationDateTime.date, "");
	strcpy(ev->creationDateTime.time, "");
	ev->creationDateTime.UTC = false;
	strcpy(ev->startDateTime.date, "");
	strcpy(ev->startDateTime.time, "");
	ev->startDateTime.UTC = false;
	ev->alarms = initializeList(*printAlarm, *deleteAlarm, *compareAlarms);
	ev->properties = initializeList(*printProperty, *deleteProperty, *compareProperties);
	return ev;
}

/**
 * Creates an Alarm pointer, initializing all values
 * @return an allocated and fully initialized Alarm pointer
 */
Alarm* createAlarm() {
	Alarm *alm = NULL;
	alm = malloc(sizeof(Alarm));
	strcpy(alm->action, "");
	alm->trigger = NULL;
	alm->properties = initializeList(*printProperty, *deleteProperty, *compareProperties);
	return alm;
}

/**
 * Parses and creates an Property pointer, initializing all values, when given its line and delimiter index
 * @param line -the line being parsed
 * @param delimIndex -the index of the delimiter in the passed string
 * @return an allocated and fully initialized Property pointer
 */
Property* createProperty(char* line, int delimIndex) {
	Property *prop = NULL;
	prop = malloc(sizeof(Property) + strlen(line+1+delimIndex)+1);
	strncpy(prop->propName, line, delimIndex);
	prop->propName[delimIndex] = '\0';
	strcpy(prop->propDescr, line+1+delimIndex);
	prop->propDescr[strlen(line+1+delimIndex)] = '\0';
	return prop;
}

// ==========================  BASIC FUNCTIONS ==========================

/**
 * Compares two strings, ignoring case
 * modified from https://code.woboq.org/userspace/glibc/string/strcmp.c.html#23
 */
int strcmpic(const char *p1, const char *p2) {
	const unsigned char *s1 = (const unsigned char *) p1;
	const unsigned char *s2 = (const unsigned char *) p2;
	unsigned char c1, c2;
	do {
		c1 = toupper((unsigned char) *s1++);
		c2 = toupper((unsigned char) *s2++);
		if (c1 == '\0')
        	return c1 - c2;
    } while (c1 == c2);
	return c1 - c2;
}

/**
 * Compares first n characters of two strings, ignoring case
 * modified from https://code.woboq.org/userspace/glibc/string/strncmp.c.html
 */
int strncmpic(const char *s1, const char *s2, size_t n) {
	unsigned char c1 = '\0';
	unsigned char c2 = '\0';
	if (n >= 4) {
		size_t n4 = n >> 2;
		do {
			c1 = toupper((unsigned char) *s1++);
			c2 = toupper((unsigned char) *s2++);
			if (c1 == '\0' || c1 != c2)
				return c1 - c2;

			c1 = toupper((unsigned char) *s1++);
			c2 = toupper((unsigned char) *s2++);
			if (c1 == '\0' || c1 != c2)
				return c1 - c2;

			c1 = toupper((unsigned char) *s1++);
			c2 = toupper((unsigned char) *s2++);
			if (c1 == '\0' || c1 != c2)
				return c1 - c2;

			c1 = toupper((unsigned char) *s1++);
			c2 = toupper((unsigned char) *s2++);
			if (c1 == '\0' || c1 != c2)
				return c1 - c2;
		} while (--n4 > 0);
		n &= 3;
	}
	while (n > 0) {
		c1 = toupper((unsigned char) *s1++);
		c2 = toupper((unsigned char) *s2++);
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		n--;
	}
	return c1 - c2;
}
