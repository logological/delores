/*----------------------------------------------------------------------------
File    : $Id: dl_strdup.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Header file for strdup() function

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

#ifndef STRDUP__H
#define STRDUP__H

#include "dl_malloc.h"
#include "dl_inline.h"

/* Determine whether the compiler supports strdup() */
#if !(defined __GLIBC__ &&  (defined _BSD_SOURCE || defined _SVID_SOURCE)) \
 || defined DL_USE_BGET
__inline__ char *dl_strdup(const char *s);
#else
#  undef dl_strdup
#  define dl_strdup(x) strdup(x)
#endif

#endif

