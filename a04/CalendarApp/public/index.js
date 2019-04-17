// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    /**
     * On Page Load -this AJAX function executes
     * --> populates the File Log Panel table with all valid iCal files on the server (or displays "No files")
     * --> populates the View Calendar Panel drop-down list with all valid iCal files on the server (or remains if there are no files)
     * --> populates the View Calendar Panel table with the event list info of the first valid file on the server (or remains if there are no files)
     */
    $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/getuploadfilelist',
        success: function (data) {
            
            console.log(data);

            // display error message if no files on server
            if(data.length == 0) {

                console.log("'No files' should be displayed in file log panel");
                
                // get the table for file log panel and calc row length
                var tableBody = document.getElementById("tableFileLog").getElementsByTagName("tbody")[0];
                var rowLength = tableBody.rows.length;
                var row = tableBody.insertRow(rowLength);

                // Insert new cells in the empty row
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);

                // Add some default text to the new cells:
                cell1.innerHTML = "No files";
                cell2.innerHTML = "-";
                cell3.innerHTML = "-";
                cell4.innerHTML = "-";
                cell5.innerHTML = "-";                
            } else {
                // properly fill in the table
                populateFileLogPanelTable(true);
            }
            // properly fill in the view calendar panel drop-down list and table
            repopulateDropDown("none");

            // ---- A4 LOGIN AND DATABASE FUNCTIONALITY: DISABLED-STATE ENSURANCE ----

            // make sure login button and input fields start enabled
            var loginItems = Array.from(document.getElementsByName("dbLogin"));
            for(var i = 0; i < loginItems.length; i++) {
                loginItems[i].disabled = false;
            }
            
            // make sure database functionality starts disabled
            var a4Buttons = Array.from(document.getElementsByName("db"));
            var rdoButtons = Array.from(document.getElementsByName("rdoQueries"));
            for(var i = 0; i < a4Buttons.length; i++) {
                a4Buttons[i].disabled = true;
            }
            for(var i = 0; i < rdoButtons.length; i++) {
                rdoButtons[i].disabled = true;
            }

            // only enable store all files button if there's at least one file
            var selCalView = document.getElementById("selectCalView");
            if(selCalView.options.length <= 0) {
                document.getElementById("storeAllFilesButton").disabled = true;
            } else {
                document.getElementById("storeAllFilesButton").disabled = false;
            }

        },
        error: function(error) {
            // display failure msg
            var textToAdd = "Error: Unable to retrieve files from server - " + error;
            addStatusJump(textToAdd);
        },
        fail: function(error) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve files from server - " + error;
            addStatusJump(textToAdd);
        }
    });

    // ====================================================== END OF ON-PAGE-LOAD ======================================================

    /**
     * Clear Button : click listener -- Clears the Status Panel output display
     */
    document.getElementById("clearButton").onclick = function() {
        // reset status panel text to nothing
        document.getElementById("pStatusDisplay").innerHTML = "";
    }

    /**
     * File Browser -- Displays error message in status panel if no file has been selected yet
     */
    document.getElementById("filebrowser").oninvalid = function(e) {
        // display error msg if the file browser is invalid
        if (!e.target.validity.valid) {
            var textToAdd = "Error: No file selected";
            addStatus(textToAdd);
        }
    }

    /**
     * Upload : Listens for the Upload Button being pressed, submitting the appropriate form (with the file browser)
     */
    //document.getElementById("uploadButton").onclick = function() {
    $('#uploadform').submit(function(e) {
        e.preventDefault(); // don't refresh page on error

        // get the file browser
        var myFileBrowser = document.getElementById("filebrowser");
        myFileBrowser.setAttribute("type", "file");

        // check that the selected file name from the file browser is valid
        var fname = myFileBrowser.files[0].name;
        var len = fname.length;
        if( len < 5 || fname.charAt(len-4) != '.' || fname.charAt(len-3) != 'i' || fname.charAt(len-2) != 'c' || fname.charAt(len-1) != 's' ) {
            // display error msg, leave function due to invalid input
            var textToAdd = "Error: this is an invalid filename ('" + fname + "'), cannot upload this";
            addStatusJump(textToAdd);
            return;
        }

        // get the calendar view panel drop down
        var selCalView = document.getElementById("selectCalView");
        var numOptions = selCalView.options.length;

        // check that the file does not does not already exist on the server
        for(var i = numOptions-1; i >= 0; i--) {
            if(selCalView.options[i].text === fname) {
                // display error msg, leave function due to invalid input
                var textToAdd = "Error: ICal file '" + fname + "' already exists on server, cannot upload this";
                addStatusJump(textToAdd); 
                return;
            }
        }
        
        // create form data and add the name of the file to upload
        var fd = new FormData();
        fd.append("uploadFile", myFileBrowser.files[0]);

        /**
         * Upload File to Server -this AJAX function executes
         * --> uploads the designated file to the server
         * --> repopulates the File Log Panel table with all valid iCal files on the server (or displays "No files")
         * --> repopulates the View Calendar Panel drop-down list with all valid iCal files on the server (or remains if there are no files)
         * --> populates the View Calendar Panel table with the event list info of the uploaded iCal file
         */
        $.ajax({
            type: "post",
            url: "/upload",
            data: fd,
            processData: false,
            contentType: false,
            success: function(data) {
                // check to delete "No files" row first
                var tableBody = document.getElementById("tableFileLog").getElementsByTagName("tbody")[0];
                if(tableBody.getElementsByTagName("tr")[0].getElementsByTagName("td")[0].textContent === "No files") {

                    console.log("Deleted 'No files' row from file log panel");
                    
                    tableBody.deleteRow(0);
                }

                // properly fill the tables for the file log panel and view calendar panel, and the view calendar panel drop-down list
                populateFileLogPanelTable(true);
                repopulateDropDown(myFileBrowser.files[0].name);

                // display succcess msg
                var textToAdd = "File '" + myFileBrowser.files[0].name + "' uploaded successfully";
                addStatus(textToAdd);

                // auto scroll file log table to bottom (for row of newly uploaded file)
                var objDiv = document.getElementById("divFileLogTable");
                objDiv.scrollTop = 0;

            },
            error: function(error) {
                // display error msg
                var textToAdd = "Error: No file selected - " + error;
                addStatusJump(textToAdd);
            },
            fail: function(error) {
                // display error msg
                var textToAdd = "Failure: No file selected - " + error;
                addStatusJump(textToAdd);
            }
        });
    });

    /**
     * Create Event : Listens for the Add Event Button being pressed, submitting the appropriate form (with all inputs to create and add an event to an iCal file on the server)
     */
    $('#createeventform').submit(function(e) {
        e.preventDefault(); // don't refresh page on error

        // get all data from the form elements
        var selCalView = document.getElementById("selectCalView");

        var txtUID = document.getElementById("txtUID");
        var txtSummary = document.getElementById("txtSUMMARY");
        var cbSummary = document.getElementById("cbSUMMARY");
        var dateDTSTAMP = document.getElementById("dateDTSTAMP");
        var txtTimeDTSTAMP = document.getElementById("txtTimeDTSTAMP");
        var cbUTCDTSTAMP = document.getElementById("cbUTCDTSTAMP");
        var dateDTSTART = document.getElementById("dateDTSTART");
        var txtTimeDTSTART = document.getElementById("txtTimeDTSTART");
        var cbUTCDTSTART = document.getElementById("cbUTCDTSTART");

        // check that the dates' years are 4 digits
        if(dateDTSTAMP.value[4] != '-' || dateDTSTART.value[4] != '-') {
            // display error msg, leave function due to invalid input
            var textToAdd = "Error: The date's year must be 4 digits";
            addStatusJump(textToAdd);
            return;
        }

        // construct Date-Time struct strings in format specific to the file
        var dtstampStr = "" + dateDTSTAMP.value.substring(0,4) + dateDTSTAMP.value.substring(5,7) + dateDTSTAMP.value.substring(8,10) +
                "T" + txtTimeDTSTAMP.value.substring(0,2) + txtTimeDTSTAMP.value.substring(3,5) + txtTimeDTSTAMP.value.substring(6,8);
        if(cbUTCDTSTAMP.checked) {
            dtstampStr += "Z";
        }
        var dtstartStr = "" + dateDTSTART.value.substring(0,4) + dateDTSTART.value.substring(5,7) + dateDTSTART.value.substring(8,10) +
                "T" + txtTimeDTSTART.value.substring(0,2) + txtTimeDTSTART.value.substring(3,5) + txtTimeDTSTART.value.substring(6,8);
        if(cbUTCDTSTART.checked) {
            dtstartStr += "Z";
        }

        // create the JSON string to pass
        var jsonAsString = JSON.stringify([ 
            { filename : selCalView.value }, 
            { UID : txtUID.value, DTSTAMP : dtstampStr, DTSTART : dtstartStr, includeSUMMARY : cbSummary.checked, SUMMARY : txtSummary.value} 
        ]);

        /**
         * Add Event to file on server -this AJAX function executes
         * --> adds a new event to an iCal file already on the server
         * --> repopulates the File Log Panel table with all valid iCal files on the server (or displays "No files")
         * --> repopulates the View Calendar Panel drop-down list with all valid iCal files on the server (or remains if there are no files)
         * --> repopulates the View Calendar Panel table with the new event list info after adding the event to the iCal file
         */
        $.ajax({
            type: "post",
            url: "/createevent",
            dataType: "json",
            data: jsonAsString, 
            contentType: 'application/json; charset=utf-8',
            headers: {'Content-Type':'application/json; charset=utf-8'},
            processData: false,
            success: function(isValidEv) {
                // make sure that everything worked properly on the back-end
                if(isValidEv.ICalErrorCodeReturn != "OK") {
                    // display error msg
                    var textToAdd = "Error adding event: '" + isValidEv.ICalErrorCodeReturn + "' to '" + selCalView.value + "'";
                    addStatusJump(textToAdd);

                } else {
                    // properly fill the tables for the file log panel and view calendar panel, and the view calendar panel drop-down list
                    populateFileLogPanelTable(true);
                    repopulateDropDown(selCalView.value);

                    // display success msg
                    var textToAdd = "Successfully added event to '" + selCalView.value + "'";
                    addStatus(textToAdd);
                    // auto scroll file log table to bottom (for row of newly edited file)
                    var objDiv = document.getElementById("divFileLogTable");
                    objDiv.scrollTop = 0;
                }
            },
            error: function(isValidEv) {
                // display error msg
                var textToAdd = "Error: Could not add the event '" + txtSummary.value + "' to the calendar '" + selCalView.value + 
                        "' due to the following error: '" + isValidEv.ICalErrorCodeReturn + "'";
                addStatusJump(textToAdd);   
            },
            fail: function(isValidEv) {
                // display error msg
                var textToAdd = "Failure: Could not add the event '" + txtSummary.value + "' to the calendar '" + selCalView.value + 
                        "' due to the following error: '" + isValidEv.ICalErrorCodeReturn + "'";
                addStatusJump(textToAdd);           
            }
        });
    });

    /**
     * Create Calendar : Listens for the Create Calendar Button being pressed, submitting the appropriate form (with all inputs to create a new iCal file)
     */
    $('#createcalform').submit(function(e) {
        e.preventDefault(); // if comment this out, change input type to 'button' instead of 'submit'       

        // get all data from the form elements
        var selCalView = document.getElementById("selectCalView");

        var txtFileName = document.getElementById("txtFileNamecc");
        var txtProdID = document.getElementById("txtPRODIDcc");
        var txtNumVersion = document.getElementById("txtNumVERSIONcc");

        var txtUID = document.getElementById("txtUIDcc");
        var txtSummary = document.getElementById("txtSUMMARYcc");
        var cbSummary = document.getElementById("cbSUMMARYcc");
        var dateDTSTAMP = document.getElementById("dateDTSTAMPcc");
        var txtTimeDTSTAMP = document.getElementById("txtTimeDTSTAMPcc");
        var cbUTCDTSTAMP = document.getElementById("cbUTCDTSTAMPcc");
        var dateDTSTART = document.getElementById("dateDTSTARTcc");
        var txtTimeDTSTART = document.getElementById("txtTimeDTSTARTcc");
        var cbUTCDTSTART = document.getElementById("cbUTCDTSTARTcc");

        // check that the file does not does not already exist on the server
        var numOptions = selCalView.options.length;
        for(var i = numOptions-1; i >= 0; i--) {
            if(selCalView.options[i].text === txtFileName.value) {
                // display error msg, leave function due to invalid input
                var textToAdd = "Error: ICal file '" + txtFileName.value + "' already exists on server, cannot create a new iCal file with this same filename";
                addStatusJump(textToAdd);
                return;
            }
        }

        // check that the dates' years are 4 digits
        if(dateDTSTAMP.value[4] != '-' || dateDTSTART.value[4] != '-') {
            // display error msg, leave function due to invalid input
            var textToAdd = "Error: The date's year must be 4 digits";
            addStatusJump(textToAdd);
            return;
        }

        // construct Date-Time struct strings in format specific to the file
        var dtstampStr = "" + dateDTSTAMP.value.substring(0,4) + dateDTSTAMP.value.substring(5,7) + dateDTSTAMP.value.substring(8,10) +
                "T" + txtTimeDTSTAMP.value.substring(0,2) + txtTimeDTSTAMP.value.substring(3,5) + txtTimeDTSTAMP.value.substring(6,8);
        if(cbUTCDTSTAMP.checked) {
            dtstampStr += "Z";
        }
        var dtstartStr = "" + dateDTSTART.value.substring(0,4) + dateDTSTART.value.substring(5,7) + dateDTSTART.value.substring(8,10) +
                "T" + txtTimeDTSTART.value.substring(0,2) + txtTimeDTSTART.value.substring(3,5) + txtTimeDTSTART.value.substring(6,8);
        if(cbUTCDTSTART.checked) {
            dtstartStr += "Z";
        }
        
        // create the JSON string to pass
        var jsonAsString = JSON.stringify([ 
            { filename : txtFileName.value }, 
            { version : parseInt(txtNumVersion.value), prodID : txtProdID.value },
            { UID : txtUID.value, DTSTAMP : dtstampStr, DTSTART : dtstartStr, includeSUMMARY : cbSummary.checked, SUMMARY : txtSummary.value} 
        ]);
        
        /**
         * Create New Calendar directly on server -this AJAX function executes
         * --> creates a new iCal file directly on the server
         * --> repopulates the File Log Panel table with all valid iCal files on the server (or displays "No files")
         * --> repopulates the View Calendar Panel drop-down list with all valid iCal files on the server (or remains if there are no files)
         * --> populates the View Calendar Panel table with the event list info of the newly created iCal file
         */
        $.ajax({
            type: "post",
            url: "/createcal",
            dataType: "json",
            data: jsonAsString,
            contentType: 'application/json; charset=utf-8',
            headers: {'Content-Type':'application/json; charset=utf-8'},
            processData: false,
            success: function(isValidCal) {
                // make sure that everything worked properly on the back-end
                if(isValidCal.ICalErrorCodeReturn != "OK") {
                    // display error msg
                    var textToAdd = "Error creating calendar: '" + isValidCal.ICalErrorCodeReturn + "'";
                    addStatusJump(textToAdd);

                } else {
                    // check to delete "No files" row first
                    var tableBody = document.getElementById("tableFileLog").getElementsByTagName("tbody")[0];
                    if(tableBody.getElementsByTagName("tr")[0].getElementsByTagName("td")[0] === "No files") {

                        console.log("Deleted 'No files' row from file log panel");
                        
                        tableBody.deleteRow(0);
                    }

                    // properly fill the tables for the file log panel and view calendar panel, and the view calendar panel drop-down list
                    populateFileLogPanelTable(true);
                    repopulateDropDown(txtFileName.value);

                    // display success msg
                    var textToAdd = "Created and added calendar '" + txtFileName.value + "' successfully";
                    addStatus(textToAdd);
                    // auto scroll file log table to bottom (for row of newly created file)
                    var objDiv = document.getElementById("divFileLogTable");
                    objDiv.scrollTop = 0;
                    $('html,body').scrollTop(0); // jump to top of page to show success in status
                }
            },
            error: function(isValidCal) {
                // display error msg
                var textToAdd = "Error: could not create calendar: '" + isValidCal.ICalErrorCodeReturn + "'";
                addStatusJump(textToAdd);
            },
            fail: function(isValidCal) {
                // display error msg
                var textToAdd = "Failure: could not create calendar: '" + isValidCal.ICalErrorCodeReturn + "'";
                addStatusJump(textToAdd);
            }
        });
    });

    /**
     * -Calendar View Panel- Drop Down : Listens for a new drop down list item being selected
     */
    document.getElementById("selectCalView").onchange = function() {
        // refills the calendar view panel table with the selected file's event list summary info
        populateViewCalPanelTable(this.value);

        // display msg in status panel
        var textToAdd = "Calendar View Panel: Selected '" + this.value + "'";
        addStatus(textToAdd);
    }

    // ========================================================================= A4 BUTTONS =========================================================================

    /**
     * Login to Database : Listens for the Login Button being pressed, submitting the appropriate form (with all login credential field-inputs)
     */
    $('#loginform').submit(function(e) {
        e.preventDefault();

        // get all data from the form elements
        var txtDBName = document.getElementById("txtDBName");
        var txtUsername = document.getElementById("txtUsername");
        var pwdPassword = document.getElementById("pwdPassword");

        /**
         * Login to the database -this AJAX function executes
         * --> tries to establish a connection to the database using the given login credentials
         * --> disables and enables DB login and functionality appropriately
         * --> displays DB status and a success/error msg
         */
        $.ajax({
            type: 'get',
            url: '/dblogin',
            dataType: 'json',
            data: { DBNAME: txtDBName.value, USERNAME: txtUsername.value, PASSWORD: pwdPassword.value },
            success: function (data) {

                // make sure it worked
                if(data.loginSuccess == true) {
                    textToAdd = "Login to database '" + txtDBName.value + "' was successful for user '" + txtUsername.value + "'";
                    addStatus(textToAdd);

                    // enable all database functionality
                    var a4Buttons = Array.from(document.getElementsByName("db"));
                    var rdoButtons = Array.from(document.getElementsByName("rdoQueries"));
                    for(var i = 0; i < a4Buttons.length; i++) {
                        a4Buttons[i].disabled = false;
                    }
                    for(var i = 0; i < rdoButtons.length; i++) {
                        rdoButtons[i].disabled = false;
                    }

                    // only enable store all files button if there's at least one file
                    var selCalView = document.getElementById("selectCalView");
                    if(selCalView.options.length <= 0) {
                        document.getElementById("storeAllFilesButton").disabled = true;
                    } else {
                        document.getElementById("storeAllFilesButton").disabled = false;
                    }

                    // disable login button and input fields
                    var loginItems = Array.from(document.getElementsByName("dbLogin"));
                    for(var i = 0; i < loginItems.length; i++) {
                        loginItems[i].disabled = true;
                    }

                    // display database status upon successful login
                    getDBStatus();
                    
                } else {
                    // display error msg
                    var textToAdd = "Login Failed: database '" + txtDBName.value + "' login was unsuccessful for user '" + txtUsername.value + "', please try again";
                    addStatusJump(textToAdd);
                }

            },
            error: function() {
                // display error msg
                var textToAdd = "Error: database '" + txtDBName.value + "' login was unsuccessful for user '" + txtUsername.value + "', please try again";
                addStatusJump(textToAdd);
            },
            fail: function() {
                // display error msg
                var textToAdd = "Failure: database '" + txtDBName.value + "' login was unsuccessful for user '" + txtUsername.value + "', please try again";
                addStatusJump(textToAdd);
            }
        });
    });

    /**
     * Store All Files : Listens for the Store All Files Button being pressed, stores all uploaded files in the database if they don't already exist there
     */
    document.getElementById('storeAllFilesButton').onclick = function() {
        // display output in status panel
        var textToAdd = "Storing all uploaded files in the database...";
        addStatus(textToAdd);

        // store all uploaded iCal files into the database, then display its status afterwards
        storeAllFilesInDB();
        getDBStatus();
    };

    /**
     * Clear All Data : Listens for the Clear All Data Button being pressed and deletes all rows in each table in the database, then displays the database's status (should be all 0)
     */
    document.getElementById('clearAllDataButton').onclick = function() {
        clearAllDBTableData();
        getDBStatus();
    };

    /**
     * Display DB Status : Listens for the Display DB Status Button being pressed, shows the database status in the Status Panel
     */
    document.getElementById('displayDBStatusButton').onclick = function() {
        getDBStatus();
    };

    /**
     * Execute Query : Listens for the Execute Query Button being pressed, carries out the corresponding query and shows the result in the Status Panel / table
     */
    $('#executequeryform').submit(function(e) {
        e.preventDefault();

        // get the selected radio button
        var selectedQueryRdo;
        for(var i = 1; i <= 6; i++) {
            var currRdo = document.getElementById("rdoQuery" + i);
            if(currRdo.checked) {
                selectedQueryRdo = currRdo;
                break;
            }
        }

        // check which is the selected radio button, run that query
        if(selectedQueryRdo.value == "rdoQuery1") {
            queryDB1();
        } else if(selectedQueryRdo.value == "rdoQuery2") {
            queryDB2();
        } else if(selectedQueryRdo.value == "rdoQuery3") {
            queryDB3();
        } else if(selectedQueryRdo.value == "rdoQuery4") {
            queryDB4();
        } else if(selectedQueryRdo.value == "rdoQuery5") {
            queryDB5();
        } else if(selectedQueryRdo.value == "rdoQuery6") {
            queryDB6();
        } 

    });
});

