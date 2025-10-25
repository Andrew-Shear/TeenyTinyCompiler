#include <stdio.h>
#include <string.h>
#include "emit.h"
#include "parse.h"
#include "lex.h"

int main(int argc, char *argv[]) {
	printf("Teeny Tiny Compiler\n");

	FILE *teenytinyFile = fopen("hello.teeny", "r");
	if (teenytinyFile == NULL) {
		printf("File could not be opened.\n");
		return 1;
	}
	Lexer *lex = Lexer_create(teenytinyFile);
	Parser *par = Parser_create(lex);
	
	Parser_program(par);
	printf("Parsing completed.\n");

	Parser_kill(par);
	return 0;
}
