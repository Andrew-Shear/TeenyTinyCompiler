#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

// only use when needed because i said so, probably could change?
AST *astGlobal;

AST *AST_create(Lexer *lex) {
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

	ast->lex = lex;
	ast->seenStrInput = 0;

	astGlobal = ast;
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
	astNode->subType = eOF;
	return astNode;
}

void AST_abort(const char *message) {
	printf("ERROR AT LINE #%d:\n", astGlobal->lex->lineNumber);
	printf("%s\n", message);
	exit(1);
}

void ASTNode_add(ASTNode *parent, ASTNode *child) {
	List_push(parent->children, child);
}

void AST_add(AST *parent, ASTNode *child) {
	List_push(parent->children, child);
}

void ASTNode_kill(ASTNode *node) {
	if (node == NULL)
		return;
	if (node->token != NULL) {
		Token_kill(node->token);
	}
	LIST_FOREACH(node->children, first, next, cur) {
		ASTNode_kill((ASTNode *) cur->value);
	}
	List_destroy(node->children);
	free(node);
}

void AST_check(AST *ast) {
	LIST_FOREACH(ast->children, first, next, cur) {
		AST_checkStatement((ASTNode *) cur->value);
	}
}

void AST_checkStatement(ASTNode *statement) {
	switch (statement->token->type) {
		ListNode *current;
		ASTNode *temp;

		case PRINT:
			// need to check if the comparison is valid
			temp = (ASTNode *) statement->children->first->value;
			AST_checkComparison(temp);
			break;

		case IF:
		case ELSEIF:
			current = statement->children->first;
			AST_checkComparison((ASTNode *) current->value);
			current = current->next;
			while (current != NULL) {
				temp = (ASTNode *) current->value;
				AST_checkStatement(temp);
				current = current->next;
			}
			break;

		case ELSE:
			current = statement->children->first;
			while (current != NULL) {
				temp = (ASTNode *) current->value;
				AST_checkStatement(temp);
				current = current->next;
			}
			break;
			

		case WHILE:
			current = statement->children->first;
			AST_checkComparison((ASTNode *) current->value);
			current = current->next;
			while (current != NULL) {
				temp = (ASTNode *) current->value;
				AST_checkStatement(temp);
				current = current->next;
			}
			break;

		case FOR:
			current = statement->children->first;
			TokenType symbol = AST_getSymbolType(((ASTNode *) current->value)->token->text);
			((ASTNode *) current->value)->subType = symbol;
			current = current->next;
			TokenType expression1 = AST_checkExpression((ASTNode *) current->value);
			current = current->next;
			TokenType expression2 = AST_checkExpression((ASTNode *) current->value);
			if (symbol != INT_VAR && symbol != FLOAT_VAR)
				AST_abort("Invalid variable type in for loop.");
			if (expression1 != INT_VAR && expression1 != FLOAT_VAR)
				AST_abort("Invalid expression type in for loop.");
			if (expression2 != INT_VAR && expression2 != FLOAT_VAR)
				AST_abort("Invalid expression type in for loop.");
			current = current->next;
			while (current != NULL) {
				temp = (ASTNode *) current->value;
				AST_checkStatement(temp);
				current = current->next;
			}
			break;

		case LABEL:
		case GOTO:
		case INPUT:
			if (astGlobal->seenStrInput == 0) {
				astGlobal->seenStrInput = 1;
			}

			current = statement->children->first;
			temp = (ASTNode *) current->value;
			temp->subType = AST_getSymbolType(temp->token->text);
			if (temp->subType == STRING_VAR)
				AST_abort("Strings are not allowed in input.");
			break;

		case LET:
			current = statement->children->first;
			temp = (ASTNode *) current->value;
			TokenType symbolType = AST_getSymbolType(temp->token->text);
			temp->subType = symbolType;

			current = current->next;
			temp = (ASTNode *) current->value;
			TokenType expType = AST_checkComparison(temp);
			AST_getSubType(symbolType, expType, EQ);
			break;

		default:
			AST_abort("How did we get here?");
			break;
	}
}

