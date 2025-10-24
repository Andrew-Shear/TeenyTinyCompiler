#include <stdio.h>
#include <string.h>
#include "lex.h"

int main(int argc, char *argv[]) {
	FILE *teenytinyFile = fopen("hello.teeny", "r");
	if (teenytinyFile == NULL) {
		printf("File could not be opened.\n");
		return 1;
	}
	Lexer *lex = Lexer_create(teenytinyFile);
	Token *t = Lexer_getToken(lex);
	while (t->type != eOF) {
		printf("%i\n", t->type);
		Token_kill(t);
		t = Lexer_getToken(lex);
	}

	Lexer_kill(lex);
	Token_kill(t);
	return 0;
}
