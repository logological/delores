/*----------------------------------------------------------------------------
File    : $Id: cmd_line_args.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Header file for lexer for parsing command line arguments to dl
Author  : Tristan Miller
----------------------------------------------------------------------------*/

#include "dl_malloc.h"
#include "dl_stdbool.h"
#include <limits.h>

typedef struct {
  size_t atomTableSize;
  size_t ruleTableSize;
#ifdef DL_USE_BGET
  bufsize bgetPoolIncrement;
#endif
  char inputFile[PATH_MAX + 1];
  _Bool quietMode;
} cmdargs_t;

cmdargs_t getCmdLineArgs(int argc, char *argv[]);
void printVersionInformation(void);
