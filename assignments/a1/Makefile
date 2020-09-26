# -*- MakeFile -*-

all: backend frontend test

backend: backend.o calculator.o a1_lib.o
	gcc backend.o calculator.o a1_lib.o -o backend

frontend: frontend.o a1_lib.o
	gcc frontend.o a1_lib.o -o frontend

test: test.o a1_lib.o
	gcc test.o a1_lib.o -o test

backend.o: backend.c a1_lib.h
	gcc -c backend.c

frontend.o: frontend.c a1_lib.h
	gcc -c frontend.c

test.o: test.c a1_lib.h
	gcc -c test.c

a1_lib.o: a1_lib.c
	gcc -c a1_lib.c

calculator.o: calculator.c
	gcc -c calculator.c

clean:
	rm *.o backend frontend test