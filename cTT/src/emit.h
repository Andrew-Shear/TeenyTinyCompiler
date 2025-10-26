#ifndef EMIT_H
#define EMIT_H

#define HEADER_NAME ".header"
#define CODE_NAME ".code"

typedef struct Emitter {
	char *fullPath;
	FILE *header;
	FILE *code;
} Emitter;

Emitter *Emitter_create(char *path);

void Emitter_kill(Emitter *emit);

void Emitter_emit(Emitter *emit, char *code);

void Emitter_emitLine(Emitter *emit, char *code);

void Emitter_header(Emitter *emit, char *code);

void Emitter_headerLine(Emitter *emit, char *code);

void Emitter_writeFile(Emitter *emit);

#endif
