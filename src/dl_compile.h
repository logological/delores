/*----------------------------------------------------------------------------
File    : $Id: dl_compile.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Figures out compiler-specific information for the dl project
Notes   : This should be the *first* file included in any project, since it
          determines what function prototypes in the standard libraries get
          compiled in.  In particular, it must come before #including
          <string.h> on gcc.

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
