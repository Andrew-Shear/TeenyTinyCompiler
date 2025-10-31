#ifndef PARSE_H
#define PARSE_H

#include "lex.h"
#include "list.h"
#include "ast.h"
#include "emit.h"

typedef struct Parser {
	Lexer *lex;
	AST *ast;
	Token *curToken;
	Token *peekToken;
} Parser;	

Parser *Parser_create(Lexer *lex, AST *ast);

void Parser_kill(Parser *par);

void Parser_nextToken(Parser *par);

void Parser_abort(Parser *par, char *message);

void Parser_match(Parser *par, TokenType type);

void Parser_program(Parser *par);

ASTNode *Parser_statement(Parser *par);

ASTNode *Parser_comparison(Parser *par);

ASTNode *Parser_expression(Parser *par);

ASTNode *Parser_term(Parser *par);

ASTNode *Parser_unary(Parser *par);

ASTNode *Parser_primary(Parser *par);

void Parser_nl(Parser *par);

int List_contains(List *l, char *word);

#endif
