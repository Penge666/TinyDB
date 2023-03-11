main:
	gcc *.c -o main
run:
	./main
clean:
	rm -rf main store.db
format: *.c
	clang-format -style=Google -i *.c *.h
