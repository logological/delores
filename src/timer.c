/*----------------------------------------------------------------------------
File    : $Id: timer.c,v 1.5 2003-12-11 11:25:15 psy Exp $
What    : Functions for timing

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

#include <stdlib.h>
#include "dl_malloc.h"
#include "timer.h"

/*----------------------------------------------------------------------------
Function: newCpuTimer
Purpose : creates a new CPU timer (i.e., a clock_t variable)
Args    : none
Returns : pointer to new CPU timer, or NULL if out of memory
----------------------------------------------------------------------------*/
inline cpuTimer *newCpuTimer(void) {
  return balloc(sizeof(cpuTimer));
}

/*----------------------------------------------------------------------------
Function: resetCpuTimer
Purpose : resets the specified CPU timer to 0
Args    : pointer to the CPU timer to start
Returns : nothing
----------------------------------------------------------------------------*/
inline void resetCpuTimer(cpuTimer *t) {
  *t=clock();
}

/*----------------------------------------------------------------------------
Function: readCpuTimer
Purpose : reads the specified CPU timer
Args    : pointer to CPU timer to read
Returns : time elapsed, in seconds
----------------------------------------------------------------------------*/
inline double readCpuTimer(cpuTimer *t) {
  return ((clock()-*t)/(double)CLOCKS_PER_SEC);
}

/*----------------------------------------------------------------------------
Function: freeCpuTimer
Purpose : frees memory associated with the specified CPU timer
Args    : pointer to CPU timer to free
Returns : nothing
----------------------------------------------------------------------------*/
inline void freeCpuTimer(cpuTimer *t) {
  bfree(t);
}

/*----------------------------------------------------------------------------
Function: newRealTimer
Purpose : creates a new real-time timer (i.e., a clock_t variable)
Args    : none
Returns : pointer to new real-time timer, or NULL if out of memory
----------------------------------------------------------------------------*/
inline realTimer *newRealTimer(void) {
  return balloc(sizeof(realTimer));
}

/*----------------------------------------------------------------------------
Function: resetRealTimer
Purpose : resets the specified real-time timer to 0
Args    : pointer to the real-time timer to start
Returns : nothing
----------------------------------------------------------------------------*/
inline void resetRealTimer(realTimer *t) {
#if REAL_TIMER_METHOD==0
  *t=0;
#elif REAL_TIMER_METHOD==1
  ftime(t);
#endif
}

/*----------------------------------------------------------------------------
Function: readRealTimer
Purpose : reads the specified real-time timer
Args    : pointer to real-time timer to read
Returns : time elapsed, in seconds
----------------------------------------------------------------------------*/
inline double readRealTimer(realTimer *t) {
#if REAL_TIMER_METHOD==0
  return 0;
#elif REAL_TIMER_METHOD==1
  realTimer endTime;
  ftime(&endTime);
  return (endTime.time-t->time+(endTime.millitm-t->millitm)/(double)1000);
#endif
}

/*----------------------------------------------------------------------------
Function: freeRealTimer
Purpose : frees memory associated with the specified real-time timer
Args    : pointer to real-time timer to free
Returns : nothing
----------------------------------------------------------------------------*/
inline void freeRealTimer(realTimer *t) {
#if REAL_TIMER_METHOD==0 || REAL_TIMER_METHOD==1
  bfree(t);
#endif
}

