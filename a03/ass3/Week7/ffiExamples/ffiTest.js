
//************************* Using FFI *************************
//Sort of like import
var ffi = require('ffi');


//*** Set up a function from the C math library ***
var libm = ffi.Library('libm', {
  'ceil': [ 'double', [ 'double' ] ]
});

//Call a function from the C math library
let c = libm.ceil(1.5); // 2
console.log("ffiTest.js: libm.ceil(1.5) returned "+c+"\n");


/**** Set up functions from our shared library ****

We create a new object called sharedLib and the C functions become its methods

*/
let sharedLib = ffi.Library('./sharedLib', {
  'printFunc': [ 'void', [] ],		//return type first, argument list second
									//for void input type, leave argument list empty
  'addTwo': [ 'int', [ 'int' ] ],	//int return, int argument
  'getDesc' : [ 'string', [] ],
  'putDesc' : [ 'void', [ 'string' ] ],
});

//Call a C function and pass a string to it
sharedLib.putDesc("Stuff got changed");

//Call a C function returning an int
c = sharedLib.addTwo(15); // return 15+2
console.log("\nffiTest.js: sharedLib.addTwo(15) returned "+c+"\n");

//Call a C function returning nothing, which prints something to command line
sharedLib.printFunc();

//Call a C function that takes no arguments, returns a string, and display its value
let descJSON = sharedLib.getDesc();
console.log("ffiTest.js: sharedLib.getDesc() returned "+descJSON+"\n");




