/*----------------------------------------------------------------------------
File    : $Id: dl_strdup.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Header file for strdup() function
Author  : Tristan Miller
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

