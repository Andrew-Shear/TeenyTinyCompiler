#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

AST *AST_create() {
	AST *ast = malloc(sizeof(AST));
	if (ast == NULL) {
		return NULL;
	}
	ast->children = List_create();
	if (ast->children == NULL) {
		free(ast);
		printf("Unable to allocate memory for ast children list.\n");
		exit(1);
	}

	ast->symbols = List_create();
	if (ast->symbols == NULL) {
		free(ast->children);
		free(ast);
		printf("Unable to allocate memory for symbols list.\n");
		exit(1);
	}

	ast->labelsDeclared = List_create();
	if (ast->labelsDeclared == NULL) {
		free(ast->children);
		free(ast->symbols);
		free(ast);
		printf("Unable to allocate memory for labelsDeclared list.\n");
		exit(1);
	}
	ast->labelsGotoed = List_create();
	if (ast->labelsGotoed == NULL) {
		free(ast->children);
		free(ast->symbols);
		free(ast->labelsDeclared);
		free(ast);
		printf("Unable to allocate memory for labelsGotoed list.\n");
		exit(1);
	}
	return ast;
}

ASTNode *ASTNode_create(Token *t) {
	ASTNode *astNode = malloc(sizeof(ASTNode));
	if (astNode == NULL) {
		printf("memory allocated to ast was null.\n");
		return NULL;
	}

	astNode->token = Token_copy(t);
	astNode->children = List_create();
	return astNode;
}

void ASTNode_add(ASTNode *parent, ASTNode *child) {
	List_push(parent->children, child);
}

void AST_add(AST *parent, ASTNode *child) {
	List_push(parent->children, child);
}

void ASTNode_kill(ASTNode *node) {
	if (node->token != NULL) {
		Token_kill(node->token);
	}
	LIST_FOREACH(node->children, first, next, cur) {
		ASTNode_kill((ASTNode *) cur->value);
	}
	List_clear_destroy(node->children);
	free(node);
}

void AST_emit(AST *ast) {
	Emitter_headerLine("#include <stdio.h>");
	Emitter_headerLine("int main (void) {");
	AST_emitSymbolHeaders(ast->symbols);

	LIST_FOREACH(ast->children, first, next, cur) {
		AST_statement((ASTNode *) cur->value);
	}

	Emitter_emitLine("return 0;");
	Emitter_emitLine("}");
}

void AST_emitSymbolHeaders(List *symbols) {
	LIST_FOREACH(symbols, first, next, cur) {
		Emitter_header("float ");
		Emitter_header((char *) cur->value);
		Emitter_headerLine(";");
	}
}

void AST_kill(AST *ast) {
	if (ast == NULL) {
		return;
	}
	LIST_FOREACH(ast->children, first, next, cur) {
		ASTNode_kill((ASTNode *) cur->value);
	}
	List_clear_destroy(ast->symbols);
	List_clear_destroy(ast->labelsDeclared);
	List_clear_destroy(ast->labelsGotoed);
	free(ast);
}

