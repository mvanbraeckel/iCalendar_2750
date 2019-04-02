'use strict'

const mysql = require('mysql');
const connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : 'yourUserName',
    password : 'yourPassword',
    database : 'yourDBname'
});


connection.connect();

// Create an SQL statement - multiple inserts at once
let records =  
    "INSERT INTO student (last_name, first_name, mark) VALUES ('Hugo','Victor','B+'),('Rudin','Walter','A-'),('Stevens','Richard','C')";

//Run the insert statement
connection.query(records, function (err, rows, fields) {
	if (err) 
    	console.log("Something went wrong. "+err);
    else{
    	//Call SELECT only after we are certain that table was populated - i.e. in the callback of the INSERT
    	connection.query("SELECT * FROM student ORDER BY first_name", function (err, rows, fields) {  
    	    if (err) 
    	        console.log("Something went wrong. "+err);
    	    else {
    	        console.log("Database contents sorted by first name:");
    	        for (let row of rows){
    	            console.log("ID: "+row.id+" Last name: "+row.last_name+" First name: "+row.first_name+" mark: "+row.mark );
    	        }
    	  		//Only call SELECT after we know that the first SELECT was executed and was successful
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
		    			//Only call DELETE after we ran all the SELECT queries
		    			connection.query("delete from student", function (err, rows, fields) {
	    					if (err) 
	    						console.log("Something went wrong. "+err);
	    					else{
	    						console.log("Cleared data from table");
	    						
	    						//Only close the connection when we're sure theere are no more queries
	    						connection.end();
	    					}
	    				});//End of delete call
	    	    	}
    	    	});//End of select by last name call
    		}
		});//End of select by first name call
    }
});//End of insert call