// ====================================================== HELPER FUNCTIONS ======================================================

/**
 * Adds more text to the Status Panel output display, automatically scrolling to newest meessage
 * @param {string} textToAdd 
 */
function addStatus(textToAdd) {
    // append new text onto the status display panel
    var statusDisplay = document.getElementById("pStatusDisplay");
    var paraBreak = document.createElement("BR");
    var textToAdd = document.createTextNode(textToAdd);
    statusDisplay.appendChild(textToAdd);
    statusDisplay.appendChild(paraBreak);

    // scrolls to bottom of Status Panel output automatically
    var objDiv = document.getElementById("divStatusDisplay");
    objDiv.scrollTop = objDiv.scrollHeight;
}

/**
 * Adds more text to the Status Panel output display, automatically scrolling to newest meessage, and jumping to the top to see it
 * @param {string} textToAdd 
 */
function addStatusJump(textToAdd) {
    // append new text onto the status display panel, automatically scrolling to bottom, then jumps to top of page
    addStatus(textToAdd);
    $('html,body').scrollTop(0);
}

// ==================================================== AJAX HELPER FUNCTIONS ====================================================

/**
 * Adds a button listener for extracting optional properties to an already existing button
 * @param {int} bNum -button id number
 */
function addExtractOptPropsButtonListener(bNum) {
    document.getElementById("extractOptPropsButton"+bNum).addEventListener("click", extractOptPropsListener=function() {
        // get the file name and event number list index
        var fname = document.getElementById("selectCalView").value;
        var evNum = bNum-1;

        /**
         * Display all of the designated event's optional properties in the Status Panel -this AJAX function executes
         * --> retrieves the property list of the desired event from the specified iCal file
         * --> displays all optional properties in the status panel
         */
        $.ajax({
            type: "get",
            url: "/vieweventprops",
            dataType: "json",
            data: { filename : fname, eventNum : evNum }, 
            success: function(data) {
                // display output header in status panel
                var textToAdd = "[Event #" + bNum + " Optional Properties]:"
                addStatus(textToAdd);
                
                // check if there are any properties in the event
                if(data.length == 0) {
                    // display msg in status panel
                    textToAdd = "--no optional properties--";
                    addStatus(textToAdd);
                } else {
                    // display each property's info in status panel
                    for(var i = 0; i < data.length; i++) {
                        textToAdd = "("+ (i+1) + ") Property: " + data[i].propName + "\t--> " + data[i].propDescr;
                        addStatus(textToAdd);
                    }
                }
            },
            error: function(error) {
                // display error msg
                var textToAdd = "Error: Could not retrieve the optional properties list for event #" + bNum + " from the iCal file '" + fname + "'";
                addStatus(textToAdd);
            },
            fail: function(error) {
                // display error msg
                var textToAdd = "Failure: Could not retrieve the optional properties list for event #" + bNum + " from the iCal file '" + fname + "'";
                addStatus(textToAdd);
            }
        });

        // jumps to top of page automatically
        $(window).scrollTop(0);
    });
}

