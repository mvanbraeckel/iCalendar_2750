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

// =========================== WRITING HELPERS ===========================

/**
 * Writes a list of properties to the file
 * @param FILE *fp -the file to be written to
 * @param List *propList -the list of properties
 */
void writePropertyList(FILE *fp, List* propList) {
	ListIterator iter = createIterator(propList);
	Property* prop;
	while( (prop = nextElement(&iter)) != NULL ) {
		fprintf(fp, "%s:%s\r\n", prop->propName, prop->propDescr);
	}
}

/**
 * Writes a list of events to the file
 * @param FILE *fp -the file to be written to
 * @param List *evList -the list of events
 */
void writeEventList(FILE *fp, List* evList) {
	ListIterator iter = createIterator(evList);
	Event* ev;
	while( (ev = nextElement(&iter)) != NULL ) {
		fprintf(fp, "BEGIN:VEVENT\r\n");

		fprintf(fp, "UID:%s\r\n", ev->UID);
		// write DTSTAMP to file
		if(ev->creationDateTime.UTC) {
			fprintf(fp, "DTSTAMP:%sT%sZ\r\n", ev->creationDateTime.date, ev->creationDateTime.time);
		} else {
			fprintf(fp, "DTSTAMP:%sT%s\r\n", ev->creationDateTime.date, ev->creationDateTime.time);
		}
		// write DTSTART to file
		if(ev->startDateTime.UTC) {
			fprintf(fp, "DTSTART:%sT%sZ\r\n", ev->startDateTime.date, ev->startDateTime.time);
		} else {
			fprintf(fp, "DTSTART:%sT%s\r\n", ev->startDateTime.date, ev->startDateTime.time);
		}
		// write all event properties
		writePropertyList(fp, ev->properties);
		// write all event alarms
		writeAlarmList(fp, ev->alarms);

		fprintf(fp, "END:VEVENT\r\n");
	}
}

/**
 * Writes a list of alarms to the file
 * @param FILE *fp -the file to be written to
 * @param List *almList -the list of alarms
 */
void writeAlarmList(FILE *fp, List* almList) {
	ListIterator iter = createIterator(almList);
	Alarm* alm;
	while( (alm = nextElement(&iter)) != NULL ) {
		fprintf(fp, "BEGIN:VALARM\r\n");

		fprintf(fp, "ACTION:%s\r\n", alm->action);
		fprintf(fp, "TRIGGER:%s\r\n", alm->trigger);
		// write all alarm properties
		writePropertyList(fp, alm->properties);

		fprintf(fp, "END:VALARM\r\n");
	}
}

// ========================== VALIDATION HELPERS ==========================

/**
 * Checks that the iCal properties are all valid
 * @param Calendar *obj -the cal to be checked
 * @return OK if valid, otherwise the first error that occurs
 */
ICalErrorCode validateICal(const Calendar *obj) {
	// make sure it exists
	if(obj == NULL) {
		return INV_CAL;
	}
	// declare booleans
	bool CALSCALE = false;
	bool METHOD = false;

	// validate iCal (version, prodid, iCal prop list)
	if(!(obj->version > 0.0)) {
		return INV_CAL;
		//return INV_VER;
	}
	if(obj->prodID == NULL || strcmp(obj->prodID, "") == 0) {
		return INV_CAL;
		//return INV_PRODID;
	}
	// validate its lists (not null, events has at least one)
	if(obj->events == NULL || getLength(obj->events) == 0 || obj->properties == NULL) {
		return INV_CAL;
	}
	
	// validate the iCal prop list
	ListIterator calPropIter = createIterator(obj->properties);
	Property* prop;

	while( (prop = nextElement(&calPropIter)) != NULL ) {
		// check that property name and description have data
		if( prop->propName == NULL || strcmp(prop->propName, "") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr, "") == 0 ) {
			return INV_CAL;
		}
		// check for valid prop names (optional, cannot occur more than once)
		else if(!CALSCALE && strcmpic("CALSCALE", prop->propName) == 0) {
			CALSCALE = true;
		} else if(!METHOD && strcmpic("METHOD", prop->propName) == 0) {
			METHOD = true;
		} 
		// all other property names are invalid iCal properties (includes duplicate cases)
		else {
			return INV_CAL;
		}
	} // end while loop
	return OK;
}

