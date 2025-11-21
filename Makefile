all: sga

sga: *.c *.h
	gcc -o sga *.c -Wall -Wextra -g -std=c11 -lm

clean:
	rm -f sga *.o *.dat *.idx
