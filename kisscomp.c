#include <kissfuck.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

/*
 * KISS brainfuck iterpreter. Compiler to the bytecode.
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

struct compstate {
	uint8_t*      bytecode;
	uint16_t      instp; // instruction pointer
	uint16_t      jmptbl[65536][2];
	uint16_t      jmptblpos;
	enum bytecode currcode;
	int           codedata;
	int           line;
};

#define NEXT_CHAR() ch = getc(f);

static void compiler_message(struct compstate* x, const char* s) {
	fprintf(stderr, "[compiler] : %s (at line %i)\n", s, x->line);
}

// sets instruction with short argument (8 bits)
static inline void pushbc1(struct compstate* x, enum bytecode b, uint8_t v) {
	x->bytecode[x->instp++] = b;
	x->bytecode[x->instp++] = v; // constant value
};

// sets instruction with long argument (16 bits, 1 byte alligment)
static inline void pushbc2(struct compstate* x, enum bytecode b, uint16_t v) {
	x->bytecode[x->instp++] = b;
	*(uint16_t*)(x->bytecode + x->instp) = v; // constant value
	x->instp += 2;
};

// links jumps instructions to each other (on instruction below each other)
static int finallize_jumppair(struct compstate* x, uint16_t f, uint16_t s) {
	// next instructions after JPZ and JPNZ instructions
	uint16_t start = f + sizeof(uint16_t) + 1;
	uint16_t end   = s + sizeof(uint16_t) + 1;
	// set jump distanations 
	*(uint16_t*)(x->bytecode + f + 1) = end;
	*(uint16_t*)(x->bytecode + s + 1) = start;
	return ERR_OK;
}

// tries to finnalize jumppair (returns 1 on success)
static int trytofinnalize(struct compstate* x) {
	uint16_t* start = &x->jmptbl[x->jmptblpos][1];
	uint16_t* end   = &x->jmptbl[x->jmptblpos][0];

	if (*start && *end) {
		// table can be finnalized!
		assert(finallize_jumppair(x, *start, *end) == ERR_OK);
		*start = 0;
		*end   = 0;
		return 1;
	}
	return 0;
}

static void addjumpstart(struct compstate* x) {
	// if jump table is busy
	if (x->jmptbl[x->jmptblpos][0]) x->jmptblpos++;
	// add start of the loop to the jumptable
	x->jmptbl[x->jmptblpos][0] = x->instp;

	// instruction will be rewrited later
	pushbc2(x, BC_JPZ, 0);

	if (trytofinnalize(x)) {
		// move forward if next jumpfield is not empity
		uint16_t npos = x->jmptblpos + 1;
		if (x->jmptbl[npos][0] || x->jmptbl[npos][1]) x->jmptblpos = npos;
	}
}

static void addjumpend(struct compstate* x) {
	// if jump table is busy
	if (x->jmptbl[x->jmptblpos][1]) x->jmptblpos--;
	// add start of the loop to the jumptable
	x->jmptbl[x->jmptblpos][1] = x->instp;

	// instruction data will be rewrited later
	pushbc2(x, BC_JPNZ, 0);

	if (trytofinnalize(x)) {
		// move backward if next jumpfield is not empity
		uint16_t npos = x->jmptblpos - 1;
		if (x->jmptbl[npos][0] || x->jmptbl[npos][1]) x->jmptblpos = npos;
	}
}

// finnaly adds instruction to bytecode
static void newcode(struct compstate* x, enum bytecode n) {
	enum bytecode b = x->currcode;
	int v = x->codedata;

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
		assert(v > 0 && "Value is belowequal zero");
		pushbc1(x, b, v);
	} else if (b == BC_HALT) {
		pushbc1(x, b, 0);
	}

	x->currcode = n;
	x->codedata = 0;
}

/*
 * Compilation functions :p
 */
int loadcode(struct kissfuck* kf, const char* filename) {
	if (kf == ERR_ERR) return ERR_ERR;
	int status = ERR_OK;
	char ch;

	struct compstate  state = {0};
	state.currcode = 128;
	struct compstate* x = &state;
	x->bytecode = kf->bytecode;

	// init
	stopcode(kf);

	FILE *f = fopen(filename, "r");
	if (!f) {
		fprintf(stderr, "[compiler] : can't open file %s!\n", filename);
		return ERR_ERR;
	}

	while ((ch = getc(f)) && !feof(f)) {

		if (CHAR_COMM(ch)) { // comment
			while (getc(f) != '\n') {};
			x->line++; 
		} else if (VALID_TOKEN(ch)) {
			int val = 0;
			switch (ch) {
			// + and -
			case TOKEN_INC :
				val = 2; 
				_fallthrough()
			case TOKEN_DEC :
				val--;
				if (x->currcode != BC_ADD && x->currcode != BC_SET) newcode(x, BC_ADD);
				x->codedata += val;
			break;
			// ,
			case TOKEN_IN :
				if (x->currcode != BC_IN)  newcode(x, BC_IN);
				x->codedata += 1;
			break;
			// .
			case TOKEN_OUT :
				if (x->currcode != BC_OUT) newcode(x, BC_OUT);
				x->codedata += 1;
			break;
			// NEXT and PREV operations are mutually exclusive
			// operations like addition and substraction
			// > and <
			case TOKEN_NEXT :
				val = 2;
				_fallthrough()
			case TOKEN_PREV :
				val--;
				if (x->currcode != BC_NEXT) newcode(x, BC_NEXT);
				x->codedata += val;
			break;
			// Loops a bit harder to make ;p
			// We need to use jumptable for this
			// [
			case TOKEN_LOOP :
				newcode(x, BC_JPZ); // close other bytecodes
				addjumpstart(x); // save code position to the jump table
				break;
			// ]
			case TOKEN_POOL :
				newcode(x, BC_JPNZ); // close other bytecodes
				addjumpend(x); // save code position to the jumptable
				// instruction body will be rewrited later...
				pushbc2(x, BC_JPNZ, 0);

			break;
			default: 
				compiler_message(x, "impossible!");
				status = ERR_ERR;
				goto end_it;
			break;
		}} else if (ch == '\n' || ch == '\r') {
				x->line++;
		};
	};
	newcode(x, 128);

	if (x->jmptblpos != 0) {
		compiler_message(x, "Code is not balanced!");
		fprintf(stderr, "[compiler] : unused branches count %i!\n", x->jmptblpos);
		status = ERR_ERR;
		goto end_it;
	}	

	end_it :
	fclose(f);
	return status;
}


