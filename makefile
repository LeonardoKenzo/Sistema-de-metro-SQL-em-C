all: dataRegister.o functionalities.o headerRegister.o main.c
	gcc dataRegister.o functionalities.o headerRegister.o main.c -o main -std=c99 -Wall -lm
dataRegister.o:
	gcc -c dataRegister.c -o dataRegister.o
functionalities.o:
	gcc -c functionalities.c -o functionalities.o
headerRegister.o:
	gcc -c headerRegister.c -o headerRegister.o
run:
	./main
clean:
	rm *.o
	rm *.exe
memory:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./main