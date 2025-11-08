#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

Parser *Parser_create(Lexer *lex, AST *ast) {
	Parser *par = malloc(sizeof(Parser));
	if (par == NULL) {
		Lexer_kill(lex);
		printf("Unable to allocate memory for parser.\n");
		exit(1);
	}
	
	par->lex = lex;
	par->ast = ast;
	par->curToken = Lexer_getToken(lex);
	par->peekToken = Lexer_getToken(lex);
	return par;
}

void Parser_kill(Parser *par) {
	if (par == NULL)
		return;
	Token_kill(par->curToken);
	Token_kill(par->peekToken);
	free(par);
}

void Parser_nextToken(Parser *par) {
	Token_kill(par->curToken);
	par->curToken = par->peekToken;
	par->peekToken = Lexer_getToken(par->lex);
}

void Parser_abort(Parser *par, char *message) {
	printf("ERROR AT LINE #%d:\n", par->lex->lineNumber);
	printf(message);
	printf("\n");
	Lexer_kill(par->lex);
	AST_kill(par->ast);
	Parser_kill(par);
	exit(1);
}	

void Parser_match(Parser *par, TokenType type) {
	if (par->curToken->type != type) {
		Parser_abort(par, "Syntax error.");
	}
	Parser_nextToken(par);
}

// program ::= {statement}
void Parser_program(Parser *par) {
	while (par->curToken->type == NEWLINE) {
		Parser_nextToken(par);
	}

	ASTNode *statement;
	while (par->curToken->type != eOF) {
		statement = Parser_statement(par);
		AST_add(par->ast, statement);
	}

	LIST_FOREACH(par->ast->labelsGotoed, first, next, cur) {
		if (!(List_contains(par->ast->labelsDeclared, (char *) cur->value))) {
			Parser_abort(par, "Attempted to GOTO an undeclared label.");
		}
	}
}

// statement ::= print | if | while | for | label | goto | let | input
ASTNode *Parser_statement(Parser *par) {
	ASTNode *statement = NULL;

	switch (par->curToken->type) {
		case PRINT:
			statement = Parser_print(par);
			break;
			
		case IF:
			statement = Parser_if(par);
			break;
		
		case WHILE:
			statement = Parser_while(par);
			break;
			
		case FOR:
			statement = Parser_for(par);
			break;

		case LABEL:
			statement = Parser_label(par);
			break;

		case GOTO:
			statement = Parser_goto(par);
			break;

		case LET:
			statement = Parser_let(par);
			break;

		case INPUT:
			statement = Parser_input(par);
			break;
		
		default:
			Parser_abort(par, "Invalid statement.");
	}

	Parser_nl(par);

	return statement;
}

// print ::= "PRINT" (expression | string) nl
ASTNode *Parser_print(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);
	if (par->curToken->type == STRING) {
		ASTNode *child = ASTNode_create(par->curToken);
		Parser_nextToken(par);
		ASTNode_add(statement, child);
	} else {
		ASTNode_add(statement, Parser_expression(par));
	}

	return statement;
}

// if ::= "IF" comparison "THEN" nl {statement} {"ELSEIF" comparison "THEN" nl {statement}} [ELSE nl {statement}] "ENDIF" nl
ASTNode *Parser_if(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	
	Parser_nextToken(par);
	ASTNode_add(statement, Parser_comparison(par));

	Parser_match(par, THEN);
	Parser_nl(par);

	while (par->curToken->type != ENDIF && par->curToken->type != ELSEIF && par->curToken->type != ELSE) {
		ASTNode_add(statement, Parser_statement(par));
	}
	
	ASTNode *previous = statement;
	while (par->curToken->type == ELSEIF) {
		ASTNode *current = ASTNode_create(par->curToken);
		ASTNode_add(previous, current);
		
		Parser_nextToken(par);
		ASTNode_add(current, Parser_comparison(par));

		Parser_match(par, THEN);
		Parser_nl(par);

		while (par->curToken->type != ENDIF && par->curToken->type != ELSEIF && par->curToken->type != ELSE) {
			ASTNode_add(current, Parser_statement(par));
		}

		previous = current;
		current = NULL;
	}

	if (par->curToken->type == ELSE) {
		ASTNode *current = ASTNode_create(par->curToken);
		ASTNode_add(previous, current);

		Parser_nextToken(par);
		Parser_nl(par);

		while (par->curToken->type != ENDIF) {
			ASTNode_add(current, Parser_statement(par));
		}
	}

	Parser_match(par, ENDIF);
	return statement;
}


