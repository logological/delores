/*----------------------------------------------------------------------------
File    : $Id: dl_stdint.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Figures out compiler-specific information on standard integer types
Author  : Tristan Miller
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
