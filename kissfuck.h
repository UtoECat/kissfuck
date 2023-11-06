#include <stdio.h>

/*
 * KISS brainfuck iterpreter.
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

/*
 * GLOBAL FLAGS
 */

#define unreachable() __builtin_unreachable()
#define _fallthrough() __attribute__((fallthrough));
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#define DEBUG_FLAG 0
#define EXTENSIONS_FLAG 1

/*
 * ERRORS
 */

#define ERR_OK  1
#define ERR_ERR 0

/*
 * LEXER and COMILER section
 */

#define TOKEN_INC  '+'
#define TOKEN_DEC  '-'
#define TOKEN_IN   ','
#define TOKEN_OUT  '.'
#define TOKEN_NEXT '>'
#define TOKEN_PREV '<'
#define TOKEN_LOOP '['
#define TOKEN_POOL ']'

#if EXTENSIONS_FLAG==1
#define TOKEN_CODE '{'
#define TOKEN_EDOC '}'
#endif

#define VALID_TOKEN(A) \
	(A == TOKEN_INC ||A == TOKEN_DEC ||\
	 A == TOKEN_IN  ||A == TOKEN_OUT ||\
	 A == TOKEN_NEXT||A == TOKEN_PREV||\
	 A == TOKEN_LOOP||A == TOKEN_POOL)

#define CHAR_COMM(A) (A == '/' || A == '#')
#define VALID_CHAR(A) (VALID_TOKEN(A) || CHAR_COMM(A))

/*
 * BYTECODE
 */

enum bytecode {
	// bc   = id // (args in bytes count) desc
	// LOW LEVEL CONSTANT OPERATIONS
	BC_HALT = 0, // (0) stops execution
	BC_SET  = 1, // (1) set current cell value == '[-]'
	BC_ADD  = 2, // (1) adds constant number
	BC_IN   = 3, // (1) input 
	BC_OUT  = 4, // (1) output
	BC_NEXT = 5, // (2) goes to next N cell
	BC_JPZ  = 6, // (2) jump if zero forward
	BC_JPNZ = 7, // (2) jump if not zero backward
	
	// HIGH LEVEL OPERATIONS
	BC_COPY = 8, // (1) copies value from curr cell to next N cell
	BC_NUNZ = 9, // (2) jump next N cells until not zero == '[>]'
};

#define BC_OPTIMIZABLE(A) (A < BC_JPZ)

/*
 * Basic structures and functions
 */

#include <stdint.h>

struct kissfuck {
	// common section
	uint16_t cellp, instp;
	uint8_t  bytecode[65536];
	uint8_t  array   [65536];
	// debug section
	int line;
};

struct kissfuck* makectx();
int  loadcode(struct kissfuck* restrict const, const char* filename);
int  execcode(struct kissfuck*);
void stopcode(struct kissfuck*);
int  dumpcode(struct kissfuck*);
void freectx (struct kissfuck*);