// while ::= "WHILE" comparison "REPEAT" nl {statement} "ENDWHILE" nl
ASTNode *Parser_while(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);
	ASTNode_add(statement, Parser_comparison(par));

	Parser_match(par, REPEAT);
	Parser_nl(par);

	while (par->curToken->type != ENDWHILE) {
		ASTNode_add(statement, Parser_statement(par));
	}

	Parser_match(par, ENDWHILE);
	return statement;
}

// for ::= "FOR" ["INT" || "FLOAT"] ident "=" expression "TO" expression "REPEAT" nl {statement} "ENDFOR" nl
ASTNode *Parser_for(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);

	ASTNode *variable = Parser_variable(par);
	if (variable != NULL) {
		// declaring a new symbol 
		TokenType varType = variable->token->type;
		ASTNode_kill(variable);
		if (varType != INT && varType != FLOAT) {
			Parser_abort(par, "Attempted to create a for loop without an integer or float.");
		}

		if (AST_seenSymbol(par->ast, par->curToken->text)) {
			Parser_abort(par, "Attemped to declare a variable that was previously used.");
		}

		AST_addSymbol(par->ast, par->curToken->text, varType);
	} else {
		// not declaring a new symbol, so if i haven't seen it, kill the program.
		if (!(AST_seenSymbol(par->ast, par->curToken->text))){
			Parser_abort(par, "Attempted to set a new variable without declaring a type.");
		}
	}

	ASTNode_add(statement, ASTNode_create(par->curToken));
	Parser_match(par, IDENT);
	Parser_match(par, EQ);
	ASTNode_add(statement, Parser_expression(par));

	Parser_match(par, TO);
	ASTNode_add(statement, Parser_expression(par));
	Parser_match(par, REPEAT);
	Parser_nl(par);

	while (par->curToken->type != ENDFOR) {
		ASTNode_add(statement, Parser_statement(par));
	}

	Parser_match(par, ENDFOR);
	
	return statement;
}

// label ::= "LABEL" ident nl
ASTNode *Parser_label(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);

	if (List_contains(par->ast->labelsDeclared, par->curToken->text))
		Parser_abort(par, "Label declared twice.");
	List_push(par->ast->labelsDeclared, strdup(par->curToken->text));
	ASTNode_add(statement, ASTNode_create(par->curToken));

	Parser_match(par, IDENT);
	return statement;
}

// goto ::= "GOTO" ident nl
ASTNode *Parser_goto(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);

	List_push(par->ast->labelsGotoed, strdup(par->curToken->text));
	ASTNode_add(statement, ASTNode_create(par->curToken));
	
	Parser_match(par, IDENT);
	return statement;
}

// let ::= "LET" variable ident "=" expression nl
ASTNode *Parser_let(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);

	ASTNode *variable = Parser_variable(par);
	if (variable != NULL) {
		// declaring a new symbol 
		TokenType varType = variable->token->type;
		ASTNode_kill(variable);
		if (AST_seenSymbol(par->ast, par->curToken->text)) {
			Parser_abort(par, "Attemped to declare a variable that was previously used.\n");
		}
		AST_addSymbol(par->ast, par->curToken->text, varType);
	} else {
		// not declaring a new symbol, so if i haven't seen it, kill the program.
		if (!(AST_seenSymbol(par->ast, par->curToken->text))){
			Parser_abort(par, "Attempted to set a new variable without declaring a type.\n");
		}
	}

	ASTNode_add(statement, ASTNode_create(par->curToken));
	Parser_match(par, IDENT);
	Parser_match(par, EQ);
	ASTNode_add(statement, Parser_expression(par));

	return statement;
}

// input ::= "INPUT" variable ident nl
ASTNode *Parser_input(Parser *par) {
	ASTNode *statement = ASTNode_create(par->curToken);
	Parser_nextToken(par);

	ASTNode *variable = Parser_variable(par);
	if (variable != NULL) {
		// declaring a new symbol 
		TokenType varType = variable->token->type;
		ASTNode_kill(variable);
		if (AST_seenSymbol(par->ast, par->curToken->text)) {
			Parser_abort(par, "Attemped to declare a variable that was previously used.\n");
		}
		AST_addSymbol(par->ast, par->curToken->text, varType);
	} else {
		// not declaring a new symbol, so if i haven't seen it, kill the program.
		if (!(AST_seenSymbol(par->ast, par->curToken->text))){
			Parser_abort(par, "Attempted to set a new variable without declaring a type.\n");
		}
	}

	ASTNode_add(statement, ASTNode_create(par->curToken));
	Parser_match(par, IDENT);

	return statement;
}

