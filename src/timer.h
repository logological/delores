/*----------------------------------------------------------------------------
File    : $Id: timer.h,v 1.3 2003-12-09 19:44:18 psy Exp $
What    : Timing functions header

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

#ifndef TIMER__H
#define TIMER__H

#include "dl_inline.h"

/*
** Set up macros and function prototypes for the ANSI standard CPU timer
** which measures CPU time used.
*/
#include <time.h>
typedef clock_t cpuTimer;

__inline__ cpuTimer *newCpuTimer(void);
__inline__ void resetCpuTimer(cpuTimer *t);
__inline__ double readCpuTimer(cpuTimer *t);
__inline__ void freeCpuTimer(cpuTimer *t);

/*
** This section defines our favourite non-ANSI timing functions. If your
** compiler is not listed, #define a new value of REAL_TIMER_METHOD and make
** the appropriate changes to timer.h and timer.c.
*/
#if defined __STRICT_ANSI__
#  define REAL_TIMER_METHOD 0
#elif defined __GNUC__
#  define REAL_TIMER_METHOD 1
#  include <sys/timeb.h>
#else
#  define REAL_TIMER_METHOD 0
#endif

#if REAL_TIMER_METHOD==0
   typedef int realTimer;
#elif REAL_TIMER_METHOD==1
   typedef struct timeb realTimer;
#endif

__inline__ realTimer *newRealTimer(void);
__inline__ void resetRealTimer(realTimer *t);
__inline__ double readRealTimer(realTimer *t);
__inline__ void freeRealTimer(realTimer *t);

#endif