/**
 * Adds a button listener for showing alarms to an already existing button
 * @param {int} bNum -button id number
 */
function addShowAlarmsButtonListener(bNum) {
    document.getElementById("showAlarmsButton"+bNum).addEventListener("click", showAlarmsListener=function() {
        // get the file name and event number list index
        var fname = document.getElementById("selectCalView").value;
        var evNum = bNum-1;

        /**
         * Display all of the designated event's alarms in the Status Panel -this AJAX function executes
         * --> retrieves the alarm list of the desired event from the specified iCal file
         * --> displays all alarms in the status panel
         */
        $.ajax({
            type: "get",
            url: "/vieweventalarms",
            dataType: "json",
            data: { filename : fname, eventNum : evNum }, 
            success: function(data) {
                // display output header in status panel
                var textToAdd = "[Event #" + bNum + " Alarms]:"
                addStatus(textToAdd);
                
                // check if there are any alarms in the event
                if(data.length == 0) {
                    // display msg in status panel
                    textToAdd = "--no alarms--";
                    addStatus(textToAdd);
                } else {
                    // display each alarm's summary info in status panel
                    for(var i = 0; i < data.length; i++) {
                        textToAdd = "("+ (i+1) + ") Action: " + data[i].action + " | Trigger: " + data[i].trigger + " | Number of Properties: " + data[i].numProps;
                        addStatus(textToAdd);
                    }
                }
            },
            error: function(error) {
                // display err msg
                var textToAdd = "Error: Could not retrieve the alarm list for event #" + bNum + " from the iCal file '" + fname + "'";
                addStatus(textToAdd);
            },
            fail: function(error) {
                // display err msg
                var textToAdd = "Failure: Could not retrieve the alarm list for event #" + bNum + " from the iCal file '" + fname + "'";
                addStatus(textToAdd);
            }
        });

        // jumps to top of page automatically
        $(window).scrollTop(0);
    });
}