TokenType AST_checkComparison(ASTNode *comparison) {
	if (!AST_isComparisonOperator(comparison->token->type)) {
		// it's not a comparison, it's just an expression.
		return AST_checkExpression(comparison);
	}
	ASTNode *child1 = (ASTNode *) comparison->children->first->value;
	ASTNode *child2 = (ASTNode *) comparison->children->last->value;

	TokenType type1 = eOF;
	TokenType type2 = eOF;

	if (AST_isComparisonOperator(child1->token->type))
		type1 = AST_checkComparison(child1);
	else
		type1 = AST_checkExpression(child1);
	
	if (AST_isComparisonOperator(child2->token->type))
		type2 = AST_checkComparison(child2);
	else
		type2 = AST_checkExpression(child2);

	comparison->subType = AST_getSubType(type1, type2, comparison->token->type);
	return comparison->subType;
}

TokenType AST_checkExpression(ASTNode *expression) {
	if (!(expression->token->type == PLUS
		|| expression->token->type == MINUS
		|| expression->token->type == ASTERISK 
		|| expression->token->type == SLASH)) {
		switch (expression->token->type) {
			case IDENT:
				expression->subType = AST_getSymbolType(expression->token->text);
				break;
			case STRING:
				expression->subType = STRING_VAR;
				break;
			case NUMBERINT:
				expression->subType = INT_VAR;
				break;
			case NUMBERFLOAT:
				expression->subType = FLOAT_VAR;
				break;
			case BOOL:
				expression->subType = BOOL_VAR;
				break;
			default:
				AST_abort("checkExpression how did I get here?");
				break;
		}
		return expression->subType;
	}

	ListNode *child1 = expression->children->first;
	ASTNode *child1Node = (ASTNode *) child1->value;
	if (child1Node->token->type == LEFTPAREN) {
		child1 = child1->next;
		child1Node = (ASTNode *) child1->value;
	}

	TokenType type1 = AST_checkExpression(child1Node);
	ASTNode *child2Node = (ASTNode *) child1->next->value;
	TokenType type2 = AST_checkExpression(child2Node);
	expression->subType = AST_getSubType(type1, type2, expression->token->type);
	return expression->subType;
}

TokenType AST_getSubType(TokenType type1, TokenType type2, TokenType operation) {
	switch (operation) {
		case PLUS:
			if (type1 == INT_VAR && type2 == INT_VAR)
				return INT_VAR;
			else if ((type1 == INT_VAR || type1 == FLOAT_VAR) && (type2 == INT_VAR || type2 == FLOAT_VAR))
				return FLOAT_VAR;
			else if (type1 == STRING_VAR && type2 == STRING_VAR)
				return STRING_VAR;
			else
				AST_abort("Invalid types in operation.");
			break;

		case MINUS:
		case ASTERISK:
		case SLASH:
			if (type1 == INT_VAR && type2 == INT_VAR)
				return INT_VAR;
			else if ((type1 == INT_VAR || type1 == FLOAT_VAR) && (type2 == INT_VAR || type2 == FLOAT_VAR))
				return FLOAT_VAR;
			else
				AST_abort("Invalid types in operation.");
			break;

		case LTEQ:
		case GTEQ:
		case GT:
		case LT:
			if ((type1 == INT_VAR || type1 == FLOAT_VAR) && (type2 == INT_VAR || type2 == FLOAT_VAR))
				return BOOL_VAR;
			else
				AST_abort("Invalid types in comparison.");
			break;

		case EQEQ:
		case NOTEQ:
			if ((type1 == INT_VAR || type1 == FLOAT_VAR) && (type2 == INT_VAR || type2 == FLOAT_VAR))
				return BOOL_VAR;
			else if (type1 == BOOL_VAR && type2 == BOOL_VAR)
				return BOOL_VAR;
			else if (type1 == STRING_VAR && type2 == STRING_VAR)
				return STRING_VAR;
			else
				AST_abort("Invalid types in equals/not equals comparison.");
			break;

		case EQ:
			// this one is used for LET, return val doesn't really matter
			if (type1 == INT_VAR && type2 == INT_VAR)
				return INT_VAR;
			else if ((type1 == INT_VAR || type1 == FLOAT_VAR) && (type2 == INT_VAR || type2 == FLOAT_VAR))
				return FLOAT_VAR;
			else if (type1 == STRING_VAR && type2 == STRING_VAR)
				return STRING_VAR;
			else if (type1 == BOOL_VAR && type2 == BOOL_VAR)
				return BOOL_VAR;
			else
				AST_abort("Invalid types in equals.");
			break;

		default:
			AST_abort("Invalid operation.");
			break;
	}
	AST_abort("getSubType how did I get here?");
	return eOF;
}

