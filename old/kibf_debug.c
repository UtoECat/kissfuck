#include <stdio.h>

/*
 * KISS brainfuck iterpreter. (Debug version)
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

#define VALIDATOR_ECHO

unsigned short pos  = 0;
unsigned short ins  = 0;
#ifdef LINE_DEBUGGING
unsigned short line = 1;
#endif
size_t operations = 0;
unsigned char  mem [65535] = {0};
unsigned char  code[65535] = {0};

#define VALID_CHAR(c) (c == '+' || c == '-' || c == '[' || c == ']' || c == '<' || c == '>' || c == '.' || c == ',')

int read_and_validate(FILE* F) {
	unsigned short i = 0;
	unsigned short brc = 0;
	#ifdef VALIDATOR_ECHO
	int ln = 1;
	fprintf(stderr, "\nLINE %i\t", ln);
	#endif
	int c = getc(F);

	while (!feof(F) && i < 65530) {
		// load only code
		if (VALID_CHAR(c)) {
			code[i++] = c;
			if (c == '[') brc++;
			if (c == ']') brc--;
			#ifdef VALIDATOR_ECHO
			putc(c, stderr);
			#endif
		}

		// ignore comments
		if (c == '#' || c == '/') {
			while (getc(F) != '\n' && feof(F) == 0) {}
			#ifdef LINE_DEBUGGING
			code[i++] = 'L';
			#endif
		}

		#if defined(LINE_DEBUGGING) || defined(VALIDATOR_ECHO)
		// line counting for debugging
		if (c == '\n') {
			code[i++] = 'L';
			#ifdef VALIDATOR_ECHO
			fprintf(stderr, "\nLINE %i\t", ++ln);
			#endif
		}
		#endif

		c = getc(F);
	}

	// check errors
	if (feof(F) == 0) {
		fprintf(stderr, "CODE OWERFLOW!\n");
		return -1;
	}
	if (brc != 0) {
		fprintf(stderr, "CODE LOOPS NOT BALANCED\n!");
		return -2;
	}

	return 0;
}

#include <assert.h>

int execute () {
	// brackets passed
	unsigned short brc = 1;

	switch (code[ins]) {
			// arith
			case '+' : mem[pos]++;  break;
			case '-' : mem[pos]--;  break;
			// memory (ovweflow is allowed :p)
			case '<' : pos--; break;
			case '>' : pos++; break;
			// io
			case '.' : 
				int out = mem[pos]; 
				putchar(out);
			break;
			case ',' :
				mem[pos] = getchar();
			break;

			// loop and condition
			case '[' : 
				if (mem[pos]) break;
				while(brc) {
					switch (code[++ins]) {
						case '[' : brc++; break;
						case ']' : brc--; break;
						default  : break;
					}
				} 
			break;
			case ']' : 
				if (!mem[pos]) break;
				while(brc) {
					switch (code[--ins]) {
						case '[' : brc--; break;
						case ']' : brc++; break;
						default  : break;
					}
				} 
			break;
			// halt
			case 0 :
				return 0;
			break;

			#ifdef LINE_DEBUGGING
			// line debugging
			case 'L' :
				fprintf(stderr, "\nLINE %i\n", ++line);
			break;
			#endif

			// still ignore strange opcodes
			default : break;
	}
	operations++;

	#ifdef DEEP_DEBUGGING
	if (code[ins] == '[') {
		fprintf(stderr, "%c%c(%i)! ", code[ins], mem[pos] ? 'D' : 'J', ins);
	} else if (code[ins] == ']') {
		fprintf(stderr, "%c%c(%i)! ", code[ins], !mem[pos] ? 'D' : 'J', ins);
	} else fprintf(stderr, "%cD(%i)! ", code[ins], ins);
	#endif
	ins++;
	return 1;
}

int main(int cnt, char** argv) {

	FILE* F = fopen(cnt > 1 ? argv[1] : "alg.txt", "r");
	if (!F) {
		fprintf(stderr, "ERROPEN\n");
		return 0;
	}
	if (read_and_validate(F)) {
		fprintf(stderr, "ERRLOAD\n");
		return 0;
	};
	fclose(F);

	while (execute()) {}

	fflush(stdout);

	#ifndef NDEBUG
	fprintf(stderr, "\nTRACEBACK (%li operations summary) :", operations);
	unsigned int val = 0;

	for (size_t i = 100; i <= pos; i++) {
		val = mem[pos];
		if (i % 5 == 0) {
			fprintf(stderr, "\n [0x%04x] :\t", (unsigned int)i - 100);
		}
		fprintf(stderr, "0x%02X\t", val);
	}
	
	fprintf(stderr, "\nEND OF TRACEBACK !\n");
	#endif
	return 0;
}
