/*----------------------------------------------------------------------------
File    : $Id: dl_malloc.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Header file for memory allocation functions
Author  : Tristan Miller
----------------------------------------------------------------------------*/

#ifndef DL_MALLOC__H
#define DL_MALLOC__H

#define DL_USE_BGET                   /* Use BGET package instead of malloc */

#include "dl_inline.h"               /* Allow inline funtions, if supported */

__inline__ void *balloc(size_t size);

#undef bfree

#ifdef DL_USE_BGET
# define bfree(x) brel(x)
# include "bget.h"
  void bufferPoolInitialize(bufsize bufferPoolIncrement);
#else
# define bfree(x) free(x)
#endif
 
#endif
