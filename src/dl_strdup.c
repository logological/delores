/*-----------------------------------------------------------------------------
File    : $Id: dl_strdup.c,v 1.4 2003-12-12 13:51:05 psy Exp $
Purpose : Returns a newly allocated area of memory that contains a duplicate
          of the string pointed to by s. The memory returned by this call must
          be freed by the caller. 
Args    : s -- the string to duplicate
Returns : the newly-allocated string, or NULL if there is no memory
-----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dl_malloc.h"
#include "dl_strdup.h"

#ifndef dl_strdup

inline char *dl_strdup(const char *s) {
  size_t len=strlen(s)+1;
  char *newstr=balloc(len);
  return newstr?(char *)memcpy(newstr,s,len):NULL;
}

#endif
