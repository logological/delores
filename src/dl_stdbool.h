/*----------------------------------------------------------------------------
File    : $Id: dl_stdbool.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Figures out compiler-specific information on boolean variables

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
