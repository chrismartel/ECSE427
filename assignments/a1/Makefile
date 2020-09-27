# -*- MakeFile -*-

all: backend frontend 

backend: backend.o calculator.o a1_lib.o mystringlib.o
	gcc backend.o calculator.o a1_lib.o mystringlib.o -o backend

frontend: frontend.o a1_lib.o mystringlib.o
	gcc frontend.o a1_lib.o mystringlib.o -o frontend

backend.o: backend.c a1_lib.h mystringlib.h
	gcc -c backend.c

frontend.o: frontend.c a1_lib.h mystringlib.h
	gcc -c frontend.c

a1_lib.o: a1_lib.c
	gcc -c a1_lib.c

calculator.o: calculator.c
	gcc -c calculator.c

mystringlib.o: mystringlib.c
	gcc -c mystringlib.c

clean:
	rm *.o backend frontend test