/*----------------------------------------------------------------------------
File    : $Id: dl_stdbool.h,v 1.7 2003-12-12 19:30:00 psy Exp $
What    : Figures out compiler-specific information on boolean variables

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

#ifndef DL_STDBOOL__H
#define DL_STDBOOL__H

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_STDBOOL_H
#  include <stdbool.h>
#else
#  if ! HAVE__BOOL
#    ifdef __cplusplus
typedef bool _Bool;
#    else
typedef unsigned char _Bool;
#    endif
#  endif
#  define bool _Bool
#  define false 0
#  define true 1
#  define __bool_true_false_are_defined 1
#endif

#endif
