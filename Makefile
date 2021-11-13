all: pgm.o
	gcc -Wall main.c -o main pgm.o -lm

pgm.o: pgm.c
	gcc -c -Wall pgm.c 

test: 
	gcc -Wall test.c -o test

clean:
	rm *.o main *.pgm

clean-test:
	rm test