/**
 * Checks that the event properties are all valid
 * @param Event *ev -the event to be checked
 * @return OK if valid, otherwise the first error that occurs
 */
ICalErrorCode validateEvent(const Event *ev) {
	// make sure it exists
	if(ev == NULL) {
		return INV_EVENT;
	}
	// delcare variables
	int numValidEvPropsOnce = 16;
	char validEvPropsOnceNames[16][15] = { "CLASS", "CREATED", "DESCRIPTION", "GEO", "LAST-MODIFIED", "LOCATION", "ORGANIZER", "PRIORITY",
			"SEQUENCE", "STATUS", "SUMMARY", "TRANSP", "URL", "RECURRENCE-ID", "DTEND", "DURATION" };
	bool validEvPropsOnceFound[numValidEvPropsOnce];
	for(int i = 0; i < numValidEvPropsOnce; i++) {
		validEvPropsOnceFound[i] = false;
	}
	int numValidEvPropsMany = 10;
	char validEvPropsManyNames[10][15] = { "ATTACH", "ATTENDEE", "CATEGORIES", "COMMENT", "CONTACT", "EXDATE", "RELATED-TO", "RESOURCES", "RDATE", "RRULE" };

	// validate event (uid, dtstamp, dtstart, ev prop list)
	if( ev->UID == NULL || strcmp(ev->UID, "") == 0 || !validateDT(ev->creationDateTime) || !validateDT(ev->startDateTime) ) {
		return INV_EVENT;
	}
	// validate its lists
	if(ev->alarms == NULL || ev->properties == NULL) {
		return INV_EVENT;
	}

	// validate ev prop list
	ListIterator evPropIter = createIterator(ev->properties);
	Property* prop;

	while( (prop = nextElement(&evPropIter)) != NULL ) {
		// check that property name and description have data
		if( prop->propName == NULL || strcmp(prop->propName, "") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr, "") == 0) {
			return INV_EVENT;
		}
		bool matchFound = false;

		// check for valid prop names (optional, cannot occur more than once)
		for(int i = 0; i < numValidEvPropsOnce; i++) {
			if(!validEvPropsOnceFound[i] && strcmpic(prop->propName, validEvPropsOnceNames[i]) == 0) {
				matchFound = true;
				validEvPropsOnceFound[i] = true;
			}
		}
		// check for valid prop names (optional, can occur multiple times)
		for(int i = 0; i < numValidEvPropsMany; i++) {
			if(strcmpic(prop->propName, validEvPropsManyNames[i]) == 0) {
				matchFound = true;
			}
		}
		// all other property names are invalid event properties (includes duplicate cases)
		if(!matchFound) {
			return INV_EVENT;
		}
	} // end while loop
	
	// check for DTEND+DURATION bad-pair (optional, only one can occur)
	if(validEvPropsOnceFound[numValidEvPropsOnce-2] && validEvPropsOnceFound[numValidEvPropsOnce-1]) {
		return INV_EVENT;
	}
	return OK;
}

/**
 * Checks that the alarm properties are all valid
 * @param Alarm *alm -the alarm to be checked
 * @return OK if valid, otherwise the first error that occurs
 */
ICalErrorCode validateAlarm(const Alarm *alm) {
	// make sure it exists
	if(alm == NULL) {
		return INV_ALARM;
	}
	// declare booleans
	bool ATTACH = false;
	bool DURATION = false;
	bool REPEAT = false;

	// validate alarm (action [must be audioprop rule], trigger, alm prop list)
	if(alm->action == NULL || strcmpic(alm->action, "AUDIO") != 0 || alm->trigger == NULL || strcmp(alm->trigger, "") == 0) {
		return INV_ALARM;
	}
	// validate its lists
	if(alm->properties == NULL) {
		return INV_ALARM;
	}
	
	// validate alm prop list
	ListIterator almPropIter = createIterator(alm->properties);
	Property* prop;

	while( (prop = nextElement(&almPropIter)) != NULL ) {
		// check that property name and description have data
		if( prop->propName == NULL || strcmp(prop->propName, "") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr, "") == 0 ) {
			return INV_ALARM;
		}
		// check for valid prop names (optional, cannot occur more than once)
		else if(!DURATION && strcmpic("DURATION", prop->propName) == 0) {
			DURATION = true;
		} else if(!REPEAT && strcmpic("REPEAT", prop->propName) == 0) {
			REPEAT = true;
		} else if(!ATTACH && strcmpic("ATTACH", prop->propName) == 0) {
			ATTACH = true;
		}
		// all other property names are invalid alarm properties (includes duplicate cases)
		else {
			return INV_ALARM;
		}
	} // end while loop

	// check for duration+repeat pair (optional, must occur together or not at all, cannot occur more than once)
	if(DURATION != REPEAT) {
		return INV_ALARM;
	}
	return OK;
}

