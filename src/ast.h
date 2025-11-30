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
	TokenType subType;
	int lineNumber;
} ASTNode;

typedef struct AST {
	List *children;
	List *symbols;
	List *labelsDeclared;
	List *labelsGotoed;
	Lexer *lex;
	int seenStrInput;
	int currentLineNumber;
} AST;


typedef struct Symbol {
	char *text;
	TokenType type;
} Symbol;

AST *AST_create(Lexer *lex);

ASTNode *ASTNode_create(Token *t);

void AST_abort(const char *message);

void ASTNode_add(ASTNode *parent, ASTNode *child);

void AST_add(AST *parent, ASTNode *child);

void AST_emitSymbolHeaders(List *symbols);

void AST_emitSymbolFrees(List *symbols);

void ASTNode_kill(ASTNode *node);

void AST_check(AST *ast);

void AST_checkStatement(ASTNode *node);

TokenType AST_checkComparison(ASTNode *node);

TokenType AST_checkExpression(ASTNode *node);

TokenType AST_getSubType(TokenType type1, TokenType type2, TokenType operation);

void AST_emit(AST *ast);

void AST_kill(AST *ast);

void AST_killSymbols(AST *ast);

void AST_statement(ASTNode *statement);

int AST_isComparisonOperator(TokenType t);

void AST_comparison(ASTNode *comparison);

void AST_expression(ASTNode *expression);

int AST_seenSymbol(AST *ast, char *name);

void AST_addSymbol(AST *ast, char *text, TokenType type);

void Symbol_kill(Symbol *s);

TokenType AST_getSymbolType(char *text);

#endif
