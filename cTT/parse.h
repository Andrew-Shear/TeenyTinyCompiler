#ifndef PARSE_H
#define PARSE_H

#include "lex.h"

typedef struct Parser {
	Lexer *lex;
	Token *curToken;
	Token *peekToken;
} Parser;	

Parser *Parser_create(Lexer *lex);

void Parser_kill(Parser *par);

void Parser_nextToken(Parser *par);

void Parser_abort(Parser *par, char *message);

void Parser_match(Parser *par, TokenType type);

void Parser_program(Parser *par);

void Parser_statement(Parser *par);

void Parser_comparison(Parser *par);

void Parser_expression(Parser *par);

void Parser_term(Parser *par);

void Parser_unary(Parser *par);

void Parser_primary(Parser *par);

void Parser_nl(Parser *par);

#endif