void AST_statement(ASTNode *statement) {
	ASTNode *temp = NULL;
	ASTNode *temp2 = NULL;
	switch (statement->token->type) {
		// statement.children = List(string | expression)
		case PRINT:
			temp = (ASTNode *) (List_pop(statement->children));
			if (temp->token->type == STRING) {	
				Emitter_emit("printf(\"");
				Emitter_emit(temp->token->text);
				Emitter_emitLine("\\n\");");
			} else {
				Emitter_emit("printf(\"%");
				Emitter_emit(".2f\\n\", (float)(");
				AST_expression(temp);
				Emitter_emitLine("));");
			}
			break;
			
		// statement.children = List(comparison, {statement}, [ELSEIF | ELSE])
		// overflow on purpose!
		case IF:
		case ELSEIF:
			Emitter_emit("if(");
			temp = (ASTNode *) (List_shift(statement->children));
			AST_comparison(temp);
			Emitter_emitLine("){");
			ASTNode_kill(temp);

			temp = List_shift(statement->children);
			while (temp != NULL && temp->token->type != ELSEIF && temp->token->type != ELSE) {
				AST_statement(temp);
				ASTNode_kill(temp);
				temp = List_shift(statement->children);
			}
			
			if (temp != NULL && temp->token->type == ELSEIF) {
				Emitter_emit("} else ");
				AST_statement(temp); // ELSEIF == IF
			} else if (temp != NULL && temp->token->type == ELSE){
				Emitter_emitLine("} else {");
				while (temp->children->first != NULL) {
					temp2 = (ASTNode *) List_shift(temp->children);
					AST_statement(temp2);
					ASTNode_kill(temp2);
					temp2 = NULL;
				}
				Emitter_emitLine("}");
			} else {
				Emitter_emitLine("}");
			}

			break;
		
		// statement.children = List(comparison, {statement})
		case WHILE:
			Emitter_emit("while (");
			temp = (ASTNode *) List_shift(statement->children);
			AST_comparison(temp);
			Emitter_emitLine(") {");

			while (statement->children->first != NULL) {
				temp2 = (ASTNode *) List_shift(statement->children);
				AST_statement(temp2);
				ASTNode_kill(temp2);
				temp2 = NULL;
			}
			Emitter_emitLine("}");
			break;
		
		// statement.children = List(IDENT)
		case LABEL:
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit(temp->token->text);
			Emitter_emitLine(":");
			break;

		// statement.children = List(IDENT)
		case GOTO:
			Emitter_emit("goto ");
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit(temp->token->text);
			Emitter_emitLine(";");
			break;

		// statement.children = List(IDENT, expression)
		case LET:
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit(temp->token->text);
			Emitter_emit(" = ");

			temp2 = (ASTNode *) List_shift(statement->children);
			AST_expression(temp2);
			Emitter_emitLine(";");
			break;

		// statement.children = List(IDENT)
		case INPUT:
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit("if(0 == scanf(\"%");
			Emitter_emit("f\", &");
			Emitter_emit(temp->token->text);
			Emitter_emitLine(")) {");
			Emitter_emit(temp->token->text);
			Emitter_emitLine(" = 0;");
			Emitter_emit("scanf(\"%");
			Emitter_emitLine("*s\");");
			Emitter_emitLine("}");
			break;
	}
	if (temp != NULL) {
		ASTNode_kill(temp);
	}
	if (temp2 != NULL) {
		ASTNode_kill(temp2);
	}

}

// comparison.children = List(comparisionOperator)
// comparisonOperator.children = List(comparison | expression, comparison | expression)
void AST_comparison(ASTNode *comparison) {
	ASTNode *child1 = (ASTNode *) List_shift(comparison->children);
	ASTNode *child2 = (ASTNode *) List_shift(comparison->children);

	if (child1->token->type == EQEQ
		|| child1->token->type == NOTEQ
		|| child1->token->type == LT
		|| child1->token->type == LTEQ
		|| child1->token->type == GT 
		|| child1->token->type == GTEQ) {
		AST_comparison(child1);
	} else {
		AST_expression(child1);
	}

	Emitter_emit(comparison->token->text);
	
	if (child2->token->type == EQEQ
		|| child2->token->type == NOTEQ
		|| child2->token->type == LT
		|| child2->token->type == LTEQ
		|| child2->token->type == GT 
		|| child2->token->type == GTEQ) {
		AST_comparison(child2);
	} else {
		AST_expression(child2);
	}

	ASTNode_kill(child1);
	ASTNode_kill(child2);
}

//  if expression->token->text is an operator:
// 		expression.children = List(["("], expression, expression)
// 	else:
// 		expression.children = List(["("])
void AST_expression(ASTNode *expression) {
	if (!(expression->token->type == PLUS
		|| expression->token->type == MINUS
		|| expression->token->type == ASTERISK 
		|| expression->token->type == SLASH)) {
		Emitter_emit(expression->token->text);
		return;
	}
	ASTNode *child1 = (ASTNode *) List_shift(expression->children);
	int paren = 0;
	if (child1->token->type == LEFTPAREN) {
		Emitter_emit("(");
		paren = 1;
		child1 = (ASTNode *) List_shift(expression->children);
	}
	ASTNode *child2 = (ASTNode *) List_shift(expression->children);

	AST_expression(child1);
	Emitter_emit(expression->token->text);
	AST_expression(child2);

	if (paren == 1) {
		Emitter_emit(")");
	}

	ASTNode_kill(child1);
	ASTNode_kill(child2);
}

