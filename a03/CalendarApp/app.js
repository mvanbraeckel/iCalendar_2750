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
