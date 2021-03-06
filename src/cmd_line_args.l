%{
/*----------------------------------------------------------------------------
File    : $Id: cmd_line_args.l,v 1.21 2003-12-14 14:47:34 psy Exp $
What    : Lexer for parsing command line arguments to dl

Copyright (C) 1999, 2000 Michael Maher <mjm@math.luc.edu>
Copyright (C) 1999, 2000, 2003 Tristan Miller <Tristan.Miller@dfki.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.

----------------------------------------------------------------------------*/

static size_t getTableSize(char *s);
static size_t closestPrime(size_t n);

#if HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "dl_malloc.h"
#include "dl_stdint.h"
#include "dl_stdbool.h"
#include "dl.h"
#include "cmd_line_args.h"

#ifndef PRIME_FILE
#  ifndef DL_DATADIR
#    define DL_DATADIR "/usr/local/share"
#  endif
#  define PRIME_FILE DL_DATADIR "/delores/primes.txt"
#endif

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
  "--help",    "display this information",
  "--version", "display version information and quit",
  "-q",        "quiet mode (suppress nonessential messages)",
  "-a <n>",    "make atom table size the smallest prime >= <n>",
  "-r <n>",    "make rule table size the smallest prime >= <n>",
  "-A <n>",    "make atom table size exactly <n>",
  "-R <n>",    "make rule table size exactly <n>",
#ifdef DL_USE_BGET
  "-m <n>",    "pre-allocate memory in <n>-byte chunks",
#endif
  NULL
};

%}

space [ \t\n]
uint [0-9]+         
%option prefix="xx"
%option outfile="lex.yy.c"
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
  fprintf(stderr,"Usage: %s [options] [file]\n", PACKAGE);
  fprintf(stderr,"Options:\n");
  for (opts = Options; *opts; opts += 2)
    fprintf(stderr,"  %-9s  %s\n", *opts, *(opts + 1));
  exit(EXIT_SUCCESS);
}

<option>(-version) {
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
    if (strtoumax(xxtext, NULL, 0) > BUFSIZE_MAX) {
      fprintf(stderr, "%s: cannot set bget pool increment > %" PRIuMAX
              "\n", PACKAGE, (uintmax_t)BUFSIZE_MAX);
      exit(EXIT_FAILURE);
    }
    returnData.bgetPoolIncrement = ATOBUFSIZE(xxtext);
  }
#endif
  BEGIN INITIAL;
}

<INITIAL>[^-]+.* {
  if (returnData.inputFile[0] == '\0')
    strncpy(returnData.inputFile, xxtext, FILENAME_MAX);
  else {
    fprintf(stderr, "%s: error at `%s'; can't open multiple files (yet)\n",
            PACKAGE, xxtext);
    exit(EXIT_FAILURE);
  }
}

<_r,_R,_a,_A,_m>.+ {
  fprintf(stderr, "%s: error at `%s' -- expecting unsigned integer\n",
          PACKAGE, xxtext);
  exit(EXIT_FAILURE);
}

<option>[^aArRmv]+ {
  fprintf(stderr, "%s: unrecognized option `%s'\n", PACKAGE, xxtext);
  fprintf(stderr, "Invoke as `%s --help' for command-line options.\n\n",
          PACKAGE);
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
  uintmax_t tableSize = strtoumax(s, NULL, 0);
  if (tableSize == 0 || tableSize > TABLE_MAX) {
    fprintf(stderr,"%s: Table size must be in [1..%" PRIuMAX "]\n",
            PACKAGE, (uintmax_t)TABLE_MAX);
    exit(EXIT_FAILURE);
  }
  return (size_t)tableSize;
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
            PACKAGE);
    exit(EXIT_FAILURE);
  }
  
  while (fscanf(f, "%" SCNuMAX, &prime) != EOF && prime < n);
  fclose(f);

#ifdef DL_DEBUG
  if (prime > SIZE_MAX) {
    fprintf(stderr, "%s: " PRIME_FILE " contains a prime larger than %"
            PRIuMAX "\n", PACKAGE, (uintmax_t)SIZE_MAX);
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
  puts(
    PACKAGE_STRING "\n"
#ifdef DL_USE_BGET
    "(includes BGET memory management by John Walker)\n"
#endif
    "Copyright (C) 1999, 2000 Michael Maher\n"
    "Copyright (C) 1999, 2000, 2003 Tristan Miller\n"
    "This is free software; see the source for copying conditions.  There is NO\n"
    "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
    );

}

#undef TABLE_MAX
#undef DEFAULT_ATOMTABLESIZE
#undef DEFAULT_RULETABLESIZE
#undef PRIME_FILE
#ifdef DL_USE_BGET
#  undef DEFAULT_BGETPOOLINCREMENT
#endif
