all: sga

sga: *.c *.h
	gcc -o sga *.c

clean:
	rm -f sga *.o *.dat *.idx
