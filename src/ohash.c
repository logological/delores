/*----------------------------------------------------------------------------
File    : $Id: ohash.c,v 1.5 2003-12-10 19:57:14 psy Exp $
What    : Hash table functions

Based on public domain code by Jerry Coffin and HenkJan Wolthuis.
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
#include "ohash.h"
#include "dl_malloc.h"

/* ----------------------------------------------------------------------------
** Allocate space for the hash table of the specified size and return a
** pointer to the table.  All pointers in the table are initialized to NULL.
** If there is insufficient memory, the table size is set to 0 and NULL is
** returned.
*/

hashTable *hashConstructTable(hashTable *table, size_t size,
                              size_t (*hash)(const void *key, size_t n),
                              int (*compare)(const void *key1,
                                             const void *key2)) {
  table->population = 0;
  table->hash = hash;
  table->compare = compare;
#ifdef HASH_PROFILE
  table->collisions = 0;
#endif

  if ((size > (size_t)(-1) / sizeof(hashBucket *)) || size == 0) {
    table->size = 0;
    return NULL;
  }

  table->table = (hashBucket **)balloc(size * sizeof(hashBucket *));
  
  if (table->table == NULL) {
    table->size = 0;
    return NULL;
  }

  table->size = size;
  while (size)
    (table->table)[--size] = NULL;

  return table;
}


/* ----------------------------------------------------------------------------
** Insert `datum', which is uniquely identified by `key', into the hash table.
** Return pointer to the datum in the hash table corresponding to the given
** key, or NULL on failure (insufficient memory).  Note that the return value
** equals `datum' if and only if there is not already a datum with the same
** key in the hash table.
*/

void *hashInsert(const void *key, const void *datum, hashTable *table) {
  size_t hashVal = table->hash(key, table->size);
  hashBucket *bucket;
  int cmp;

  /*
  ** This cell is already in use (i.e. we have a collision).  Search through
  ** the list of buckets to determine if this key is already in the table,
  ** and if so, return a pointer its datum.  If not, insert the key in the
  ** list of buckets such that the list is kept in sorted order.
  */

  for (bucket = (table->table)[hashVal]; bucket; bucket = bucket->next) {
    cmp = table->compare(key, bucket->key);

    /* This key is already in the table; return existing datum */
    if (cmp == 0)
      return bucket->datum;

#ifdef HASH_PROFILE
      table->collisions++;
#endif

    /* This key needs to be inserted at the beginning of the list of buckets */
    if (cmp < 0) {
      hashBucket *newBucket;
      if (!(newBucket = (hashBucket *)balloc(sizeof(hashBucket))))
        return NULL;
      newBucket->key = (void *)key;
      newBucket->datum = (void *)datum;
      newBucket->next = bucket;
      (table->table)[hashVal] = newBucket;
      table->population++;
      return (void *)datum;
    }

    /* This key needs to be inserted after the current bucket */
    else if (bucket->next == NULL ||
             table->compare(key, bucket->next->key) < 0) {
      hashBucket *newBucket;
      if (!(newBucket = (hashBucket *)balloc(sizeof(hashBucket))))
        return NULL;
      newBucket->key = (void *)key;
      newBucket->datum = (void *)datum;
      newBucket->next = bucket->next;
      bucket->next = newBucket;
      table->population++;
      return (void *)datum;
    }
  }

  /*
  ** This cell of the hash table hasn't been used yet.  We simply
  ** allocate space for the new bucket and point the table cell at it.
  */
  
  if (!(bucket = (hashBucket *)balloc(sizeof(hashBucket))))
    return NULL;

  bucket->key = (void *)key;
  bucket->datum = (void *)datum;
  bucket->next = NULL;
  (table->table)[hashVal] = bucket;
  table->population++;
  return (void *)datum;
}


/* ----------------------------------------------------------------------------
** Look up a key and return the associated datum.  Return NULL if the key is
** not in the table.
*/

