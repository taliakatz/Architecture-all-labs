all: exec

c-lib: encoder.c
	gcc -g -m32 -c -o encoder.o encoder.c

exec: c-lib
	gcc -g -m32  encoder.o -o encoder
	rm encoder.o

.PHONY: clean
clean:
	rm -rf ./*.o main