/*----------------------------------------------------------------------------
Filename: dl_compile.h
What    : Figures out compiler-specific information for the dl project
Author  : Tristan Miller
Notes   : This should be the *first* file included in any project, since it
          determines what function prototypes in the standard libraries get
          compiled in.  In particular, it must come before #including
          <string.h> on gcc.
----------------------------------------------------------------------------*/

#ifndef DL_COMPILE__H
#define DL_COMPILE__H

/*
** The following macros apply to glibc environments, but must be defined
** prior to including any system library
*/
/* Turn on ANSI C9X features (required for <stdint.h>) */
#define _ISOC9X_SOURCE
/* Turn on POSIX, 4.3BSD, and SVID features (required for strdup()) */
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1


#include <stdlib.h>


#ifndef __GNUC__
/*
** Ignore __extension__, which is a GNU-specific do-nothing macro for
** suppressing certain ANSI warnings
*/
#  define __extension__
#endif

#endif
