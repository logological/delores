/*----------------------------------------------------------------------------
File    : $Id: dl_malloc.c,v 1.7 2003-12-14 14:47:34 psy Exp $
What    : Memory allocation functions

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

#include "dl.h"
#include <stdlib.h>
#include <limits.h>
#include "dl_malloc.h"
#ifdef DL_USE_BGET
#  include "bget.h"
#endif

/*-----------------------------------------------------------------------------
Function: balloc
Purpose : Allocates a buffer of the given size, and aborts on failure
Args    : size -- number of bytes to allocate
Returns : pointer to the allocated memory on success, otherwise aborts program
-----------------------------------------------------------------------------*/
inline void *balloc(size_t size) {
  void *p;

#ifdef DL_USE_BGET

  /*
  ** BGET does its allocation in terms of long integers rather than size_t's.
  ** If the following error message ever occurs, try recompiling without
  ** DL_USE_BGET #defined.
  */
  if (size > BUFSIZE_MAX) {
    yyerror("cannot allocate > BUFSIZE_MAX bytes using BGET");
    exit(EXIT_FAILURE);
  }

  if ((p=bget(size))==NULL) {
    yyerror("out of memory!");
    exit(EXIT_FAILURE);
  }

#else

  if ((p=malloc(size))==NULL) {
    yyerror("out of memory!");
    exit(EXIT_FAILURE);
  }

#endif

  return p;
}


#ifdef DL_USE_BGET

/*-----------------------------------------------------------------------------
Function: bufferPoolExpand
Purpose : Expands the BGET buffer pool
Args    : size -- number of bytes to add to the pool
Returns : pointer to the allocated memory on success, or NULL on failure
-----------------------------------------------------------------------------*/
static void *bufferPoolExpand(bufsize size) {
  return malloc((size_t) size);
}


/*-----------------------------------------------------------------------------
Function: bufferPoolShrink
Purpose : Frees unused buffer pools
Args    : buf -- pointer to the buffer pool to free
Returns : nothing
-----------------------------------------------------------------------------*/
static void bufferPoolShrink(void *buf) {
  free((char *) buf);
}


/*-----------------------------------------------------------------------------
Function: bufferPoolInitialize
Purpose : Allocates space in the buffer pool and registers the buffer expan-
          sion/contraction control functions
Args    : bufferPoolIncrement -- memory pool will be increased by chunks of
                                 this many bytes at a time
Returns : nothing
Notes   : If you are using BGET, this function must be called at the beginning
          of the program, before any call to balloc().  It need be called but
          once.
-----------------------------------------------------------------------------*/
void bufferPoolInitialize(bufsize bufferPoolIncrement) {
  static void *bufferPool = NULL;

  /* Ignore repeat calls to this function */
  if (bufferPool)
    return;

  bufferPool = malloc(bufferPoolIncrement);
  if (!bufferPool) {
    yyerror("out of memory!");
    exit(EXIT_FAILURE);
  }

  bpool(bufferPool, bufferPoolIncrement);
  bectl(NULL, bufferPoolExpand, bufferPoolShrink, bufferPoolIncrement);
}

#endif