// comparison ::= expression (("==" | "!=" | ">" | ">=" | "<" | "<=") expression)+
ASTNode *Parser_comparison(Parser *par) {
	ASTNode *expression = Parser_expression(par);
	ASTNode *comparison = ASTNode_create(par->curToken);
	ASTNode_add(comparison, expression);

	if (par->curToken->type == GT   ||
			par->curToken->type == GTEQ ||
			par->curToken->type == LT   ||
			par->curToken->type == LTEQ ||
			par->curToken->type == EQEQ ||
		par->curToken->type == NOTEQ) {
		
		Parser_nextToken(par);
		ASTNode_add(comparison, Parser_expression(par));
	} else {
		ASTNode_kill(comparison);
		Parser_abort(par, "Expected comparison operator.");
	}
	
	ASTNode *previous = comparison;
	ASTNode *newNode = NULL;
	while (par->curToken->type == GT   ||
		par->curToken->type == GTEQ ||
		par->curToken->type == LT   ||
		par->curToken->type == LTEQ ||
		par->curToken->type == EQEQ ||
		par->curToken->type == NOTEQ) {
		
		newNode = ASTNode_create(par->curToken);
		ASTNode_add(newNode, List_pop(previous->children));
		ASTNode_add(previous, newNode);

		Parser_nextToken(par);
		ASTNode_add(newNode, Parser_expression(par));
		previous = newNode;
	}

	return comparison;
}

// expression ::= ["("] term [( "-" | "+" ) expression] [")"]
ASTNode *Parser_expression(Parser *par) {
	ASTNode *expression = NULL;
	int paren = 0;
	Token *parenToken = NULL;
	if (par->curToken->type == LEFTPAREN) {
		paren = 1;
		parenToken = par->curToken;
		Parser_nextToken(par);
	}

	ASTNode *term = Parser_term(par);
	if (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		expression = ASTNode_create(par->curToken);
		ASTNode_add(expression, term);

		Parser_nextToken(par);
		ASTNode_add(expression, Parser_expression(par));
	} else {
		expression = term;
	}

	if (paren == 1 && par->curToken->type == RIGHTPAREN) {
		Parser_nextToken(par);
		ASTNode *paren = ASTNode_create(parenToken);
		List_unshift(expression->children, paren);
	} else if (paren == 1) {
		Parser_abort(par, "Missing closing parenthesis.");
	} 

	return expression;
}

// term ::= unary {( "/" | "*" ) unary}
ASTNode *Parser_term(Parser *par) {
	ASTNode *term = Parser_unary(par);
	ASTNode *temp = NULL;

	while (par->curToken->type == ASTERISK || par->curToken->type == SLASH) {
		temp = term;
		term = ASTNode_create(par->curToken);
		ASTNode_add(term, temp);

		Parser_nextToken(par);
		ASTNode_add(term, Parser_unary(par));
	}

	return term;
}

// unary ::= ["+" | "-"] primary
ASTNode *Parser_unary(Parser *par) {
	ASTNode *unary = NULL;
	if (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		unary = ASTNode_create(par->curToken);

		Token *zeroToken = malloc(sizeof(Token));
		zeroToken->text = strdup("0");
		zeroToken->type = NUMBER;

		ASTNode *zeroNode = ASTNode_create(zeroToken);

		ASTNode_add(unary, zeroNode);
		Parser_nextToken(par);
		ASTNode_add(unary, Parser_primary(par));
	} else {
		unary = Parser_primary(par);
	}

	return unary;
}

// primary ::= number | ident | string
ASTNode *Parser_primary(Parser *par) {
	ASTNode *primary = ASTNode_create(par->curToken);

	if (par->curToken->type == NUMBER || par->curToken->type == STRING) {
		Parser_nextToken(par);
	} else if (par->curToken->type == IDENT) {
		if (!(AST_seenSymbol(par->ast, par->curToken->text))) {
			Parser_abort(par, "Referenced variable before assignment.");
		}
		Parser_nextToken(par);
	} else {
		Parser_abort(par, "Unexpected token in primary.");
	}

	return primary;
}

// variable ::= ["INT" | "FLOAT" | "BOOL" | "STRING"]
ASTNode *Parser_variable(Parser *par) {
	ASTNode *variable = NULL;

	switch (par->curToken->type) {
		case INT:
		case FLOAT:
		case BOOL:
		case STRING:
			variable = ASTNode_create(par->curToken);
			Parser_nextToken(par);
			break;
	}

	return variable;
}

// nl ::= '\n'+
void Parser_nl(Parser *par) {
	Parser_match(par, NEWLINE);
	while (par->curToken->type == NEWLINE) {
		Parser_nextToken(par);
	}
}

int List_contains(List *l, char *word) {
	if (l->first == NULL)
		return 0;
	if (l->first == l->last)
		return strcmp((char *) l->first->value, word) == 0;

	ListNode *n = l->first;
	while (n != NULL) {
		if (strcmp((char *) n->value, word) == 0) {
			return 1;
		}
		n = n->next;
	}
	return 0;
}

