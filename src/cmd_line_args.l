%{
/*----------------------------------------------------------------------------
Filename: cmd_line_args.l
What    : Lexer for parsing command line arguments to dl
Author  : Tristan Miller
----------------------------------------------------------------------------*/

static size_t getTableSize(char *s);
static size_t closestPrime(size_t n);

#include "dl_compile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "dl_malloc.h"
#include "dl_stdint.h"
#include "dl_stdbool.h"
#include "dl.h"
#include "cmd_line_args.h"

#define PRIME_FILE "primes.txt"
#define DEFAULT_ATOMTABLESIZE 1000003UL
#define DEFAULT_RULETABLESIZE 1000003UL
#ifdef DL_USE_BGET
#  define DEFAULT_BGETPOOLINCREMENT 65536L
#endif

static cmdargs_t returnData = {
  DEFAULT_ATOMTABLESIZE,
  DEFAULT_RULETABLESIZE,
#ifdef DL_USE_BGET
  DEFAULT_BGETPOOLINCREMENT,
#endif
  "",
  false
};

static int Arg_number = 1;
static int ARGC;
static char **ARGV;
static const char *Options[] = {
  "--help",   "display this information",
  "-v",       "display version information and quit",
  "-q",       "quiet mode (suppress nonessential messages)",
  "-a <n>",   "make atom table size the smallest prime >= <n>",
  "-r <n>",   "make rule table size the smallest prime >= <n>",
  "-A <n>",   "make atom table size exactly <n>",
  "-R <n>",   "make rule table size exactly <n>",
#ifdef DL_USE_BGET
  "-m <n>",   "pre-allocate memory in <n>-byte chunks",
#endif
  NULL
};

%}

space [ \t\n]
uint [0-9]+         
%x _a _A _r _R _m option
%option noyywrap
%option never-interactive
%option nounput

%%

<INITIAL>^- BEGIN option;

<option>(h)     |
<option>(\?)    |
<option>(help)  |
<option>(-help) {
  char const **opts;
  fprintf(stderr,"Usage: %s [options] [file]\n", DL_PROGNAME);
  fprintf(stderr,"Options:\n");
  for (opts = Options; *opts; opts += 2)
	fprintf(stderr,"  %-6s  %s\n", *opts, *(opts + 1));
  exit(EXIT_SUCCESS);
}

<option>v {
  printVersionInformation();
  exit(EXIT_SUCCESS);
}

<option>q {
  returnData.quietMode = true;
  BEGIN INITIAL;
}

<option>a BEGIN _a;
<option>A BEGIN _A;
<option>r BEGIN _r;
<option>R BEGIN _R;
<option>m BEGIN _m;

<_a>{uint} {
  if (returnData.atomTableSize == DEFAULT_ATOMTABLESIZE)
	returnData.atomTableSize = closestPrime(getTableSize(xxtext));
  BEGIN INITIAL;
}

<_A>{uint} {
  if (returnData.atomTableSize == DEFAULT_ATOMTABLESIZE)
	returnData.atomTableSize = getTableSize(xxtext);
  BEGIN INITIAL;
}

<_r>{uint} {
  if (returnData.ruleTableSize == DEFAULT_RULETABLESIZE)
	returnData.ruleTableSize = closestPrime(getTableSize(xxtext));
  BEGIN INITIAL;
}

<_R>{uint} {
  if (returnData.ruleTableSize == DEFAULT_RULETABLESIZE)
	returnData.atomTableSize = getTableSize(xxtext);
  BEGIN INITIAL;
}

<_m>{uint} {
#ifdef DL_USE_BGET
  if (returnData.bgetPoolIncrement == DEFAULT_BGETPOOLINCREMENT) {
	if (strtoul(xxtext, NULL, 0) > BUFSIZE_MAX) {
	  __extension__
	  fprintf(stderr, "%s: cannot set bget pool increment > %" UINTMAX_FMT
			  "\n", DL_PROGNAME, (uintmax_t)BUFSIZE_MAX);
	  exit(EXIT_FAILURE);
	}
	returnData.bgetPoolIncrement = ATOBUFSIZE(xxtext);
  }
#endif
  BEGIN INITIAL;
}