void AST_emit(AST *ast) {
	Emitter_emitLine("#include <stdio.h>");
	Emitter_emitLine("#include <stdlib.h>");
	Emitter_emitLine("#include <string.h>");
	Emitter_emitLine("");
	Emitter_emitLine("int main (void) {");

	AST_emitSymbolHeaders(ast->symbols);
	if (ast->seenStrInput)
		Emitter_emitLine("size_t len = 0;");

	LIST_FOREACH(ast->children, first, next, cur) {
		AST_statement((ASTNode *) cur->value);
	}

	AST_emitSymbolFrees(ast->symbols);

	Emitter_emitLine("return 0;");
	Emitter_emitLine("}");
}

void AST_emitSymbolHeaders(List *symbols) {
	LIST_FOREACH(symbols, first, next, cur) {
		Symbol *s = (Symbol *) cur->value;
		if (s->type == FLOAT_VAR)
			Emitter_emit("float ");
		else if (s->type == INT_VAR)
			Emitter_emit("int ");
		else if (s->type == BOOL_VAR)
			Emitter_emit("int ");	
		else if (s->type == STRING_VAR)
			Emitter_emit("char *");
		Emitter_emit(s->text);
		Emitter_emitLine(";");
	}
}

void AST_emitSymbolFrees(List *symbols) {
	LIST_FOREACH(symbols, first, next, cur) {
		Symbol *s = (Symbol *) cur->value;
		if (s->type == STRING_VAR) {
			Emitter_emit("free(");
			Emitter_emit(s->text);
			Emitter_emitLine(");");
		}
	}
}

void AST_kill(AST *ast) {
	if (ast == NULL) {
		return;
	}
	LIST_FOREACH(ast->children, first, next, cur) {
		ASTNode_kill((ASTNode *) cur->value);
	}
	AST_killSymbols(ast);
	List_destroy(ast->symbols);
	List_clear_destroy(ast->labelsDeclared);
	List_clear_destroy(ast->labelsGotoed);
	List_destroy(ast->children);
	free(ast);
}

void AST_killSymbols(AST *ast) {
	LIST_FOREACH(ast->symbols, first, next, cur) {
		Symbol_kill((Symbol *) cur->value);
	}

}

// TODO: fix with type checking
void AST_statement(ASTNode *statement) {
	ASTNode *temp = NULL;
	ASTNode *temp2 = NULL;
	switch (statement->token->type) {
		// statement.children = List(comparison)
		case PRINT:
			temp = (ASTNode *) (List_pop(statement->children));
			Emitter_emit("printf(\"%");
			if (temp->subType == INT_VAR || temp->subType == BOOL_VAR) {
				Emitter_emit("d\\n\", (");
			} else if (temp->subType == STRING_VAR) {
				Emitter_emit("s\\n\", (");
			} else {
				Emitter_emit(".2f\\n\", (");
			}
			AST_comparison(temp);
			Emitter_emitLine("));");
			break;
			
		// statement.children = List(comparison, {statement}, [ELSEIF | ELSE])
		// else	   ::= statement.children = {statement}
		// overflow on purpose!
		case IF:
		case ELSEIF:
			Emitter_emit("if(");
			temp = (ASTNode *) (List_shift(statement->children));
			AST_comparison(temp);
			Emitter_emitLine("){");
			ASTNode_kill(temp);

			temp = (ASTNode *) List_shift(statement->children);
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
		
		// statement.children = List(IDENT, expression, expression, {statement});
		case FOR:
			Emitter_emit("for (");
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit(temp->token->text);
			Emitter_emit(" = ");

			temp2 = (ASTNode *) List_shift(statement->children);
			AST_expression(temp2);
			Emitter_emit("; ");
			Emitter_emit(temp->token->text);
			Emitter_emit(" <= ");
			
			ASTNode_kill(temp2);
			temp2 = (ASTNode *) List_shift(statement->children);
			AST_expression(temp2);

			Emitter_emit("; ");
			Emitter_emit(temp->token->text);
			Emitter_emitLine("++) {");
			ASTNode_kill(temp2);

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

		// statement.children = List(IDENT, comparison)
		case LET:
			temp = (ASTNode *) List_shift(statement->children);
			Emitter_emit(temp->token->text);
			Emitter_emit(" = ");

			temp2 = (ASTNode *) List_shift(statement->children);
			if (AST_getSymbolType(temp->token->text) == STRING_VAR) {
				Emitter_emit("strdup(");
				AST_comparison(temp2);
				Emitter_emitLine(");");
				break;
			}

			AST_comparison(temp2);
			if (AST_getSymbolType(temp->token->text) == BOOL_VAR)
				Emitter_emit(" == 0 ? 0 : 1");
			Emitter_emitLine(";");
			break;

		// statement.children = List(IDENT)
		case INPUT:
			temp = (ASTNode *) List_shift(statement->children);
			TokenType symType = AST_getSymbolType(temp->token->text); 
			if (symType == STRING_VAR) {
				Emitter_emit("getline(&");
				Emitter_emit(temp->token->text);
				Emitter_emitLine(", &len, stdin);");
				break;
			}

			Emitter_emit("if(0 == scanf(\"%");
			if (symType == INT_VAR || symType == BOOL_VAR)
				Emitter_emit("d\", &");
			else  // FLOAT_VAR
				Emitter_emit("f\", &");

			Emitter_emit(temp->token->text);
			Emitter_emitLine(")) {");
			Emitter_emit(temp->token->text);
			Emitter_emitLine(" = 0;");
			Emitter_emit("scanf(\"%");
			Emitter_emitLine("*s\");");
			Emitter_emitLine("}");
			if (AST_getSymbolType(temp->token->text) == BOOL_VAR) {
				Emitter_emit(temp->token->text);
				Emitter_emit(" = ");
				Emitter_emit(temp->token->text);
				Emitter_emitLine(" == 0 ? 0 : 1;");
			}
			break;

		default:
			AST_abort("checkStatement how did I get here?");
			break;
	}
	if (temp != NULL) {
		ASTNode_kill(temp);
	}
	if (temp2 != NULL) {
		ASTNode_kill(temp2);
	}

}

