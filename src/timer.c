/*----------------------------------------------------------------------------
Filename: timer.c
What    : Functions for timing
Author  : Tristan Miller
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
__inline__ cpuTimer *newCpuTimer(void) {
  return (cpuTimer *)balloc(sizeof(cpuTimer));
}

/*----------------------------------------------------------------------------
Function: resetCpuTimer
Purpose : resets the specified CPU timer to 0
Args    : pointer to the CPU timer to start
Returns : nothing
----------------------------------------------------------------------------*/
__inline__ void resetCpuTimer(cpuTimer *t) {
  *t=clock();
}

/*----------------------------------------------------------------------------
Function: readCpuTimer
Purpose : reads the specified CPU timer
Args    : pointer to CPU timer to read
Returns : time elapsed, in seconds
----------------------------------------------------------------------------*/
__inline__ double readCpuTimer(cpuTimer *t) {
  return ((clock()-*t)/(double)CLOCKS_PER_SEC);
}

/*----------------------------------------------------------------------------
Function: freeCpuTimer
Purpose : frees memory associated with the specified CPU timer
Args    : pointer to CPU timer to free
Returns : nothing
----------------------------------------------------------------------------*/
__inline__ void freeCpuTimer(cpuTimer *t) {
  bfree(t);
}

/*----------------------------------------------------------------------------
Function: newRealTimer
Purpose : creates a new real-time timer (i.e., a clock_t variable)
Args    : none
Returns : pointer to new real-time timer, or NULL if out of memory
----------------------------------------------------------------------------*/
__inline__ realTimer *newRealTimer(void) {
  return (realTimer *)balloc(sizeof(realTimer));
}

/*----------------------------------------------------------------------------
Function: resetRealTimer
Purpose : resets the specified real-time timer to 0
Args    : pointer to the real-time timer to start
Returns : nothing
----------------------------------------------------------------------------*/
__inline__ void resetRealTimer(realTimer *t) {
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
__inline__ double readRealTimer(realTimer *t) {
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
__inline__ void freeRealTimer(realTimer *t) {
#if REAL_TIMER_METHOD==0 || REAL_TIMER_METHOD==1
  bfree(t);
#endif
}

