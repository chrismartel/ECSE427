///// LIBRARY SOURCE FILES /////

sut.c is the library implementing:
	1. the required sut library functions 
	2. the computation executor kernel thread function c_exec()
	3. the IO executor kernel thread function i_exec()

** The library contains a "io_mock" boolean. This boolean is set to false by default. 
   When set to false, the library IO functions will proceed to remote process connection and 
   send/receive data from remote process. 
   When the boolean is true, the library IO functions will mock the server connection and mock
   the data sending/receiving. (testing mode purpose)


rpc.c is the library from assignment 1 implementing the functions to create a server connection,
connect to a server, send message to server, receive message from server etc.

queue.h is the provided queue wrapper header file.

///// TEST FILES /////
In the "/test/" folder are located some test files.

///// COMPILATION /////

To compile the library source files into binary, run the command "make a2". This will throw some
errors since the reference to the pthread functions will be undefined, but the files will compile succesfully.
To compile the test files in the "/test/" folder, run "make tests".

***Since the pthread library is used in the sut.c library, when compiling a file into an executable, 
"-lpthread" must be added at the end of the command. Ex: gcc test1.o sut.o rpc.o -o test1 -lpthread. 