/**
 * Checks that the date-time values are all valid
 * @param DateTime dt -the date-time to be checked
 * @return true if it's valid
 */
bool validateDT(const DateTime dt) {
	bool validDT = true;
	if( dt.date == NULL || strcmp(dt.date, "") == 0 || strlen(dt.date) != 8 ||
			dt.time == NULL || strcmp(dt.time, "") == 0 || strlen(dt.time) != 6 ) {
		return false;
	}

	char part[5] = "0000\0";
	// check valid year, month, day
	// year
	strncpy(part, dt.date, 4);
	part[4] = '\0';
	// month
	strncpy(part, dt.date+4, 2);
	part[2] = '\0';
	int month = atoi(part);
	if(month == 0 || month > 12) {
		validDT = false;
	}
	// day (not 0 and doesn't exceed max day of its month)
	strncpy(part, dt.date+6, 2);
	part[2] = '\0';
	int day = atoi(part);
	if( day == 0 || (month == 2 && day > 29) || (((month < 8 && month % 2 == 1) || (month > 7 && month % 2 == 0)) && day > 31) ||
			(((month < 7 && month % 2 == 0) || (month > 6 && month % 2 == 0)) && day > 30) ) {
		validDT = false;
	}

	// check valid hours, minutes, seconds
	// hour
	strncpy(part, dt.time, 2);
	part[2] = '\0';
	if(atoi(part) > 23) {
		validDT = false;
	}
	// minute
	strncpy(part, dt.time+2, 2);
	part[2] = '\0';
	if(atoi(part) > 59) {
		validDT = false;
	}
	// second
	strncpy(part, dt.time+4, 2);
	part[2] = '\0';
	if(atoi(part) > 60) {
		validDT = false;
	} /*else if(atoi(part) == 60) { // set to 59 seconds if 60 seconds (not supported) -can't do because can't change w/in validateCalendar()
		dt.time[4] = '5';
		dt.time[5] = '9';
	}*/

	return validDT;
}

// ========================= SEARCH LIST HELPERS =========================

/**
 * Used for findElement() -- custom compare that searches for matching property name in a prop list
 * @param first -current prop of the prop list
 * @param second -string to match to the prop name
 * @return true if it finds a match
 */
bool searchPropertyList(const void* first, const void* second) {
	// make sure they exist
	if (first == NULL || second == NULL) return false;
	
	// create temps
	Property *prop = (Property*)first;
	char *searchRecord = (char*)second;
	
	// make sure prop has valid data
	if(prop->propName == NULL || strcmp(prop->propName, "") == 0 || prop->propDescr == NULL || strcmp(prop->propDescr, "") == 0) {
		return false;
	}

	// check if equal
	if(strcmpic(prop->propName, searchRecord) == 0) {
		return true;
	} else {
		return false;
	}
}

// ========================= JSON PARSING HELPER =========================

/**
 * Checks if a char is a hex character [0-9a-f]
 * @param char c -the char to be checked
 * @return 1 if true, 0 if false
 */
int ishex(char c) {
	if( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ) {
		return 1;
	} else {
		return 0;
	}
}


/** Function to converting an Alarm into a JSON string
 *@pre Alarm is not NULL
 *@post Alarm has not been modified in any way
 *@return A string in JSON format
 *@param alarm - a pointer to an Alarm struct
 **/
