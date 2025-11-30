CFLAGS = -g -Wall
CC = gcc

TEENYS = $(wildcard ./*.teeny, examples/*.teeny)

.PHONY: all $(TEENYS)

all: $(TEENYS)

compile:
		$(CC) src/teenytiny.c src/parse.c src/lex.c src/emit.c src/list.c src/ast.c -o src/teenytiny

$(TEENYS):
		@src/teenytiny $@
		@$(CC) out.c -o $(notdir $(patsubst %.teeny,%,$@))
		@rm .header
		@rm .code
		@rm out.c

