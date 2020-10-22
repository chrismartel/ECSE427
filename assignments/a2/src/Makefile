# -*- MakeFile -*-

sut: sut.o rpc.o

sut.o: sut.c 
	gcc -o sut.o -c sut.c

rpc.o: rpc.c
	gcc -o rpc.o -c rpc.c

clean:
	rm sut.o rpc.o