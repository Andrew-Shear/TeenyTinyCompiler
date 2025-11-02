#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "list.h"
#include "emit.h"

typedef struct ASTNode {
	Token *token;
	List *children;
} ASTNode;

typedef struct AST {
	List *children;
	List *symbols;
	List *labelsDeclared;
	List *labelsGotoed;
} AST;


typedef struct Symbol {
	char *text;
	TokenType type;
} Symbol;

AST *AST_create();

ASTNode *ASTNode_create(Token *t);

void ASTNode_add(ASTNode *parent, ASTNode *child);

void AST_add(AST *parent, ASTNode *child);

void AST_emitSymbolHeaders(List *symbols);

void ASTNode_kill(ASTNode *node);

void AST_emit(AST *ast);

void AST_kill(AST *ast);

void AST_killSymbols(AST *ast);

void AST_statement(ASTNode *statement);

void AST_comparison(ASTNode *comparison);

void AST_expression(ASTNode *expression);

int AST_seenSymbol(AST *ast, char *name);

void AST_addSymbol(AST *ast, char *text, TokenType type);

void Symbol_kill(Symbol *s);

TokenType AST_getSymbolType(char *text);

#endif