/**
 * Repopulates the file log panel's table using the file's on the server
 * @param {boolean} displayErrMsg -if true, displays an error msg when a file on the server is invalid
 */
function populateFileLogPanelTable(displayErrMsg) {
    /**
     * Populates File Log Panel table -this AJAX function executes
     * --> retrieves the summary info of all files on the server
     * --> only displays valid iCal files in the file log panel
     *  --> will display when a file on the server is invalid if displayErrMsg is true
     */
    $.ajax({
        type: 'get',
        url: '/filelogtable',
        success: function (data) {
            // get the table for calendar view panel and calc row length
            var tableBody = document.getElementById("tableFileLog").getElementsByTagName("tbody")[0];
            var rowLength = tableBody.rows.length;

            // remove all rows from tbody first
            for(var i = rowLength-1; i >= 0; i--) {
                tableBody.deleteRow(i);
            }

            // recalculate row length after deleting
            rowLength = tableBody.rows.length;
            var validFileCount = 0;

            // fill in each row of the file log panel table using valid iCal files on server
            for(var i = 0; i < data.length; i++) {
                // check if the file is valid
                if( data[i][0].valid ) {
                    // insert empty row at end
                    var row = tableBody.insertRow(rowLength + validFileCount);

                    // Insert new cells in the empty row
                    var cell1 = row.insertCell(0); //filename
                    var cell2 = row.insertCell(1); //version
                    var cell3 = row.insertCell(2); //prodID
                    var cell4 = row.insertCell(3); //events
                    var cell5 = row.insertCell(4); //props

                    // fill in each cell with the corresponding information (adding style attributes as necessary)
                    cell1.innerHTML = "<a href=\"./uploads/" + data[i][0].filename + "\">" + data[i][0].filename + "</a>";
                    cell2.innerHTML = data[i][1].version;
                    cell3.innerHTML = data[i][1].prodID;
                    cell3.setAttribute("class", "width-td td-wrap");
                    cell4.innerHTML = data[i][1].numEvents;
                    cell5.innerHTML = data[i][1].numProps;

                    validFileCount++; //accumulate

                } else {
                    // only print error messages if the passed function parameter says to
                    if(displayErrMsg) {
                        // display error msg for the current file
                        var textToAdd = "Error: iCalendar file '" + data[i][0].filename + "' is invalid, and thus won't be displayed (instead, it will be deleted from the server)";
                        addStatusJump(textToAdd);
                    }
                }
            } // end for loop

            // display error message if no files on server
            if(validFileCount <= 0) {

                console.log("'No files' should be displayed in file log panel");
                
                // get the table for file log panel and calc row length
                var tableBody = document.getElementById("tableFileLog").getElementsByTagName("tbody")[0];
                var rowLength = tableBody.rows.length;
                var row = tableBody.insertRow(rowLength);

                // Insert new cells in the empty row
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);

                // Add some default text to the new cells:
                cell1.innerHTML = "No files";
                cell2.innerHTML = "-";
                cell3.innerHTML = "-";
                cell4.innerHTML = "-";
                cell5.innerHTML = "-";

                // repopulate the drop down list
                repopulateDropDown("none");
            }
        },
        error: function(data) {
            // display error msg
            var textToAdd = "Error: Could not fill the 'File Log Calendar Panel' with initial data";
            addStatusJump(textToAdd);
        },
        fail: function(data) {
            // display error msg
            var textToAdd = "Failure: Could not fill the 'File Log Calendar Panel' with initial data";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Fills the View Calendar Panel's table with the corresponding data from the currently selected file in the drop down list
 * (NOTE: only called after repopulating the drop down list becuase it refreshes and validates the drop down list for onlt valid files from the server)
 * @param {String} fname -the file name of the iCal we want 
 */
function populateViewCalPanelTable(fname) {
    // check that the file name from is valid first, if it's not --> delete all bad file name options from the drop down list, fill with a valid one if it exists instead
    var len = fname.length;
    if( len < 5 || fname.charAt(len-4) != '.' || fname.charAt(len-3) != 'i' || fname.charAt(len-2) != 'c' || fname.charAt(len-1) != 's' ) {
        // display err msg
        if(fname != "") {
            var textToAdd = "Error: this is an invalid filename ('" + fname + "'), cannot fill 'View Calendar Panel' - must be a '.ics' file";
            addStatusJump(textToAdd);
        } else {
            console.log("populateViewCalPanelTable() Error: this is an invalid filename ('" + fname + "'), cannot fill 'View Calendar Panel' - must be a '.ics' file");
        }

        // get the calendar view panel drop down
        var selCalView = document.getElementById("selectCalView");
        var numOptions = selCalView.options.length;

        // loop through all the drop down list's options, delete all options with invalid file names
        for(var i = numOptions-1; i >= 0; i--) {
            var opt = selCalView.options[i].text;
            if(opt.length < 5 || opt.charAt(opt.length-4) != '.' || opt.charAt(opt.length-3) != 'i' || opt.charAt(opt.length-2) != 'c' || opt.charAt(opt.length-1) != 's' ) {
                selCalView.remove(i);
            }
        }

        // if the drop down list still has any options, fill table with first valid file name
        if(selCalView.options.length > 0) {
            selCalView.value = selCalView.options[0];
            populateViewCalPanelTable(selCalView.options[0]);
        } else {
            return; // leave function, don't change the contents of the view calendar panel's table
        }
    }

    /**
     * Populates View Calendar Panel table -this AJAX function executes
     * --> retrieves the event list info for the designated iCal file from the server
     * --> displays all of the iCal file's events' info in the table
     */
    $.ajax({
        type: "get",
        url: "/viewcal",
        dataType: "json",
        data: { filename : fname }, 
        success: function(data) {
            // get the table for calendar view panel and calc row length
            var tableBody = document.getElementById("tableCalView").getElementsByTagName("tbody")[0];
            var rowLength = tableBody.rows.length;

            // remove all rows from tbody first
            for(var i = rowLength-1; i >= 0; i--) {
                tableBody.deleteRow(i);
            }
            // recalculate row length after deleting
            rowLength = tableBody.rows.length;

            // loop through the event list and use it to populate the table
            for(var i = 0; i < data.length; i++) {
                // insert empty row at end
                var row = tableBody.insertRow(rowLength + i);

                // Insert new cells in the empty row
                var cell1 = row.insertCell(0); //eventNum
                var cell2 = row.insertCell(1); //start date
                var cell3 = row.insertCell(2); //start time
                var cell4 = row.insertCell(3); //summary
                var cell5 = row.insertCell(4); //props
                var cell6 = row.insertCell(5); //alarms

                // fill in each cell with the corresponding information (adding style attributes as necessary)
                cell1.innerHTML = "" + (i + 1);
                var startDate = data[i].startDT.date;
                cell2.innerHTML = startDate.substring(0, 4) + "-" + startDate.substring(4, 6) + "-" + startDate.substring(6, 8);
                var startTime = data[i].startDT.time;
                cell3.innerHTML = startTime.substring(0, 2) + ":" + startTime.substring(2, 4) + ":" + startTime.substring(4, 6);
                if(data[i].startDT.isUTC) {
                    cell3.innerHTML += " (UTC)";
                }
                cell4.innerHTML = data[i].summary;
                cell4.setAttribute("class", "left-align width-td td-wrap");

                // add a button with an event listener to show optional properties for the event via click
                cell5.innerHTML = "<form ref=\"extractoptpropsform" + (i+1) + "\" id=\"extractoptpropsform" + (i+1) + "\"> <div class=\"form-group\"> " + 
                        "<input type=\"button\" class=\"btn btn-link btn-block link-button no-background\" id=\"extractOptPropsButton" + (i+1) + 
                        "\" name=\"extractOptPropsButton\" value=\"" + data[i].numProps + "\"> </div> </form>";
                cell5.setAttribute("title", "(Click to View)");
                addExtractOptPropsButtonListener(i+1);

                // add a button with an event listener to show alarms for the event via click
                cell6.innerHTML = "<form ref=\"showalarmsform" + (i+1) + "\" id=\"showalarmsform" + (i+1) + "\"> <div class=\"form-group\"> " + 
                        "<input type=\"button\" class=\"btn btn-link btn-block link-button no-background\" id=\"showAlarmsButton" + (i+1) + 
                        "\" name=\"showAlarmsButton\" value=\"" + data[i].numAlarms + "\"> </div> </form>";
                cell6.setAttribute("title", "(Click to View)");
                addShowAlarmsButtonListener(i+1);
            } // end for loop
        },
        error: function(error) {
            // display error msg if it's an empty filename
            if(fname != "") {
                var textToAdd = "Error: Could not add the events from calendar '" + fname + "' to the 'View Calendar Panel'";
                addStatusJump(textToAdd);
            } else {
                console.log("Error: Could not add the events from calendar '" + fname + "' to the 'View Calendar Panel'");
            }            
        },
        fail: function(error) {
            // display error msg if it's an empty filename
            if(fname != "") {
                var textToAdd = "Failure: Could not add the events from calendar '" + fname + "' to the 'View Calendar Panel'";
                addStatusJump(textToAdd);
            } else {
                console.log("Failure: Could not add the events from calendar '" + fname + "' to the 'View Calendar Panel'");
            }    
        }
    });
}

/**
 * Refills the View Calendar Panel's drop down list with the corresponding data from the server, then updates the table
 * @param {String} fnameToDisplay -the filename of the iCal we want to display (if it's valid)
 */
function repopulateDropDown(fnameToDisplay) {
    /**
     * Populates View Calendar Panel drop down -this AJAX function executes
     * --> retrieves the list of files on the server
     * --> displays all valid iCal file's as options in the drop down list
     */
    $.ajax({
        type: 'get',
        url: '/filelogtable',
        success: function (data) {
            // get the drop down list
            var selCalView = document.getElementById("selectCalView");

            // remove all current options first
            var len = selCalView.options.length;
            for (var i = len-1; i >= 0; i--) {
                selCalView.remove(i);
            }

            // declare variables
            var foundValid = false;
            var fnameToDisplayValid = false;
            var firstValidFile = "";

            // add each filename to the drop down as an option only if the iCal file is valid
            for (var i = 0; i < data.length; i++) {
                // check if the file is valid
                if( data[i][0].valid ) {
                    // see if the desired filename is valid
                    if(data[i][0].filename === fnameToDisplay) {
                        fnameToDisplayValid = true;
                    }
                    // get first valid file name
                    if(!foundValid) {
                        firstValidFile = data[i][0].filename;
                    }
                    foundValid = true;

                    // cal view drop down list, add it to the end of the list
                    var newOption = document.createElement("OPTION");
                    newOption.text = data[i][0].filename;
                    newOption.value = data[i][0].filename;
                    selCalView.add(newOption);
                } /*else { // don't need to display error message here (populating the file log panel will, so here it would be duplicating extra messages)
                    // append new text onto the status display panel
                    var textToAdd = "Error: iCalendar file '" + data[i][0].filename + "' is invalid, and thus won't be displayed";
                    addStatus(textToAdd);
                    // jumps to top of page automatically
                    $(window).scrollTop(0);
                }*/
            }

            // fill in the view calendar panel's table with events, if it exists
            // if there are no valid files on the server, disable the add event button
            // also, do the same for the store all files button
            if(fnameToDisplayValid) {
                document.getElementById("addEventButton").disabled = false;
                document.getElementById("storeAllFilesButton").disabled = false;
                populateViewCalPanelTable(fnameToDisplay);
                selCalView.value = fnameToDisplay;
            } else if(foundValid) {
                document.getElementById("addEventButton").disabled = false;
                document.getElementById("storeAllFilesButton").disabled = false;
                populateViewCalPanelTable(firstValidFile);
                selCalView.value = firstValidFile;
            } else {
                document.getElementById("addEventButton").disabled = true;
                document.getElementById("storeAllFilesButton").disabled = true;
                selCalView.value = ""; //reset
            }
        },
        error: function(error) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve files from server - " + error;
            addStatusJump(textToAdd);
        },
        fail: function(error) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve files from server - " + error;
            addStatusJump(textToAdd);
        }
    });
}

// ==================================================== AJAX A4 HELPER FUNCTIONS ====================================================

/**
 * Stores all uploaded iCal files in the database
 */
function storeAllFilesInDB() {
    /**
     * Stores all uploaded iCal files in the database -this AJAX function executes
     * --> queries the database and inserts each iCal file as a row in the database (FILE table) if it doesn't already exist
     * --> of course, subsequently inserting appropritate rows in the EVENT and ALARM tables as well
     */
    $.ajax({
        type: 'get',
        url: '/dbstore',
        success: function(data) {
            // display success msg
            var textToAdd = "Successfully stored all uploaded files in the database";
            addStatusJump(textToAdd);
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to store all uploaded files in the database";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to store all uploaded files in the database";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Clears all rows in each table in the database
 */
function clearAllDBTableData() {
    /**
     * Clears all rows in each table in the database -this AJAX function executes
     * --> queries the database and deletes all rows from FILE table, it will cascade and delete all rows in the other tables too
     */
    $.ajax({
        type: 'get',
        url: '/dbclear',
        dataType: 'json',
        success: function (data) {
            // make sure it worked
            if(data.clearSuccess) {
                // display success msg in Status Panel
                var textToAdd = "All rows were cleared from the database successfully";
                addStatusJump(textToAdd);
            } else {
                // display error msg
                var textToAdd = "Oops, unexpected issue: Unable to clear all rows from the database";
                addStatusJump(textToAdd);
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to clear all rows from the database - " + err;
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to clear all rows from the database - " + err;
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Retrieves and displays the database status info in the Status Panel
 */
function getDBStatus() {
    /**
     * Retrieves and displays the database status info in the Status Panel -this AJAX function executes
     * --> queries the database to get #of files, events, and alarms from each corresponding table
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbstatus',
        dataType: 'json',
        success: function (data) {
            // display database status in Status Panel
            //var textToAdd = "Display DB Status Button clicked";
            addStatusJump(data.dbStatusStr);
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve database status - " + err;
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve database status - " + err;
            addStatusJump(textToAdd);
        }
    });
}

// ==================================================== AJAX A4 EXECUTE QUERY FUNCTIONS ====================================================

/**
 * Queries the database and retrieves all events in the EVENT table sorted by start date-time
 */
function queryDB1() {
    /**
     * Retrieves and displays all events in the database sorted by start date-time -this AJAX function executes
     * --> queries the database to get all rows from the EVENT table
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery1',
        dataType: 'json',
        success: function (data) {
            // data sent back is just 'rows' for the EVENT table query ordered by start datetime

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            newTable.setAttribute("class", "event-query-table");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>event_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th colspan=\"3\">summary</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th colspan=\"2\">start_time</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th colspan=\"2\">location</th>";
            var headCell5 = headRow.insertCell(4).outerHTML = "<th colspan=\"2\">organizer</th>";
            var headCell6 = headRow.insertCell(5).outerHTML = "<th>cal_file</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);

            /*// display output header in status panel
            var textToAdd = "[All EVENTs ordered by start date-time]:"
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);
                var cell6 = row.insertCell(5);

                // add default text to cells
                cell1.innerHTML = "No events";
                cell2.innerHTML = "-";
                cell2.setAttribute("colspan", "3");
                cell3.innerHTML = "-";
                cell3.setAttribute("colspan", "2");
                cell4.innerHTML = "-";
                cell4.setAttribute("colspan", "2");
                cell5.innerHTML = "-";
                cell5.setAttribute("colspan", "2");
                cell6.innerHTML = "-";
                
                /*// add default msg
                var textToAdd = "--no events--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    cell2.setAttribute("class", "left-align width-triple-btn td-wrap");
                    cell2.setAttribute("colspan", "3");
                    var cell3 = bodyRow.insertCell(2);
                    cell3.setAttribute("class", "width-dt td-wrap");
                    cell3.setAttribute("colspan", "2");
                    var cell4 = bodyRow.insertCell(3);
                    cell4.setAttribute("class", "width-td td-wrap");
                    cell4.setAttribute("colspan", "2");
                    var cell5 = bodyRow.insertCell(4);
                    cell5.setAttribute("class", "width-td td-wrap");
                    cell5.setAttribute("colspan", "2");
                    var cell6 = bodyRow.insertCell(5);

                    // add text to cells
                    cell1.innerHTML = "" + row.event_id;
                    if(row.summary != null) cell2.innerHTML = "" + row.summary;
                    cell3.innerHTML = "" + row.start_time;
                    if(row.location != null) cell4.innerHTML = "" + row.location;
                    if(row.organizer != null) cell5.innerHTML = "" + row.organizer;
                    cell6.innerHTML = "" + row.cal_file;

                    /*// summary, start_time, location, organizer
                    textToAdd = "("+ (i+1) + ") Summary: " + row.summary + "\t|\tStart Date-Time: " + row.start_time + 
                            "\t|\tLocation: " + row.location + "\t|\tOrganizer: " + row.organizer + "\t|\tCal Ref Number: " + row.cal_file;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all events ordered by start date";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all events ordered by start date";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Queries the database and retrieves all events for the specified iCal file
 */
function queryDB2() {
    // get the inputted filename to search the database
    var txtQueryInput = document.getElementById("txtQueryInput");

    // validate the filename first
    var len = txtQueryInput.value.length;
    if( len <= 4 || txtQueryInput.value[len-4] != '.' || txtQueryInput.value[len-3] != 'i' || txtQueryInput.value[len-2] != 'c' || txtQueryInput.value[len-1] != 's') {
        // display error msg, before leaving early
        var textToAdd = "Oops: please enter a '.ics' filename in the seaerch query field";
        addStatusJump(textToAdd);
        return;
    } else if( len > 60 ) {
        // display error msg, before leaving early
        var textToAdd = "Oops: filename cannot be longer than 60 characters, please try again";
        addStatusJump(textToAdd);
        return;
    }
    
    /**
     * Retrieves and displays all events in the database for the specified iCal file -this AJAX function executes
     * --> queries the database to get all rows from the EVENT table for the specified iCal file
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery2',
        dataType: 'json',
        data: { filename: txtQueryInput.value},
        success: function (data) {
            // data sent back is just 'rows' for the EVENT table query

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            newTable.setAttribute("class", "event-query-table");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>event_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th colspan=\"3\">summary</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th colspan=\"2\">start_time</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th colspan=\"2\">location</th>";
            var headCell5 = headRow.insertCell(4).outerHTML = "<th colspan=\"2\">organizer</th>";
            var headCell6 = headRow.insertCell(5).outerHTML = "<th>cal_file</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);
            
            /*// display output header in status panel
            var textToAdd = "[All EVENTs for the file '" + txtQueryInput.value + "']:";
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);
                var cell6 = row.insertCell(5);

                // add default text to cells
                cell1.innerHTML = "No events for filename '" + txtQueryInput.value + "'";
                cell2.innerHTML = "-";
                cell2.setAttribute("colspan", "3");
                cell3.innerHTML = "-";
                cell3.setAttribute("colspan", "2");
                cell4.innerHTML = "-";
                cell4.setAttribute("colspan", "2");
                cell5.innerHTML = "-";
                cell5.setAttribute("colspan", "2");
                cell6.innerHTML = "-";

                /*// add default msg
                var textToAdd = "--no events for filename '" + txtQueryInput.value + "'--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    cell2.setAttribute("class", "left-align width-triple-btn td-wrap");
                    cell2.setAttribute("colspan", "3");
                    var cell3 = bodyRow.insertCell(2);
                    cell3.setAttribute("class", "width-dt td-wrap");
                    cell3.setAttribute("colspan", "2");
                    var cell4 = bodyRow.insertCell(3);
                    cell4.setAttribute("class", "width-td td-wrap");
                    cell4.setAttribute("colspan", "2");
                    var cell5 = bodyRow.insertCell(4);
                    cell5.setAttribute("class", "width-td td-wrap");
                    cell5.setAttribute("colspan", "2");
                    var cell6 = bodyRow.insertCell(5);

                    // add text to cells
                    cell1.innerHTML = "" + row.event_id;
                    if(row.summary != null) cell2.innerHTML = "" + row.summary;
                    cell3.innerHTML = "" + row.start_time;
                    if(row.location != null) cell4.innerHTML = "" + row.location;
                    if(row.organizer != null) cell5.innerHTML = "" + row.organizer;
                    cell6.innerHTML = "" + row.cal_file;

                    /*// summary, start_time, location, organizer
                    textToAdd = "("+ (i+1) + ") Summary: " + row.summary + "\t|\tStart Date-Time: " + row.start_time + 
                            "\t|\tLocation: " + row.location + "\t|\tOrganizer: " + row.organizer + "\t|\tCal Ref Number: " + row.cal_file;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all events for the file '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all events for the file '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Queries the database and retrieves all potentially conflicting events in the EVENT table
 */
function queryDB3() {
    /**
     * Retrieves and displays all potentially conflicting events in the database -this AJAX function executes
     * --> queries the database to get all rows from the EVENT table that have the same start date-time
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery3',
        dataType: 'json',
        success: function (data) {
            // data sent back is just 'rows' for the EVENT table query ordered by start datetime

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            newTable.setAttribute("class", "event-query-table");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>event_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th colspan=\"3\">summary</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th colspan=\"2\">start_time</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th colspan=\"2\">location</th>";
            var headCell5 = headRow.insertCell(4).outerHTML = "<th colspan=\"2\">organizer</th>";
            var headCell6 = headRow.insertCell(5).outerHTML = "<th>cal_file</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);
            
            /*// display output header in status panel
            var textToAdd = "[All conflicting EVENTs]:";
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);
                var cell6 = row.insertCell(5);

                // add default text to cells
                cell1.innerHTML = "No potentially conflicting events";
                cell2.innerHTML = "-";
                cell2.setAttribute("colspan", "3");
                cell3.innerHTML = "-";
                cell3.setAttribute("colspan", "2");
                cell4.innerHTML = "-";
                cell4.setAttribute("colspan", "2");
                cell5.innerHTML = "-";
                cell5.setAttribute("colspan", "2");
                cell6.innerHTML = "-";

                /*// add default msg
                var textToAdd = "--no conflicting events--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    cell2.setAttribute("class", "left-align width-triple-btn td-wrap");
                    cell2.setAttribute("colspan", "3");
                    var cell3 = bodyRow.insertCell(2);
                    cell3.setAttribute("class", "width-dt td-wrap");
                    cell3.setAttribute("colspan", "2");
                    var cell4 = bodyRow.insertCell(3);
                    cell4.setAttribute("class", "width-td td-wrap");
                    cell4.setAttribute("colspan", "2");
                    var cell5 = bodyRow.insertCell(4);
                    cell5.setAttribute("class", "width-td td-wrap");
                    cell5.setAttribute("colspan", "2");
                    var cell6 = bodyRow.insertCell(5);

                    // add text to cells
                    cell1.innerHTML = "" + row.event_id;
                    if(row.summary != null) cell2.innerHTML = "" + row.summary;
                    cell3.innerHTML = "" + row.start_time;
                    if(row.location != null) cell4.innerHTML = "" + row.location;
                    if(row.organizer != null) cell5.innerHTML = "" + row.organizer;
                    cell6.innerHTML = "" + row.cal_file;
                    
                    /*// summary, start_time, location, organizer
                    textToAdd = "("+ (i+1) + ") Summary: " + row.summary + "\t|\tStart Date-Time: " + row.start_time + 
                            "\t|\tLocation: " + row.location + "\t|\tOrganizer: " + row.organizer + "\t|\tCal Ref Number: " + row.cal_file;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all conflicting events";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all conflicting events";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Queries the database and retrieves all alarms with the given action
 */
function queryDB4() {
    // get the inputted action to search the database
    var txtQueryInput = document.getElementById("txtQueryInput");

    // validate the action first (just needs to be at least one char)
    if(txtQueryInput.value.length < 1) {
        // display error msg, before leaving early
        var textToAdd = "Oops: please enter an alarm action in the seaerch query field";
        addStatusJump(textToAdd);
        return;
    } else if( txtQueryInput.value.length > 256 ) {
        // display error msg, before leaving early
        var textToAdd = "Oops: action cannot be longer than 256 characters, please try again";
        addStatusJump(textToAdd);
        return;
    }

    /**
     * Retrieves and displays all alarms in the database with the given action -this AJAX function executes
     * --> queries the database to get all rows from the ALARM table that have the specified action
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery4',
        dataType: 'json',
        data: { action: txtQueryInput.value},
        success: function (data) {
            // data sent back is just 'rows' for the ALARM table query

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>alarm_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th>action</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th>trigger</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th>event</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);
            
            /*// display output header in status panel
            var textToAdd = "[All ALARMs with action '" + txtQueryInput.value + "']:";
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);

                // add default text to cells
                cell1.innerHTML = "No alarms with action '" + txtQueryInput.value + "'";
                cell2.innerHTML = "-";
                cell3.innerHTML = "-";
                cell4.innerHTML = "-";

                /*// add default msg
                var textToAdd = "--no alarms with action '" + txtQueryInput.value + "'--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    var cell3 = bodyRow.insertCell(2);
                    var cell4 = bodyRow.insertCell(3);

                    // add text to cells
                    cell1.innerHTML = "" + row.alarm_id;
                    cell2.innerHTML = "" + row.action;
                    cell3.innerHTML = "" + row.trigger;
                    cell4.innerHTML = "" + row.event;
                    
                    /*// action, trigger, event ref number
                    textToAdd = "("+ (i+1) + ") Action: " + row.action + "\t|\tTrigger: " + row.trigger+ "\t|\tEvent Ref Number: " + row.event;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all alarms with the action '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all alarms with the action '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Queries the database and retrieves all events with the given location
 */
function queryDB5() {
    // get the inputted action to search the database
    var txtQueryInput = document.getElementById("txtQueryInput");

    // validate the location first (just needs to be at least one char)
    if(txtQueryInput.value.length < 1) {
        // display error msg, before leaving early
        var textToAdd = "Oops: please enter an event location in the seaerch query field";
        addStatusJump(textToAdd);
        return;
    } else if( txtQueryInput.value.length > 60 ) {
        // display error msg, before leaving early
        var textToAdd = "Oops: location cannot be longer than 60 characters, please try again";
        addStatusJump(textToAdd);
        return;
    }

    /**
     * Retrieves and displays all events in the database with the given location -this AJAX function executes
     * --> queries the database to get all rows from the EVENT table that have the specified location
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery5',
        dataType: 'json',
        data: { location: txtQueryInput.value},
        success: function (data) {
            // data sent back is just 'rows' for the EVENT table query

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            newTable.setAttribute("class", "event-query-table");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>event_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th colspan=\"3\">summary</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th colspan=\"2\">start_time</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th colspan=\"2\">location</th>";
            var headCell5 = headRow.insertCell(4).outerHTML = "<th colspan=\"2\">organizer</th>";
            var headCell6 = headRow.insertCell(5).outerHTML = "<th>cal_file</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);
            
            /*// display output header in status panel
            var textToAdd = "[All EVENTs with location '" + txtQueryInput.value + "']:";
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);
                var cell6 = row.insertCell(5);

                // add default text to cells
                cell1.innerHTML = "No events with location '" + txtQueryInput.value + "'";
                cell2.innerHTML = "-";
                cell2.setAttribute("colspan", "3");
                cell3.innerHTML = "-";
                cell3.setAttribute("colspan", "2");
                cell4.innerHTML = "-";
                cell4.setAttribute("colspan", "2");
                cell5.innerHTML = "-";
                cell5.setAttribute("colspan", "2");
                cell6.innerHTML = "-";

                /*// add default msg
                var textToAdd = "--no events with location '" + txtQueryInput.value + "'--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    cell2.setAttribute("class", "left-align width-triple-btn td-wrap");
                    cell2.setAttribute("colspan", "3");
                    var cell3 = bodyRow.insertCell(2);
                    cell3.setAttribute("class", "width-dt td-wrap");
                    cell3.setAttribute("colspan", "2");
                    var cell4 = bodyRow.insertCell(3);
                    cell4.setAttribute("class", "width-td td-wrap");
                    cell4.setAttribute("colspan", "2");
                    var cell5 = bodyRow.insertCell(4);
                    cell5.setAttribute("class", "width-td td-wrap");
                    cell5.setAttribute("colspan", "2");
                    var cell6 = bodyRow.insertCell(5);

                    // add text to cells
                    cell1.innerHTML = "" + row.event_id;
                    if(row.summary != null) cell2.innerHTML = "" + row.summary;
                    cell3.innerHTML = "" + row.start_time;
                    if(row.location != null) cell4.innerHTML = "" + row.location;
                    if(row.organizer != null) cell5.innerHTML = "" + row.organizer;
                    cell6.innerHTML = "" + row.cal_file;
                    
                    /*// summary, start_time, location, organizer
                    textToAdd = "("+ (i+1) + ") Summary: " + row.summary + "\t|\tStart Date-Time: " + row.start_time + 
                            "\t|\tLocation: " + row.location + "\t|\tOrganizer: " + row.organizer + "\t|\tCal Ref Number: " + row.cal_file;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all events with the location '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all events with the location '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        }
    });
}

/**
 * Queries the database and retrieves all events with the given organizer
 */
function queryDB6() {
    // get the inputted action to search the database
    var txtQueryInput = document.getElementById("txtQueryInput");

    // validate the organizer first (just needs to be at least one char)
    if(txtQueryInput.value.length < 1) {
        // display error msg, before leaving early
        var textToAdd = "Oops: please enter an event organizer in the seaerch query field";
        addStatusJump(textToAdd);
        return;
    } else if( txtQueryInput.value.length > 256 ) {
        // display error msg, before leaving early
        var textToAdd = "Oops: organizer cannot be longer than 256 characters, please try again";
        addStatusJump(textToAdd);
        return;
    }

    /**
     * Retrieves and displays all events in the database with the given organizer -this AJAX function executes
     * --> queries the database to get all rows from the EVENT table that have the specified organizer
     * --> displays this info in the Status Panel
     */
    $.ajax({
        type: 'get',
        url: '/dbquery6',
        dataType: 'json',
        data: { organizer: txtQueryInput.value},
        success: function (data) {
            // data sent back is just 'rows' for the EVENT table query

            console.log(data);

            // get the panel for showing query results
            var tableDiv = document.getElementById("divQueryTable");

            // remove the current table first, then create the table, head, body, and header row
            tableDiv.children[0].remove();

            var newTable = document.createElement("TABLE");
            newTable.setAttribute("id", "queryTable");
            newTable.setAttribute("class", "event-query-table");
            tableDiv.appendChild(newTable);

            // create the table head and header row
            var newTableHead = document.createElement("THEAD");
            newTable.appendChild(newTableHead);

            var headRow = newTableHead.insertRow(0);
            var headCell1 = headRow.insertCell(0).outerHTML = "<th>event_id</th>";
            var headCell2 = headRow.insertCell(1).outerHTML = "<th colspan=\"3\">summary</th>";
            var headCell3 = headRow.insertCell(2).outerHTML = "<th colspan=\"2\">start_time</th>";
            var headCell4 = headRow.insertCell(3).outerHTML = "<th colspan=\"2\">location</th>";
            var headCell5 = headRow.insertCell(4).outerHTML = "<th colspan=\"2\">organizer</th>";
            var headCell6 = headRow.insertCell(5).outerHTML = "<th>cal_file</th>";

            var newTableBody = document.createElement("TBODY");
            newTable.appendChild(newTableBody);
            
            /*// display output header in status panel
            var textToAdd = "[All EVENTs with organizer '" + txtQueryInput.value + "']:";
            addStatus(textToAdd);*/

            // check if it found any
            if(data.length <= 0) {
                // add default empty table row data
                var row = newTableBody.insertRow(0);
                var cell1 = row.insertCell(0);
                var cell2 = row.insertCell(1);
                var cell3 = row.insertCell(2);
                var cell4 = row.insertCell(3);
                var cell5 = row.insertCell(4);
                var cell6 = row.insertCell(5);

                // add default text to cells
                cell1.innerHTML = "No events with organizer '" + txtQueryInput.value + "'";
                cell2.innerHTML = "-";
                cell2.setAttribute("colspan", "3");
                cell3.innerHTML = "-";
                cell3.setAttribute("colspan", "2");
                cell4.innerHTML = "-";
                cell4.setAttribute("colspan", "2");
                cell5.innerHTML = "-";
                cell5.setAttribute("colspan", "2");
                cell6.innerHTML = "-";

                /*// add default msg
                var textToAdd = "--no events with organizer '" + txtQueryInput.value + "'--";
                addStatus(textToAdd);*/

            } else {
                // loop through and print each row to the status panel
                var i = 0;
                for(let row of data) {
                    // add a row to the table
                    var bodyRow = newTableBody.insertRow(i);
                    var cell1 = bodyRow.insertCell(0);
                    var cell2 = bodyRow.insertCell(1);
                    cell2.setAttribute("class", "left-align width-triple-btn td-wrap");
                    cell2.setAttribute("colspan", "3");
                    var cell3 = bodyRow.insertCell(2);
                    cell3.setAttribute("class", "width-dt td-wrap");
                    cell3.setAttribute("colspan", "2");
                    var cell4 = bodyRow.insertCell(3);
                    cell4.setAttribute("class", "width-td td-wrap");
                    cell4.setAttribute("colspan", "2");
                    var cell5 = bodyRow.insertCell(4);
                    cell5.setAttribute("class", "width-td td-wrap");
                    cell5.setAttribute("colspan", "2");
                    var cell6 = bodyRow.insertCell(5);

                    // add text to cells
                    cell1.innerHTML = "" + row.event_id;
                    if(row.summary != null) cell2.innerHTML = "" + row.summary;
                    cell3.innerHTML = "" + row.start_time;
                    if(row.location != null) cell4.innerHTML = "" + row.location;
                    if(row.organizer != null) cell5.innerHTML = "" + row.organizer;
                    cell6.innerHTML = "" + row.cal_file;
                    
                    /*// summary, start_time, location, organizer
                    textToAdd = "("+ (i+1) + ") Summary: " + row.summary + "\t|\tStart Date-Time: " + row.start_time + 
                            "\t|\tLocation: " + row.location + "\t|\tOrganizer: " + row.organizer + "\t|\tCal Ref Number: " + row.cal_file;
                    addStatus(textToAdd);*/

                    i++;
                }
            }
        },
        error: function(err) {
            // display error msg
            var textToAdd = "Error: Unable to retrieve all events with the organizer '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        },
        fail: function(err) {
            // display error msg
            var textToAdd = "Failure: Unable to retrieve all events with the organizer '" + txtQueryInput.value + "' (doesn't exist)";
            addStatusJump(textToAdd);
        }
    });
}