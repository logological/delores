/*----------------------------------------------------------------------------
File    : $Id: dl_malloc.h,v 1.9 2003-12-14 14:47:34 psy Exp $
What    : Header file for memory allocation functions

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

#ifndef DL_MALLOC__H
#define DL_MALLOC__H

/* Uncomment the following line to use the BGET package instead of malloc */
/* #define DL_USE_BGET */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

inline void *balloc(size_t size);

#undef bfree

#ifdef DL_USE_BGET
# define bfree(x) brel(x)
# include "bget.h"
  void bufferPoolInitialize(bufsize bufferPoolIncrement);
#else
# define bfree(x) free(x)
#endif
 
#endif
