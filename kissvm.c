#include <kissfuck.h>

/*
 * KISS brainfuck iterpreter. (Virtual machine)
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

#include <string.h>
#include <signal.h>
#include <stddef.h>

// load next short

void stopcode (struct kissfuck* x) {
	memset(x->array, 0, 65535);
	x->instp = 0; x->cellp = 0;
}

/*
 * VM
 */

static int has_signal = 0;

static void handler(int) {
	has_signal = 1;
}

#define cell()  (x->array[SP])
#define reads() ((uint8_t)x->bytecode[IP])
#define readl() *(uint16_t*)(x->bytecode + (IP))

int execcode (struct kissfuck* const x) {
	signal(SIGINT, handler);

	int     works = 1;
	register uint16_t IP  = x->instp;
	register uint16_t SP  = x->cellp;
	has_signal   = 0;

	while (works) {
		if (unlikely(has_signal)) {
			fprintf(stderr, "\n[vm] : INTERRUPTED!\n");
			break;
		};
		uint8_t C = reads();
		IP++;
		switch (C) {
		case BC_HALT:
			fflush(stdout); works = 0; IP--;
		break;
		case BC_SET :
			cell() = reads();
			IP++;
		break;
		case BC_ADD :
			cell() += reads();
			IP++;
		break;
		case BC_IN  :
			// ignore other characters
			for (int i = 0; i < (reads() - 1); i++) getchar();
			cell() = getchar();
			IP++;
		break;
		case BC_OUT :
			for (int i = 0; i < reads(); i++) putchar(cell());
			IP++;
		break;
		case BC_NEXT:
			SP += readl();
			IP += 2;
		break;
		case BC_JPZ :
			if (!cell()) IP = readl();
			else IP += 2; // skip readed value only if we skips jump
		break;
		case BC_JPNZ:
			if (cell())  IP = readl();
			else IP += 2; // skip readed value only if we skips jump
		break;
		case BC_NUNZ:
			while (cell()) SP += readl();
			IP += 2;
		break;
		default :
			// you will never get here!
			unreachable();
		break;
		};
	};
	
	// save values
	x->cellp = SP;
	x->instp = IP;
	signal(SIGINT, SIG_DFL);
	return ERR_OK;
}
