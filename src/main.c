/*----------------------------------------------------------------------------
File    : $Id: main.c,v 1.9 2003-12-13 15:13:58 psy Exp $
What    : Initialization routine for interpreter

Copyright (C) 1999, 2000 Michael Maher <mjm@math.luc.edu>
Copyright (C) 1999, 2000, 2003 Tristan Miller <psychonaut@nothingisreal.com>

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif
#include <string.h>
#include <stdio.h>
#include "timer.h"
#include "dl_malloc.h"
#include "dl.h"
#include "dl_stdint.h"
#include "cmd_line_args.h"


/*----------------------------------------------------------------------------
Global variable declarations
----------------------------------------------------------------------------*/
hashTable atomTable, ruleTable;
_Bool InteractiveMode;


/*----------------------------------------------------------------------------
Local function prototypes
----------------------------------------------------------------------------*/
static int hashStrcmp(const void *k1, const void *k2);
static size_t hashKey(const void *key, size_t n);
static void hashFreeAtom(void *a);
static void hashFreeRule(void *r);


/*----------------------------------------------------------------------------
Function: main
Purpose : run parser
Args    : none
Returns : 0 on success, nonzero on error
----------------------------------------------------------------------------*/
int main(int argc, char *argv[]) {

  /* Get command line parameters */
  cmdargs_t args = getCmdLineArgs(argc, argv);

  /* Switch stdin to file specified on command line, if given */
  if (args.inputFile[0]) {
    if (!freopen(args.inputFile, "r", stdin)) {
      fprintf(stderr, "%s: %s: no such file or directory\n", PACKAGE,
              args.inputFile);
      return EXIT_FAILURE;
    }
    InteractiveMode = false;
  }
  else
    /*
    ** The following should probably be changed to something like
    **    Interactivemode = POSIX ? isatty(fileno(stdin)) : true;
    ** once we figure out how to test for POSIX compliance with Autoconf
    */
    InteractiveMode = true;

#ifdef DL_USE_BGET
  /* If using BGET, we need to initialize the memory pool */
  bufferPoolInitialize(args.bgetPoolIncrement);
#endif
  
  /* Initialize atom table with [neg] true, [neg] false */
  if (!hashConstructTable(&atomTable, args.atomTableSize, hashKey,
                           hashStrcmp)) {
    fprintf(stderr,"%s: atom table too big for available memory!\n",
            PACKAGE);
    return EXIT_FAILURE;
  }
  else {
    Atom *a = initAtom("true");
    a->plus_DELTA = true;
    a->plus_delta = true;
    a->minus_DELTA_neg = true;
    a->minus_delta_neg = true;
    a = initAtom("false");
    a->minus_DELTA = true;
    a->minus_delta = true;
    a->plus_DELTA_neg = true;
    a->plus_delta_neg = true;
  }

  /* Initialize rule table */
  if (!hashConstructTable(&ruleTable, args.ruleTableSize, hashKey,
                          hashStrcmp)) {
    fprintf(stderr,"%s: rule table too big for available memory!\n",
            PACKAGE);
    return EXIT_FAILURE;
  }
  
  /* Title screen */
  if (!args.quietMode) {
    printVersionInformation();
    printf("Invoke as `%s --help' for command-line options.\n\n", PACKAGE);
      printf("Atom table size: %" PRIuMAX "\n",
             (uintmax_t)args.atomTableSize);
      printf("Rule table size: %" PRIuMAX "\n", 
             (uintmax_t)args.ruleTableSize);
#ifdef DL_USE_BGET
      printf("Memory chunk size: %" BUFSIZE_FMT "\n", 
             args.bgetPoolIncrement);
#endif
    putc('\n', stdout);
  }

#ifdef DL_PROFILE
  {  
  cpuTimer *ctimer = newCpuTimer();           /* For tracking CPU time used */
  realTimer *rtimer = newRealTimer();      /* For tracking actual time used */

  /* Start the timers */
  resetRealTimer(rtimer);
  resetCpuTimer(ctimer);
#endif

  /* Begin parsing */
  if (yyparse()) {
    fprintf(stderr, "%s: *** errors in input -- interpretation failed\n",
            PACKAGE);
    return EXIT_FAILURE;
  }

#ifdef DL_PROFILE
  printf("CPU time elapsed = %g\n", readCpuTimer(ctimer));
  printf("Real time elapsed = %g\n", readRealTimer(rtimer));

  freeCpuTimer(ctimer);
  freeRealTimer(rtimer);
  }
#endif

#ifdef HASH_PROFILE
  printf("Atom collisions: %" PRIuMAX "\n", atomTable.collisions);
  printf("Rule collisions: %" PRIuMAX "\n", ruleTable.collisions);
  printf("Total: %" PRIuMAX "\n", atomTable.collisions + ruleTable.collisions);
#endif

 /*
 ** The rule table must be freed before the atom table, because the rule
 ** table references the atom table
 */
#ifdef DL_DEBUG
  fprintf(stderr,"Freeing hash tables\n");
#endif
  hashDeleteTable(&ruleTable, hashFreeRule);
  hashDeleteTable(&atomTable, hashFreeAtom);
  return EXIT_SUCCESS;
}


/*----------------------------------------------------------------------------
Function: hashFreeAtom
Purpose : frees memory associated with a given atom, but doesn't bother
          cleaning up any associated pointers
Args    : a -- pointer to atom to free
Returns : nothing
Notes   : Don't use this function to delete an atom; it should be used only
          when deleting an entire hash table
----------------------------------------------------------------------------*/
void hashFreeAtom(void *a) {
  bfree(((Atom *)a)->id);
  bfree(a);
}


/*----------------------------------------------------------------------------
Function: hashFreeRule
Purpose : frees memory associated with a given rule, but doesn't bother
          cleaning up all associated pointers
Args    : r -- pointer to rule to free
Returns : nothing
Notes   : Don't use this function to delete a rule; it should be used only
          when deleting an entire hash table
----------------------------------------------------------------------------*/
void hashFreeRule(void *r) {
  /* Free rule body */
  while (deleteFirstLiteral(&(((Rule *)r)->body)));

  /* Free rule ID and rule itself */
  bfree(((Rule *)r)->id);
  bfree(r);
}


/*----------------------------------------------------------------------------
Function: hashString
Purpose : a hashing function which converts a string into a size_t, x, such
          that 0 <= x < n
Args    : key -- string to convert
          n -- number of possible mappings for key
Returns : some size_t representing `key'
----------------------------------------------------------------------------*/
size_t hashKey(const void *key, size_t n) {
  const char *k = key;
  unsigned int i = 0;
  union {
    size_t s;
    unsigned char c[sizeof(size_t)];
  } hashVal = {0};

  while (*k)
    hashVal.c[i++ % sizeof(size_t)] += (unsigned char)(*k++);

  return hashVal.s % n;
}


/*----------------------------------------------------------------------------
Function: hashStrcmp
Purpose : a wrapper for strcmp() (for use with hash table routines)
Args    : k1, k2 -- strings to compare
Returns : strcmp(k1, k2)
----------------------------------------------------------------------------*/
int hashStrcmp(const void *k1, const void *k2) {
  return strcmp((const char *)k1, (const char *)k2);
}
