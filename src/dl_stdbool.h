/*----------------------------------------------------------------------------
File    : $Id: dl_stdbool.h,v 1.2 2003-12-09 19:15:29 psy Exp $
What    : Figures out compiler-specific information on boolean variables
Author  : Tristan Miller
----------------------------------------------------------------------------*/

#ifndef DL_STDBOOL__H
#define DL_STDBOOL__H

/* Set up boolean data structure */
#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#  include <stdbool.h>
#else
   typedef int _Bool;
#  define true 1
#  define false 0
#endif

#endif