int AST_isComparisonOperator(TokenType t) {
	return (t == EQEQ
		|| t == NOTEQ
		|| t == LT
		|| t == LTEQ
		|| t == GT 
		|| t == GTEQ);
}

//  if comparison->token->text is a comparison operator:
//		comparison ::= comparison.children = comparison | expression, comparison | expression
// 	else:
//  	comparison ::= comparison.children = ()
void AST_comparison(ASTNode *comparison) {
	if (!AST_isComparisonOperator(comparison->token->type)) {
		AST_expression(comparison);
		return;
	}

	ASTNode *child1 = (ASTNode *) List_shift(comparison->children);
	ASTNode *child2 = (ASTNode *) List_shift(comparison->children);

	if (child1->subType == STRING_VAR) {
		// we're comparing two strings
		Emitter_emit("strcmp(");
		AST_comparison(child1);
		Emitter_emit(", ");
		AST_comparison(child2);
		Emitter_emit(") ");
		Emitter_emit(comparison->token->text);
		Emitter_emit(" 0");
		return;
	}

	AST_comparison(child1);

	Emitter_emit(comparison->token->text);
	
	AST_comparison(child2);

	ASTNode_kill(child1);
	ASTNode_kill(child2);
}

//  if expression->token->text is an operator:
// 		expression.children = List(["("], expression, expression)
// 	else:
// 		expression.children = List()
void AST_expression(ASTNode *expression) {
	if (expression->token->type == STRING) {
		Emitter_emit("\"");
		Emitter_emit(expression->token->text);
		Emitter_emit("\"");
		return;
	} else if (!(expression->token->type == PLUS
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

int AST_seenSymbol(AST *ast, char *name) {
	LIST_FOREACH(ast->symbols, first, next, cur) {
		Symbol *c = (Symbol *) cur->value;
		if (strcmp(c->text, name) == 0) {
			return 1;
		}
	}
	return 0;
}

void AST_addSymbol(AST *ast, char *text, TokenType type) {
	Symbol *s = malloc(sizeof(Symbol));
	s->text = strdup(text);
	s->type = type;
	List_push(ast->symbols, s);
}

TokenType AST_getSymbolType(char *text) {
	LIST_FOREACH(astGlobal->symbols, first, next, cur) {
		Symbol *c = (Symbol *) cur->value;
		if (strcmp(c->text, text) == 0) {
			return c->type;
		}
	}
	return eOF;
}

void Symbol_kill(Symbol *s) {
	if (s == NULL)
		return;
	free(s->text);
	free(s);
}
