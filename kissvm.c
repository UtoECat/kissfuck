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
#define reads() ((uint8_t)BC[IP])
#define readl() *(uint16_t*)(BC + (IP))

int execcode (struct kissfuck* restrict const x) {
	signal(SIGINT, handler);

	register uint16_t IP  = x->instp;
	register uint16_t SP  = x->cellp;
	register const uint8_t *BC = x->bytecode;
	has_signal   = 0;

	while (BC) {
		if (unlikely(has_signal)) {
			fprintf(stderr, "\n[vm] : INTERRUPTED!\n");
			break;
		};
		uint8_t C = reads();
		IP++;
		switch (C) {
		case BC_HALT:
			fflush(stdout); BC = NULL; IP--;
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
			uint8_t m = reads();
			for (int i = 0; i < (m - 1); i++) getchar();
			cell() = getchar();
			IP++;
		break;
		case BC_OUT :
			m = reads();
			for (int i = 0; i < m; i++) putchar(cell());
			IP++;
		break;
		case BC_NEXT:
			SP += readl();
			IP += 2;
		break;
		case BC_JPZ :
			if (!cell()) IP += readl();
			else IP += 2; // skip readed value only if we skips jump
		break;
		case BC_JPNZ:
			if (cell())  IP += readl();
			else IP += 2; // skip readed value only if we skips jump
		break;
		case BC_NUNZ: {
			uint16_t cnt = readl();
			while (cell()) SP += cnt;
			IP += 2;
		}
		break;
		default :
			// you will never get here!
			unreachable();
			//fprintf(stderr, "BAD INSTRUCTION! pos = 0x%05X\n", IP);
			//return ERR_ERR;
		break;
		};
	};
	
	// save values
	x->cellp = SP;
	x->instp = IP;
	signal(SIGINT, SIG_DFL);
	return ERR_OK;
}
