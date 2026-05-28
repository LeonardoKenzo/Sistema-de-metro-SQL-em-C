all: dataRegister.o functionalities.o headerRegister.o fornecidas.o utils.o main.c
	gcc dataRegister.o functionalities.o headerRegister.o fornecidas.o utils.o main.c -o main -std=c99 -Wall -lm
dataRegister.o:
	gcc -c dataRegister.c -o dataRegister.o
functionalities.o:
	gcc -c functionalities.c -o functionalities.o
headerRegister.o:
	gcc -c headerRegister.c -o headerRegister.o
fornecidas.o:
	gcc -c fornecidas.c -o fornecidas.o
utils.o:
	gcc -c utils.c -o utils.o
run:
	./main
clean:
	rm *.o *.exe *.zip main
memory:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./main
zip:
	zip -r Trabalho-arquivo.zip *.c *.h makefile