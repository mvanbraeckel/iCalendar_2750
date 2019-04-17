'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

// enable stuff
app.use(fileUpload());
const bodyParser = require("body-parser");
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Set up functions from shared library
let sharedLib = ffi.Library('./sharedLib', {
    //NOTE: return type first, argument list second (leave empty if no parameters are taken)
    'a3getFileLogPanelTableInfo' : [ 'string', [ 'string' ] ],
    'a3getEventList' : [ 'string', [ 'string' ] ],
    'a3getPropertyList' : [ 'string', [ 'string', 'int' ] ],
    'a3getAlarmList' : [ 'string', [ 'string', 'int' ] ],
    'a3CreateCalendar' : [ 'string', [ 'string', 'string', 'string' ] ],
    'a3AddEventToCalendar' : [ 'string', [ 'string', 'string' ] ],
});

// for A4 database
const mysql = require('mysql');
var connection = null;

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
    res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
    //Feel free to change the contents of style.css to prettify your Web app
    res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
    fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
    if(!req.files) {
        return res.status(400).send('No files were uploaded.');
    }
    
    let uploadFile = req.files.uploadFile;

    // Use the mv() method to place the file somewhere on your server
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
        if(err) {
            return res.status(500).send(err);
        }

        res.redirect('/');
    });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
    fs.stat('uploads/' + req.params.name, function(err, stat) {
        console.log(err);
        if(err == null) {
            res.sendFile(path.join(__dirname+'/uploads/' + req.params.name)); // what i am sending
        } else {
            res.send(''); // var type here eg. 'JSON'
        }
    });
});

//******************** Your code goes here ******************** 

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

// ==================================================== GET ====================================================

/**
 * Retrieves a list of all uploaded files on the server
 * @param res -the list of all uploaded files on the server
 */
app.get("/getuploadfilelist", function(req, res) {
    let list = fs.readdirSync("./uploads/");
    res.send(JSON.parse(JSON.stringify(list)));
});

/**
 * Used to populate the File Log Table
 * @param res -an array of JSONs (each one contains the filename, if it's a valid file, and an iCal's summary info for a row in the table)
 */
app.get('/filelogtable', function(req, res) {
    let fileList = fs.readdirSync("./uploads/");
    let calSummaryJSONs = [];

    for(var i = 0; i < fileList.length; i++) {
        var rowJSONstr = sharedLib.a3getFileLogPanelTableInfo("./uploads/" + fileList[i]);

        // make sure file was valid
        if(rowJSONstr != null) {
            var rowJSON = [ { filename: fileList[i], valid: true }, JSON.parse(rowJSONstr) ];
            calSummaryJSONs.push(rowJSON);
        } else {
            calSummaryJSONs.push( [ {filename: fileList[i], valid: false} ] );
            // delete the invalid file
            fs.unlinkSync("./uploads/" + fileList[i]);
        }        
    }
    res.send( calSummaryJSONs );
});

/**
 * Used to populate the View Calendar Table
 * @param req -a JSON containing just the desired iCal filename
 * @param res -an array of JSONs that represents the event list info from the desired iCal file (each one is the overview info for a row in the table)
 */
app.get('/viewcal', function(req, res) {
    var evListJSONstr = sharedLib.a3getEventList("./uploads/" + req.query.filename);

    // make sure file was valid
    if(evListJSONstr != null) {
        res.send( JSON.parse(evListJSONstr) );
    } else {
        res.status(400).send("Error: '" + req.query.filename + "' is invalid.");
    }
});

/**
 * Retrieves an event's property list
 * @param req -a JSON containing just the desired iCal filename and the index of the desired event in the iCal's event list
 * @param res -an array of JSONs that represents the property list info from the desired iCal file's specified event (each one is the summary info for the status panel)
 */
app.get('/vieweventprops', function(req, res) {
    var propListJSONstr = sharedLib.a3getPropertyList("./uploads/" + req.query.filename, req.query.eventNum);

    // make sure file was valid
    if(propListJSONstr != null) {
        res.send( JSON.parse(propListJSONstr) );
    } else {
        res.status(400).send("Error: '" + req.query.filename + "' is invalid.");
    }
});

/**
 * Retrieves an event's alarm list
 * @param req -a JSON containing just the desired iCal filename and the index of the desired event in the iCal's event list
 * @param res -an array of JSONs that represents the alarm list info from the desired iCal file's specified event (each one is the summary info for the status panel)
 */
