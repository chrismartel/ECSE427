# -*- MakeFile -*-

# compile all
all: a2 tests

# compile libraries

a2: sut.o rpc.o

sut.o: sut.c 
	gcc -o sut.o -c sut.c

rpc.o: rpc.c
	gcc -o rpc.o -c rpc.c

small_server: small_server.c rpc.o
	gcc small_server.c rpc.o -o small_server

#  compile tests

tests: test1 test2 test3 test4 test5

test1.o: test/test1.c 
	gcc -c -o test/test1.o test/test1.c

test2.o: test/test2.c 
	gcc -c -o test/test2.o test/test2.c

test3.o: test/test3.c 
	gcc -c -o test/test3.o test/test3.c	

test4.o: test/test4.c 
	gcc -c -o test/test4.o test/test4.c

test5.o: test/test5.c 
	gcc -c -o test/test5.o test/test5.c


test1: test/test1.o sut.o rpc.o
	gcc test/test1.o sut.o rpc.o -o test/test1 -lpthread

test2: test/test2.o sut.o rpc.o
	gcc test/test2.o sut.o rpc.o -o test/test2 -lpthread

test3: test/test3.o sut.o rpc.o
	gcc test/test3.o sut.o rpc.o -o test/test3 -lpthread

test4: test/test4.o sut.o rpc.o
	gcc test/test4.o sut.o rpc.o -o test/test4 -lpthread

test5: test/test5.o sut.o rpc.o
	gcc test/test5.o sut.o rpc.o -o test/test5 -lpthread


# clean up
clean:
	rm sut.o rpc.o ; cd test ; rm test1 test2 test3 test4 test5 test1.o test2.o test3.o test4.o test5.o

