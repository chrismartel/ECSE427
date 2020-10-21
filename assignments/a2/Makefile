# -*- MakeFile -*-

all: tests tut

sut.o: sut.c 
	gcc -c  sut.c

rpc.o: rpc.c
	gcc -c rpc.c

mystringlib.o: mystringlib.c
	gcc -c mystringlib.c

# test
test1.o: test1.c 
	gcc -c test1.c

test2.o: test2.c
	gcc -c test2.c

test3.o: test3.c
	gcc -c test3.c	

test4.o: test4.c
	gcc -c test4.c	

test5.o: test5.c
	gcc -c test5.c	

test1: test1.o sut.o rpc.o mystringlib.o
	gcc test1.o sut.o rpc.o mystringlib.o -o test1 -lpthread

test2: test2.o sut.o rpc.o mystringlib.o
	gcc test2.o sut.o rpc.o mystringlib.o -o test2 -lpthread

test3: test3.o sut.o rpc.o mystringlib.o
	gcc test3.o sut.o rpc.o mystringlib.o -o test3 -lpthread

test4: test4.o sut.o rpc.o mystringlib.o
	gcc test4.o sut.o rpc.o mystringlib.o -o test4 -lpthread

test5: test5.o sut.o rpc.o mystringlib.o
	gcc test5.o sut.o rpc.o mystringlib.o -o test5 -lpthread

tests: test1 test2 test3 test4 test5

# tutorials
ult: ult.c
	gcc ult.c -o ult

pthreads: pthreads.c
	gcc pthreads.c -o pthreads -l pthread

tut: ult pthreads

clean:
	rm *.o test1 test2 test3 test4 test5 ult pthreads