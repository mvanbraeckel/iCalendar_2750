'use strict'

const mysql = require('mysql');
const connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : 'yourUserName',
    password : 'yourPassword',
    database : 'yourDBname'
});


connection.connect();

// **** uncomment if you want to create the table on the fly ****

/*
connection.query("create table student (id int not null auto_increment,  last_name char(15),  first_name char(15), mark char(2), primary key(id) )", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
});
*/

// *** If you uncommented code above, remember to also uncomment the "drop table" code at the bottom of this file ****


// Create an array of SQL statements 
let records = [ 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Hugo','Victor','B+')", 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Rudin','Walter','A-')", 
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Stevens','Richard','C')"
];

for (let rec of records){
    connection.query(rec, function (err, rows, fields) {
        if (err) console.log("Something went wrong. "+err);
    });
}

connection.query("SELECT * FROM student ORDER BY last_name", function (err, rows, fields) {
  //Throw an error if we cannot run the query 
    if (err) 
        console.log("Something went wrong. "+err);
    else {
        console.log("Database contents sorted by last name:");

        //Rows is an array of objects.  Each object has fields corresponding to table columns
        for (let row of rows){
            console.log("ID: "+row.id+" Last name: "+row.last_name+" First name: "+row.first_name+" mark: "+row.mark );
        }
    }

    //*** Uncomment the code below if you want ot see the data in rows and fields ***

    /*
    console.log("Rows:");
    for (let row of rows){
        console.log(row);
    }
    console.log("Fields:");
    for (let field of fields){
        console.log(field);
    }
    */

});

connection.query("SELECT * FROM student ORDER BY first_name", function (err, rows, fields) {
    if (err) 
        console.log("Something went wrong. "+err);
    else {
        console.log("Database contents sorted by first name:");
        for (let row of rows){
            console.log("ID: "+row.id+" Last name: "+row.last_name+" First name: "+row.first_name+" mark: "+row.mark );
        }
    }

});

connection.query("delete from student", function (err, rows, fields) {
    if (err) 
        console.log("Something went wrong. "+err);
    else
        console.log("Cleared data from table");
});


// **** uncomment this if you also uncomment the "create table" code at the front ****
/*
connection.query("drop table student", function (err, rows, fields) {
    if (err) console.log("Something went wrong. "+err);
});
*/

connection.end();


