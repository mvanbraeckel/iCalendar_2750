/*
 * @author Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * @course CIS*2750: Software Systems Integration and Development - A3
 * @version 020/03/2019
 * @file CIS2750_A3_README.txt
 * @brief Explains notes and assumptions about my assignment and how to interact with the website (any assumptions were confirmed with TAs or prof before)
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