/*----------------------------------------------------------------------------
File    : $Id: timer.h,v 1.10 2003-12-14 14:47:35 psy Exp $
What    : Timing functions header

Copyright (C) 1999, 2000 Michael Maher <mjm@math.luc.edu>
Copyright (C) 1999, 2000, 2003 Tristan Miller <Tristan.Miller@dfki.de>

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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

/*
** Set up macros and function prototypes for the ANSI standard CPU timer
** which measures CPU time used.
*/
#include <time.h>
typedef clock_t cpuTimer;

inline cpuTimer *newCpuTimer(void);
inline void resetCpuTimer(cpuTimer *t);
inline double readCpuTimer(cpuTimer *t);
inline void freeCpuTimer(cpuTimer *t);

#if HAVE_SYS_TIMEB_H && HAVE_FTIME
#  define REAL_TIMER_METHOD 1
#  include <sys/timeb.h>
   typedef struct timeb realTimer;
#else
#  define REAL_TIMER_METHOD 0
   typedef int realTimer;
#endif

inline realTimer *newRealTimer(void);
inline void resetRealTimer(realTimer *t);
inline double readRealTimer(realTimer *t);
inline void freeRealTimer(realTimer *t);

#endif

