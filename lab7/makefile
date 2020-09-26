all: exec

c-lib: hexeditplus.c
	gcc -g -m32 -c -o hexeditplus.o hexeditplus.c

exec: c-lib
	gcc -g -m32 hexeditplus.o -o hexeditplus
	
.PHONY: clean
clean:
	rm -rf ./*.o hexeditplus
