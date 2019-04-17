/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A1
 * @version 27/01/2019
 * @file TestHarness.c
 * @brief Runs the main to test stuff
 */

// ============================== INCLUDES ==============================
//#include "LinkedListAPI.h"
#include "CalendarParser.h"
#include "ParserHelper.h"

// ======================================================================

int main(int argc, char **argv) {
    // testing createCalendar
    Calendar *myCal;
    
    //ICalErrorCode wasReturned = createCalendar("mLineProp1.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalEvtProp.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalSimpleNoUTC.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalSimpleUTCComments.ics", &myCal);
    ICalErrorCode wasReturned = createCalendar("testCalEvtPropAlm.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("megaCalFolded1.ics", &myCal);

    //ICalErrorCode wasReturned = createCalendar("missingEndCal.ics", &myCal);

    //ICalErrorCode wasReturned = createCalendar(myfilename, &myCal);

    char* errorStr = printError(wasReturned);
    printf("ICalErrorCode from createCalendar = '%s'\n", errorStr);
    free(errorStr);

    if(myCal == NULL) {
        printf("cal is null\n");
    } else {
        printf("cal not null\n");
    }

    char *s = printCalendar(myCal);
    printf("Now printing the calendar:\n\n%s\n", s);
    free(s);

    // ===================
    char filename[] = "my.ics";

    ICalErrorCode verrCode = validateCalendar(myCal);
    char *verr = printError(verrCode);
    printf("validate cal: %s\n", verr);
    free(verr);

    if(verrCode == OK) {
        char *perr = printError(writeCalendar(filename, myCal));
        printf("write cal: %s\n", perr);
        free(perr);
    } else {
        printf("did not writeCalendar because validateCalendar was not OK\n");
    }
    
    /*// ===================

    // cal + JSON testing

    char* calAsJSON = calendarToJSON(myCal);
    printf("\ncalAsJSON:\n%s\n", calAsJSON);
    free(calAsJSON);

    calAsJSON = calendarToJSON(NULL);
    printf("\nNULL calAsJSON:\n%s\n", calAsJSON);

    Calendar *tempCal = JSONtoCalendar(calAsJSON);
    if(tempCal != NULL) {
        char* calAsJSONcheck = printCalendar(tempCal);
        printf("\nshould not occur: tempCal from created JSON being printed:\n\n%s\n", calAsJSONcheck);
        free(calAsJSONcheck);
        deleteCalendar(tempCal);
    } else {
        printf("\nshould occur: tempCal from created JSON was null\n");
    }

    free(calAsJSON);
    
    Calendar *tempCal1 = JSONtoCalendar("{\"version\":2,\"prodID\":\"-//hacksw/handcal//NONSGML v1.0//EN\"}");
    if(tempCal1 != NULL) {
        char* calAsJSONcheck = printCalendar(tempCal1);
        printf("\ntempCal1 from created JSON being printed:\n\n%s\n", calAsJSONcheck);
        free(calAsJSONcheck);
        deleteCalendar(tempCal1);
    } else {
        printf("\ntempCal1 from created JSON was null\n");
    }

    Calendar *tempCal2 = JSONtoCalendar("{\"version\":12345.12345678,\"prodID\":\"a\"}");
    if(tempCal2 != NULL) {
        char* calAsJSONcheck = printCalendar(tempCal2);
        printf("\ntempCal2 from created JSON being printed:\n\n%s\n", calAsJSONcheck);
        free(calAsJSONcheck);
        deleteCalendar(tempCal2);
    } else {
        printf("\ntempCal2 from created JSON was null\n");
    }

    // ===================
    
    // event + JSON testing

    ListIterator evPropIter = createIterator(myCal->events);
	Event* ev;
	while( (ev = nextElement(&evPropIter)) != NULL ) {
        char* evAsJSON = eventToJSON(ev);
        printf("\nevAsJSON:\n%s\n", evAsJSON);

        Event *tempEv = JSONtoEvent(evAsJSON);
        if(tempEv != NULL) {
            printf("\nshould not occur: tempEv from created JSON being printed:\n\nUID = '%s'\n", tempEv->UID);
            deleteEvent(tempEv);
        } else {
            printf("\nshould occur: tempEv from created JSON was null\n");
        }

        free(evAsJSON);
    }

    char* evAsJSON = eventToJSON(NULL);
    printf("\nNULL evAsJSON:\n%s\n", evAsJSON);
    free(evAsJSON);

    Event *tempEv1 = JSONtoEvent("{\"UID\":\"myValue\"}");
    if(tempEv1 != NULL) {
        printf("\ntempEv1 from created JSON being printed:\n\nUID = '%s'\n", tempEv1->UID);
        deleteEvent(tempEv1);
    } else {
        printf("\ntempEv1 from created JSON was null\n");
    }

    Event *tempEv2 = JSONtoEvent("{\"UID\":\"a\"}");
    if(tempEv2 != NULL) {
        printf("\ntempEv2 from created JSON being printed:\n\nUID = '%s'\n", tempEv2->UID);
        deleteEvent(tempEv2);
    } else {
        printf("\ntempEv2 from created JSON was null\n");
    }

    Event *tempEv3 = JSONtoEvent("{\"UID\":\"uid1@exa\b\f\r\n\t\\\"mple.com\"}");
    if(tempEv3 != NULL) {
        printf("\ntempEv3 from created JSON being printed:\n\nUID = '%s'\n", tempEv3->UID);
        deleteEvent(tempEv3);
    } else {
        printf("\ntempEv3 from created JSON was null\n");
    }

    // event list + JSON testing
    char* evListAsJSON = eventListToJSON(myCal->events);
    printf("\nevListAsJSON:\n%s\n", evListAsJSON);
    free(evListAsJSON);

    // ===================

    // add event testing -- create calendars then add copies of their events to the main calendar

    char fn2[] = "mLineProp1.ics";
    Calendar *cal2;
    ICalErrorCode calCreateErr = createCalendar(fn2, &cal2);
    char* calErrStr = printError(calCreateErr);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", fn2, calErrStr);
    free(calErrStr);
    if(cal2 == NULL) {
        printf("cal2 is null\n");
    } else {
        printf("cal2 not null\n");
    }
    evPropIter = createIterator(cal2->events);
	while( (ev = nextElement(&evPropIter)) != NULL ) {
        Event *eventCopy = cloneEvent(ev);
        // only add if it was successful
        if(eventCopy == NULL) {
            printf(" ---- creating event copy from cal2 returned NULL\n");
        } else {
            addEvent(myCal, eventCopy);
        }
    }
    deleteCalendar(cal2);

    char fn3[] = "testCalEvtProp.ics";
    Calendar *cal3;
    calCreateErr = createCalendar(fn3, &cal3);
    calErrStr = printError(calCreateErr);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", fn3, calErrStr);
    free(calErrStr);
    if(cal3 == NULL) {
        printf("cal3 is null\n");
    } else {
        printf("cal3 not null\n");
    }
    evPropIter = createIterator(cal3->events);
	while( (ev = nextElement(&evPropIter)) != NULL ) {
        Event *eventCopy = cloneEvent(ev);
        // only add if it was successful
        if(eventCopy == NULL) {
            printf(" ---- creating event copy from cal3 returned NULL\n");
        } else {
            addEvent(myCal, eventCopy);
        }
    }
    deleteCalendar(cal3);

    char fn4[] = "testCalSimpleNoUTC.ics";
    Calendar *cal4;
    calCreateErr = createCalendar(fn4, &cal4);
    calErrStr = printError(calCreateErr);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", fn4, calErrStr);
    free(calErrStr);
    if(cal4 == NULL) {
        printf("cal4 is null\n");
    } else {
        printf("cal4 not null\n");
    }
    evPropIter = createIterator(cal4->events);
	while( (ev = nextElement(&evPropIter)) != NULL ) {
        Event *eventCopy = cloneEvent(ev);
        // only add if it was successful
        if(eventCopy == NULL) {
            printf(" ---- creating event copy from cal4 returned NULL\n");
        } else {
            addEvent(myCal, eventCopy);
        }
    }
    deleteCalendar(cal4);

    char fn5[] = "testCalSimpleUTCComments.ics";
    Calendar *cal5;
    calCreateErr = createCalendar(fn5, &cal5);
    calErrStr = printError(calCreateErr);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", fn5, calErrStr);
    free(calErrStr);
    if(cal5 == NULL) {
        printf("cal5 is null\n");
    } else {
        printf("cal5 not null\n");
    }
    evPropIter = createIterator(cal5->events);
	while( (ev = nextElement(&evPropIter)) != NULL ) {
        Event *eventCopy = cloneEvent(ev);
        // only add if it was successful
        if(eventCopy == NULL) {
            printf(" ---- creating event copy from cal5 returned NULL\n");
        } else {
            addEvent(myCal, eventCopy);
        }
    }
    deleteCalendar(cal5);

    // validate and write testing (with added events)
    char filename2[] = "myWithAddedEvents.ics";

    verrCode = validateCalendar(myCal);
    verr = printError(verrCode);
    printf("\nvalidate cal after adding events: %s\n", verr);
    free(verr);

    if(verrCode == OK) {
        char *perr = printError(writeCalendar(filename2, myCal));
        printf("write cal after adding events: %s\n", perr);
        free(perr);
    } else {
        printf("did not writeCalendar after adding events because validateCalendar was not OK\n");
    }

    // print myCal with the added events
    s = printCalendar(myCal);
    printf("\nNow printing the calendar after adding events:\n\n%s\n", s);
    free(s);

    // cal + JSON testing (with added events)
    calAsJSON = calendarToJSON(myCal);
    printf("\ncalAsJSON after adding events:\n%s\n", calAsJSON);
    free(calAsJSON);

    // event list + JSON testing (with added events)
    evListAsJSON = eventListToJSON(myCal->events);
    printf("\nevListAsJSON after adding events:\n%s\n", evListAsJSON);
    free(evListAsJSON);

    // read written calendar testing after adding events
    Calendar *writtenCalWAE;
    ICalErrorCode createErrWAE = createCalendar(filename2, &writtenCalWAE);
    char* createErrStrWAE = printError(createErrWAE);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", filename2, createErrStrWAE);
    free(createErrStrWAE);

    if(writtenCalWAE == NULL) {
        printf("writtenCalWAE is null\n");
    } else {
        printf("writtenCalWAE not null\n");
    }
    deleteCalendar(writtenCalWAE);*/


    deleteCalendar(myCal);

    // ===================
    
    // read written calendar testing
    Calendar *writtenCal;
    ICalErrorCode createErr = createCalendar(filename, &writtenCal);
    char* createErrStr = printError(createErr);
    printf("\nICalErrorCode from createCalendar using '%s' = '%s'\n", filename, createErrStr);
    free(createErrStr);

    if(writtenCal == NULL) {
        printf("writtenCal is null\n");
    } else {
        printf("writtenCal not null\n");
    }
    deleteCalendar(writtenCal);

    // ===================

    Calendar *tempCal = JSONtoCalendar("{\"version\":3.456,\"prodID\":\"proIDval\"}");
    if(tempCal != NULL) {
        printf("\ntesting JSONtoCalendar #1: tempCal from created JSON being printed:\n\nversion = %f   |   prodID = '%s'\n", tempCal->version, tempCal->prodID);
        deleteCalendar(tempCal);
    } else {
        printf("\ntesting JSONtoCalendar #1: tempCal from created JSON was null\n");
    }

    Event *tempEv = JSONtoEvent("{\"UID\":1}");
    if(tempEv != NULL) {
        printf("\ntesting JSONtoEvent #1: tempEv from created JSON being printed:\n\nUID = '%s'\n", tempEv->UID);
        deleteEvent(tempEv);
    } else {
        printf("\ntesting JSONtoEvent #1: tempEv from created JSON was null\n");
    }

    // ===================

    /*NOTE: these are all true
    if(CR == '\r') {
        printf("CR = \\r confirmed!\n");
    }
    if(LF == '\n') {
        printf("LF = \\n confirmed!\n");
    }
    char s[3];
    s[0] = CR;
    s[1] = LF;
    s[2] = '\0';
    if(strcmp(s, "\r\n") == 0) {
        printf("CRLF\\0 = \\r\\n confirmed!\n");
    }*/

    /*char str[40] = "{\"version\":verVal,\"prodID\":\"prodIDVal\"}";
    char myS[40];
    float myF = 0.0;
    sscanf(str, "%s:%f", myS, &myF);
    char ff[100] = "2.9}";
    myF = atof(ff);
    printf("myS = '%s' | myF = '%f'\n", myS, myF);*/

    /*char validEvPropsOnceNames[17][15] = { "CLASS", "CREATED", "DESCRIPTION", "GEO", "LAST-MODIFIED", "LOCATION", "ORGANIZER", "PRIORITY",
			"SEQUENCE", "STATUS", "SUMMARY", "TRANSP", "URL", "RECURRENCE-ID", "RRULE", "DTEND", "DURATION" };
    for(int i = 0; i < 17; i++) {
        if(strchr(validEvPropsOnceNames[i], '\0') == NULL) {
            printf("\tprop[%d] ('%s') null char missing\n", i, validEvPropsOnceNames[i]);
        } else {
            printf("prop[%d] ('%s') null char found\n", i, validEvPropsOnceNames[i]);
        }
    }*/

    printf("\n");
    return 0;
}