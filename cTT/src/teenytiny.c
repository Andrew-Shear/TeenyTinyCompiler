#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "emit.h"
#include "parse.h"
#include "lex.h"
#include "list.h"

Lexer *lex;
AST *ast;
Parser *par;

void killAll() {
	Lexer_kill(lex);
	AST_kill(ast);
	Parser_kill(par);
	Emitter_kill();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Must give a file to compile.\n");
		exit(1);
	}
	
	if (atexit(killAll) != 0) {
		printf("killAll was not registered as exit function.\n");
		exit(1);
	}

	printf("Compiling %s...\n", argv[1]);

	FILE *teenytinyFile = fopen(argv[1], "r");
	if (teenytinyFile == NULL) {
		printf("File could not be opened.\n");
		return 1;
	}
	lex = Lexer_create(teenytinyFile);

	Emitter_create("out.c");

	ast = AST_create();
	par = Parser_create(lex, ast);
	
	Parser_program(par);

	AST_emit(ast);
	Emitter_writeFile();
	printf("Compiling completed.\n\n");

	return 0;
}
