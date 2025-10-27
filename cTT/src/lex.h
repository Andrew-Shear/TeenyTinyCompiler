#ifndef LEX_H
#define LEX_H

struct Lexer;
typedef struct Lexer {
	FILE *source;
	char curChar;
	char nextChar;
	int lineNumber;
} Lexer;

enum TokenType;
typedef enum TokenType {
	eOF = -1,
	NEWLINE = 0,
	NUMBER = 1,
	IDENT = 2,
	STRING = 3,
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
	RIGHTPAREN = 213
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
	{ENDWHILE, "ENDWHILE"}
};

Lexer *Lexer_create(FILE *source);

void Lexer_kill(Lexer *lex);

void Lexer_nextChar(Lexer *lex);

char Lexer_peek(Lexer *lex);

void Lexer_abort(Lexer *lex, Token *t, char *message);

Token *Lexer_getToken(Lexer *lex);

void Token_kill(Token *t);

void Lexer_readString(Lexer *lex, Token *t);

void Lexer_readNumber(Lexer *lex, Token *t);

void Lexer_readSymbol(Lexer *lex, Token *t);

TokenType Lexer_getKeyword(char *text);

#endif
