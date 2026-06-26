CFLAGS = -g -Wall -Wextra
CC = gcc

compile:
	$(CC) $(CFLAGS) src/teenytiny.c src/parse.c src/lex.c src/emit.c src/list.c src/ast.c -o src/teenytiny