char* alarmToJSON(const Alarm* alarm) {
	char* toReturn = NULL;
	// make sure it exists
	if(alarm == NULL) {
		toReturn = malloc(3);
		strcpy(toReturn, "{}");
		toReturn[2] = '\0';
		return toReturn;
	}
	
	// get action (should only be AUDIO though)
	char* actVal = NULL;							// need to free
	actVal = stringToJSON(alarm->action);
	
	// get trigger
	char* trigVal = NULL;							// need to free
	trigVal = stringToJSON(alarm->trigger);
	
	// get #of properties
	int propVal = 2;
	if(alarm->properties != NULL) {
		propVal += getLength(alarm->properties);
	}

	// create JSON string [39 + actVal len (max 200) + trigVal len + 20 = ~75 + (2 string len)]
	toReturn = malloc(75 + strlen(actVal) + strlen(trigVal));
	sprintf(toReturn, "{\"action\":\"%s\",\"trigger\":\"%s\",\"numProps\":%d}", actVal, trigVal, propVal);
	toReturn[strlen(toReturn)] = '\0';
	
	// free temps before returning
	free(actVal);
	free(trigVal);
	return toReturn;
	// "{\"action\":\"actVal\",\"trigger\":\"trigVal\",\"numProps\":propval}"
}

/** Function to converting an Alarm list into a JSON string
 *@pre Alarm list is not NULL
 *@post Alarm list has not been modified in any way
 *@return A string in JSON format
 *@param alarmList - a pointer to an Alarm list
 **/
char* alarmListToJSON(const List* alarmList) {
	char* toReturn = malloc(3);
	// make sure it exists
	if(alarmList == NULL) {
		strcpy(toReturn, "[]");
		toReturn[2] = '\0';
		return toReturn;
	}
	// start the output string
	toReturn[0] = '[';
	toReturn[1] = '\0';

	// loop through list, properly adding commas and reallocating when adding each new alarm JSON string
	ListIterator almIter = createIterator((List*)alarmList);
	Alarm* alm;
	int counter = 0;

	while( (alm = nextElement(&almIter)) != NULL ) {
		// get curr alarm as JSON string, reallocate mem
		char* almStr = alarmToJSON(alm);		// need to free
		toReturn = realloc(toReturn, strlen(toReturn)+strlen(almStr)+5);

		// appropriately add commas if necessary, then add curr alarm JSON string 
		if(counter != 0) {
			strcat(toReturn, ",");
		}
		strcat(toReturn, almStr);
		toReturn[strlen(toReturn)] = '\0';

		// free temp and iterate
		free(almStr);
		counter++;
	}

	// end return string, then return it
	int len = strlen(toReturn);
	toReturn[len] = ']';
	toReturn[len+1] = '\0';
	return toReturn;
	// "[AlmtString1,AlmtString2,…,AlmtStringN]"
}

/** Function to converting a Property into a JSON string
 *@pre Property is not NULL
 *@post Property has not been modified in any way
 *@return A string in JSON format
 *@param property - a pointer to a Property struct
 **/
char* propertyToJSON(const Property* property) {
	char* toReturn = NULL;
	// make sure it exists
	if(property == NULL) {
		toReturn = malloc(3);
		strcpy(toReturn, "{}");
		toReturn[2] = '\0';
		return toReturn;
	}
	
	// get property name
	char* nameVal = NULL;							// need to free
	nameVal = stringToJSON(property->propName);

	// get property description
	char* descrVal = NULL;							// need to free
	descrVal = stringToJSON(property->propDescr);

	// create JSON string [31 + nameVal len (max 200) + descrVal len = ~35 + (2 string len)]
	toReturn = malloc(35 + strlen(nameVal) + strlen(descrVal));
	sprintf(toReturn, "{\"propName\":\"%s\",\"propDescr\":\"%s\"}", nameVal, descrVal);
	toReturn[strlen(toReturn)] = '\0';
	
	// free temps before returning
	free(nameVal);
	free(descrVal);
	return toReturn;
	// "{\"propName\":\"nameVal\",\"propDescr\":\"descrVal\"}"
}

/** Function to converting a Property list into a JSON string
 *@pre Property list is not NULL
 *@post Property list has not been modified in any way
 *@return A string in JSON format
 *@param propertyList - a pointer to a Property list
 **/
