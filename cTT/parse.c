#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

Parser *Parser_create(Lexer *lex) {
	Parser *par = malloc(sizeof(Parser));
	if (par == NULL) {
		Lexer_kill(lex);
		printf("Unable to allocate memory for parser.\n");
		exit(1);
	}
	
	par->lex = lex;
	par->curToken = Lexer_getToken(lex);
	par->peekToken = Lexer_getToken(lex);
	return par;
}

void Parser_kill(Parser *par) {
	Lexer_kill(par->lex);
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
	printf(message);
	printf("\n");
	Lexer_kill(par->lex);
	Parser_kill(par);
	exit(1);
}	

void Parser_match(Parser *par, TokenType type) {
	if (par->curToken->type != type) {
		Parser_abort(par, "Syntax error.");
	}
	Parser_nextToken(par);
}

void Parser_program(Parser *par) {
	printf("PROGRAM\n");

	while (par->curToken->type != eOF) {
		Parser_statement(par);
	}
}

void Parser_statement(Parser *par) {
	switch (par->curToken->type) {
		case PRINT:
			printf("STATEMENT-PRINT\n");
			Parser_nextToken(par);
			if (par->curToken->type == STRING) {
				Parser_nextToken(par);
			} else {
				Parser_expression(par);
			}
			break;
			
		case IF:
			printf("STATEMENT-IF\n");
			Parser_nextToken(par);
			Parser_comparison(par);

			Parser_match(par, THEN);
			Parser_nl(par);

			while (par->curToken->type != ENDIF) {
				Parser_statement(par);
			}

			Parser_match(par, ENDIF);
			break;
		
		case WHILE:
			printf("STATEMENT-WHILE\n");
			Parser_nextToken(par);
			Parser_comparison(par);

			Parser_match(par, REPEAT);
			Parser_nl(par);

			while (par->curToken->type != ENDWHILE) {
				Parser_statement(par);
			}

			Parser_match(par, ENDWHILE);
			break;
			
		case LABEL:
			printf("STATEMENT-LABEL\n");
			Parser_nextToken(par);
			Parser_match(par, IDENT);
			break;

		case GOTO:
			printf("STATEMENT-GOTO\n");
			Parser_nextToken(par);
			Parser_match(par, IDENT);
			break;

		case LET:
			printf("STATEMENT-LET\n");
			Parser_nextToken(par);
			Parser_match(par, IDENT);
			Parser_match(par, EQ);
			Parser_expression(par);
			break;

		case INPUT:
			printf("STATEMENT-INPUT\n");
			Parser_nextToken(par);
			Parser_match(par, IDENT);
			break;
		
		default:
			Parser_abort(par, "Invalid statement.");
	}
	Parser_nl(par);
}

void Parser_comparison(Parser *par) {
	printf("COMPARISON\n");

	Parser_expression(par);
	if (par->curToken->type == GT   ||
		par->curToken->type == GTEQ ||
		par->curToken->type == LT   ||
		par->curToken->type == LTEQ ||
		par->curToken->type == EQEQ ||
		par->curToken->type == NOTEQ) {
		Parser_nextToken(par);
		Parser_expression(par);
	} else {
		Parser_abort(par, "Expected comparison operator.");
	}

	while (par->curToken->type == GT   ||
		par->curToken->type == GTEQ ||
		par->curToken->type == LT   ||
		par->curToken->type == LTEQ ||
		par->curToken->type == EQEQ ||
		par->curToken->type == NOTEQ) {
		Parser_nextToken(par);
		Parser_expression(par);
	}
}

void Parser_expression(Parser *par) {
	printf("EXPRESSION\n");

	Parser_term(par);
	while (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		Parser_nextToken(par);
		Parser_term(par);
	}
}

void Parser_term(Parser *par) {
	printf("TERM\n");

	Parser_unary(par);
	while (par->curToken->type == ASTERISK || par->curToken->type == SLASH) {
		Parser_nextToken(par);
		Parser_unary(par);
	}
}

void Parser_unary(Parser *par) {
	printf("UNARY\n");

	if (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		Parser_nextToken(par);
	}
	Parser_primary(par);
}

void Parser_primary(Parser *par) {
	printf("PRIMARY (");
	printf(par->curToken->text);
	printf(")\n");

	if (par->curToken->type == NUMBER) {
		Parser_nextToken(par);
	} else if (par->curToken->type == IDENT) {
		Parser_nextToken(par);
	} else {
		Parser_abort(par, "Unexpected token in primary.");
	}
}

void Parser_nl(Parser *par) {
	printf("NEWLINE\n");

	Parser_match(par, NEWLINE);
	while (par->curToken->type == NEWLINE) {
		Parser_nextToken(par);
	}
}

