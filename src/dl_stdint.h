/*----------------------------------------------------------------------------
File    : $Id: dl_stdint.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Figures out compiler-specific information on standard integer types

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

#ifndef DL_STDINT__H
#define DL_STDINT__H

#include <stdlib.h>

/*
** Determine the largest unsigned integer type and its printf/scanf format
** specifier
*/
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L) || \
    (defined __GLIBC__ && defined _ISOC9X_SOURCE)
#  include <stdint.h>
#  define UINTMAX_FMT "llu"
#  define ATOSIZE_T(x) atoll(x)
#elif defined __sun__
#  include <inttypes.h>
#  define UINTMAX_FMT "llu"
#  define ATOSIZE_T(x) atoll(x)
#else
   typedef unsigned long int uintmax_t;
#  define UINTMAX_FMT "lu"
#  define ATOSIZE_T(x) atol(x)
#endif

/* Determine the largest value representable in a size_t variable */
#if (defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L) || \
    (defined __GLIBC__ && defined _ISOC9X_SOURCE)
#  include <stdint.h>
#else
   /* Some non-C9X compilers #define SIZE_MAX in <limits.h> */
#  include <limits.h>
#  ifndef SIZE_MAX
#    define SIZE_MAX ((size_t)(-1))
#  endif
#endif

#endif