app.get('/vieweventalarms', function(req, res) {
    var almListJSONstr = sharedLib.a3getAlarmList("./uploads/" + req.query.filename, req.query.eventNum);

    // make sure file was valid
    if(almListJSONstr != null) {
        res.send( JSON.parse(almListJSONstr) );
    } else {
        res.status(400).send("Error: '" + req.query.filename + "' is invalid.");
    }
});

// =================================================== A4 GET ===================================================

/**
 * Establishes a connection to the database using the given login credentials
 * @param req -a JSON containing the given login credentials
 * @param res -a JSON containing a boolean signalling if it was successful or not
 */
app.get('/dblogin', function(req, res) {
    // establish connection to database
    connection = mysql.createConnection({
        host     : 'dursley.socs.uoguelph.ca',
        user     :  req.query.USERNAME,
        password :  req.query.PASSWORD,
        database :  req.query.DBNAME
    });

    console.log(req.query);

    // actually connect, account for errors that may occur
    connection.connect( function(err) {
        if(err) {
            console.log("login failed!");
            res.status(400).send( "Login unsuccessful: database '" + req.query.DBNAME + "' login was unsuccesful for '" + req.query.USERNAME + "': " + err );
        } else {
            console.log("login successful!");
            // create the database tables if they don't exist yet
            createAllDBTables();
            res.send( { loginSuccess: true } );
        }
    });
});

/**
 * Stores all uploaded files in the database
 */
