file2.o: file2.c file2.h
	gcc -c file2.c
file1.o: file1.c file1.h file2.h file2.c
	gcc -c file1.c
file2: file2.o
	gcc file2.o
file1: file1.o file2.o
	gcc file1.o file2.o

clean:
	rm -f *.o a.out