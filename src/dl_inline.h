/*----------------------------------------------------------------------------
File    : $Id: dl_inline.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Figures out compiler-specific information on inline functions
Author  : Tristan Miller
----------------------------------------------------------------------------*/

#ifndef DL_INLINE__H
#define DL_INLINE__H

#include <stdlib.h>

#undef DL_USE_INLINE

#if defined __GNUC__
/* Do nothing; __inline__ is already defined by GCC */
#elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
/* Enable inline functions on C9X compilers (enabled in gcc by default) */
#  define __inline__ inline
#else
/* Disable inline functions on non-C9X compilers */
#  define __inline__
#endif

#endif
