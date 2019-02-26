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
    
    ICalErrorCode wasReturned = createCalendar("mLineProp1.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalEvtProp.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalSimpleNoUTC.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalSimpleUTCComments.ics", &myCal);
    //ICalErrorCode wasReturned = createCalendar("testCalEvtPropAlm.ics", &myCal);

    //ICalErrorCode wasReturned = createCalendar("missingEndCal.ics", &myCal);

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

    deleteCalendar(myCal);

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

    return 0;
}