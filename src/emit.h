#ifndef EMIT_H
#define EMIT_H

#define HEADER_NAME ".header"
#define CODE_NAME ".code"

typedef struct Emitter {
	char *fullPath;
	FILE *header;
	FILE *code;
} Emitter;

void Emitter_create(char *path);

void Emitter_kill();

void Emitter_emit(char *code);

void Emitter_emitLine(char *code);

void Emitter_header(char *code);

void Emitter_headerLine(char *code);

void Emitter_writeFile();

#endif
