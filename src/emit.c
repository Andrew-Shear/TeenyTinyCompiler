#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emit.h"

Emitter *emit;

void Emitter_create(char *path) {
	Emitter *emitter = malloc(sizeof(Emitter));
	emitter->fullPath = strdup(path);
	
	// this is to make sure it exists for makefile
	FILE *outputFile = fopen(emitter->fullPath, "w");
	fclose(outputFile);

	emitter->header = fopen(HEADER_NAME, "w");
	emitter->code = fopen(CODE_NAME, "w");

	emit = emitter;
}

void Emitter_kill() {
	if (emit == NULL)
		return;
	fclose(emit->header);
	fclose(emit->code);
	free(emit->fullPath);
	free(emit);
}

void Emitter_emit(char *code) {
	fputs(code, emit->code);
}

void Emitter_emitLine(char *code) {
	fputs(code, emit->code);
	fputs("\n", emit->code);
}

void Emitter_header(char *code) {
	fputs(code, emit->header);
}

void Emitter_headerLine(char *code) {
	fputs(code, emit->header);
	fputs("\n", emit->header);
}

void Emitter_writeFile() {
	FILE *outputFile = fopen(emit->fullPath, "w");
	fclose(emit->header);
	fclose(emit->code);
	emit->header = fopen(HEADER_NAME, "r");
	emit->code = fopen(CODE_NAME, "r");
	
	int c;

	while ((c = fgetc(emit->header)) != EOF) {
		fputc(c, outputFile);
	}
	while ((c = fgetc(emit->code)) != EOF) {
		fputc(c, outputFile);
	}
	fclose(outputFile);
}
