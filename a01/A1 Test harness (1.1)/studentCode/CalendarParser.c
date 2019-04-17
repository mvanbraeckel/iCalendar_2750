/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A1
 * @version 05/02/2019
 * @file CalendarParser.c
 * @brief Runs the CalendarParser.h file
 */

// ============================== INCLUDES ==============================
#include "CalendarParser.h"
#include "ParserHelper.h"

// ======================================================================

/**
 * Function to create a Calendar object based on the contents of an iCalendar file.
 * @pre File name cannot be an empty string or NULL.  File name must have the .ics extension.
		File represented by this name must exist and must be readable.
 * @post Either:
        A valid calendar has been created, its address was stored in the variable obj, and OK was returned
		or 
		An error occurred, the calendar was not created, all temporary memory was freed, obj was set to NULL, and the 
		appropriate error code was returned
 * @return the error code indicating success or the error encountered when parsing the calendar
 * @param fileName - a string containing the name of the iCalendar file
 * @param a double pointer to a Calendar struct that needs to be allocated
 */
ICalErrorCode createCalendar(char* fileName, Calendar** obj) {
	*obj = NULL;
	// check file name: null, empty, '.ics'
	if(fileName == NULL || strcmp(fileName, "") == 0) {
		return INV_FILE;

	} else {
		int length = strlen(fileName);
		if(length < 5 || fileName[length-4] != '.' || fileName[length-3] != 'i' || fileName[length-2] != 'c' || fileName[length-1] != 's') {
			return INV_FILE;
		}
	}

	FILE *fp;
	char *entireFile;

	// read file all at once if it exists
	fp = fopen(fileName, "r");

	if(fp == NULL) {
		return INV_FILE;

	} else {
		fseek(fp, 0, SEEK_END); // moves to end of file
		int len = ftell(fp); // calculates number of chars (using total bytes)
		fseek(fp, 0, SEEK_SET); // moves it back to beginning

		entireFile = calloc(len+1, sizeof(char)); // init the array
		fread(entireFile, sizeof(char), len, fp); // reads entire file into the array at once
		entireFile[len] = '\0';
	}
	fclose(fp);

	// check file wasn't an empty string and that it ends in LF (of CRLF)
	if( entireFile == NULL || strcmp(entireFile, "") == 0 || strlen(entireFile) < 2 || entireFile[strlen(entireFile)-2] != CR || entireFile[strlen(entireFile)-1] != LF ) {
		free(entireFile);
		return INV_FILE;
	}

	// ------------------------------------ REMOVE ------------------------------------

	//printf("\n\tBEFORE: entireFile:\n%s\n", entireFile);

	int numFoldedLines = 0;

	char *temp = calloc(strlen(entireFile)+1, sizeof(char));
	char *ending = calloc(strlen(entireFile)+1, sizeof(char));
	int numLines = 0;

	bool inComment = false;
	int commentIndex = -1;
	
	// remove all comments
	for(int i = 0; i < strlen(entireFile); i++) {
		// look for start of a comment ()
		if( i > 1 && entireFile[i] == ';' && entireFile[i-1] == LF && entireFile[i-2]) {
			inComment = true;
			commentIndex = i;
		} else if( i == 0 && entireFile[i] == ';' ) {
			inComment = true;
			commentIndex = i;
		}
		// check for invalid file errors
		if( entireFile[i] == CR && entireFile[i+1] != LF) {
			// free everything, retun INV_FILE error for invalid line ending
			free(entireFile);
			free(temp);
			free(ending);
			return INV_FILE;

		} else if( entireFile[i] == LF && (i == 0 || (i > 0 && entireFile[i-1] != CR)) ) {
			// free everything, retun INV_FILE error for invalid line ending
			free(entireFile);
			free(temp);
			free(ending);
			return INV_FILE;
		} else if( entireFile[i] == CR && entireFile[i+1] == LF ) {
			if( i == 0 && (entireFile[i] == CR || entireFile[i] == LF) ) {
				// free everything, retun INV_FILE error for invalid line ending
				free(entireFile);
				free(temp);
				free(ending);
				return INV_FILE;
			} else if( i < strlen(entireFile)-2 && (entireFile[i+2] == CR || entireFile[i+2] == LF) ) {
				// free everything, retun INV_FILE error for invalid line ending
				free(entireFile);
				free(temp);
				free(ending);
				return INV_FILE;
			}
		}
		// look for end of the comment line
		if( inComment && entireFile[i] == CR && entireFile[i+1] == LF ) {
			entireFile[i] = entireFile[i+1] = '^';
			
			// get first half
			//temp = realloc(temp, strlen(entireFile)-2);
			strncpy(temp, entireFile, commentIndex);
			temp[commentIndex] = '\0';
			// get second half
			//ending = realloc(ending, strlen(entireFile)-i);
			strcpy(ending, entireFile+i+2);
			ending[strlen(entireFile+i+2)] = '\0';
			// put back
			strcat(temp, ending);
			strcpy(entireFile, temp);

			//printf("\n\ttemp @index=%d:\n%s\n", i, temp);
			//printf("\n\tending @index=%d:\n%s\n", i, ending);
			i = commentIndex;
			i--;
			inComment = false;
		}
	}

	// ------------------------------------ UNFOLD ------------------------------------
	
	// search for char sequence indicator of folded line
	for(int i = 0; i < strlen(entireFile); i++) {
		if( entireFile[i] == CR && entireFile[i+1] == LF ) {
			numLines++;
			if( i == 0 && (entireFile[i] == CR || entireFile[i] == LF) ) {
				// free everything, retun INV_FILE error for invalid line ending
				free(entireFile);
				free(temp);
				free(ending);
				return INV_FILE;
			}
			if( i < strlen(entireFile)-2 && (entireFile[i+2] == SPACE || entireFile[i+2] == HTAB) ) {
				// actually unfold the line
				numLines--; // doesn't count as a line

				entireFile[i] = entireFile[i+1] = entireFile[i+2] = '^';
				numFoldedLines++;

				// get first half
				//temp = realloc(temp, strlen(entireFile)-2);
				strncpy(temp, entireFile, i);
				temp[i] = '\0';
				// get second half
				//ending = realloc(ending, strlen(entireFile)-i);
				strcpy(ending, entireFile+i+3);
				ending[strlen(entireFile+i+3)] = '\0';
				// put back
				strcat(temp, ending);
				strcpy(entireFile, temp);

				/*printf("\n\ttemp @index=%d:\n%s\n", i, temp);
				printf("\n\tending @index=%d:\n%s\n", i, ending);*/
				i--;
			} else if( i < strlen(entireFile)-2 && (entireFile[i+2] == CR || entireFile[i+2] == LF) ) {
				// free everything, retun INV_FILE error for invalid line ending
				free(entireFile);
				free(temp);
				free(ending);
				return INV_FILE;
			}

		} else if( i < strlen(entireFile)-1 && entireFile[i] == CR && entireFile[i+1] != LF) {
			// free everything, retun INV_FILE error for invalid line ending
			free(entireFile);
			free(temp);
			free(ending);
			return INV_FILE;

		} else if( entireFile[i] == LF && (i == 0 || (i > 0 && entireFile[i-1] != CR)) ) {
			// free everything, retun INV_FILE error for invalid line ending
			free(entireFile);
			free(temp);
			free(ending);
			return INV_FILE;
		}
	}
	free(temp);
	free(ending);

	/*printf("\n\tnumLines = %d\n", numLines);
	printf("\n\tnumFoldedLines = %d\n", numFoldedLines);

	printf("\n\tAFTER: entireFile:\n%s\n", entireFile);*/

	// -------------------- STRING ARRAY -----------------------

	// create array of strings and holder for each line
	char **lines = malloc(numLines * sizeof(char*));
	char *token = strtok(entireFile, "\r\n");

	// store each line into the array of strings
	int k = 0;
	while(token != NULL) {
		lines[k] = calloc(strlen(token)+1, sizeof(char));
		strcpy(lines[k], token);
		lines[k][strlen(lines[k])] = '\0';
		// iterate
		token = strtok(NULL, "\r\n");
		k++;
	}
	free(entireFile); // do not need anymore

	// ---------------------- READ -----------------------

	// create iCal since it's a good file so far
	Calendar *myCal = malloc(sizeof(Calendar));
	myCal->version = -1.0;
	strcpy(myCal->prodID, "");

	myCal->events = initializeList(*printEvent, *deleteEvent, *compareEvents);
	myCal->properties = initializeList(*printProperty, *deleteProperty, *compareProperties);
	
	/*// create holder for content lines
	char *contentLine = calloc(strlen(entireFile)+1, sizeof(char));
	contentLine[strlen(contentLine)] = '\0';*/

	// holders for current event and current alarm being parsed, and for index of delimiter
	Event *currEvent = NULL;
	bool currEventAdded = true;
	Alarm *currAlarm = NULL;
	bool currAlarmAdded = true;
	int delimIndex = -1;

	// create boolean flags: parsing nested level, end of iCal component, required attributes
	bool inCal = false;
	bool inEvent = false;
	bool inAlarm = false;

	bool endCal = false;
	// required attributes
	bool VERSION = false;
	bool PRODID = false;

	bool UID = false;
	bool DTSTAMP = false;
	bool DTSTART = false;

	bool ACTION = false;
	bool TRIGGER = false;

	// loop through and parse each line
	for(int i = 0; i < numLines; i++) {
		// search for index of the first delimiting character (';' or ':'), gives error if neither is found
		for(int j = 0; j < strlen(lines[i]); j++) {
			if(lines[i][j] == ';' || lines[i][j] == ':') {
				delimIndex = j;
				break;
			}
			delimIndex = -1;
		}
		/*if(delimIndex == -1) {

			//printf("delim char not found\n");

			// delim char not found
			freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
			return INV_FILE;
		}*/

		//printf("A:lines[%d] = '%s' --delimIndex = '%d'\n", i, lines[i], delimIndex);

		if(lines[i][0] == ';') { // ------------------------------------ ignore comments ---------------------------------------
			continue;

		} else if(inAlarm) { // ----------------------------------------- parse alarm -----------------------------------------
			// valid things: action, trigger, other alarm properties, and end alarm
			if(strncmpic(lines[i], "ACTION", 6) == 0 && delimIndex == 6) {
				// check if duplicate or empty
				if(ACTION || strcmp(lines[i]+1+6, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_ALARM;
				}
				ACTION = true;
				strcpy(currAlarm->action, lines[i]+1+6);

			} else if(strncmpic(lines[i], "TRIGGER", 7) == 0 && delimIndex == 7) {
				// check if duplicate or empty
				if(TRIGGER || strcmp(lines[i]+1+7, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_ALARM;
				}
				TRIGGER = true;
				// give attribute memory, then copy, then '\0'
				currAlarm->trigger = realloc(currAlarm->trigger, strlen(lines[i]+1+7)+1 * sizeof(char));
				strcpy(currAlarm->trigger, lines[i]+1+7);
				currAlarm->trigger[strlen(lines[i]+1+7)] = '\0';

			} 
			// check BEGIN: legal = none
			else if(strncmpic(lines[i], "BEGIN:", 5+1) == 0) {
				if(strcmpic(lines[i]+1+5, "VALARM") == 0 || strcmpic(lines[i]+1+5, "VEVENT") == 0 || strcmpic(lines[i]+1+delimIndex, "VCALENDAR") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_ALARM;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different BEGIN:etc '%s'\n", lines[i]+1+5);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check BEGIN:
			// check END: legal = VALARM
			else if(strncmpic(lines[i], "END:", 3+1) == 0) {
				if(strcmpic(lines[i]+1+3, "VALARM") == 0) {
					inAlarm = false;
					// check nothing was missing
					if(!ACTION || !TRIGGER) {
						freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
						return INV_ALARM;
					}
					// reset flags
					ACTION = false;
					TRIGGER = false;
					// add it to the list
					insertBack(currEvent->alarms, currAlarm);
					currAlarmAdded = true;

					//printf("now out of alarm\n");

				} else if(strcmpic(lines[i]+1+3, "VCALENDAR") == 0 || strcmpic(lines[i]+1+3, "VEVENT") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_ALARM;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different END:etc '%s'\n", lines[i]+1+3);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check END:
			else {
				// check that delimiter exists and something is actually before and after it
				if(delimIndex <= 0 || strcmp(lines[i]+1+delimIndex, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_ALARM;
				}
				// create and add alarm property
				insertBack(currAlarm->properties, createProperty(lines[i], delimIndex));
			}
			
		} else if(inEvent) { // ----------------------------------------- parse event -----------------------------------------
			// valid things: UID, create-time, start-time, other event properties, begin alarms, and end event
			if(strncmpic(lines[i], "UID", 3) == 0 && delimIndex == 3) {
				// check if duplicate or empty
				if(UID || strcmp(lines[i]+1+3, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_EVENT;
				}
				UID = true;
				strcpy(currEvent->UID, lines[i]+1+3);

			} else if(strncmpic(lines[i], "DTSTAMP", 7) == 0 && delimIndex == 7) {
				// use helper to parse content into the DateTime struct
				ICalErrorCode successfulParseDT = parseDT(DTSTAMP, lines[i]+1+7, &currEvent->creationDateTime);
				if(successfulParseDT != OK) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return successfulParseDT;
				}
				DTSTAMP = true;

			} else if(strncmpic(lines[i], "DTSTART", 7) == 0 && delimIndex == 7) {
				// use helper to parse content into the DateTime struct
				ICalErrorCode successfulParseDT = parseDT(DTSTART, lines[i]+1+7, &currEvent->startDateTime);
				if(successfulParseDT != OK) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return successfulParseDT;
				}
				DTSTART = true;

			} 
			// check BEGIN: legal = VALARM
			else if(strncmpic(lines[i], "BEGIN:", 5+1) == 0) {
				if(strcmpic(lines[i]+1+5, "VALARM") == 0) {
					inAlarm = true;
					// now it will be in parse alarm state

					//printf("now inAlarm\n");

					// create alarm, using the holder
					currAlarm = createAlarm();
					currAlarmAdded = false;

				} else if(strcmpic(lines[i]+1+5, "VEVENT") == 0 || strcmpic(lines[i]+1+5, "VCALENDAR") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_EVENT;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different BEGIN:etc '%s'\n", lines[i]+1+5);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check BEGIN:
			// check END: legal = VEVENT
			else if(strncmpic(lines[i], "END:", 3+1) == 0) {
				if(strcmpic(lines[i]+1+3, "VEVENT") == 0) {
					inEvent = false;
					// check nothing was missing
					if(!UID || !DTSTAMP || !DTSTART) {
						freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
						return INV_EVENT;
					}
					// reset flags
					UID = false;
					DTSTAMP = false;
					DTSTART = false;
					// add it to the list
					insertBack(myCal->events, currEvent);
					currEventAdded = true;

					//printf("now out of event\n");

				} else if(strcmpic(lines[i]+1+3, "VCALENDAR") == 0 || strcmpic(lines[i]+1+3, "VALARM") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_EVENT;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different END:etc '%s'\n", lines[i]+1+3);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check END:
			else {
				// check that something is actually before and after the delimiter
				if(delimIndex <= 0 || strcmp(lines[i]+1+delimIndex, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_EVENT;
				}
				// create and add event property
				insertBack(currEvent->properties, createProperty(lines[i], delimIndex));
			}
		
		} else if(inCal) {  // ------------------------------------------ parse iCal -----------------------------------------
			// valid things: version, product ID, begin events, iCal properties, and end iCal
			if(strncmpic(lines[i], "VERSION:", 7+1) == 0) {
				// check if duplicate or empty string
				if(VERSION) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return DUP_VER;
				} else if(strcmp(lines[i]+1+7, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_VER;
				}
				VERSION = true;
				// copy into iCal
				char *ptr;
				myCal->version = (float)strtod(lines[i]+1+7, &ptr);
				// if invalid, free everything, then return INV_VER error (make sure it's not a negative number too)
				if(strcmp(ptr, "") != 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_VER;
				} else if(!(myCal->version > 0.0)) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_VER;
				}
				

			} else if(strncmpic(lines[i], "PRODID", 6) == 0 && delimIndex == 6) {
				// check if duplicate or empty string
				if(PRODID) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return DUP_PRODID;
				} else if(strcmp(lines[i]+1+6, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_PRODID;
				}
				PRODID = true;
				// copy into iCal
				strcpy(myCal->prodID, lines[i]+1+6);

			}
			// check BEGIN: legal = VEVENT
			else if(strncmpic(lines[i], "BEGIN:", 5+1) == 0) {
				if(strcmpic(lines[i]+1+5, "VEVENT") == 0) {
					inEvent = true;
					// now it will be in parse event state

					//printf("now inEvent\n");

					// create event, using the holder
					currEvent = createEvent();
					currEventAdded = false;

				} else if(strcmpic(lines[i]+1+5, "VALARM") == 0 || strcmpic(lines[i]+1+5, "VCALENDAR") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different BEGIN:etc '%s'\n", lines[i]+1+5);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check BEGIN:
			// check END: legal = VCALENDAR
			else if(strncmpic(lines[i], "END:", 3+1) == 0) {
				if(strcmpic(lines[i]+1+3, "VCALENDAR") == 0) {
					inCal = false;
					endCal = true;
					// check nothing was missing
					if(!VERSION || !PRODID || getLength(myCal->events) == 0) {
						freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
						return INV_CAL;
					}
					// reset flags
					VERSION = false;
					PRODID = false;

					//printf("now out of cal\n");

				} else if(strcmpic(lines[i]+1+3, "VEVENT") == 0 || strcmpic(lines[i]+1+3, "VALARM") == 0) {
					// free everything, return error TODO
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;

				} else {
					// free everything, return error TODO

					//printf("not supposed to happen: found a different END:etc '%s'\n", lines[i]+1+3);

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}

			} // end check END:
			else {
				// check that something is actually before and after the delimiter
				if(delimIndex <= 0 || strcmp(lines[i]+1+delimIndex, "") == 0) {
					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}
				// create and add iCal property
				insertBack(myCal->properties, createProperty(lines[i], delimIndex));
			}

		} // ----------------------------------------- end parse iCal -----------------------------------------
		else { // not inCal and line is not a comment
			if(!endCal) { // not endCal: must be BEGIN:VCALENDAR or else it's a bad file (bc first line)
				if( strcmpic(lines[i], "BEGIN:VCALENDAR") == 0 ) {
					inCal = true;
					//printf("now inCal\n");

				} else {
					// free everything, then return error TODO

					//printf("not supposed to happen: BEGIN:VCALENDAR was not first line (other than comments)\n");

					freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
					return INV_CAL;
				}
			} else {
				// free everything, return error TODO

				//printf("not supposed to happen: found lines after END:VCALENDAR\n");

				freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
				return INV_CAL;
			}
		}

		//printf("B:lines[%d] = '%s'\n", i, lines[i]);
	}

	// check for missing components (eg. missing iCal body), if bool flags are still on, etc.

	// ---------------------- FREE -----------------------
	if(inAlarm) {
		freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
		return INV_ALARM;
	} else if(inEvent) {
		freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
		return INV_EVENT;
	} else if(inCal || !endCal) {
		freeParserTemp(currEvent, currAlarm, lines, numLines, myCal, currEventAdded, currAlarmAdded);
		return INV_CAL;
	} 

	// free temporary since it was success and attach the created iCal before leaving
	freeStringArray(lines, numLines);

	*obj = myCal;
	return OK;
}


/** Function to delete all calendar content and free all the memory.
 *@pre Calendar object exists, is not null, and has not been freed
 *@post Calendar object had been freed
 *@return none
 *@param obj - a pointer to a Calendar struct
**/
void deleteCalendar(Calendar* obj) {
	// only free things if they exist
	if(obj == NULL) {
		return;
	}
	freeList(obj->events);
	freeList(obj->properties);
	free(obj);

	//obj = NULL; // do i need this?
}


/** Function to create a string representation of a Calendar object.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a string representing the Calendar contents has been created
 *@return a string contianing a humanly readable representation of a Calendar object
 *@param obj - a pointer to a Calendar struct
 * NOTE: allocated memory must be handled in returned location (i.e. by main)
**/
char* printCalendar(const Calendar* obj) {
	char *toReturn;
	// if it doesn't exist, print DNE
	if(obj == NULL) {
		toReturn = calloc(14, sizeof(char));
		strcpy(toReturn, "CALENDAR DNE\n");
		toReturn[13] = '\0';
		return toReturn;
	}

	// create temps
	char* propStr = toString(((Calendar*)obj)->properties);
	char* evStr = toString(obj->events);
	
	// create string for printing
	toReturn = malloc(strlen(obj->prodID)+strlen(propStr)+strlen(evStr)+150);
	sprintf(toReturn, "--CALENDAR--\n%-23s\t%f\n%-23s\t%s", "Version:", obj->version, "Product ID:", obj->prodID);

	// cat property list as string
	strcat(toReturn, propStr);
	strcat(toReturn, "\n\n");
	free(propStr);

	// cat event list as string
	strcat(toReturn, evStr);
	free(evStr);

	toReturn[strlen(toReturn)] = '\0';
	return toReturn;
}


/** Function to "convert" the ICalErrorCode into a humanly redabale string.
 *@return a string contaning a humanly readable representation of the error code by indexing into 
          the descr array using he error code enum value as an index
 *@param err - an error code
**/
char* printError(ICalErrorCode err) {
	char* toReturn = malloc(50);
	if(err == OK) {
		strcpy(toReturn, "OK");
	} else if(err == INV_FILE) {
		strcpy(toReturn, "Invalid File");
	} else if(err == INV_CAL) {
		strcpy(toReturn, "Invalid Calendar");
	} else if(err == INV_VER) {
		strcpy(toReturn, "Invalid Version");
	} else if(err == DUP_VER) {
		strcpy(toReturn, "Duplicate Version");
	} else if(err == INV_PRODID) {
		strcpy(toReturn, "Invalid Product ID");
	} else if(err == DUP_PRODID) {
		strcpy(toReturn, "Duplicate Product ID");
	} else if(err == INV_EVENT) {
		strcpy(toReturn, "Invalid Event");
	} else if(err == INV_DT) {
		strcpy(toReturn, "Invalid Date-Time");
	} else if(err == INV_ALARM) {
		strcpy(toReturn, "Invalid Alarm");
	} else if(err == WRITE_ERROR) {
		strcpy(toReturn, "Write Error");
	} else if(err == OTHER_ERROR) {
		strcpy(toReturn, "Other Error");
	} else {
		strcpy(toReturn, "Unknown Error");
	}
	return toReturn;
}


// ************* A2 functionality - MUST be implemented ***************

/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre Calendar object exists, is not null, and is valid
 *@post Calendar has not been modified in any way, and a file representing the
        Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when parsing the calendar
 *@param obj - a pointer to a Calendar struct
 **/
/** Function to writing a Calendar object into a file in iCalendar format.
 *@pre
    Calendar object exists, and is not NULL.
    fileName is not NULL, has the correct extension
 *@post Calendar has not been modified in any way, and a file representing the
       Calendar contents in iCalendar format has been created
 *@return the error code indicating success or the error encountered when traversing the Calendar
 *@param
    obj - a pointer to a Calendar struct
 	fileName - the name of the output file
 **/
ICalErrorCode writeCalendar(char* fileName, const Calendar* obj) {
	return OK;
}

/** Function to validating an existing a Calendar object
 *@pre Calendar object exists and is not NULL
 *@post Calendar has not been modified in any way
 *@return the error code indicating success or the error encountered when validating the calendar
 *@param obj - a pointer to a Calendar struct
 **/
ICalErrorCode validateCalendar(const Calendar* obj) {
	return OK;
}


// *********************************************************************

// ************* List helper functions - MUST be implemented *************** 
void deleteEvent(void* toBeDeleted) {
	Event *myEvent = (Event*)toBeDeleted;
	// only free things if they exist
	if(myEvent == NULL) {
		return;
	}
	freeList(myEvent->alarms);
	freeList(myEvent->properties);
	free(toBeDeleted);
}
int compareEvents(const void* first, const void* second) {
	// make sure they exist
	if (first == NULL || second == NULL){
		return 0;
	}
	
	// create temps
	Event *ev1 = (Event*)first;
	Event *ev2 = (Event*)second;
	
	// compare and return (Date-Time: start, then creation) (date, then time for each)
	if(compareDates(&(ev1->startDateTime), &(ev2->startDateTime)) == 0) {
		return compareDates(&(ev1->creationDateTime), &(ev2->creationDateTime));
	} else {
		return compareDates(&(ev1->startDateTime), &(ev2->startDateTime));
	}
}
char* printEvent(void* toBePrinted) {
	char *toReturn;
	// if it doesn't exist, print DNE
	if(toBePrinted == NULL) {
		toReturn = calloc(11, sizeof(char));
		strcpy(toReturn, "EVENT DNE\n");
		toReturn[10] = '\0';
		return toReturn;
	}

	// create temps
	Event *myEvent = (Event*)toBePrinted;
	char* creationDT = printDate( &(myEvent->creationDateTime) );
	char* startDT = printDate( &(myEvent->startDateTime) );
	char* propStr = toString(myEvent->properties);
	char* almStr = toString(myEvent->alarms);
	
	// create string for printing event list
	toReturn = malloc(strlen(myEvent->UID)+2*sizeof(DateTime)+strlen(creationDT)+strlen(startDT)+strlen(propStr)+strlen(almStr)+60);
	sprintf(toReturn, "\n--EVENT--\n%-23s\t%s\nCreation %s\nStart %s", "UID:", myEvent->UID, creationDT, startDT);
	free(creationDT);
	free(startDT);

	// cat property list as string
	strcat(toReturn, propStr);
	strcat(toReturn, "\n");
	free(propStr);

	// cat alarm list as string
	strcat(toReturn, almStr);
	free(almStr);

	toReturn[strlen(toReturn)] = '\0';
	return toReturn;
}

// -------------------------------------------------------------------------

void deleteAlarm(void* toBeDeleted) {
	Alarm *myAlm = (Alarm*)toBeDeleted;
	// only free things if they exist
	if(myAlm == NULL) {
		return;
	}
	if(myAlm->trigger != NULL) {
		free(myAlm->trigger);
	}
	freeList(myAlm->properties);
	free(toBeDeleted);
}
int compareAlarms(const void* first, const void* second) {
	// make sure they exist
	if (first == NULL || second == NULL){
		return 0;
	}
	
	// create temps
	Alarm *alm1 = (Alarm*)first;
	Alarm *alm2 = (Alarm*)second;
	
	// compare and return (alphabetical: trigger, then action)
	if(strcmpic(alm1->trigger, alm2->trigger) == 0) {
		return strcmpic(alm1->action, alm2->action);
	} else {
		return strcmpic(alm1->trigger, alm2->trigger);
	}
}
char* printAlarm(void* toBePrinted) {
	char *toReturn;
	// if it doesn't exist, print DNE
	if(toBePrinted == NULL) {
		toReturn = calloc(11, sizeof(char));
		strcpy(toReturn, "ALARM DNE\n");
		toReturn[10] = '\0';
		return toReturn;
	}
	// create temps
	Alarm *myAlm = (Alarm*)toBePrinted;
	char* propStr = toString(myAlm->properties);

	// create string for printing alarm list
	toReturn = malloc(strlen(myAlm->action)+strlen(myAlm->trigger)+strlen(propStr)+65);
	sprintf(toReturn, "--ALARM--\n%-23s\t%s\n%-23s\t%s", "Action:", myAlm->action, "Trigger:", myAlm->trigger);

	// cat property list as string, free its temp after
	strcat(toReturn, propStr);
	strcat(toReturn, "\n");
	free(propStr);

	toReturn[strlen(toReturn)] = '\0';
	return toReturn;
}

// -------------------------------------------------------------------------

void deleteProperty(void* toBeDeleted) {
	Property *myProp = (Property*)toBeDeleted;
	// only free if it exists
	if(myProp == NULL) {
		return;
	}
	free(myProp);
}
int compareProperties(const void* first, const void* second) {
	// make sure they exist
	if (first == NULL || second == NULL){
		return 0;
	}
	
	// create temps
	Property *prop1 = (Property*)first;
	Property *prop2 = (Property*)second;
	
	// compare and return (alphabetical: name, then description)
	if(strcmpic(prop1->propName, prop2->propName) == 0) {
		return strcmpic(prop1->propDescr, prop2->propDescr);
	} else {
		return strcmpic(prop1->propName, prop2->propName);
	}
}
char* printProperty(void* toBePrinted) {
	char *toReturn;
	// if it doesn't exist, print DNE
	if(toBePrinted == NULL) {
		toReturn = calloc(14, sizeof(char));
		strcpy(toReturn, "PROPERTY DNE\n");
		toReturn[13] = '\0';
		return toReturn;
	}
	Property *myProp = (Property*)toBePrinted;
	// create string for printing property list, use temp to add a ':' to propName for printing
	toReturn = malloc(strlen(myProp->propName)+strlen(myProp->propDescr)+30);
	char tempName[strlen(myProp->propName)+2];
	strcpy(tempName, myProp->propName);
	strcat(tempName, ":\0");
	sprintf(toReturn, "%-23s\t%s", tempName, myProp->propDescr);

	toReturn[strlen(toReturn)] = '\0';
	return toReturn;
}

// -------------------------------------------------------------------------

void deleteDate(void* toBeDeleted) {
	DateTime *myDT = (DateTime*)toBeDeleted;
	// only free if it exists
	if(myDT == NULL) {
		return;
	}
	free(myDT);
}
int compareDates(const void* first, const void* second) {
	// remember to account for either being NULL
	// for now, just returning 0 is fine by A1 description (module 1)
	//return 0;

	// make sure they exist
	if (first == NULL || second == NULL){
		return 0;
	}
	
	// create temps
	DateTime *dt1 = (DateTime*)first;
	DateTime *dt2 = (DateTime*)second;

	// compare and return (date, then time)
	if(strcmpic(dt1->date, dt2->date) == 0) {
		return strcmpic(dt1->time, dt2->time);
	} else {
		return strcmpic(dt1->date, dt2->date);
	}
}
char* printDate(void* toBePrinted) {
	// create temps
	DateTime *myDT = (DateTime*)toBePrinted;
	char date[9];
	char dtTime[7];
	strcpy(date, myDT->date);
	date[8] = '\0';
	strcpy(dtTime, myDT->time);
	dtTime[6] = '\0';

	char* toReturn = calloc(50, sizeof(char));
	toReturn[49] = '\0';
	strcpy(toReturn, "Date-Time:\t");
	// month
	if(strncmp(date+4, "01", 2) == 0) {
		strcat(toReturn, "January ");
	} else if(strncmp(date+4, "02", 2) == 0) {
		strcat(toReturn, "February ");
	} else if(strncmp(date+4, "03", 2) == 0) {
		strcat(toReturn, "March ");
	} else if(strncmp(date+4, "04", 2) == 0) {
		strcat(toReturn, "April ");
	} else if(strncmp(date+4, "05", 2) == 0) {
		strcat(toReturn, "May ");
	} else if(strncmp(date+4, "06", 2) == 0) {
		strcat(toReturn, "June ");
	} else if(strncmp(date+4, "07", 2) == 0) {
		strcat(toReturn, "July ");
	} else if(strncmp(date+4, "08", 2) == 0) {
		strcat(toReturn, "August ");
	} else if(strncmp(date+4, "09", 2) == 0) {
		strcat(toReturn, "September ");
	} else if(strncmp(date+4, "10", 2) == 0) {
		strcat(toReturn, "October ");
	} else if(strncmp(date+4, "11", 2) == 0) {
		strcat(toReturn, "November ");
	} else { // 12
		strcat(toReturn, "December ");
	}
	// day
	strcat(toReturn, date+6);
	strcat(toReturn, ", ");
	// year
	strncat(toReturn, date, 4);
	strcat(toReturn, " - ");
	// hour
	strncat(toReturn, dtTime, 2);
	strcat(toReturn, ":");
	// minute
	strncat(toReturn, dtTime+2, 2);
	strcat(toReturn, ":");
	// day
	strcat(toReturn, dtTime+4);
	// UTC
	if(myDT->UTC == true) {
		strcat(toReturn, " UTC");
	}
	// terminate and return
	toReturn[strlen(toReturn)] = '\0';
	return toReturn;
}

// ***************************************************************************