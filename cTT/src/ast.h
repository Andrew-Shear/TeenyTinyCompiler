#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
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

AST *AST_create();

ASTNode *ASTNode_create(Token *t);

void ASTNode_add(ASTNode *parent, ASTNode *child);

void AST_add(AST *parent, ASTNode *child);

void AST_emitSymbolHeaders(List *symbols);

void ASTNode_kill(ASTNode *node);

void AST_emit(AST *ast);

void AST_kill(AST *ast);

void AST_statement(ASTNode *statement);

void AST_comparison(ASTNode *comparison);

void AST_expression(ASTNode *expression);

#endif