char* propertyListToJSON(const List* propertyList) {
	char* toReturn = malloc(3);
	// make sure it exists
	if(propertyList == NULL) {
		strcpy(toReturn, "[]");
		toReturn[2] = '\0';
		return toReturn;
	}
	// start the output string
	toReturn[0] = '[';
	toReturn[1] = '\0';

	// loop through list, properly adding commas and reallocating when adding each new property JSON string
	ListIterator propIter = createIterator((List*)propertyList);
	Property* prop;
	int counter = 0;

	while( (prop = nextElement(&propIter)) != NULL ) {
		// get curr property as JSON string, reallocate mem
		char* propStr = propertyToJSON(prop);		// need to free
		toReturn = realloc(toReturn, strlen(toReturn)+strlen(propStr)+5);

		// appropriately add commas if necessary, then add curr property JSON string 
		if(counter != 0) {
			strcat(toReturn, ",");
		}
		strcat(toReturn, propStr);
		toReturn[strlen(toReturn)] = '\0';

		// free temp and iterate
		free(propStr);
		counter++;
	}

	// end return string, then return it
	int len = strlen(toReturn);
	toReturn[len] = ']';
	toReturn[len+1] = '\0';
	return toReturn;
	// "[PropString1,PropString2,…,PropStringN]"
}

/** Function to converting a string into a JSON-readied string (account for '\' and '"')
 *@pre str is not NULL
 *@post str has not been modified in any way
 *@return An allocated string readied for JSON format
 *@param str - a the string to be prepped and readied
 **/
char* stringToJSON(const char* str) {
	char* prepStr = NULL;
	if(str == NULL) {
		prepStr = malloc(1);
		prepStr[0] = '\0';
	} else {
		// count number of backslashes
		int len = strlen(str);
		int c = 0;
		for(int i = 0; i < len; i++) {
			if(str[i] == '\\' || str[i] == '\"') c++;
		}
		// create mem, add one char at a time to account for escaping chars
		prepStr = calloc(len+c+1, sizeof(char));
		int j = 0;
		for(int i = 0; i < len; i++) {
			if(str[i] == '\\' || str[i] == '\"') {
				prepStr[j++] = '\\';
			}
			prepStr[j++] = str[i];
		}
		prepStr[j] = '\0';
	}
	return prepStr;
}

// ======================= ADD EVENT TESTING HELPER =======================

/**
 * Clones an event
 * @param Event *toClone -the event to be cloned (copied)
 * @return a new allocated event that is a clone of the given one, or NULL if any errors (invalid event, NULL ptrs) are encountered
 */
