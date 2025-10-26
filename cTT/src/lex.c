#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "lex.h"

Lexer *Lexer_create(FILE *source) {
	Lexer *lex = malloc(sizeof(Lexer));
	lex->source = source;
	lex->curChar = ' ';
	lex->nextChar = ' ';

	Lexer_nextChar(lex);
	Lexer_nextChar(lex);
	return lex;
}

void Lexer_kill(Lexer *lex) {
	fclose(lex->source);
	free(lex);
}

void Lexer_nextChar(Lexer *lex) {
	lex->curChar = lex->nextChar;
	int ch = fgetc(lex->source);
	if (ch == EOF) {
		lex->nextChar = '\0';
	} else {
		lex->nextChar = (char) ch;
	}
}

char Lexer_peek(Lexer *lex) {
	return lex->nextChar;	
}

void Lexer_abort(Lexer *lex, Token *t, char *message) {
	printf(message);
	printf("\n");
	Lexer_kill(lex);
	Token_kill(t);
	exit(1);
}

Token *Lexer_getToken(Lexer *lex) {
	while (lex->curChar == ' ' || 
			lex->curChar == '\t' ||
			lex->curChar == '\r') {
		Lexer_nextChar(lex);
	}
	if (lex->curChar == '#') {
		while (lex->curChar != '\n') {
			Lexer_nextChar(lex);
		}
	}

	Token *t = malloc(sizeof(Token));
	if (t == NULL) {
		printf("unable to allocate memory for Token\n");
		return NULL;
	}

	switch (lex->curChar) {

		case '+':
			t->text = strdup("+");
			t->type = PLUS;
			break;

		case '-':
			t->text = strdup("-");
			t->type = MINUS;
			break;
			
		case '*':
			t->text = strdup("*");
			t->type = ASTERISK;
			break;

		case '/':
			t->text = strdup("/");
			t->type = SLASH;
			break;

		case '=':
			if (lex->nextChar == '=') {
				Lexer_nextChar(lex);
				t->text = strdup("==");
				t->type = EQEQ;
			} else {
				t->text = strdup("=");
				t->type = EQ;
			}
			break;

		case '>':
			if (lex->nextChar == '=') {
				Lexer_nextChar(lex);
				t->text = strdup(">=");
				t->type = GTEQ;
			} else {
				t->text = strdup(">");
				t->type = GT;
			}
			break;

		case '<':
			if (lex->nextChar == '=') {
				Lexer_nextChar(lex);
				t->text = strdup("<=");
				t->type = LTEQ;
			} else {
				t->text = strdup("<");
				t->type = LT;
			}
			break;
			
		case '!':
			if (lex->nextChar == '=') {
				Lexer_nextChar(lex);
				t->text = strdup("!=");
				t->type = NOTEQ;
			} else {
				Lexer_abort(lex, t, "Expected !=, got !");
			}
			break;

		case '\"':
			Lexer_nextChar(lex);
			Lexer_readString(lex, t);
			break;

		case '\n':
			t->text = strdup("\n");
			t->type = NEWLINE;
			break;

		case '\0':
			t->text = strdup("");
			t->type = eOF;
			break;

		default:
			if (isdigit(lex->curChar)) {
				Lexer_readNumber(lex, t);
			} else if (isalpha(lex->curChar)) {
				Lexer_readSymbol(lex, t);   
			} else {
				printf("unknown character: %c\n", lex->curChar);
				t->text = strdup("");
				t->type = 0;
			}
	}
	
	Lexer_nextChar(lex);
	return t;
}

void Token_kill(Token *t) {
	free(t->text);
	free(t);
}

void Lexer_readString(Lexer *lex, Token *t) {
	// lex->nextChar is the first character in the string.
	// startPos is the index of the first character in the string.
	long startPos = ftell(lex->source)-2;

	while (lex->curChar != '\"') {
		if (lex->curChar == '\r' || lex->curChar == '\n' || lex->curChar == '\t' || lex->curChar == '\\' || lex->curChar == '%') {
			Lexer_abort(lex, t, "Illegal character in string.");
		}
		Lexer_nextChar(lex);
	}
	// endPos is where the ending " is
	long endPos = ftell(lex->source)-2;	
	fseek(lex->source, startPos, SEEK_SET);
	char text[endPos-startPos+1];
	fgets(text, endPos-startPos+1, lex->source);
	// we have just read up to the final quotations, but our nextChar expects us 
	// to be two characters further.
	fgetc(lex->source);
	fgetc(lex->source);
	t->text = strdup(text);
	t->type = STRING;
}

void Lexer_readNumber(Lexer *lex, Token *t) {
	// lex->curChar is the first character in the number.
	long startPos = ftell(lex->source)-2;

	while (isalnum(lex->nextChar)) {
		Lexer_nextChar(lex);
	}
	if (lex->nextChar == '.') {
		// decimal
		Lexer_nextChar(lex);
		if (!isalnum(lex->nextChar)) {
			Lexer_abort(lex, t, "Illegal character after decimal in number.");
		}
		while (isalnum(lex->nextChar)) {
			Lexer_nextChar(lex);
		}
	}
	// lex->curChar is the last digit in the number.
	// startPos is the first digit, endPos is the last digit
	long endPos = ftell(lex->source)-2;	
	fseek(lex->source, startPos, SEEK_SET);
	char num[endPos-startPos+2];
	// starting from startPos, read endPos-startPos+1 characters
	fgets(num, endPos-startPos+2, lex->source);
	// now, the pointer is pointing to the character after, so we have to move one forward.
	fgetc(lex->source);
	t->text = strdup(num);
	t->type = NUMBER;
}

void Lexer_readSymbol(Lexer *lex, Token *t) {
	// lex->curChar is the first character.
	long startPos = ftell(lex->source)-2;

	while (isalnum(lex->nextChar)) {
		Lexer_nextChar(lex);
	}

	// lex->curChar is the last alnum in the symbol.
	long endPos = ftell(lex->source)-2;
	fseek(lex->source, startPos, SEEK_SET);
	char text[endPos-startPos+2];
	// starting from startPos, read endPos-startPos+1 characters
	fgets(text, endPos-startPos+2, lex->source);
	// now, the pointer is pointing to the character after, so we have to move one forward.
	fgetc(lex->source);
	t->text = strdup(text);
	t->type = Lexer_getKeyword(text);
}

TokenType Lexer_getKeyword(char *text) {
	int i;
	for (i = 0; i < sizeof(keywordConversion)/sizeof(keywordConversion[0]); i++) {
		if (!strcmp(text, keywordConversion[i].str))
			return keywordConversion[i].val;
	}
	return IDENT;
}

