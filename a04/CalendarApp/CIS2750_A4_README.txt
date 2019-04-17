/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A4
 * @version 05/04/2019
 * @file CIS2750_A4_README.txt
 * @brief Explains notes and assumptions about my assignment and how to interact with the website (any assumptions and/or notes were confirmed with TAs and/or prof before)
 */

 - Of course, I followed the specifications of all A3 documentation (both modules, A3 stub, A3, and A3 grading scheme)
 - Also, this will conform with any and all previous assumptions and notes from earlier assignments

 - I am also providing an empty 'bin' folder in the 'parser' folder, in regards to my submission structure (where all intermediate .o and .so files are stored)
 - Makefile will not contain version to compile on a mac because it just needs to work on nomachine
 - merged both .so into sharedLib.so as I was told
 - I just made the title (tab name) "VB CalendarApp" since it shouldn't really matter

 - I use text wrapping for the cells in the table, so unless the SUMMARY or PRODID are very long without any spaces, the horizontal scrolling will most likely not be seen
 - For vertical scrolling: Since only enabling it on 4+ or 2+ (status panel, tables respectively) lines/rows would make it very difficult to see information and one would 
    need to scroll a lot, I changed it to be longer. (Approximately 6-8, depends on text wrapping)
        (NOTE: status panel does not wrap text, instead I print status with new lines --> makes it easier to read)

 - When status messages appear I will sometimes jump up to the top of the page so the user can see (not reloading the page) (this is especially true for error messages)
    --> error messages that appear will include information about what exactly occured (eg. what happened, and the filename it occured for)
    (NOTE: for almost every interaction the user has with the site, I give some kind of message in the status panel,
        whether good or bad : eg. selecting a new file in the drop down)

 - I repopulate the file log panel table and the calendar view panel table and drop down lists when the user interacts with the site
    --> I do this because I use insertSorted and just adding the new upload or created calendar at the end would not work properly with retrieving by event number
    --> This was an easier fix to simply repopulate these elements
    (eg. if create calendar returns an error, it will repopulate these elements, depending on which functionality failed for which case, it will sometimes leave the 
        calendar view panel unchanged, but will usually repopulate it with the first valid file on the server)

 - If there are any invalid files on the server on page load, it will display them all in status panel, but not in the other panels
    --> uploading invalid '.ics' files will put them on the server and display a success manage because this operation succeeded,
        but then they won't be displayed in the panels
        --> whenever a file is uploaded, it will also display all invalid files on the server as error messages in the status panel

 - How to show alrams and optional properties for an event in the Calendar View panel:
    --> uses the same drop down, it populates the table with the info for an event in each row
        --> click on the number in the properties column of the table to show the optional properties in the status panel for that row's event
        --> click on the number in the alarms column of the table to show the alarms in the status panel for that row's event

 - How user input is validated:
    1.  Since the buttons are in a form, I used a 'submit' function as a listener. For each input, I put pattern regex restrictions and required properties as appropriate
    2.  Since not everything can be caught in (1), I do more if statement error checking before the AJAX call,
        that will also display appropriate error messages in the status panel and return
    3.  If all input has been properly validated as much as it can be, last line of defence is the C backend code for
        createCalendar/validateCalendar/writeCalendar will return an error that I catch and display appropriately in the status panel

// A4 Additions to readme

NOTE: I changed how it handles invalid iCal files: now, if it finds an invalid cal, it will display an error message and then delete it from the uplaods folder
ASSUMPTION: 'Console Panel' is the same thing as 'Status Panel' --> I display error messages, other messages in the status panel the same way
            (sometimes jumping to the top of the page so the user knows an error or something occured, rather than them scrolling up manually to check)

NOTE: All my A4 stuff is in a new panel at the bottom of the page (as requested)
NOTE: Sometimes (very rarely), things may be weird and happen out of order (eg. display status 0,0,0 when storing files when it actually worked), but this is super rare and due to lag or something random
        As comfirmed with TA, this shouldn't be a problem: if it happens, just redo and try again (everything works for A4 properly)
        Also, a similar weird thing happens if the web app has been open for a period of time, it will try and do something like store all files

NOTE: Until the user logins in to the database successfully, all the database functionality is disabled. Upon login success, database functionality is enabled, while login is disabled
NOTE: Since login functionality is disabled after logging in, and prof saisd not to end the connection, if you want to login under new credentials, just refresh the page.

NOTE: Storing all files, clearing the database, and displaying the database status all display the database status after they finish their functionality (as requested)
ASSUMPTION: When storing all files: if a file doesn't exist in a database, it is added; whereas if it already exists, nothing is done (as PDF says and was confirmed by TA)

NOTE: To execute query, first select a radio button, then fill in the search bar appropriately if necessary, then click the button which displays results in the table below it
NOTE: I used a text field for input because it can be used for all 4 search queries, and for filename search,
        it makes sense in case there's a file in the database but not in the uploads folder for some reason. I also check input and display error msg in status panel if it's bad.
        EXAMPLE: must be a '.ics' file for query 2, things cannot exceed the max length (using the length from the database)
ASSUMPTION: As the PDF says query 1 is 'overly-simplistic', I assume that query 2 and 3 are meeting complexity requirements.
            Also, I confirmed with TA that if a query has a condition
            that just means it has a WHERE (because that's a condition of the query), thus if it has a WHERE 'condition' (like query 2) it meets complexity requirements.

NOTE: Please find my 'A4demo.ics' file in the 'uploads' folder of my submission

NOTE: for using A4demo.ics to test the 3 required queries - Query #:
    1. Returns all 3 events ordered by start date-time (i.e. 1997, 1997, 1998)

    2. Search using 'A4demo.ics' will return its 3 events, 
        searching some random text like 'oiasdqjwndqmcxbopeq.ics' will not change the table display, but will display that it doesn't exist in the database in the status panel

    3. Returns 2 events with the same start date-time (locations are KITCHENER and GUELPH)

NOTE: my extra 3 queries I made for the assignment - Query #:
    4. Search the database for all alarms with a user-inputted action --> since it has a WHERE action = 'user_input' condition, it meets complexity requirements
        --> since our iCal only supports AUDIO files, searching for 'AUDIO' will get all alarms, whereas searching for anything else will return no results
        EXAMPLE using A4demo.ics: search using 'AUDIO' will return all 4 alarms, 
                                    searching 'DISPLAY' will return no results

    5. Search the database for all events with a user-inputted location --> since it has a WHERE location = 'user_input' condition, it meets complexity requirements
        EXAMPLE using A4demo.ics: search using 'GUELPH' will return 1 event (the organizer=mvanbrae@uoguelph.ca), 
                                    searching some random text like 'oiasdqjwndqmcxbopeq' will return no results

    6. Search the database for all events with a user-inputted organizer --> since it has a WHERE organizer = 'user_input' condition, it meets complexity requirements
        EXAMPLE using A4demo.ics: search using 'mvanbrae@uoguelph.ca' will return 2 events (locations are GUELPH and WATERLOO),
                                    searching some random text like 'oiasdqjwndqmcxbopeq@uoguelph.ca' will return no results