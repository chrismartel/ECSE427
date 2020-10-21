# -*- MakeFile -*-

# convention:
# target: dependencies 
#	action where space before is a tab

testall: test1 test2 test3 test4 test5

test1: test1.o sut.o
	gcc test1.o sut.o -o test1

test2: test2.o sut.o
	gcc test2.o sut.o -o test2

test3: test3.o sut.o
	gcc test3.o sut.o -o test3

test4: test4.o sut.o
	gcc test4.o sut.o -o test4

test5: test5.o sut.o
	gcc test5.o sut.o -o test5

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
	rm -f *.o test1 test2 test3 test4 test5

#Tutorial

pthreads: pthreads.c
	gcc pthreads.c -o pthreads -l pthread 