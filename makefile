INPUT = entrada.txt

build: main.c
	gcc -o programa.exe main.c

run: programa.exe
	./programa.exe $(INPUT)