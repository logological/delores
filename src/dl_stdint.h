/*----------------------------------------------------------------------------
File    : $Id: dl_stdint.h,v 1.7 2003-12-12 14:14:01 psy Exp $
What    : Figures out compiler-specific information on standard integer types

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

#ifndef DL_STDINT__H
#define DL_STDINT__H

#include <stdlib.h>

/*
** Determine the largest unsigned integer type and its printf/scanf format
** specifier
*/

#if HAVE_INTTYPES_H
#  include <inttypes.h>
#endif

#if HAVE_UINTMAX_T
#  ifndef PRIuMAX
#    error You appear to have a very strange compiler which has uintmax_t but not PRIuMAX.  Please contact the DELORES bug reports address and provide the details of your compiler.
#  endif
#  ifndef SCNuMAX
#    error You appear to have a very strange compiler which has uintmax_t but not SCNuMAX.  Please contact the DELORES bug reports address and provide the details of your compiler.
#  endif
#  if ! HAVE_STRTOUMAX
#    error You appear to have a very strange compiler which has uintmax_t but not strtoumax().  Please contact the DELORES bug reports address and provide the details of your compiler.
#  endif
#else
#  undef PRIuMAX
#  undef SCNuMAX
#  if HAVE_UNSIGNED_LONG_LONG
     typedef unsigned long long uintmax_t;
#    define PRIuMAX "llu"
#    define SCNuMAX "llu"
#    if HAVE_STRTOULL && ! HAVE_STRTOUMAX
#      define strtoumax strtoull
#    else
#      error You appear to have a very strange compiler which has strtoumax but not uintmax_t and/or strtoull().  Please contact the DELORES bug reports address and provide the details of your compiler.
#    endif
#  else
     typedef unsigned long int uintmax_t;
#    define PRIuMAX "lu"
#    define SCNuMAX "lu"
#    if HAVE_STRTOUL && ! HAVE_STRTOUMAX
#      define strtoumax strtoul
#    else
#      error You appear to have a very strange compiler which has strtoumax but not uintmax_t and/or strtoul().  Please contact the DELORES bug reports address and provide the details of your compiler.
#    endif
#  endif
#endif

/* Determine the largest value representable in a size_t variable */
#if HAVE_STDINT_H
#  include <stdint.h>
#else
   /* Some non-C99 compilers #define SIZE_MAX in <limits.h> */
#  if HAVE_LIMITS_H
#    include <limits.h>
#  endif
#  ifndef SIZE_MAX
#    define SIZE_MAX ((size_t)(-1))
#  endif
#endif

#endif
