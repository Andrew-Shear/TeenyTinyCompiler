#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emit.h"

Emitter *Emitter_create(char *path) {
	Emitter *emit = malloc(sizeof(Emitter));
	emit->fullPath = strdup(path);
	emit->header = fopen(HEADER_NAME, "w");
	emit->code = fopen(CODE_NAME, "w");

	return emit;
}

void Emitter_kill(Emitter *emit) {
	fclose(emit->header);
	fclose(emit->code);
	free(emit->fullPath);
	free(emit);
}

void Emitter_emit(Emitter *emit, char *code) {
	fputs(code, emit->code);
}

void Emitter_emitLine(Emitter *emit, char *code) {
	fputs(code, emit->code);
	fputs("\n", emit->code);
}

void Emitter_header(Emitter *emit, char *code) {
	fputs(code, emit->header);
}

void Emitter_headerLine(Emitter *emit, char *code) {
	fputs(code, emit->header);
	fputs("\n", emit->header);
}

void Emitter_writeFile(Emitter *emit) {
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
