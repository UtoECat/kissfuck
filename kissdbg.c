#include <kissfuck.h>

/*
 * KISS brainfuck iterpreter. (Debug bytecode)
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */


#include <string.h>
#include <signal.h>
#include <stddef.h>

static const char* bcname[] = {
	"HALT", "SET ", "ADD ", "IN  ", "OUT ", "NEXT", "JPZ ", "JPNZ"
};

/*
 * Debugging
 */

static uint16_t labels[65536] = {0};

static int get_label(uint16_t label) {
	int i = 0;
	uint16_t *arr = labels;
	while (arr[i] != 0 && arr[i] != label) {i++;}
	if (arr[i] == 0) arr[i] = label; // add to table
	return i;
}

static int find_label(uint16_t label) {
	int i = 0;
	uint16_t *arr = labels;
	while (arr[i] != 0 && arr[i] != label) {i++;}
	if (arr[i] == 0) return -1;
	return i;
}

void printbytecode (struct kissfuck* x, int i, int arg) {
	int l = find_label(i);
	if (l >= 0) fprintf(stderr, "(0x%05X) L%03i\t: ", i, l);
	else fprintf(stderr, "(0x%05X)   \t: ", i);
	enum bytecode bc = x->bytecode[i];
	if (bc <= 7) fprintf(stderr, bcname[bc]);
	else {
		int v = bc;
		fprintf(stderr, "?%03i", v);
	}

	if (arg == 0) { // HALT
		fprintf(stderr, "       | ");
	} else if (arg == 1) {
		int val = x->bytecode[i + 1];
		fprintf(stderr, "   %03i | ", val);
	} else if (arg == 2) { // NEXT
		int val = *(uint16_t*)(x->bytecode + i + 1);
		fprintf(stderr, " %05i | ", val);
	} else if (arg == 3) { // JPZ and JPNZ
		get_label(i + 3); // save next instruction as  
		int val = *(uint16_t*)(x->bytecode + i + 1);
		val = get_label(val);
		if (val < 0) fprintf(stderr, "  ???  | ");
		else fprintf(stderr, " L%03i  | ", val);
		arg = 2;
	}

	for (int j = 0; j < arg + 1; j++) {
		int val = x->bytecode[i + j];
		fprintf(stderr, "%02X ", val);
	}
	fprintf(stderr, "\n");
}

static void printunkn(struct kissfuck* x, int i) {
	int l = find_label(i);
	if (l >= 0) fprintf(stderr, "L%03i\t: ", l);
	else fprintf(stderr, "   \t: ");
	int bc = x->bytecode[i];
	fprintf(stderr, "?%03i", bc);
	fprintf(stderr, "       | ");
	int val = x->bytecode[i];
	fprintf(stderr, "%02X ", val);
	fprintf(stderr, "\n");
}

int dumpcode(struct kissfuck* x) {
	fprintf(stderr, "[debugger] : bytecode dump :\n");
	fprintf(stderr, "(ADDR   )LABEL\t: CODE  ARG1 | \tHEX\n");
	uint8_t ch = 254;
	int i = 0;

	while (ch != BC_HALT && i < 65535) {
		ch = x->bytecode[i];
		if (ch == BC_JPZ || ch == BC_JPNZ) {
			printbytecode(x, i, 3);
			i += 3;
		} else if (ch == BC_NEXT) {
			printbytecode(x, i, 2);
			i += 3;
		}	else if (ch == BC_HALT) {
			printbytecode(x, i, 0);
			i++;
		} else if (ch == BC_SET || ch == BC_ADD || ch == BC_IN || ch == BC_OUT) {
			printbytecode(x, i, 1);
			i += 2;
		}	else {
			printunkn(x, i);
			i++;
		}
	}

	return ERR_OK;
}