<INITIAL>[^-]+.* {
  if (returnData.inputFile[0] == '\0')
	strncpy(returnData.inputFile, xxtext, PATH_MAX);
  else {
	fprintf(stderr, "%s: error at `%s'; can't open multiple files (yet)\n",
			DL_PROGNAME, xxtext);
	exit(EXIT_FAILURE);
  }
}

<_r,_R,_a,_A,_m>.+ {
  fprintf(stderr, "%s: error at `%s' -- expecting unsigned integer\n",
		  DL_PROGNAME, xxtext);
  exit(EXIT_FAILURE);
}

<option>[^aArRmv]+ {
  fprintf(stderr, "%s: unrecognized option `%s'\n", DL_PROGNAME, xxtext);
  fprintf(stderr, "Invoke as `%s --help' for command-line options.\n\n",
		  DL_PROGNAME);
  exit(EXIT_FAILURE);
}

<<EOF>> {
  if (++Arg_number >= ARGC) {
	yyterminate();
  }
  else {
	xx_delete_buffer(YY_CURRENT_BUFFER);
	xx_scan_string(ARGV[Arg_number]);
  }
}

%%

#ifdef DL_USE_BGET
#define TABLE_MAX BUFSIZE_MAX
#else
#define TABLE_MAX SIZE_MAX
#endif


/*----------------------------------------------------------------------------
Function: getCmdLineArgs
Purpose : parses command line arguments and returns relevant information
Args    : argc -- number of command line arguments
          argv -- list of command line arguments
Returns : structure specifying data obtained from the command line arguments
----------------------------------------------------------------------------*/
cmdargs_t getCmdLineArgs(int argc, char *argv[]) {
  if (argc > 1) {
	ARGC=argc;
	ARGV=argv;
	xx_scan_string(ARGV[1]);
	xxlex();
  }
  return returnData;
}


/*----------------------------------------------------------------------------
Function: closestPrime
Purpose : finds the smallest prime in prime file >= given argument
Args    : n -- number at which to start prime search
Returns : smallest prime in prime file >= given argument, or largest prime
          in prime file if given argument > largest prime in prime file
----------------------------------------------------------------------------*/
size_t getTableSize(char *s) {
  size_t tableSize = ATOSIZE_T(s);
  if (tableSize == 0 || tableSize > TABLE_MAX) {
	__extension__
	fprintf(stderr,"%s: Table size must be in [1..%" UINTMAX_FMT "]\n",
			DL_PROGNAME, (uintmax_t)TABLE_MAX);
	exit(EXIT_FAILURE);
  }
  return tableSize;
}


/*----------------------------------------------------------------------------
Function: closestPrime
Purpose : finds the smallest prime in prime file >= given argument
Args    : n -- number at which to start prime search
Returns : smallest prime in prime file >= given argument, or largest prime
          in prime file if given argument > largest prime in prime file
----------------------------------------------------------------------------*/
size_t closestPrime(size_t n) {
  FILE *f;
  uintmax_t prime;

  if (!(f = fopen(PRIME_FILE, "r"))) {
	fprintf(stderr, "%s: could not open prime file `" PRIME_FILE "'\n",
			DL_PROGNAME);
	exit(EXIT_FAILURE);
  }
  
  while (__extension__ fscanf(f, "%" UINTMAX_FMT, &prime) != EOF && prime < n);
  fclose(f);

#ifdef DL_DEBUG
  if (prime > SIZE_MAX) {
	__extension__
	fprintf(stderr, "%s: " PRIME_FILE " contains a prime larger than %"
			UINTMAX_FMT "\n", DL_PROGNAME, (uintmax_t)SIZE_MAX);
	exit(EXIT_FAILURE);
  }
#endif

  return (size_t)prime;
}


/*----------------------------------------------------------------------------
Function: printVersionInformation
Purpose : prints version information
Args    : none
Returns : nothing
----------------------------------------------------------------------------*/
void printVersionInformation(void) {
  printf("%s version %s -- Copyright 2000 Michael Maher\n", DL_TITLE, DL_VER);
}

#undef TABLE_MAX
#undef DEFAULT_ATOMTABLESIZE
#undef DEFAULT_RULETABLESIZE
#undef PRIME_FILE
#ifdef DL_USE_BGET
#  undef DEFAULT_BGETPOOLINCREMENT
#endif