void *hashLookup(const void *key, const hashTable *table) {
  hashBucket *bucket;
  int cmp;

  for (bucket = (table->table)[table->hash(key, table->size)];
       bucket; bucket = bucket->next) {
    cmp = table->compare(key, bucket->key);
    if (cmp == 0)
      return bucket->datum;
    else if (cmp < 0 || bucket->next == NULL)
      return NULL;
  }

  return NULL;
}


/* ----------------------------------------------------------------------------
** Delete a bucket from the hash table and return its associated datum, or
** NULL if the key is not in the table.
*/

void *hashDelete(const void *key, hashTable *table) {
  size_t hashVal = table->hash(key, table->size);
  hashBucket *bucket, *last;
  void *datum;
  int cmp;

  /*
  ** Traverse the list of buckets, keeping track of the last bucket visited.
  ** When the node to delete is found, set the last bucket's `next' pointer
  ** to the bucket after the one to be excised.  Then delete the appropriate
  ** bucket and return a pointer to its datum.
  */

  for (bucket = (table->table)[hashVal], last = NULL; bucket;
       last = bucket, bucket = bucket->next) {
    cmp = table->compare(key, bucket->key);
    if (cmp == 0) {
      if (last)
        last->next = bucket->next;
      else
        (table->table)[hashVal] = bucket->next;
      datum = bucket->datum;
      bfree(bucket);
      table->population--;
      return datum;
    } else if (cmp < 0)
      return NULL;
  }

  return NULL;
}


/* ----------------------------------------------------------------------------
** This function returns the current population of (i.e. number of buckets in)
** the given hash table.
*/

size_t hashPopulation(const hashTable *table) {
  return table->population;
}

/* ----------------------------------------------------------------------------
** Delete an entire hash table by freeing each bucket and then freeing the
** storage for the table itself.  The second parameter is a pointer to a
** function which will be called on each datum in the table.  Typically,
** this function would free the memory associated with the data, though
** this depends largely on whether you want the data to disappear along
** with the table.
*/
void hashDeleteTable(hashTable *table, void (*freeDatum)(void *)) {
  hashBucket *bucket;

  while (--table->size != (size_t)(-1))
    while ((bucket = (table->table)[table->size])) {
      (table->table)[table->size] = bucket->next;
      if (freeDatum)
        freeDatum(bucket->datum);
      bfree(bucket);
    }

  bfree(table->table);
  table->size = 0;
  table->population = 0;
  table->table = NULL;
}


/* ----------------------------------------------------------------------------
** This function initializes an iterator for use with a hash table.
*/

hashIterator *hashInitIterator(hashIterator *i, const hashTable *table) {
  if (i) {
    i->bucket = NULL;
    i->index = 0;
    i->table = (hashTable *)table;
  }
  return i;
}


/* ----------------------------------------------------------------------------
** This function returns a new datum from the table associated with the given
** iterator, or NULL if there is no more new data to return.  A subsequent call
** to this function after it has returned NULL will make it behave as if the
** iterator was newly initialized.
*/

void *hashGetDatum(hashIterator *i) {
  if (i->bucket) {

    /*
    ** If we're not yet at the end of the list, find the next bucket in the
    ** current cell's list of buckets
    */

    if (i->bucket->next) {
      i->bucket = i->bucket->next;
      return i->bucket->datum;
    }

    /*
    ** There are no buckets left in the current cell's list.  Increment the
    ** hash table array index until we find the next cell that contains a
    ** bucket.
    */

    for (++i->index; i->index < i->table->size; i->index++)
      if ((i->table->table)[i->index]) {
        i->bucket = (i->table->table)[i->index];
        return i->bucket->datum;
      }
  }

  /*
  ** This iterator is freshly initialized.  Start at the first hash table array
  ** index and iterate through them until we find the first cell that contains
  ** a bucket.
  */

  else
    for (i->index = 0; i->index < i->table->size; i->index++)
      if ((i->table->table)[i->index]) {
        i->bucket = (i->table->table)[i->index];
        return i->bucket->datum;
      }

  /* 
  ** No cells containing buckets were found.  Reset the iterator and return
  ** NULL.
  */

  i->index = 0;
  i->bucket = NULL;
  return NULL;
}