app.get('/dbstore', function(req, res) {
    // ----------------------------- first store all the FILE table stuff -----------------------------
    let fileList = fs.readdirSync("./uploads/");
    let calSummaryJSONs = [];
    let evSummaryJSONs = [];

    var fileListIndex = 0;
    // loop through each iCal file's summary info and store each as a row in the database's FILE table
    for(var i = 0; i < fileList.length; i++) {
        var rowJSONcalStr = sharedLib.a3getFileLogPanelTableInfo("./uploads/" + fileList[i]);

        // make sure file was valid
        if(rowJSONcalStr != null) {
            var rowJSONcal = [ { filename: fileList[i], valid: true }, JSON.parse(rowJSONcalStr) ];
            calSummaryJSONs.push(rowJSONcal);

            // store it as a row in the database: FILE table
            var insertIntoFILEcmd = "INSERT INTO FILE (file_Name, version, prod_id) " +
                    "SELECT '" + rowJSONcal[0].filename + "', '" + rowJSONcal[1].version + "', '" + rowJSONcal[1].prodID + "' FROM DUAL " +
                    "WHERE NOT EXISTS( SELECT * FROM FILE WHERE file_Name = '" + rowJSONcal[0].filename + "' ) LIMIT 1;";

            connection.query(insertIntoFILEcmd, function (err, rows, fields) {
                if(err) {
                    console.log("Error: something went wrong while storing info in the FILE table" + err);
                } else {
                    // ----------------------------- second, store all the EVENT table stuff for the current iCal file -----------------------------
                    // get the auto-incremented cal id of the iCal file just added to the FILE table
                    var calFileID = rows.insertId;

                    console.log("calFileID = " + calFileID);
                    console.log("current file: fileListIndex = " + fileListIndex + " | file = '" + fileList[fileListIndex] + "'");

                    // an array of --> "{\"startDT\":DTval,\"numProps\":propVal,\"numAlarms\":almVal,\"summary\":\"sumVal\"}"
                    var evListJSONstr = sharedLib.a3getEventList("./uploads/" + fileList[fileListIndex]);
                    
                    //console.log("evListJSONstr = " + evListJSONstr);

                    // make sure file was valid
                    if(evListJSONstr != null) {
                        var evListJSON = JSON.parse(evListJSONstr); // parse it first

                        var evListIndex = 0;
                        // loop through each event for the current iCal file, store each as a row in the database's EVENT table
                        for(var j = 0; j < evListJSON.length; j++) {

                            //console.log("evListJSON[j] stuff (@j=" + j + ")= " + evListJSON[j].startDT + " | " + evListJSON[j].numProps + " | " + evListJSON[j].numAlarms + " | " + evListJSON[j].summary);
                            
                            // create a JSON (array) to store the filename/validity, event info, and location/organizer
                            var rowJSONev = [ { filename: fileList[fileListIndex], valid: true }, evListJSON[j], { LOCATION: null, ORGANIZER: null } ];

                            // check if location or organizer exists for the current event (need property list)
                            var propListJSONstr = sharedLib.a3getPropertyList("./uploads/" + fileList[fileListIndex], j);

                            //console.log("propListJSONstr = " + propListJSONstr);

                            // make sure file was valid
                            if(propListJSONstr != null) {
                                var propListJSON = JSON.parse(propListJSONstr); // parse it first

                                // loop through each property checking for location and organizer, set it in the JSON if match is found
                                for(var k = 0; k < propListJSON.length; k++) {
                                    if(propListJSON[k].propName.toUpperCase() == "LOCATION") {
                                        rowJSONev[2].LOCATION = "'" + propListJSON[k].propDescr + "'";
                                    } else if(propListJSON[k].propName.toUpperCase() == "ORGANIZER") {
                                        rowJSONev[2].ORGANIZER = "'" + propListJSON[k].propDescr + "'";
                                    }
                                }

                                // set summary to null if it's an empty string
                                var summaryValue = rowJSONev[1].summary;
                                if(rowJSONev[1].summary == "") {
                                    summaryValue = null;
                                } else {
                                    summaryValue = "'" + rowJSONev[1].summary + "'";
                                }

                                // get the start time of the event into the proper format
                                var startDateTime = "";
                                var startDate = rowJSONev[1].startDT.date;
                                startDateTime = startDate.substring(0, 4) + "-" + startDate.substring(4, 6) + "-" + startDate.substring(6, 8);
                                var startTime = rowJSONev[1].startDT.time;
                                startDateTime += " " + startTime.substring(0, 2) + ":" + startTime.substring(2, 4) + ":" + startTime.substring(4, 6);

                                // store it as a row in the database: EVENT table
                                var insertIntoEVENTcmd = "INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) " +
                                        "VALUES( " + summaryValue + ", '" + startDateTime + "', " + rowJSONev[2].LOCATION + ", " + 
                                        rowJSONev[2].ORGANIZER + ", '" + calFileID + "' );";
                                
                                connection.query(insertIntoEVENTcmd, function (err, rows, fields) {
                                    if(err) {
                                        console.log("Error: something went wrong while storing info in the EVENT table" + err);
                                    } else {

                                        console.log("my event table insert: " + insertIntoEVENTcmd);

                                        // ----------------------------- third, store all the ALARM table stuff for the current event of the current iCal file -----------------------------
                                        // get the auto-incremented event id of the event just added to the EVENT table
                                        var eventID = rows.insertId;

                                        console.log("eventID = " + eventID);
                                        console.log("filename = " + rowJSONev[0].filename + " | event number = " + evListIndex);

                                        // an array of --> "{\"action\":\"actVal\",\"trigger\":\"trigVal\",\"numProps\":propval}"
                                        var almListJSONstr = sharedLib.a3getAlarmList("./uploads/" + rowJSONev[0].filename, evListIndex);
                                        
                                        //console.log("almListJSONstr = " + almListJSONstr);
                                        
                                        // make sure file was valid
                                        if(almListJSONstr != null) {
                                            var almListJSON = JSON.parse(almListJSONstr); // parse it first

                                            // loop through each alarm for the current event, store each as a row in the database's EVENT table
                                            for(var k = 0; k < almListJSON.length; k++) {

                                                // store it as a row in the database: ALARM table
                                                var insertIntoALARMcmd = "INSERT INTO ALARM (action, `trigger`, event) " +
                                                        "VALUES( '" + almListJSON[k].action + "', '" + almListJSON[k].trigger + "', '" + eventID + "' );";

                                                connection.query(insertIntoALARMcmd, function (err, rows, fields) {
                                                    if(err) {
                                                        console.log("Error: something went wrong while storing info in the ALARM table" + err);
                                                    } else {
                                                        
                                                        //console.log("my alarm table insert: " + insertIntoALARMcmd);

                                                    }
                                                });

                                            } // end for loop (k) (store in ALARM table)

                                        } else {
                                            // set it false because getting the alarm list didn't work
                                            rowJSONev[0].valid = false;
                                        }
                                    }
                                    evListIndex++;
                                });

                            } else {
                                // set it false because getting the property list didn't work
                                rowJSONev[0].valid = false;
                            }
                            
                            // add it to the event list result array
                            evSummaryJSONs.push(rowJSONev);

                        } // end for loop (j) (store in EVENT table)

                    } else {
                        evSummaryJSONs.push( [ {filename: fileList[fileListIndex], valid: false} ] );
                    }
                }
                fileListIndex++;
            });

        } else {
            calSummaryJSONs.push( [ {filename: fileList[fileListIndex], valid: false} ] );
            // delete the invalid file
            fs.unlinkSync("./uploads/" + fileList[fileListIndex]);
        }        
    } // end for loop (i) (store in FILE table)
    
    //res.send( calSummaryJSONs );
});

