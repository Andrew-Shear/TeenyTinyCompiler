#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emit.h"
#include "parse.h"
#include "lex.h"

int main(int argc, char *argv[]) {
	printf("Teeny Tiny Compiler\n");

	if (argc != 2) {
		printf("Must give a file to compile.\n");
		exit(1);
	}
	FILE *teenytinyFile = fopen(argv[1], "r");
	if (teenytinyFile == NULL) {
		printf("File could not be opened.\n");
		return 1;
	}
	Lexer *lex = Lexer_create(teenytinyFile);
	Emitter *emit = Emitter_create("out.c");
	Parser *par = Parser_create(lex, emit);
	
	Parser_program(par);
	Emitter_writeFile(emit);
	printf("Compiling completed.\n");

	Parser_kill(par);
	return 0;
}
