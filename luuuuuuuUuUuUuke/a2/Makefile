# -*- MakeFile -*-

# convention:
# target: dependencies 
#	action where space before is a tab

testall: test1 test2 test3 test4 test5

sut.o: sut.c queue.h
	gcc -c sut.c 

a2_lib.o: a2_lib.c 
	gcc -c a2_lib.c 

test1: test1.o sut.o a2_lib.o
	gcc test1.o sut.o a2_lib.o -o test1 -lpthread

test2: test2.o sut.o a2_lib.o
	gcc test2.o sut.o a2_lib.o -o test2 -lpthread

test3: test3.o sut.o a2_lib.o
	gcc test3.o sut.o a2_lib.o -o test3 -lpthread

test4: test4.o sut.o a2_lib.o
	gcc test4.o sut.o a2_lib.o -o test4 -lpthread

test5: test5.o sut.o a2_lib.o
	gcc test5.o sut.o a2_lib.o -o test5 -lpthread

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

clean:
	rm -f *.o test1 test2 test3 test4 test5 sut pthreads userthreads

#Tutorial

tutorial: pthreads userthreads

pthreads: pthreads.c
	gcc pthreads.c -o pthreads -l pthread 

userthreads: userthreads.c 
	gcc userthreads.c -o userthreads

test_server: test_server.c a2_lib.o
	gcc test_server.c a2_lib.o -o test_server