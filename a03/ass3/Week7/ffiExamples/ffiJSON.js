
//************************* Using FFI *************************
//Sort of like import
let ffi = require('ffi');


/**** Set up functions from our shared library ****

We create a new object called sharedLib and the C functions become its methods

*/
let sharedLib = ffi.Library('./sharedLib', {
  'printFunc': [ 'void', [ ] ],		//return type first, argument list second
									//for void input type, leave argument list is empty
  'addTwo': [ 'int', [ 'int' ] ],	//int return, int argument
  'getDesc' : [ 'string', [] ],
  'putDesc' : [ 'void', [ 'string' ] ],
});


//Call a C function that takes no arguments, returns a string, and display its value
let descJSON = sharedLib.getDesc();
console.log("ffiTest.js: sharedLib.getDesc() returned "+descJSON+"\n");


// ************************* JSONs and loops *************************

//Create an array from JSON string.  Array entries are arrays of objects
//Each object has instance variables givenName and surname }
let family = JSON.parse(descJSON);


//Iterate through the array for-of loop
console.log("\n\nUsing for-of loop\n");
let gen = 1;
for (let generation of family){
	console.log("Genertation "+ gen);
	for (let ind of generation){
		//Let JavaScript display the object using its default string representation
		console.log(ind);

		//Display instance variables of the object
		console.log("Given name: "+ind.givenName);
		console.log("Surname: "+ind.surname);

	}
	gen += 1;
}

//Create a JSON string from array of numbers and pass it to a C function
let numArr = [1,2,3,4,5,6];
let numArrJSON = JSON.stringify(numArr);
sharedLib.putDesc(numArrJSON);

//Create a JSON string from array of strings and pass it to a C function
let strArr = ["Hello","Goodbye","Meh"];
let strArrJSON = JSON.stringify(strArr);
sharedLib.putDesc(strArrJSON);

//Create an object
let myObj = new Object();
myObj.name = 'Marine';
myObj.race = 'Terran';
myObj.health = 100;

//Create a JSON string from an object and pass it to a C function
let objJSON = JSON.stringify(myObj);
sharedLib.putDesc(objJSON);

//Create a JSON string from a 2D array of objects and pass it to a C function
let arr2DJSON = JSON.stringify(family);
sharedLib.putDesc(arr2DJSON);




//Uncomment the code below to run the traditional for loop and for-in loop

/*
//Iterate through the array using traditional for loop
console.log("\n\nUsing regular for loop\n");
for (let i = 0; i < descendants.length; i++){
	console.log("Genertation "+ (i+1));

	for (let j = 0; j < descendants[i].length; j++){
		//Let JavaScript display the object using its default string representation
		console.log(descendants[i][j]);

		//Display instance variables of the object
		console.log("Given name: "+descendants[i][j].givenName);
		console.log("Surname: "+descendants[i][j].surname);
	}
}


//Iterate through the array using for-in loop
console.log("\n\nUsing for-in loop\n");
for (let i in descendants){
	console.log("Genertation "+ (i+1));

	for (let j in descendants[i]){
		//Let JavaScript display the object using its default string representation
		console.log(descendants[i][j]);

		//Display instance variables of the object
		console.log("Given name: "+descendants[i][j].givenName);
		console.log("Surname: "+descendants[i][j].surname);
	}
}
*/


