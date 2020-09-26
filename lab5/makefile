all: exec

c-lib: shell.c
	gcc -g -m32 -c -Include -o LineParser.o LineParser.c
	gcc -g -m32 -c -Include -o shell.o shell.c

exec: c-lib
	gcc -g -m32 shell.o LineParser.o -o shell2
	rm shell2.o

.PHONY: clean
clean:
	rm -rf ./*.o shell2