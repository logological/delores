/*----------------------------------------------------------------------------
File    : $Id: dl.h,v 1.6 2003-12-11 19:08:23 psy Exp $
What    : Data structures, macros, headers, etc. shared among dl.l (the
          lexer), dl.y (the parser), and dl.c (the interpreter)

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

#ifndef DL__H
#define DL__H

/*----------------------------------------------------------------------------
Debugging flags
----------------------------------------------------------------------------*/
#if 0
#define DL_DEBUG             /* Enable debug messages in main program */
#define DL_LEXER_DEBUG       /* Enable debug messages in lexer */
#define DL_PARSER_DEBUG      /* Enable debug messages in parser */
#define DL_PROFILE           /* Time program execution */
#endif

/*----------------------------------------------------------------------------
Include files
----------------------------------------------------------------------------*/
#include "ohash.h"           /* For hashTable data structures */
#include "dl_stdbool.h"      /* For boolean data structure */
#include <stdio.h>           /* For FILE */
#include <stdlib.h>          /* For size_t */

/*----------------------------------------------------------------------------
Macros
----------------------------------------------------------------------------*/
#define DL_TITLE "DELORES"
#define DL_VER "0.59"
#define DL_PROGNAME "dl"
#define printAtom(a) fprintAtom(stdout,(a))
#define printLiteralList(l,n) fprintLiteralList(stdout,(l),(n))
#define printRule(r) fprintRule(stdout,(r))

/*----------------------------------------------------------------------------
Data structures and typedefs
----------------------------------------------------------------------------*/

typedef struct Atom Atom;
typedef struct Literal Literal;
typedef struct Rule Rule;
typedef struct RuleList RuleList;

/* Data structure for atoms */
struct Atom {
  char *id;                         /* Name of atom */
    
  bool plus_delta;
  bool minus_delta;
  bool plus_DELTA;
  bool minus_DELTA;
  bool plus_sigma;
  bool minus_sigma;
  bool plus_delta_neg;
  bool minus_delta_neg;
  bool plus_DELTA_neg;
  bool minus_DELTA_neg;
  bool plus_sigma_neg;
  bool minus_sigma_neg;
  bool unknown;
  bool unknown_neg;
  
  struct Literal *strict_occ;           /* List of strict occurrences */
  struct Literal *strict_occ_neg;
  struct Literal *defeasible_occ;       /* List of defeasible occurrences */
  struct Literal *defeasible_occ_neg;   
  struct Literal *defeater_occ;         /* List of defeater occurrences */
  struct Literal *defeater_occ_neg;
  
  struct RuleList *rule_heads;          /* List of rules with atom as head */
  struct RuleList *rule_heads_neg;
};

/* Data structure for rules */
struct Rule {
  char *id;                         /* Rule label */
  struct Atom *head;                /* Head atom */
  bool head_neg;                    /* Is head atom negated? */
  int arrow_type;                   /* SARROW, DARROW, or DEFARROW */
  struct Literal *body;             /* List of literals in body */
  unsigned long ordinal;            /* Order in which rule was entered */
};

/* Data structure for lists of rules */
struct RuleList {
  struct Rule *rule;
  struct RuleList *next;
  struct RuleList *prev;
};

/* Data structure for literals and lists thereof */
struct Literal {
  struct Atom *atom;                    /* Which atom? */
  bool neg;                             /* Is atom negated? */
  
  bool marked;                          /* Used in WF inference engine */
  
  struct Rule *rule;                    /* Pointer to rule containing this
                                           literal */
  struct Literal *next;                 /* Next literal in body */
  struct Literal *prev;                 /* Previous literal in body */
  struct Literal *down;                 /* Next literal in the equivalence
                                           class */
  struct Literal *up;                   /* Previous literal in the equivalence
                                           class */
};


/*----------------------------------------------------------------------------
Global variables
----------------------------------------------------------------------------*/
extern hashTable atomTable;      /* All atoms in the program */
extern hashTable ruleTable;      /* All rules in the program */
extern char *yy_current_file;    /* Current input file */
extern char *yytext;             /* Current lex token being examined */
extern int yylineno;             /* Current source code line being processed*/
extern bool InteractiveMode;     /* Does the program come from a TTY? */


/*----------------------------------------------------------------------------
Function prototypes
----------------------------------------------------------------------------*/
int yyparse(void);
int yylex(void);
void yyerror(const char *errmsg);
Rule *initRule(Rule *r, const char *id, const Atom *head, bool head_neg,
               int arrow, Literal *body);
Literal *initLiteral(const Atom *a, bool neg, Literal **list);
Atom *initAtom(const char *id);
void fprintLiteralList(FILE *fp, const Literal *l, size_t next);
void fprintRule(FILE *fp, const Rule *r);
void fprintAtom(FILE *fp, const Atom *a);
Rule *dupRule(const Rule *r, const char *idfmt);
Rule *addRule(Rule *r, Rule *(*func)(Rule *,Rule *(*)()));
Rule *elimDef(Rule *r1, Rule *(*func)(Rule *,Rule *(*)()));
Rule *elimSupRel_r(Rule *r1, Rule *(*func)(Rule *, Rule *(*)()));
Rule *elimSupRel(const char *r1, const char *r2,
                 Rule *(*func)(Rule *, Rule *(*)()));
void infer(void);
void inferWellFounded(void);
Rule *deleteFirstLiteral(Literal **L);

#endif
