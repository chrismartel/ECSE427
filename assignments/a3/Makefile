# -*- MakeFile -*-

a3: sma.o a3_test

sma.o: sma.c 
	gcc -o sma.o -c sma.c

a3_test.o: a3_test.c
	gcc -c -o a3_test.o a3_test.c

a3_test: a3_test.o sma.o
	gcc a3_test.o sma.o -o a3_test

clean:
	rm sma.o a3_test.o a3_test