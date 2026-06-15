INPUT = entrada.txt

build: main.c
	gcc -o programa.exe main2.c

run: programa.exe
	./programa.exe $(INPUT)