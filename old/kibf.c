#include <stdio.h>

/*
 * KISS brainfuck iterpreter. (Release version)
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

unsigned short pos  = 0; // position in memory
unsigned short ins  = 0; // position in code
size_t operations = 0;
unsigned char  mem [65535] = {0};
unsigned char  code[65535] = {0};

#define VALID_CHAR(c) (c == '+' || c == '-' || c == '[' || c == ']' || c == '<' || c == '>' || c == '.' || c == ',')

int read_and_validate(FILE* F) {
	unsigned short i = 0;
	unsigned short brc = 0;
	int c = getc(F);

	while (!feof(F) && i < 65530) {
		// load to code only code... for perfomance :p
		if (VALID_CHAR(c)) {
			code[i++] = c;
			if (c == '[') brc++;
			if (c == ']') brc--;
		}

		// ignore comments
		if (c == '#' || c == '/') while (getc(F) != '\n' && feof(F) == 0) {}

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

			// still ignore strange opcodes
			default : break;
	}
	operations++;
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

	fprintf(stderr, "\n%li Operations in summary.\n", operations);
	return 0;
}
