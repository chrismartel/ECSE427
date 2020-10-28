///// LIBRARY SOURCE FILES /////

sut.c is the library implementing:
	1. the required sut library functions 
	2. the computation executor kernel thread function c_exec()
	3. the IO executor kernel thread function i_exec()

** The read function is reading data of size BUFSIZE = 1024 **

rpc.c is the library from assignment 1 implementing the functions to create a server connection,
connect to a server, send message to server, receive message from server etc. 


queue.h is the provided queue wrapper header file.

///// TEST FILES /////
In the "/test/" folder are located some test files.

///// COMPILATION /////

To compile the library source files into binary, run the command "make a2".
To compile the test files run the command "make tests".
To compile both library files and tests run "make all".

To compile a new test:
   gcc new_test.c sut.o rpc.o -o new_test -lpthread***

** The rpc.o  and sut.o object files must be linked when compiling a new test into an executable.

***Since the pthread library is used in the sut.c library, when compiling a file into an executable, 
"-lpthread" must be added at the end of the command. 
