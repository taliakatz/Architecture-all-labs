all: exec

c-lib: menuMap.c
	gcc -g -m32 -c -o menuMap.o menuMap.c

exec: c-lib
	gcc -g -m32  menuMap.o -o menuMap
	rm menuMap.o

.PHONY: clean
clean:
	rm -rf ./*.o menuMap
