/*----------------------------------------------------------------------------
File    : $Id: cmd_line_args.h,v 1.6 2003-12-11 19:51:33 psy Exp $
What    : Header file for lexer for parsing command line arguments to dl

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

#include "dl_malloc.h"
#include "dl_stdbool.h"
#include <stdio.h>

typedef struct {
  size_t atomTableSize;
  size_t ruleTableSize;
#ifdef DL_USE_BGET
  bufsize bgetPoolIncrement;
#endif
  char inputFile[FILENAME_MAX + 1];
  bool quietMode;
} cmdargs_t;

cmdargs_t getCmdLineArgs(int argc, char *argv[]);
void printVersionInformation(void);
