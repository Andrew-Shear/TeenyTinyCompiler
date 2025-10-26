#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parse.h"

Parser *Parser_create(Lexer *lex, Emitter *emit) {
	Parser *par = malloc(sizeof(Parser));
	if (par == NULL) {
		Lexer_kill(lex);
		printf("Unable to allocate memory for parser.\n");
		exit(1);
	}

	par->symbols = List_create();
	if (par->symbols == NULL) {
		Lexer_kill(lex);
		free(par);
		printf("Unable to allocate memory for symbols list.\n");
		exit(1);
	}

	par->labelsDeclared = List_create();
	if (par->labelsDeclared == NULL) {
		Lexer_kill(lex);
		free(par);
		free(par->symbols);
		printf("Unable to allocate memory for labelsDeclared list.\n");
		exit(1);
	}
	par->labelsGotoed = List_create();
	if (par->labelsGotoed == NULL) {
		Lexer_kill(lex);
		free(par);
		free(par->symbols);
		free(par->labelsDeclared);
		printf("Unable to allocate memory for labelsGotoed list.\n");
		exit(1);
	}
	
	par->lex = lex;
	par->emit = emit;
	par->curToken = Lexer_getToken(lex);
	par->peekToken = Lexer_getToken(lex);
	return par;
}

void Parser_kill(Parser *par) {
	Lexer_kill(par->lex);
	Token_kill(par->curToken);
	Token_kill(par->peekToken);
	List_clear_destroy(par->symbols);
	List_clear_destroy(par->labelsDeclared);
	List_clear_destroy(par->labelsGotoed);
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
	Emitter_kill(par->emit);
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
	Emitter_headerLine(par->emit, "#include <stdio.h>");
	Emitter_headerLine(par->emit, "int main (void) {");
	
	while (par->curToken->type == NEWLINE) {
		Parser_nextToken(par);
	}

	while (par->curToken->type != eOF) {
		Parser_statement(par);
	}

	Emitter_emitLine(par->emit, "return 0;");
	Emitter_emitLine(par->emit, "}");

	LIST_FOREACH(par->labelsGotoed, first, next, cur) {
		if (!(List_contains(par->labelsDeclared, (char *) cur->value))) {
			Parser_abort(par, "Attempted to GOTO an undeclared label.");
		}
	}
}

void Parser_statement(Parser *par) {
	switch (par->curToken->type) {
		case PRINT:
			Parser_nextToken(par);
			if (par->curToken->type == STRING) {
				Emitter_emit(par->emit, "printf(\"");
				Emitter_emit(par->emit, par->curToken->text);
				Emitter_emitLine(par->emit, "\\n\");");
				Parser_nextToken(par);
			} else {
				Emitter_emit(par->emit, "printf(\"%");
				Emitter_emit(par->emit, ".2f\\n\", (float)(");
				Parser_expression(par);
				Emitter_emitLine(par->emit, "));");
			}
			break;
			
		case IF:
			Parser_nextToken(par);
			Emitter_emit(par->emit, "if(");
			Parser_comparison(par);

			Parser_match(par, THEN);
			Parser_nl(par);
			Emitter_emitLine(par->emit, "){");

			while (par->curToken->type != ENDIF) {
				Parser_statement(par);
			}

			Parser_match(par, ENDIF);
			Emitter_emitLine(par->emit, "}");
			break;
		
		case WHILE:
			Parser_nextToken(par);
			Emitter_emit(par->emit, "while(");
			Parser_comparison(par);

			Parser_match(par, REPEAT);
			Parser_nl(par);
			Emitter_emitLine(par->emit, "){");

			while (par->curToken->type != ENDWHILE) {
				Parser_statement(par);
			}

			Parser_match(par, ENDWHILE);
			Emitter_emitLine(par->emit, "}");
			break;
			
		case LABEL:
			Parser_nextToken(par);
			if (List_contains(par->labelsDeclared, par->curToken->text))
				Parser_abort(par, "Label declared twice.");
			List_push(par->labelsDeclared, strdup(par->curToken->text));

			Emitter_emit(par->emit, par->curToken->text);
			Emitter_emitLine(par->emit, ":");
			Parser_match(par, IDENT);
			break;

		case GOTO:
			Parser_nextToken(par);
			List_push(par->labelsGotoed, strdup(par->curToken->text));
			Emitter_emit(par->emit, "goto ");
			Emitter_emit(par->emit, par->curToken->text);
			Emitter_emitLine(par->emit, ";");
			Parser_match(par, IDENT);
			break;

		case LET:
			Parser_nextToken(par);

			if (!(List_contains(par->symbols, par->curToken->text))) {
				List_push(par->symbols, strdup(par->curToken->text));
				Emitter_header(par->emit, "float ");
				Emitter_header(par->emit, par->curToken->text);
				Emitter_headerLine(par->emit, ";");
			}

			Emitter_emit(par->emit, par->curToken->text);
			Emitter_emit(par->emit, " = ");

			Parser_match(par, IDENT);
			Parser_match(par, EQ);
			Parser_expression(par);
			Emitter_emitLine(par->emit, ";");
			break;

		case INPUT:
			Parser_nextToken(par);
			
			if (!(List_contains(par->symbols, par->curToken->text))) {
				List_push(par->symbols, strdup(par->curToken->text));
				Emitter_header(par->emit, "float ");
				Emitter_header(par->emit, par->curToken->text);
				Emitter_headerLine(par->emit, ";");
			}
			
			Emitter_emit(par->emit, "if(0 == scanf(\"%");
			Emitter_emit(par->emit, "f\", &");
			Emitter_emit(par->emit, par->curToken->text);
			Emitter_emitLine(par->emit, ")) {");
			Emitter_emit(par->emit, par->curToken->text);
			Emitter_emitLine(par->emit, " = 0;");
			Emitter_emit(par->emit, "scanf(\"%");
			Emitter_emitLine(par->emit, "*s\");");
			Emitter_emitLine(par->emit, "}");

			Parser_match(par, IDENT);
			break;
		
		default:
			Parser_abort(par, "Invalid statement.");
	}
	Parser_nl(par);
}

void Parser_comparison(Parser *par) {
	Parser_expression(par);
	if (par->curToken->type == GT   ||
		par->curToken->type == GTEQ ||
		par->curToken->type == LT   ||
		par->curToken->type == LTEQ ||
		par->curToken->type == EQEQ ||
		par->curToken->type == NOTEQ) {
		Emitter_emit(par->emit, par->curToken->text);
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
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
		Parser_expression(par);
	}
}

void Parser_expression(Parser *par) {
	Parser_term(par);
	while (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
		Parser_term(par);
	}
}

void Parser_term(Parser *par) {
	Parser_unary(par);
	while (par->curToken->type == ASTERISK || par->curToken->type == SLASH) {
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
		Parser_unary(par);
	}
}

void Parser_unary(Parser *par) {
	if (par->curToken->type == PLUS || par->curToken->type == MINUS) {
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
	}
	Parser_primary(par);
}

void Parser_primary(Parser *par) {
	if (par->curToken->type == NUMBER) {
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
	} else if (par->curToken->type == IDENT) {
		if (!(List_contains(par->symbols, par->curToken->text))) {
			Parser_abort(par, "Referenced variable before assignment.");
		}
		Emitter_emit(par->emit, par->curToken->text);
		Parser_nextToken(par);
	} else {
		Parser_abort(par, "Unexpected token in primary.");
	}
}

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

