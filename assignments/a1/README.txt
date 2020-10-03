backend.c : backend source file
frontend.c : frontend source file
calculator.c/calculator.h : calculator functions library (source and header files)
mystringlib.c/mystringlib.h : special string functions library (source and header files)
a1_lib.c/h : client/server connection functions + global variables (source and header file)

To compile the code into executables run: 
	make rpc 
	OR 
	make

To run backend:
	 ./backend <host_ip> <host_port>

To run a frontend:
	 ./frontend <host_ip> <host_port>

to terminate a frontend: "exit" command

to terminate a frontend and backend: "shutdown" command

Difficulties encountered:

Keep track of the number of current children processes running to keep a maximum
of 5 concurrent children processes. 
Wait for a child process to get the shutdown command in order to shutdown everything.

Solution: shared memory segment to keep track of how many children processes are running
to keep track of the ids of the children processes in order to be able to kill them all
if shutdown is called.
