#include <kissfuck.h>

/*
 * KISS brainfuck iterpreter.
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */


#include <string.h>
#include <signal.h>

static const char* bcname[] = {
	"HALT", "SET ", "ADD ", "IN  ", "OUT ", "NEXT", "JPZ ", "JPNZ"
};

/*
 * Debugging
 */

#define ENABLE_COLORS

static uint16_t labels[65536] = {65535, 0};

static int get_label(uint16_t label) {
	int i = 0;
	uint16_t *arr = labels;
	while (arr[i] != label && arr[i] != 0) {i++;}
	if (arr[i] == label) return i;
	arr[i] = label; // add to table
	return i;
}

static int find_label(uint16_t label) {
	int i = 0;
	uint16_t *arr = labels;
	while (arr[i] != label && arr[i] != 0) {i++;}
	if (arr[i] == label) return i;
	return -1;
}

static void printcode (struct kissfuck* x, int i, int arg) {
	int l = find_label(i);
	if (l >= 0) fprintf(stderr, "(0x%05X) L%03i\t: ", i, l);
	else fprintf(stderr, "(0x%05X)   \t: ", i);
	enum bytecode bc = x->bytecode[i];
	if (bc <= 7) fprintf(stderr, bcname[bc]);
	else {
		int bc = x->bytecode[i];
		fprintf(stderr, "?%03i", bc);
	}

	if (arg == 0) {
		fprintf(stderr, "       | ");
	} else if (arg == 1) {
		int val = x->bytecode[i + 1];
		fprintf(stderr, "   %03i | ", val);
	} else if (arg == 2) {
		int val = *(uint16_t*)(x->bytecode + i + 1);
		fprintf(stderr, " %05i | ", val);
	} else if (arg == 3) { // JUMP
		get_label(i + 3); // fuck yeah
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
	fprintf(stderr, "[compiler] : bytecode dump :\n");
	fprintf(stderr, "(ADDR   )LABEL\t: CODE  ARG1 | \tHEX\n");
	uint8_t ch = 254;
	int i = 0;

	while (ch != BC_HALT && i < 65535) {
		ch = x->bytecode[i];
		if (ch == BC_JPZ || ch == BC_JPNZ) {
			printcode(x, i, 3);
			i += 3;
		} else if (ch == BC_NEXT) {
			printcode(x, i, 2);
			i += 3;
		}	else if (ch == BC_HALT) {
			printcode(x, i, 0);
			i++;
		} else if (ch == BC_SET || ch == BC_ADD || ch == BC_IN || ch == BC_OUT) {
			printcode(x, i, 1);
			i += 2;
		}	else {
			printunkn(x, i);
			i++;
		}
	}

	return ERR_OK;
}

/*
 * VM
 */

// load next short
#define LOADNS (AX = x->bytecode[x->instp++])
// load next long
#define LOADNL (EAX = *(uint16_t*)(x->bytecode + x->instp)); x->instp ++; x->instp++;

#define CELLVL (x->array[x->cellp])
#define DBGVM(c) //(printcode(x, x->instp-1, c));

void stopcode (struct kissfuck* x) {
	memset(x->array, 0, 65535);
	x->instp = 0;
	x->cellp = 0;
}

void notify (const char* m) {
	fprintf(stderr, "[vm] : %s\n", m);
}

static int has_signal = 0;

static void handler(int sig) {
	has_signal = 1;
}

int execcode (struct kissfuck* x) {
	
	signal(SIGINT, handler);

	int works = 1;
	uint16_t EAX = 0;
	uint8_t   AX = 0;
	has_signal = 0;

	while (works) {
		if (has_signal) {
			fprintf(stderr, "[vm] : INTERRUPTED!\n");
			signal(SIGINT, SIG_DFL);
			return ERR_ERR;			
		};
		uint8_t C = x->bytecode[x->instp++]; 
		switch (C) {
		case BC_HALT: DBGVM(0); fflush(stdout);works = 0; x->instp--; break;
		case BC_SET : DBGVM(1);LOADNS; CELLVL  = AX; break;
		case BC_ADD : DBGVM(1);LOADNS; CELLVL += AX; break;
		case BC_IN  : DBGVM(1);LOADNS;
			notify("INPUT REQUESTED");
			for (int i = 0; i < AX; i++)
			CELLVL = getchar();
		break;
		case BC_OUT : DBGVM(1);LOADNS;
			for (int i = 0; i < AX; i++)
			putchar(CELLVL);
		break;
		case BC_NEXT: DBGVM(2); LOADNL; x->cellp += EAX;
		break;
		case BC_JPZ : DBGVM(2); LOADNL;
			if (CELLVL == 0) {
				x->instp = EAX;
		} 
		break;
		case BC_JPNZ: DBGVM(2); LOADNL;
			if (CELLVL != 0) {
				x->instp = EAX;
			}
		break;
		default :
			printunkn(x, x->instp-1);
			fprintf(stderr, "[virtual machine] : FATAL :"
				" BAD INSTRUCTION %i at address %i\n", AX, x->instp);
			signal(SIGINT, SIG_DFL);
			return ERR_ERR;
		break;
		};
	};

	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	return ERR_OK;
}
