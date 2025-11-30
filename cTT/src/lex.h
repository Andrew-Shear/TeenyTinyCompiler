#ifndef LEX_H
#define LEX_H

#include <stdio.h>

struct Lexer;
typedef struct Lexer {
	FILE *source;
	char curChar;
	char nextChar;
	int lineNumber;
} Lexer;

typedef enum TokenType {
	eOF = -1,
	NEWLINE = 0,
	NUMBERINT = 1,
	NUMBERFLOAT = 2,
	IDENT = 3,
	STRING = 4,
	LABEL = 101,
	GOTO = 102,
	PRINT = 103,
	INPUT = 104,
	LET = 105,
	IF = 106,
	THEN = 107,
	ELSEIF = 108,
	ELSE = 109,
	ENDIF = 110,
	WHILE = 111,
	REPEAT = 112,
	ENDWHILE = 113,
	FOR = 114,
	TO = 115,
	ENDFOR = 116,
	INT = 117,
	FLOAT = 118,
	BOOL = 119,
	TRUE = 120,
	FALSE = 121,
	EQ = 201,
	PLUS = 202,
	MINUS = 203,
	ASTERISK = 204,
	SLASH = 205,
	EQEQ = 206,
	NOTEQ = 207,
	LT = 208,
	LTEQ = 209,
	GT = 210,
	GTEQ = 211,
	LEFTPAREN = 212,
	RIGHTPAREN = 213,
	NOT_VAR = 301,
	INT_VAR = 302,
	STRING_VAR = 303,
	FLOAT_VAR = 304,
	BOOL_VAR = 305
} TokenType;

struct Token;
typedef struct Token {
	char *text;
	TokenType type;
} Token;

const static struct {
	TokenType val;
	const char *str;
} keywordConversion [] = {
	{LABEL, "LABEL"},
	{GOTO, "GOTO"},
	{PRINT, "PRINT"},
	{INPUT, "INPUT"},
	{LET, "LET"},
	{IF, "IF"},
	{THEN, "THEN"},
	{ENDIF, "ENDIF"},
	{ELSEIF, "ELSEIF"},
	{ELSE, "ELSE"},
	{WHILE, "WHILE"},
	{REPEAT, "REPEAT"},
	{ENDWHILE, "ENDWHILE"},
	{FOR, "FOR"},
	{TO, "TO"},
	{ENDFOR, "ENDFOR"},
	{INT_VAR, "INT"},
	{FLOAT_VAR, "FLOAT"},
	{BOOL_VAR, "BOOL"},
	{STRING_VAR, "STRING"},
	{TRUE, "TRUE"},
	{FALSE, "FALSE"}
};

Lexer *Lexer_create(FILE *source);

void Lexer_kill(Lexer *lex);

void Lexer_nextChar(Lexer *lex);

char Lexer_peek(Lexer *lex);

void Lexer_abort(Lexer *lex, Token *t, char *message);

Token *Lexer_getToken(Lexer *lex);

void Token_kill(Token *t);

Token *Token_copy(Token *t);

void Lexer_readString(Lexer *lex, Token *t);

void Lexer_readNumber(Lexer *lex, Token *t);

void Lexer_readSymbol(Lexer *lex, Token *t);

TokenType Lexer_getKeyword(char *text);

#endif
