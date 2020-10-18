# -*- MakeFile -*-

all: sut

sut: sut.o 
	gcc sut.o  -o sut

sut.o: sut.c sut.h queue.h
	gcc -c sut.c

clean:
	rm *.o sut ult pthreads

ult: ult.c
	gcc ult.c -o ult

pthreads: pthreads.c
	gcc pthreads.c -o pthreads -l pthread

tut: ult pthreads