# -*- MakeFile -*-

# convention:
# target: dependencies 
#	action where space before is a tab

rpc: backend frontend

backend: backend.o a1_lib.o
	gcc backend.o a1_lib.o -o backend

frontend: frontend.o a1_lib.o
	gcc frontend.o a1_lib.o -o backend

backend.o: backend.c a1_lib.h
	gcc -c backend.c 

frontend.o: frontend.c a1_lib.h
	gcc -c frontend.c 

a1_lib.o: a1_lib.c 
	gcc -c a1_lib.c 

clean:
	rm -f *.o frontend backend