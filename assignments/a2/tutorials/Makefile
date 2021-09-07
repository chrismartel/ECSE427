# -*- MakeFile -*-

tut: ult pthreads

ult: ult.c
	gcc ult.c -o ult

pthreads: pthreads.c
	gcc pthreads.c -o pthreads -l pthread

clean:
	rm *.o ult pthreads