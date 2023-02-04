#include <kissfuck.h>
#include <stdlib.h>

/*
 * KISS brainfuck iterpreter.
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

/*
 * Context allocation
 */

struct kissfuck* makectx() {
	void *p = calloc(sizeof(struct kissfuck), 1);
	if (!p) {
		fprintf(stderr, "Memory allocation error!\n");
		return ERR_ERR;
	}
	return p;
}

void freectx (struct kissfuck* x) {
	if (x) free(x);
}

/*
 * COMPILING
 */

static void compiler_message(struct kissfuck* x, const char* s) {
	fprintf(stderr, "[compiler] : %s (at line %i)\n", s, x->line);
}

#define INDEX_LONG_BC(x, pos) *((uint16_t*)(x->bytecode + (pos)))

static int finallize_jumppair(struct kissfuck* x, uint16_t first, uint16_t second) {
	// some assertions first
	if (x->bytecode[first] != BC_JPZ) {
		return ERR_ERR;
	}
	if (x->bytecode[second] != BC_JPNZ) {
		return ERR_ERR;
	}
	uint16_t start = first  + sizeof(uint16_t) + 1;
	uint16_t end   = second + sizeof(uint16_t) + 1;
	// set jump destinations :D
	INDEX_LONG_BC(x, first  + 1) = end;
	INDEX_LONG_BC(x, second + 1) = start;
	return ERR_OK;
}

static int checkchar(FILE* f, char ch) {
	return (!feof(f)) && ch;
}

static inline void pushbc1(struct kissfuck* x, enum bytecode b, uint8_t v) {
	x->bytecode[x->instp++] = b;
	x->bytecode[x->instp++] = v; // constant value
};

static inline void pushbc2(struct kissfuck* x, enum bytecode b, uint16_t v) {
	x->bytecode[x->instp++] = b;
	*(uint16_t*)(x->bytecode + x->instp) = v; // constant value
	x->instp += 2;
};

static inline void closecode(struct kissfuck* x, enum bytecode b, int v) {
	if (b == BC_ADD || b == BC_SET) {
		if (v == 0) {return;}
		else if (v > 0) pushbc1(x, b, v);
		else {
			v = -v;
			pushbc1(x, b, (uint8_t)(UINT8_MAX - v + 1));
		}
	} else if (b == BC_NEXT) {
		if (v == 0) {return;}
		else if (v > 0) pushbc2(x, b, v);
		else {
			v = -v;
			pushbc2(x, b, (uint16_t)(UINT16_MAX - v + 1));
		}
	} else if (b == BC_IN || b == BC_OUT) {
		if (v <= 0) {
			fprintf(stderr, "Value is below zero or equal; for normal operands :/\n");
			exit(-1);
		}
		pushbc1(x, b, v);
	} else if (b == BC_HALT) {
		pushbc1(x, b, 0);
	}
}

