/*-----------------------------------------------------------------------------
File    : $Id: dl_strdup.c,v 1.2 2003-12-09 19:15:29 psy Exp $
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

__inline__ char *dl_strdup(const char *s) {
  size_t len=strlen(s)+1;
  char *new=balloc(len);
  return new?(char *)memcpy(new,s,len):NULL;
}

#endif