/**
 * Clears all rows from all tables in the database
 * @param res -a JSON containing a boolean signalling if it was successful or not
 */
app.get('/dbclear', function(req, res) {
    connection.query("DELETE FROM FILE;", function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("clearing all rows from the database didn't work: " + err);
            res.status(400).send( "clearing all rows from the database didn't work: " + err );

        } else {
            console.log("Cleared all data from database tables successfully");

            // reset the auto increment id counter for each table
            connection.query("ALTER TABLE FILE AUTO_INCREMENT = 1;", function (err, rows, fields) {
                if(err) res.status(400).send( "Resetting the auto_increment counter for the database FILE table failed" );
            });
            connection.query("ALTER TABLE EVENT AUTO_INCREMENT = 1;", function (err, rows, fields) {
                if(err) res.status(400).send( "Resetting the auto_increment counter for the database EVENT table failed" );
            });
            connection.query("ALTER TABLE ALARM AUTO_INCREMENT = 1;", function (err, rows, fields) {
                if(err) res.status(400).send( "Resetting the auto_increment counter for the database ALARM table failed" );
            });

            res.send( { clearSuccess: true } );
        }
    });
});

/**
 * Retrieves the database's status to be displayed in the status panel for user
 * Format: "Database has N1 files, N2 events, and N3 alarms"
 * @param res -a JSON with the string-formatted database status to be displayed
 */
app.get('/dbstatus', function(req, res) {
    // get the database's status info as a JSON
    var dbStatusJSON = { cals: 0, evs: 0, alms: 0, success: true, error: "N/A"};

    // query database for number of iCal files
    connection.query("SELECT * FROM FILE", function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("displayDBStatus() - FILE query didn't work: " + err);
            dbStatusJSON.success = false;
            dbStatusJSON.error = "displayDBStatus() - FILE query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( { dbStatusStr: dbStatusJSON.error } );

        } else {
            console.log(rows.length);

            dbStatusJSON.cals = rows.length;
            // query database for number of events
            connection.query("SELECT * FROM EVENT", function (err, rows, fields) {
                // make sure it worked
                if(err) {
                    console.log("displayDBStatus() - EVENT query didn't work: " + err);
                    dbStatusJSON.success = false;
                    dbStatusJSON.error = "displayDBStatus() - EVENT query didn't work: " + err;
                    // send database status info early due to error
                    res.status(400).send( { dbStatusStr: dbStatusJSON.error } );

                } else {
                    dbStatusJSON.evs = rows.length;

                    // query database for number of alarms
                    connection.query("SELECT * FROM ALARM", function (err, rows, fields) {
                        // make sure it worked
                        if(err) {
                            console.log("displayDBStatus() - ALARM query didn't work: " + err);
                            dbStatusJSON.success = false;
                            dbStatusJSON.error = "displayDBStatus() - ALARM query didn't work: " + err;
                            // send database status info early due to error
                            res.status(400).send( { dbStatusStr: dbStatusJSON.error } );

                        } else {
                            dbStatusJSON.alms = rows.length;

                            console.log(dbStatusJSON);

                            // finally send the database status info
                            res.send( { dbStatusStr: "Database has " + dbStatusJSON.cals + " files, " + dbStatusJSON.evs + " events, and "  + dbStatusJSON.alms + " alarms" } );
                        } // end if
                    });
                } // end if
            });
        } // end if
    });
});

// ============================================== A4 GET QUERY DB ===============================================

/**
 * Retrieves all events from the database sorted by start date-time
 * @param res -a created JSON for all EVENT table rows from the query
 */
app.get('/dbquery1', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM EVENT ORDER BY start_time;";

    console.log("in /dbquery1");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB1() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB1() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // create a JSON to return using the query result rows data
            var eventTableRows = [];

            // loop through each query result row and use it to create a JSON, then add it to the array
            for(let row of rows) {
                var evTableRow = {
                    event_id: row.event_id,
                    summary: row.summary,
                    start_time: "" + row.start_time,
                    location: row.location,
                    organizer: row.organizer,
                    cal_file: row.cal_file
                }
                eventTableRows.push(evTableRow);
            }

            res.send(eventTableRows);
        }
    });
});

