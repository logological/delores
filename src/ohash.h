/*----------------------------------------------------------------------------
File    : $Id: ohash.h,v 1.6 2003-12-13 15:30:56 psy Exp $
What    : Header file for hash table module -- see hash.c for info

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

#ifndef OHASH__H
#define OHASH__H

#include <stddef.h>             /* For size_t */
#include <dl_stdint.h>

/* #define HASH_PROFILE */      /* Compile in profiling code */

/*
** A hash table consists of an array of buckets.  Each bucket holds a copy of
** the key, a pointer to the datum associated with the key, and a pointer to
** the next bucket that collided with it (if it exists).  You will not need to
** manipulate variables of this data type directly.
*/

typedef struct hashBucket {
  const void *key;
  void *datum;
  struct hashBucket *next;
} hashBucket;

/*
** Declare variables of type hashTable to create instances of a hash table.
** They must then be constructed with hashConstructTable() before use.  You
** will not need to manipulate members inside this data structure directly;
** instead, use the provided interface functions below.
*/

typedef struct hashTable {
  size_t size;
  size_t population;
  size_t (*hash)(const void *key, size_t n);
  int (*compare)(const void *key1, const void *key2);
  hashBucket **table;
#ifdef HASH_PROFILE
  /*
  ** The following variable keeps tracks of how many collisions occur when in-
  ** serting data in the hash table.  This is useful for testing the efficiency
  ** of your hashing function; if the number of collisions is much higher than 
  ** expected, you should consider changing your hashing function.  To activate
  ** this variable, uncomment the #define HASH_PROFILE line above, or use the
  ** appropriate command-line option for your compile to define this macro.
  */
  uintmax_t collisions;
#endif
} hashTable;

/*
** A hashIterator allows you to step through the hashBuckets of the hash table
** one at a time.  You will not need to manipulate members inside this data
** structure directly; instead, use the provided interface functions below.
*/

typedef struct hashIterator {
  const hashTable *table;
  const hashBucket *bucket;
  size_t index;
} hashIterator;

/*
** Function : hashConstructTable()
** Purpose  : constructs (`initializes') a newly-declared hash table
** Arguments: table   - pointer to hash table to construct
**            size    - number of array indices for hash table
**            hash    - hashing function which converts a given key into
**                      a size_t (which is an unsigned integer)
**            compare - function for comparing two hash table keys; should
**                      return negative if key1 comes "before" key2 in sorted
**                      order, positive if key1 comes "after" key2 in sorted
**                      order, and zero if both keys are the same.  If the keys
**                      you are using don't lend themselves to sorting,
**                      compare() should return positive if the keys differ
**                      and zero if they are the same.
** Returns  : pointer to the constructed hash table, or NULL on failure
** Notes    : 1) Ideally, `size' should be a prime number, and must be no
**               larger than (size_t)(-1) / sizeof(hashBucket *)
**            2) On failure, `table->size' is set to 0.
**            3) Since this is an open hash table, `size' does not indicate
**               the maximum number of buckets which can be stored in the hash
**               table, but rather the maximum number which can be stored
**               without a collision.  The true capacity of the hash table is
**               determined solely by your available memory.
**            4) If you use strings for your keys, a wrapper function for
**               <string.h>'s strcmp() should do nicely for the compare()
**               function.  For the hash function, you must write your own,
**               or use a perfect hash function generator such as gperf.
*/

hashTable *hashConstructTable(hashTable *table, size_t size,
                              size_t (*hash)(const void *key, size_t n),
                              int (*compare)(const void *key1,
                                             const void *key2));

/*
** Function : hashInsert()
** Purpose  : inserts a pointer to `datum' in the hash table, with a copy of
**            `key' as its key
** Arguments: key   - some key uniquely identifying `datum'
**            datum - the datum to insert
**            table - pointer to the hash table in which to insert
** Returns  : pointer to the datum in the hash table, or NULL on failure
** Notes    : 1) This function makes a copy neither of `datum' nor of `key'.
**               (The latter is typically included within `datum' anyway.)
**            2) If hashInsert() is passed a key which is already in the hash
**               table, the new datum will *not* overwrite the old datum.
**               Instead, the function will simply return a pointer to the
**               datum already in the hash table. (This is to prevent
**               orphaned memory; if you need to replace data, use the
**               hashDelete() function first.)
*/

void *hashInsert(const void *key, const void *datum, hashTable *table);

/*
** Function : hashLookup()
** Purpose  : yields the datum associated with a given key
** Arguments: key   - the key uniquely identifying the datum to find
**            table - pointer to the hash table to search
** Returns  : pointer to the datum matching the given key, or NULL if the
**            key does not exist in the hash table
*/

void *hashLookup(const void *key, const hashTable *table);

/*
** Function : hashDelete()
** Purpose  : deletes an entry from the hash table
** Arguments: key   - the key uniquely identifying the datum to delete
**            table - pointer to the hash table from which to delete
** Returns  : pointer to the datum associated with the key, or NULL if
**            the key was not found in the hash table
** Notes    : This function does *not* free the memory associated with the
**            datum; the calling code is ultimately responsible for this.
*/

void *hashDelete(const void *key, hashTable *table);

/*
** Function : hashPopulation()
** Purpose  : returns the number of buckets currently in the hash table
** Arguments: table - pointer to the hash table to inspect
** Returns  : number of buckets currently stored in the hash table
*/

size_t hashPopulation(const hashTable *table);

/*
** Function : hashDeleteTable()
** Purpose  : deletes the given hash table and frees memory used by it
** Arguments: table     - pointer to hash table to delete
**            freeDatum - either a function that will be called on each datum
**                        before its bucket is deleted, or NULL
** Returns  : nothing
** Notes    : Typically, you will want freeDatum() to free memory associated
**            with a given datum.  Unless the hash table stores structs which
**            include pointers to dynamically-allocated memory, you can
**            usually get away with passing free() as the second argument to
**            hashDeleteTable().  If you don't want to free memory associated
**            with the data in the table (for example, if two hash tables in
**            your program share common data), simply pass NULL as the second
**            argument.  If you'd like, you can free memory used by the hash
**            table data manually using an iterator, but you still need to call
**            hashDeleteTable() to free the memory used by the hash table
**            (i.e. the array of buckets) itself.
*/

void hashDeleteTable(hashTable *table, void (*freeDatum)(void *));

/*
** Function : hashInitIterator()
** Purpose  : initializes an iterator for use with a given table
** Arguments: i     - pointer to iterator to initialize
**            table - pointer to hash table upon which this iterator will
**                    operate
** Returns  : pointer to the newly-initialized iterator, or NULL on failure
*/

hashIterator *hashInitIterator(hashIterator *i, const hashTable *table);

/*
** Function : hashGetDatum()
** Purpose  : yields a datum which has not yet been yielded by the given
**            iterator
** Arguments: i - pointer to the iterator
** Returns  : pointer to a datum which has not yet been returned by the given
**            iterator, or NULL if all data for the iterator's hash table has
**            already been returned
** Notes    : 1) After this function has returned NULL, a subsequent call will
**               return a pointer to a datum as if the iterator had been
**               freshly initialized with hashInitIterator().
**            2) Using iterators to retrieve data is generally only sensible
**               if, between repeated calls to hashGetDatum(), the hash table
**               contents do not change. In other words, the iterator does not
**               keep track of hash table insertions and deletions, so all
**               iterators for a given hash table should be reinitialized after
**               calls to hashInsert() or hashDelete().
*/

void *hashGetDatum(hashIterator *i);

#endif /* OHASH__H */