int loadcode(struct kissfuck* x, const char* filename) {
	if (x == ERR_ERR) return ERR_ERR;

	// init
	x->line = 0;
	x->instp = 0;
	x->cellp = 0;

	FILE *f = fopen(filename, "r");
	if (!f) {
		compiler_message(x, "Can't open file!");
		return ERR_ERR;
	}

	int status   = ERR_OK;
	enum bytecode oldcode = 90;
	int  codedata         = 0;
	uint16_t jmptbl[65536][2] = {0}; // holy shit
	uint16_t jmptblpos        = 0;
	
	char ch = getc(f);
	#define NEXT_CHAR() ch = getc(f);
	while (checkchar(f, ch)) {

		if (CHAR_COMM(ch)) { // comment
			while (getc(f) != '\n') {};
			x->line++; NEXT_CHAR();
		} else if (VALID_TOKEN(ch)) {
			int val= 0;
			switch (ch) {
			// + and -
			case TOKEN_INC :
				val = 2;
				// falltrough
			case TOKEN_DEC :
				val--;
				if (oldcode != BC_ADD && oldcode != BC_SET) {
					closecode(x, oldcode, codedata);
					oldcode = BC_ADD;
					codedata = 0;
				}
				codedata += val;
			break;
			// ,
			case TOKEN_IN :
				if (oldcode != BC_IN) {
					closecode(x, oldcode, codedata);
					oldcode = BC_IN;
					codedata = 0;
				}
				codedata += 1;
			break;
			// .
			case TOKEN_OUT :
				if (oldcode != BC_OUT) {
					closecode(x, oldcode, codedata);
					oldcode = BC_OUT;
					codedata = 0;
				}
				codedata += 1;
			break;
			/*
			 * TODO: GOOCLE TRANSLATE HELP ME :(
			 * NEXT and PREV operations are взаимоисключающие like addition and substraction
			 */
			// > and <
			case TOKEN_NEXT :
				val = 2;
			case TOKEN_PREV :
				val--;
				if (oldcode != BC_NEXT) {
					closecode(x, oldcode, codedata);
					oldcode = BC_NEXT;
					codedata = 0;
				}
				codedata += val;
			break;
			/*
			 * Loops a bit harder to make ;p
			 * We need to use jumptable for this
			 */
			// [
			case TOKEN_LOOP :
				// close other bytecodes
				closecode(x, oldcode, codedata);
				oldcode = BC_JPZ;
				codedata = 0;
				// jump calculations
				if (jmptbl[jmptblpos][0]) jmptblpos++;
				jmptbl[jmptblpos][0] = x->instp;  // add start of the loop to the jumptable
				// instruction body will be rewrited later...
				pushbc2(x, BC_JPZ, 0);

				if (jmptbl[jmptblpos][1]) {	// finalized pair
					if (finallize_jumppair(x, jmptbl[jmptblpos][0], jmptbl[jmptblpos][1]) != ERR_OK) {
						compiler_message(x, "Can't finnalize jump pair! Internal error!");
						status = ERR_ERR; goto end_it;
					}
					jmptbl[jmptblpos][0] = 0; jmptbl[jmptblpos][1] = 0; // cleanup :p
					uint16_t npos = jmptblpos + 1;
					if (jmptbl[npos][0] || jmptbl[npos][1]) jmptblpos = npos; // important
				}
				break;
			// ]
			case TOKEN_POOL :
				// close other bytecodes
				closecode(x, oldcode, codedata);
				oldcode = BC_JPNZ;
				codedata = 0;
				// jump calculations
				if (jmptbl[jmptblpos][1]) jmptblpos--;
				jmptbl[jmptblpos][1] = x->instp;  // add end of the loop to the jumptable
				// instruction body will be rewrited later...
				pushbc2(x, BC_JPNZ, 0);

				if (jmptbl[jmptblpos][0]) {	// finalized pair
					if (finallize_jumppair(x, jmptbl[jmptblpos][0], jmptbl[jmptblpos][1]) != ERR_OK) {
						compiler_message(x, "Can't finnalize jump pair! Internal error!");
						status = ERR_ERR; goto end_it;
					}
					jmptbl[jmptblpos][0] = 0; jmptbl[jmptblpos][1] = 0; // cleanup :p
					uint16_t npos = jmptblpos - 1;
					if (jmptbl[npos][0] || jmptbl[npos][1]) jmptblpos = npos; // important
				}
			break;
			default: 
				compiler_message(x, "impossible!");
				status = ERR_ERR;
				goto end_it;
			break;
		};NEXT_CHAR();} else if (ch == '\n' || ch == '\r') {
				NEXT_CHAR();
				x->line++;
		} else NEXT_CHAR();
	};
	closecode(x, oldcode, codedata);

	if (jmptblpos != 0) {
		compiler_message(x, "Code is not balanced!");
		fprintf(stderr, "[compiler] : unused branches count %i!\n", jmptblpos);
		status = ERR_ERR;
		goto end_it;
	}	

	end_it :
	fclose(f);
	return status;
}


