/*----------------------------------------------------------------------------
Filename: timer.h
What    : Timing functions header
Author  : Tristan Miller
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

