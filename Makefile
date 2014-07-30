default: sudoku

sudoku.o: sudoku.c
	gcc -c sudoku.c -o sudoku.o

sudoku: sudoku.o
	gcc sudoku.o -o sudoku

clean:
	-rm -f sudoku.o
	-rm -f sudoku
