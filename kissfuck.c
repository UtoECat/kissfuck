#include <kissfuck.h>

/*
 * KISS brainfuck iterpreter.
 * Copyright (C) UtoECat 2023. All rights reserved.
 * GNU GPL License. No any warranty. 
 *
 * Some testes from the internet was passed :p
 */

/*
 * text :p
 */

#define STR_VERSION "kissfuck v.1.0\n"

#define STR_ABOUT \
	"kissfuck - KISS brainfuck interpreter.\n"\
	"Coptyright (C) UtoECat 2023. All rights reserved!\n"\
	"GNU GPL License. NO ANY WARRIANTY!\n"\
	"Github : https://"

#define STR_USAGE "\tUSAGE :\n"\
	" $ %s [-h to help]\n $ %s [-v to version]\n"\
	" $ %s [filename]\n $ %s [-d to debug] [filename]\n"

#define STR_HELP \
	"\tLANGUAGE :\n"\
	"Brainfuck operations are '+', '-', '[', ']', '<', '>', '.', ','\n"\
	"+\tIncrement value in current cell\n"\
	"-\tDecrement value in current cell\n"\
	",\tRead character from stdin to current cell\n"\
	".\tWrite character to stdout from currnt cell\n"\
	"[\tWhile current cell is not 0 loop\n"\
	"]\tRepeats loop if current cell is not 0\n"\
	"<\tChanges cell to previous\n"\
	">\tChanges cell to next\n"

static const char* default_filename = "./test.bfk";
static const char* argv0 = "NULL";

static inline void print_usage(int err) {
	if (err == 2) fprintf(stderr, "Bad Usage! ");
	if (err == 3) fprintf(stderr, "Bad Flag! ");
	if (err == 4) fprintf(stderr, " -- flags are not allowed! ");
	fprintf(stderr, STR_USAGE, argv0, argv0, argv0, argv0);
}

static inline void print_help() {
	fprintf(stderr, "%s%s", STR_ABOUT, STR_VERSION);
	print_usage(0);
	fprintf(stderr, "%s", STR_HELP);
}

int debug = 0;

static int parse_argv(int argc, char** argv) {
	argv0 = argv[0];

	// nothing to parse!
	if (argc < 2) return ERR_OK; 

	if (argc > 3) {
		print_usage(2);
		return ERR_ERR;
	}

	int i = 1;
	repeatit:
	if (argv[i] == NULL) return ERR_OK;

	if (argv[i][0] != '-') { // set file
		default_filename = argv[i];	
		return ERR_OK;
	} else switch (argv[i][1]) { // parse flag
		case '-' : print_usage(4);               return ERR_ERR;
		case 'h' : print_help();                 return ERR_ERR;
		case 'd' : 
			debug = 1; fprintf(stderr, "[vm] : debug mode enabled!\n");
			i++;
			goto repeatit;
		break;
		case 'v' : fprintf(stderr, STR_VERSION); return ERR_ERR;
		default  : print_usage(3);               return ERR_ERR;
	}
	return ERR_OK;

}

int main(int argc, char** argv) {
	if (parse_argv(argc, argv) != ERR_OK) return -1;
	struct kissfuck* K = makectx();
	if (K == ERR_ERR) return -2;
	if (loadcode(K, default_filename) != ERR_OK) {
		fprintf(stderr, "Show bytecode Dump? (y/n) : ");
		if (debug || getchar() == 'y') dumpcode(K);
		return -3;
	}
	stopcode(K);
	if (execcode(K) != ERR_OK) {
		fprintf(stderr, "Show bytecode Dump? (y/n) : ");
		if (debug || getchar() == 'y') dumpcode(K);
	}
	if (debug) dumpcode(K);
	return 0;
}
