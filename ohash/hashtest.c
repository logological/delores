#include <stddef.h>   /* For size_t   */
#include <string.h>   /* For strcmp() */
#include <stdio.h>    /* For printf() */
#include "ohash.h"

#define TRUE  1
#define FALSE 0

/* A hash function which converts a string into a size_t */
size_t hashKey(const void *key) {
  size_t hashVal = 0;
  register const char *k = key;

  while (*k)
	hashVal = (hashVal << 5) + *k++;

  return hashVal;
}

/* A string comparison function (essentially a wrapper for strcmp()) */
int compareKey(const void *k1, const void *k2) {
  return strcmp((const char *)k1, (const char *)k2);
}

/* Here's a data structure, instances of which we will store in the table */
typedef struct pontiff {
  char name[16];
  int reign;
  int canonized;
} pontiff;


int main(void) {
  int j;
  pontiff *popePtr;
  hashIterator i;
  hashTable LiberPontificalis;
  const pontiff pope[6] = {
	{"Celestine V", 1294, TRUE }, {"Benedict XI", 1303, FALSE},
	{"Johanna"    ,  855, FALSE}, {"Innocent XI", 1676, FALSE},
    {"Benedict II",  684, TRUE }, {"Adeodatus I",  615, TRUE }};

  /*
  ** Construct the Liber Pontificalis, a table of 13 indices with hashing
  ** function hashKey() and hash key comparator compareKey()
  */
  hashConstructTable(&LiberPontificalis, 13, hashKey, compareKey);

  /* Insert all popes in the Liber Pontificalis */
  for (j = 0; j < 6; j++) {
	printf("Inserting `%s'...\n",pope[j].name);
	hashInsert(pope[j].name, &pope[j], &LiberPontificalis);
  }

  /* List all popes in the Liber Pontificalis */
  printf("\nHere is the contents of the Liber Pontificalis:\n");
  hashInitIterator(&i, &LiberPontificalis);
  while ((popePtr = hashGetDatum(&i)))
	printf("%s%s was consecrated in %d.\n",
		   popePtr->canonized ? "St. " : "", popePtr->name, popePtr->reign);

  /* Canonize Benedict XI and Innocent XI */
  printf("\nCanonizing `Benedict XI'...\n");
  popePtr = hashLookup("Benedict XI", &LiberPontificalis);
  popePtr->canonized = TRUE;
  printf("Canonizing `Innocent XI'...\n");
  popePtr = hashLookup("Innocent XI", &LiberPontificalis);
  popePtr->canonized = TRUE;

  /* Remove Popess Johanna, since she wasn't a real pope */
  printf("Deleting `Johanna'...\n");
  popePtr = hashDelete("Johanna", &LiberPontificalis);

  /* List all popes in the revised Liber Pontificalis */
  printf("\nHere is the contents of the Liber Pontificalis:\n");
  while ((popePtr = hashGetDatum(&i)))
	printf("%s%s was consecrated in %d.\n",
		   popePtr->canonized ? "St. " : "", popePtr->name, popePtr->reign);

  /* Delete the Liber Pontificalis -- who needs it, anyway? */
  hashDeleteTable(&LiberPontificalis, NULL);

  return 0;
}