Event* cloneEvent(Event *toClone) {
	// make sure it exists, and validate its event contents
	if(validateEvent(toClone) != OK) {
		return NULL;
	}

	// validate all of its alarms
	ListIterator almPropIter = createIterator(toClone->alarms);
	Alarm* alm;
	while( (alm = nextElement(&almPropIter)) != NULL ) {
		// check if the alarm is valid
		if(validateAlarm(alm) != OK) {
			return NULL;
		}
	}

	// since the event has been full validated,
	// create and init a new event, then copy all of its contents over
    Event *ev = createEvent();

	// uid
	strcpy(ev->UID, toClone->UID);
	ev->UID[strlen(toClone->UID)] = '\0';

	// dtstamp
	strcpy(ev->creationDateTime.date, toClone->creationDateTime.date);
	ev->creationDateTime.date[strlen(toClone->creationDateTime.date)] = '\0';
	strcpy(ev->creationDateTime.time, toClone->creationDateTime.time);
	ev->creationDateTime.time[strlen(toClone->creationDateTime.time)] = '\0';
	ev->creationDateTime.UTC = toClone->creationDateTime.UTC;

	// dtstart
	strcpy(ev->startDateTime.date, toClone->startDateTime.date);
	ev->startDateTime.date[strlen(toClone->startDateTime.date)] = '\0';
	strcpy(ev->startDateTime.time, toClone->startDateTime.time);
	ev->startDateTime.time[strlen(toClone->startDateTime.time)] = '\0';
	ev->startDateTime.UTC = toClone->startDateTime.UTC;

	// properties
	ListIterator copyPropIter = createIterator(toClone->properties);
	Property* prop;
	while( (prop = nextElement(&copyPropIter)) != NULL ) {
		// copy property contents into temp prop, then add to property list
		Property *tempProp = NULL;
		tempProp = malloc(sizeof(Property) + strlen(prop->propDescr)+1);
		// name
		strcpy(tempProp->propName, prop->propName);
		tempProp->propName[strlen(prop->propName)] = '\0';
		// description
		strcpy(tempProp->propDescr, prop->propDescr);
		tempProp->propDescr[strlen(prop->propDescr)] = '\0';
		// add to list
		insertSorted(ev->properties, tempProp);
		// do not free
	}

	// alarms
	ListIterator copyAlmIter = createIterator(toClone->alarms);
	while( (alm = nextElement(&copyAlmIter)) != NULL ) {
		// copy alarm contents into temp alarm, then add to alarm list
		Alarm *tempAlm = createAlarm();
		// action
		strcpy(tempAlm->action, alm->action);
		tempAlm->action[strlen(alm->action)] = '\0';
		// trigger (allocate mem first)
		tempAlm->trigger = malloc(strlen(alm->trigger)+1);
		strcpy(tempAlm->trigger, alm->trigger);
		tempAlm->trigger[strlen(alm->trigger)] = '\0';

		// properties
		ListIterator copyAlmPropIter = createIterator(alm->properties);
		Property* almProp;
		while( (almProp = nextElement(&copyAlmPropIter)) != NULL ) {
			// copy property contents into temp prop, then add to property list
			Property *tempAlmProp = NULL;
			tempAlmProp = malloc(sizeof(Property) + strlen(almProp->propDescr)+1);
			// name
			strcpy(tempAlmProp->propName, almProp->propName);
			tempAlmProp->propName[strlen(almProp->propName)] = '\0';
			// description
			strcpy(tempAlmProp->propDescr, almProp->propDescr);
			tempAlmProp->propDescr[strlen(almProp->propDescr)] = '\0';
			// add to list
			insertSorted(ev->properties, tempAlmProp);
			// do not free
		}

		// add to list
		insertSorted(ev->alarms, tempAlm);
		// do not free
	}

	return ev;
}

// ============================== A3 WRAPPERS =============================

/**
 * Extract summary info from an iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want
 * @return a stringified JSON representing the summary info of the iCal file
 */
char* a3getFileLogPanelTableInfo(char* dirFilename) {
	// declare variables
	Calendar *myCal = NULL;

	// try to create the calendar, then validate it, making sure each was successful
	ICalErrorCode err = createCalendar(dirFilename, &myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	} 
	err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	} 
	
	// it it's good, get the data first, then delete the iCal before leaving
	char* rowInfo = calendarToJSON(myCal);
	deleteCalendar(myCal);
	return rowInfo;
}

/**
 * Extract event list from an iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want
 * @return a stringified JSON representing the event list of the iCal file
 */
char* a3getEventList(char* dirFilename) {
	// declare variables
	Calendar *myCal = NULL;

	// try to create the calendar, then validate it, making sure each was successful
	ICalErrorCode err = createCalendar(dirFilename, &myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	} 
	err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	}
	
	// it it's good, get the data first, then delete the iCal before leaving
	char* evListJSONstr = eventListToJSON(myCal->events);
	deleteCalendar(myCal);
	return evListJSONstr;
}

/**
 * Extract property list from an event in an iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want
 * @return a stringified JSON representing the property list of the designated event in the iCal file
 */
char* a3getPropertyList(char *dirFilename, int eventNum) {
	// declare variables
	Calendar *myCal = NULL;

	// try to create the calendar, then validate it, making sure each was successful
	ICalErrorCode err = createCalendar(dirFilename, &myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	} 
	err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	}
	
	// it it's good, get the data first, then delete the iCal before leaving
	char* propListJSONstr = NULL;
	int i = 0;
	bool match = false;

	ListIterator evPropIter = createIterator(myCal->events);
	Event* ev;
	// must loop through to find the correct event first (based on number)
	while( (ev = nextElement(&evPropIter)) != NULL ) {
		if(i++ == eventNum) {
			match = true;
			propListJSONstr = propertyListToJSON(ev->properties);
			break;
		}
	}
	// check if a matching event was found (based on number)
	if(!match) {
		deleteCalendar(myCal);
		return NULL;
	}
	deleteCalendar(myCal);
	return propListJSONstr;
}