/**
 * Retrieves all events from the database for the given iCal file
 * @param req -a JSON containing the filename to search for in the database
 * @param res -a created JSON for all EVENT table rows from the query
 */
app.get('/dbquery2', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM FILE WHERE file_Name = '" + req.query.filename + "';";

    console.log("in /dbquery2");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB2() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB2() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // check if it found a matching file name
            if(rows.length <= 0) {
                res.status(400).send( queryStatusJSON );

            } else {
                connection.query("SELECT * FROM EVENT WHERE cal_file = '" + rows[0].cal_id + "';", function (err, rows, fields) {
                    // make sure it worked
                    if(err) {
                        console.log("queryDB2() - database query didn't work: " + err);
                        queryStatusJSON.success = false;
                        queryStatusJSON.error = "queryDB2() - database query didn't work: " + err;
                        // send database status info early due to error
                        res.status(400).send( queryStatusJSON );
            
                    } else {
                        // create a JSON to return using the query result rows data
                        var eventTableRows = [];

                        // loop through each query result row and use it to create a JSON, then add it to the array
                        for(let row of rows) {
                            var evTableRow = {
                                event_id: row.event_id,
                                summary: row.summary,
                                start_time: "" + row.start_time,
                                location: row.location,
                                organizer: row.organizer,
                                cal_file: row.cal_file
                            }
                            eventTableRows.push(evTableRow);
                        }

                        res.send(eventTableRows);
                    }
                });
            }
        }
    });
});

/**
 * Retrieves all potentially conflicting events from the database
 * @param res -a created JSON for all EVENT table rows from the query
 */
app.get('/dbquery3', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM EVENT WHERE start_time IN (SELECT start_time FROM EVENT GROUP BY start_time HAVING COUNT(*) > 1) ORDER BY start_time ASC;";

    console.log("in /dbquery3");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB3() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB3() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // create a JSON to return using the query result rows data
            var eventTableRows = [];

            // loop through each query result row and use it to create a JSON, then add it to the array
            for(let row of rows) {
                var evTableRow = {
                    event_id: row.event_id,
                    summary: row.summary,
                    start_time: "" + row.start_time,
                    location: row.location,
                    organizer: row.organizer,
                    cal_file: row.cal_file
                }
                eventTableRows.push(evTableRow);
            }

            res.send(eventTableRows);
        }
    });
});

/**
 * Retrieves all alarms from the database with the given action
 * @param req -a JSON containing the action to search for in the database
 * @param res -a created JSON for all ALARM table rows from the query
 */
app.get('/dbquery4', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM ALARM WHERE action = '" + req.query.action + "';";

    console.log("in /dbquery4");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB4() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB4() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // create a JSON to return using the query result rows data
            var alarmTableRows = [];

            // loop through each query result row and use it to create a JSON, then add it to the array
            for(let row of rows) {
                var almTableRow = {
                    alarm_id: row.alarm_id,
                    action: row.action,
                    trigger: row.trigger,
                    event: row.event
                }
                alarmTableRows.push(almTableRow);
            }

            res.send(alarmTableRows);
        }
    });
});

/**
 * Retrieves all events from the database with the given location
 * @param req -a JSON containing the location to search for in the database
 * @param res -a created JSON for all EVENT table rows from the query
 */
app.get('/dbquery5', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM EVENT WHERE location = '" + req.query.location + "';";

    console.log("in /dbquery5");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB5() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB5() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // create a JSON to return using the query result rows data
            var eventTableRows = [];

            // loop through each query result row and use it to create a JSON, then add it to the array
            for(let row of rows) {
                var evTableRow = {
                    event_id: row.event_id,
                    summary: row.summary,
                    start_time: "" + row.start_time,
                    location: row.location,
                    organizer: row.organizer,
                    cal_file: row.cal_file
                }
                eventTableRows.push(evTableRow);
            }

            res.send(eventTableRows);
        }
    });
});

/**
 * Retrieves all events from the database with the given organizer
 * @param req -a JSON containing the organizer to search for in the database
 * @param res -a created JSON for all EVENT table rows from the query
 */
