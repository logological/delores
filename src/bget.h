/*----------------------------------------------------------------------------
File    : $Id: bget.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Memory allocation tool (faster replacement for malloc() and friends)
Notes   : This file has been modified by Tristan Miller to remove ancient K&R-
          style prototypes, and to add the macros BUFSIZE_MAX and ATOBUFSIZE

Based on public domain code by John Walker, Duff Kurland, and Greg Lutz.
Copyright (C) 1999, 2000 Michael Maher
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

#ifndef BGET__H
#define BGET__H

#include <limits.h>
#define BUFSIZE_MAX LONG_MAX
#define BUFSIZE_FMT "ld"
#define ATOBUFSIZE(x) atol(x)
typedef long bufsize;

void	bpool	     (void *buffer, bufsize len)  ;
void   *bget	     (bufsize size)  ;
void   *bgetz	     (bufsize size)  ;
void   *bgetr	     (void *buffer, bufsize newsize)  ;
void	brel	     (void *buf)  ;
void	bectl	     (int (*compact)(bufsize sizereq, int sequence),
		       void *(*acquire)(bufsize size),
		       void (*release)(void *buf), bufsize pool_incr)  ;
void	bstats	     (bufsize *curalloc, bufsize *totfree, bufsize *maxfree,
		       long *nget, long *nrel)  ;
void	bstatse      (bufsize *pool_incr, long *npool, long *npget,
		       long *nprel, long *ndget, long *ndrel)  ;
void	bufdump      (void *buf)  ;
void	bpoold	     (void *pool, int dumpalloc, int dumpfree)  ;
int	bpoolv	     (void *pool)  ;

#endif