/**
 * Extract alarm list from an event in an iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want
 * @return a stringified JSON representing the alarm list of the designated event in the iCal file
 */
char* a3getAlarmList(char *dirFilename, int eventNum) {
	// declare variables
	Calendar *myCal = NULL;

	// try to create the calendar, then validate it, making sure each was successful
	ICalErrorCode err = createCalendar(dirFilename, &myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	} 
	err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		return NULL;
	}
	
	// it it's good, get the data first, then delete the iCal before leaving
	char* almListJSONstr = NULL;
	int i = 0;
	bool match = false;

	// must loop through to find the correct event first
	ListIterator evPropIter = createIterator(myCal->events);
	Event* ev;
	while( (ev = nextElement(&evPropIter)) != NULL ) {
		if(i++ == eventNum) {
			match = true;
			almListJSONstr = alarmListToJSON(ev->alarms);
			break;
		}
	}
	// check if a matching event was found (based on number)
	if(!match) {
		deleteCalendar(myCal);
		return NULL;
	}
	deleteCalendar(myCal);
	return almListJSONstr;
}

/**
 * Extract calendar info and event info from a JSON to create a new iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want
 * @param char* calAsJSON -a stringified JSON representing the necessary calendar info
 * @parma char* eventAsJSON -a stringified JSON representing the necessary event info (single event)
 * @return char* printed (human-readable) ICalErrorCode indicating what happened ("OK" for success)
 */
char* a3CreateCalendar(char *dirFilename, char* calAsJSON, char* eventAsJSON) {
	// declare variables
	Calendar *myCal = NULL;
	Event *myEv = NULL;
	
	// turn JSONs into structs
	myCal = JSONtoCalendar(calAsJSON);
	myEv = JSONtoEvent(eventAsJSON);

	// check if it worked
	if(myCal == NULL || myEv == NULL) {
		deleteCalendar(myCal);
		deleteEvent((Event*)myEv);
		char* toReturn = malloc(40);
		strcpy(toReturn, "Error: calendar information is invalid");
		return toReturn;
	}

	// add the event to the calendar, then validate it
	insertSorted(myCal->events, (Event*)myEv);
	ICalErrorCode err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		char* toReturn = printError(err);
		return toReturn;
	}

	// write the calendar to the new file (the 'error' will be OK or something bad, caught in JS)
	err = writeCalendar(dirFilename, myCal);

	deleteCalendar(myCal);
	char* toReturn = printError(err);
	return toReturn;
}

/**
 * Extract  event info from a JSON to add it to the specified existing iCal file
 * @param char* dirFilename -a string of the iCal directory attached to the filename we want to add the event to
 * @parma char* eventAsJSON -a stringified JSON representing the necessary event info (single event)
 * @return char* printed (human-readable) ICalErrorCode indicating what happened ("OK" for success)
 */
char* a3AddEventToCalendar(char *dirFilename, char* eventAsJSON) {
	// declare variables
	Calendar *myCal = NULL;
	Event *myEv = NULL;
	
	// turn JSON into event struct
	myEv = JSONtoEvent(eventAsJSON);

	// check if it worked
	if(myEv == NULL) {
		deleteEvent((Event*)myEv);
		char* toReturn = malloc(40);
		strcpy(toReturn, "Error: event information is invalid");
		return toReturn;
	}

	// try to create the calendar, add the event, then validate it, making sure it was all successful
	ICalErrorCode err = createCalendar(dirFilename, &myCal);
	insertSorted(myCal->events, (Event*)myEv);
	err = validateCalendar(myCal);
	if(err != OK) {
		deleteCalendar(myCal);
		char* toReturn = printError(err);
		return toReturn;
	}

	// write the calendar to the new file (the 'error' will be OK or something bad, caught in JS)
	err = writeCalendar(dirFilename, myCal);

	deleteCalendar(myCal);
	char* toReturn = printError(err);
	return toReturn;
}