app.get('/dbquery6', function(req, res) {
    // declare variables
    var queryStatusJSON = { success: true, error: "N/A" };
    var queryCmd = "SELECT * FROM EVENT WHERE organizer = '" + req.query.organizer + "';";

    console.log("in /dbquery6");

    connection.query(queryCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("queryDB6() - database query didn't work: " + err);
            queryStatusJSON.success = false;
            queryStatusJSON.error = "queryDB6() - database query didn't work: " + err;
            // send database status info early due to error
            res.status(400).send( queryStatusJSON );

        } else {
            // create a JSON to return using the query result rows data
            var eventTableRows = [];

            // loop through each query result row and use it to create a JSON, then add it to the array
            for(let row of rows) {
                var evTableRow = {
                    event_id: row.event_id,
                    summary: row.summary,
                    start_time: "" + row.start_time,
                    location: row.location,
                    organizer: row.organizer,
                    cal_file: row.cal_file
                }
                eventTableRows.push(evTableRow);
            }

            res.send(eventTableRows);
        }
    });
});

// ==================================================== POST ====================================================

/**
 * Used to create a new calendar
 * @param req -an array of JSONs containing just the desired iCal filename, the calendar info, and the event info
 * @param res -a human-readable ICalErrorCode result ("OK" indicates success)
 */
app.post('/createcal', function(req , res){
    var returnVal = sharedLib.a3CreateCalendar("./uploads/" + req.body[0].filename, JSON.stringify(req.body[1]), JSON.stringify(req.body[2]));
    res.send({ ICalErrorCodeReturn: returnVal });
});

/**
 * Used to create a new event and add it to a designated iCal file
 * @param req -an array of JSONs containing just the desired iCal filename, and the event info
 * @param res -a human-readable ICalErrorCode result ("OK" indicates success)
 */
app.post('/createevent', function(req , res){
    var returnVal = sharedLib.a3AddEventToCalendar("./uploads/" + req.body[0].filename, JSON.stringify(req.body[1]));
    res.send({ ICalErrorCodeReturn: returnVal });
});

// ================================================= A4 HELPERS =================================================

/**
 * Creates the FILE table in the database if it doesn't exist
 */
function createDBTableFILE() {
    var createTableCmd = "CREATE TABLE IF NOT EXISTS FILE ( " +
            "cal_id      INT          AUTO_INCREMENT, " + 
            "file_Name   VARCHAR(60)  NOT NULL, " + 
            "version     INT          NOT NULL, " + 
            "prod_id     VARCHAR(256) NOT NULL, " + 
            "PRIMARY KEY (cal_id) );";
    
    connection.query(createTableCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("Error creating FILE table: " + err);
        }
    });
}
/**
 * Creates the EVENT table in the database if it doesn't exist
 */
function createDBTableEVENT() {
    var createTableCmd = "CREATE TABLE IF NOT EXISTS EVENT ( " +
            "event_id    INT             AUTO_INCREMENT, " + 
            "summary     VARCHAR(1024), " + 
            "start_time  DATETIME        NOT NULL, " + 
            "location    VARCHAR(60), " + 
            "organizer   VARCHAR(256), " + 
            "cal_file    INT             NOT NULL, " + 
            "PRIMARY KEY (event_id), " + 
            "FOREIGN KEY (cal_file)      REFERENCES FILE(cal_id) ON DELETE CASCADE );";

    connection.query(createTableCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("Error creating EVENT table: " + err);
        }
    });
}
/**
 * Creates the ALARM table in the database if it doesn't exist
 */
function createDBTableALARM() {
    var createTableCmd = "CREATE TABLE IF NOT EXISTS ALARM ( " +
            "alarm_id    INT          AUTO_INCREMENT, " + 
            "action      VARCHAR(256) NOT NULL, " + 
            "`trigger`     VARCHAR(256) NOT NULL, " + 
            "event       INT          NOT NULL, " + 
            "PRIMARY KEY (alarm_id), " + 
            "FOREIGN KEY (event)      REFERENCES EVENT(event_id) ON DELETE CASCADE );";

    connection.query(createTableCmd, function (err, rows, fields) {
        // make sure it worked
        if(err) {
            console.log("Error creating ALARM table: " + err);
        }
    });
}
/**
 * Creates all 3 tables (FILE, EVENT, ALARM) if they don't exist
 */
function createAllDBTables() {
    createDBTableFILE();
    createDBTableEVENT();
    createDBTableALARM();
}
