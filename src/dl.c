/*----------------------------------------------------------------------------
File    : $Id: dl.c,v 1.13 2003-12-18 21:05:04 psy Exp $
What    : Defeasible logic interpreter functions

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

#include <string.h>
#include "dl_strdup.h"
#include <stdlib.h>
#include <stdio.h>
#include "timer.h"
#include "dl.h"
#include "parser.h"
#include "dl_malloc.h"
#include "dl_stdint.h"


/*----------------------------------------------------------------------------
Local function prototypes
----------------------------------------------------------------------------*/
static void setRuleName(Rule *r);
static void deleteLiteral(Atom *a, _Bool neg, Literal **L);
static Literal *deleteRule(Rule *r);
static int cmpRuleOrder(const void *r1, const void *r2);
static void inferInitializeS(Literal **S);
static void inferInitializeL(Literal **L);
static void inferInitializeLambda(Literal **Lambda);
static void inferStandardAlgorithm(Literal **S, Literal **Lambda);
static _Bool checkInferenceStandard(Atom *a, _Bool neg, Literal **Lambda);
static void inferRevisedAlgorithm(Literal **S, Literal **Lambda);
static _Bool checkInferenceRevised(Atom *a, _Bool neg, Literal **Lambda);
static void resetRules(void);


/*----------------------------------------------------------------------------
Function: infer
Purpose : regular inference engine
Args    : none
Returns : nothing
Source  : "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void infer(void) {
  Literal *S = NULL;
  cpuTimer *ctimer = newCpuTimer();           /* For tracking CPU time used */
  realTimer *rtimer = newRealTimer();      /* For tracking actual time used */

  /* Start the timers */
  resetRealTimer(rtimer);
  resetCpuTimer(ctimer);
  
  /* Initialize S with relevant literals */
  inferInitializeS(&S);

  /* Execute the Standard Algorithm on S */
  inferStandardAlgorithm(&S, NULL);

  /* Print out timer data and delete timers */
  printf("CPU time elapsed = %g\n", readCpuTimer(ctimer));
  printf("Real time elapsed = %g\n", readRealTimer(rtimer));
  freeCpuTimer(ctimer);
  freeRealTimer(rtimer);
}


/*----------------------------------------------------------------------------
Function: inferWellFounded
Purpose : well-founded inference engine
Args    : none
Returns : nothing
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferWellFounded(void) {
  Literal *S = NULL, *L = NULL, *Lambda = NULL;
  cpuTimer *ctimer = newCpuTimer();           /* For tracking CPU time used */
  realTimer *rtimer = newRealTimer();      /* For tracking actual time used */

  /* Start the timers */
  resetRealTimer(rtimer);
  resetCpuTimer(ctimer);
  
  /* Initialize S, L, and Lambda with relevant literals */
  inferInitializeS(&S);
  inferInitializeL(&L);
  inferInitializeLambda(&Lambda);

  /* Execute the Standard Algorithm on S */
  inferStandardAlgorithm(&S, &Lambda);

  /*
  ** For each literal l in L, if neither +D L nor -D l, set -D l and add
  ** l to S.  This is all we need to do for strict rules unless we add failD
  ** to strict rules, since no positive conclusion depends on a negative
  ** conclusion.
  */
  while (L) {

    _Bool *plus_DELTA = L->neg ? 
      &(L->atom->plus_DELTA_neg) : &(L->atom->plus_DELTA);
    _Bool *minus_DELTA = L->neg ? 
      &(L->atom->minus_DELTA_neg) : &(L->atom->minus_DELTA);

    if (!*plus_DELTA && !*minus_DELTA) {
      *minus_DELTA = true;
      initLiteral(L->atom, L->neg, &S);
    }

    if ((L = L->next))
      bfree(L->prev);
  }
  bfree(L);

  /*
  ** Run a version of the standard algorithm whose effects can be undone.
  ** We are computing (and marking) those literals l such that +d l is
  ** possibly true.  This is only for defeasible rules.
  **
  ** An undetermined literal l is one such that neither +d l nor -d l has
  ** been proved.  For each undetermined literal l in Lambda, if +s l, mark
  ** l and set +d l and add l to S.
  */

  do {
    Literal *l;

    /* Execute the Standard Algorithm on S */
    inferStandardAlgorithm(&S, &Lambda);

    for (l = Lambda; l; l = l->next) {
      if (l->neg ? l->atom->plus_sigma_neg : l->atom->plus_sigma) {
        if (l->neg)
          l->atom->plus_delta_neg = true;
        else
          l->atom->plus_delta = true;
        l->marked = true;
        initLiteral(l->atom, l->neg, &S);
      }
    }

    inferRevisedAlgorithm(&S, &Lambda);

    /*
    ** For each undetermined literal l, if l is not marked, add -d l to S.
    ** Remove all marks and reset rules to be in the same state as prior to
    ** entering this while() loop.
    */
    
    for (l = Lambda; l; l = l->next) {
      if (!l->marked)
        initLiteral(l->atom, l->neg, &S);
      l->marked = false;
      resetRules();
    }

  } while (S);

  /* Print out timer data and delete timers */
  printf("CPU time elapsed = %g\n", readCpuTimer(ctimer));
  printf("Real time elapsed = %g\n", readRealTimer(rtimer));
  freeCpuTimer(ctimer);
  freeRealTimer(rtimer);
}


/*----------------------------------------------------------------------------
Function: inferStandardAlgorithm
Purpose : standard algorithm for inference engine
Args    : S -- list of literals
          Lambda -- another list of literals
Returns : nothing
Notes   : Lambda is used only in well-founded defeasible logic; when doing
          standard defeasible logic, Lambda should simply be NULL
Source  : "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferStandardAlgorithm(Literal **S, Literal **Lambda) {
  while (*S) {
    /*
    ** Pop a literal l from the set S and create some convenient
    ** alias variables
    */
    Literal *l = *S;
    _Bool *plus_delta = l->neg ? 
      &(l->atom->plus_delta_neg) : &(l->atom->plus_delta);
    _Bool *plus_DELTA = l->neg ? 
      &(l->atom->plus_DELTA_neg) : &(l->atom->plus_DELTA);
    _Bool *minus_delta = l->neg ? 
      &(l->atom->minus_delta_neg) : &(l->atom->minus_delta);
    _Bool *minus_DELTA = l->neg ? 
      &(l->atom->minus_DELTA_neg) : &(l->atom->minus_DELTA);
    Literal **defeater_occ = l->neg ? 
      &(l->atom->defeater_occ_neg) : &(l->atom->defeater_occ);
    Literal **strict_occ = l->neg ? 
      &(l->atom->strict_occ_neg) : &(l->atom->strict_occ);
    Literal **defeasible_occ = l->neg ? 
      &(l->atom->defeasible_occ_neg) : &(l->atom->defeasible_occ);
    
    *S = (*S)->next;
    if (*S)
      (*S)->prev = NULL;
    
#ifdef DL_DEBUG
    fprintf(stderr, "Examining %s%s...\n", l->neg ? "~" : "", l->atom->id);
#endif
    
    /* >>>>>>>>>>>>>>>>>>>>> Case +D: <<<<<<<<<<<<<<<<<<<< */
    if (*plus_DELTA) {
      Rule *r;

#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's +D\n");
#endif
      
      /* Delete all occurrences of l in defeater rule bodies */
      while (deleteFirstLiteral(defeater_occ));
      
      /* Delete all occurrences of l in strict rule bodies */
      while ((r = deleteFirstLiteral(strict_occ))) {
        /*
        ** If a strict rule with head h becomes bodiless, record +D h
        ** and add h to S.  Check inference on h and ~h; if ~h is
        ** consequently updated then add it to S also.
        */
        if (!r->body) {
          Literal *h = initLiteral(r->head, r->head_neg, S);
          if (h->neg)
            h->atom->plus_DELTA_neg = true;
          else
            h->atom->plus_DELTA = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +D %s%s\n", h->neg ? "~" : "", 
                  h->atom->id);
#endif
          checkInferenceStandard(h->atom, h->neg, Lambda);
          if (checkInferenceStandard(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
      
      /* Delete all occurrences of l in defeasible rule bodies */
      while ((r = deleteFirstLiteral(defeasible_occ))) {
        /*
        ** If a defeasible rule with head h becomes bodiless, record
        ** +s h and check inference on h and ~h.  If either is
        ** consequently updated then add it to S.
        */
#ifdef DL_DEBUG
        printf("\tDeleting %s%s from %s...\n", l->neg ? "~" : "", l->atom->id,
               r->id);
#endif
        if (!r->body) {
          if (r->head_neg)
            r->head->plus_sigma_neg = true;
          else
            r->head->plus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +s %s%s\n", 
                  r->head_neg ? "~" : "", r->head->id);
#endif
          if (checkInferenceStandard(r->head, r->head_neg, Lambda)) 
            initLiteral(r->head, r->head_neg, S);
          if (checkInferenceStandard(r->head, !r->head_neg, Lambda)) 
            initLiteral(r->head, !r->head_neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case -D: <<<<<<<<<<<<<<<<<<<< */
    if (*minus_DELTA) {
      Literal *h, *next;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's -D\n");
#endif
      
      /* Delete all rules that have l in the body */
      for (; *strict_occ; *strict_occ = next) {
        next = (*strict_occ)->down;
        h = deleteRule((*strict_occ)->rule);
        /*
        ** Whenever there are no more strict rules for h, record -D h
        ** and add h to S. Check inference on h and ~h.  If ~h is
        ** consequently updated, add it to S as well.
        */
        if (( h->neg && !h->atom->strict_occ_neg) ||
            (!h->neg && !h->atom->strict_occ)) {
          initLiteral(h->atom, h->neg, S);
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting -D %s%s\n", h->neg ? "~" : "", 
                  h->atom->id);
#endif
          if (h->neg)
            h->atom->minus_DELTA_neg = true;
          else
            h->atom->minus_DELTA = true;
          checkInferenceStandard(h->atom, h->neg, Lambda);
          if (checkInferenceStandard(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case +d: <<<<<<<<<<<<<<<<<<<< */
    if (*plus_delta) {
      Rule *r;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's +d\n");
#endif
      
      /* Delete all occurrences of l in defeasible rule bodies */
      while ((r = deleteFirstLiteral(defeasible_occ))) {
        /*
        ** If a defeasible rule with head h becomes bodiless, record
        ** +s h. Check inference on both h and ~h; if either is
        ** consequently updated, add it to S.
        */
        if (!r->body) {
          if (r->head_neg)
            r->head->plus_sigma_neg = true;
          else
            r->head->plus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +s %s%s\n", 
                  r->head_neg ? "~" : "", r->head->id);
#endif
          if (checkInferenceStandard(r->head, r->head_neg, Lambda))
            initLiteral(r->head, r->head_neg, S);
          if (checkInferenceStandard(r->head, !r->head_neg, Lambda)) 
            initLiteral(r->head, !r->head_neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case -d: <<<<<<<<<<<<<<<<<<<< */
    if (*minus_delta) {
      Literal *h, *next;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's -d\n");
#endif
      
      /* Delete all rules that have l in the body */
      for (; *defeasible_occ; *defeasible_occ = next) {
        next = (*defeasible_occ)->down;
        h = deleteRule((*defeasible_occ)->rule);
        /*
        ** Whenever there are no more defeasible rules for h, record
        ** -s h. Check inference on h and ~h; if either is consequently
        ** updated, add it to S.
        */
        if (( h->neg && !h->atom->defeasible_occ_neg) ||
            (!h->neg && !h->atom->defeasible_occ)) {
          if (h->neg)
            h->atom->minus_sigma_neg = true;
          else
            h->atom->minus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting -s %s%s\n", h->neg ? "~" : "",
                  h->atom->id);
#endif
          if (checkInferenceStandard(h->atom, h->neg, Lambda))
            initLiteral(h->atom, h->neg, S);
          if (checkInferenceStandard(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
    }
    
    bfree(l);
  } 
}


/*----------------------------------------------------------------------------
Function: inferRevisedAlgorithm
Purpose : revised algorithm for inference engine
Args    : S -- list of literals
          Lambda -- another list of literals
Returns : nothing
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferRevisedAlgorithm(Literal **S, Literal **Lambda) {

  /*
  **                    *** NOT IMPLEMENTED YET ***
  **
  ** What needs to be done: 
  **   - make changes to rules 'undoable' so that resetRules() can be
  **     called afterwards
  **
  */
  yyerror("This undocumented feature has not yet been implemented!");
  exit(EXIT_FAILURE);

  while (*S) {
    /*
    ** Pop a literal l from the set S and create some convenient
    ** alias variables
    */
    Literal *l = *S;
    _Bool *plus_delta = l->neg ? 
      &(l->atom->plus_delta_neg) : &(l->atom->plus_delta);
    _Bool *plus_DELTA = l->neg ? 
      &(l->atom->plus_DELTA_neg) : &(l->atom->plus_DELTA);
    _Bool *minus_delta = l->neg ? 
      &(l->atom->minus_delta_neg) : &(l->atom->minus_delta);
    _Bool *minus_DELTA = l->neg ? 
      &(l->atom->minus_DELTA_neg) : &(l->atom->minus_DELTA);
    Literal **defeater_occ = l->neg ? 
      &(l->atom->defeater_occ_neg) : &(l->atom->defeater_occ);
    Literal **strict_occ = l->neg ? 
      &(l->atom->strict_occ_neg) : &(l->atom->strict_occ);
    Literal **defeasible_occ = l->neg ? 
      &(l->atom->defeasible_occ_neg) : &(l->atom->defeasible_occ);
    
    *S = (*S)->next;
    if (*S)
      (*S)->prev = NULL;
    
#ifdef DL_DEBUG
    fprintf(stderr, "Examining %s%s...\n", l->neg ? "~" : "", l->atom->id);
#endif
    
    /* >>>>>>>>>>>>>>>>>>>>> Case +D: <<<<<<<<<<<<<<<<<<<< */
    if (*plus_DELTA) {
      Rule *r;

#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's +D\n");
#endif
      
      /* Delete all occurrences of l in defeater rule bodies */
      while (deleteFirstLiteral(defeater_occ));
      
      /* Delete all occurrences of l in strict rule bodies */
      while ((r = deleteFirstLiteral(strict_occ))) {
        /*
        ** If a strict rule with head h becomes bodiless, record +D h
        ** and add h to S.  Check inference on h and ~h; if ~h is
        ** consequently updated then add it to S also.
        */
        if (!r->body) {
          Literal *h = initLiteral(r->head, r->head_neg, S);
          if (h->neg)
            h->atom->plus_DELTA_neg = true;
          else
            h->atom->plus_DELTA = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +D %s%s\n", h->neg ? "~" : "", 
                  h->atom->id);
#endif
          checkInferenceRevised(h->atom, h->neg, Lambda);
          if (checkInferenceRevised(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
      
      /* Delete all occurrences of l in defeasible rule bodies */
      while ((r = deleteFirstLiteral(defeasible_occ))) {
        /*
        ** If a defeasible rule with head h becomes bodiless, record
        ** +s h and check inference on h and ~h.  If either is
        ** consequently updated then add it to S.
        */
#ifdef DL_DEBUG
        printf("\tDeleting %s%s from %s...\n", l->neg ? "~" : "", l->atom->id,
               r->id);
#endif
        if (!r->body) {
          if (r->head_neg)
            r->head->plus_sigma_neg = true;
          else
            r->head->plus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +s %s%s\n", 
                  r->head_neg ? "~" : "", r->head->id);
#endif
          if (checkInferenceRevised(r->head, r->head_neg, Lambda)) 
            initLiteral(r->head, r->head_neg, S);
          if (checkInferenceRevised(r->head, !r->head_neg, Lambda)) 
            initLiteral(r->head, !r->head_neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case -D: <<<<<<<<<<<<<<<<<<<< */
    if (*minus_DELTA) {
      Literal *h, *next;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's -D\n");
#endif
      
      /* Delete all rules that have l in the body */
      for (; *strict_occ; *strict_occ = next) {
        next = (*strict_occ)->down;
        h = deleteRule((*strict_occ)->rule);
        /*
        ** Whenever there are no more strict rules for h, record -D h
        ** and add h to S. Check inference on h and ~h.  If ~h is
        ** consequently updated, add it to S as well.
        */
        if (( h->neg && !h->atom->strict_occ_neg) ||
            (!h->neg && !h->atom->strict_occ)) {
          initLiteral(h->atom, h->neg, S);
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting -D %s%s\n", h->neg ? "~" : "", 
                  h->atom->id);
#endif
          if (h->neg)
            h->atom->minus_DELTA_neg = true;
          else
            h->atom->minus_DELTA = true;
          checkInferenceRevised(h->atom, h->neg, Lambda);
          if (checkInferenceRevised(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case +d: <<<<<<<<<<<<<<<<<<<< */
    if (*plus_delta) {
      Rule *r;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's +d\n");
#endif
      
      /* Delete all occurrences of l in defeasible rule bodies */
      while ((r = deleteFirstLiteral(defeasible_occ))) {
        /*
        ** If a defeasible rule with head h becomes bodiless, record
        ** +s h. Check inference on both h and ~h; if either is
        ** consequently updated, add it to S.
        */
        if (!r->body) {
          if (r->head_neg)
            r->head->plus_sigma_neg = true;
          else
            r->head->plus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting +s %s%s\n", 
                  r->head_neg ? "~" : "", r->head->id);
#endif
          if (checkInferenceRevised(r->head, r->head_neg, Lambda))
            initLiteral(r->head, r->head_neg, S);
          if (checkInferenceRevised(r->head, !r->head_neg, Lambda)) 
            initLiteral(r->head, !r->head_neg, S);
        }
      }
    }
    
    /* >>>>>>>>>>>>>>>>>>>>> Case -d: <<<<<<<<<<<<<<<<<<<< */
    if (*minus_delta) {
      Literal *h, *next;
#ifdef DL_DEBUG
      fprintf(stderr, "\tIt's -d\n");
#endif
      
      /* Delete all rules that have l in the body */
      for (; *defeasible_occ; *defeasible_occ = next) {
        next = (*defeasible_occ)->down;
        h = deleteRule((*defeasible_occ)->rule);
        /*
        ** Whenever there are no more defeasible rules for h, record
        ** -s h. Check inference on h and ~h; if either is consequently
        ** updated, add it to S.
        */
        if (( h->neg && !h->atom->defeasible_occ_neg) ||
            (!h->neg && !h->atom->defeasible_occ)) {
          if (h->neg)
            h->atom->minus_sigma_neg = true;
          else
            h->atom->minus_sigma = true;
#ifdef DL_DEBUG
          fprintf(stderr, "\t\tsetting -s %s%s\n", h->neg ? "~" : "",
                  h->atom->id);
#endif
          if (checkInferenceRevised(h->atom, h->neg, Lambda))
            initLiteral(h->atom, h->neg, S);
          if (checkInferenceRevised(h->atom, !h->neg, Lambda))
            initLiteral(h->atom, !h->neg, S);
        }
      }
    }
    
    bfree(l);
  } 
}


/*----------------------------------------------------------------------------
Function: inferInitializeL
Purpose : produces a list of literals which appear as the head of a strict
          rule
Args    : pointer to a list of literals
Returns : nothing
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferInitializeL(Literal **L) {
  hashIterator i;
  Rule *r;

  /* Find all strict rules and add their heads to L */
  hashInitIterator(&i, &ruleTable);
  while ((r = hashGetDatum(&i)))
    if (r->arrow_type == SARROW)
      initLiteral(r->head, r->head_neg, L);

#ifdef DL_DEBUG
  /* Print out what's initally in L */
  fprintf(stderr, "L = ");
  fprintLiteralList(stderr, *L, offsetof(Literal, next));
  fputc('\n', stderr);
#endif
}


/*----------------------------------------------------------------------------
Function: inferInitializeLambda
Purpose : produces a list of all literals in use in the program
Args    : pointer to a list of literals
Returns : nothing
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferInitializeLambda(Literal **Lambda) {
  hashIterator i;
  Atom *a;

  /* Find all atoms and add negated and non-negated instances to Lambda */
  hashInitIterator(&i, &atomTable);
  while ((a = hashGetDatum(&i))) {
    initLiteral(a, true, Lambda);
    initLiteral(a, false, Lambda);
  } 

#if 0
  /**************************************************************************
  ** This version assumes that Lambda is to be filled only with those
  ** literals which occur in rules
  **************************************************************************/
  hashIterator i;
  Rule *r;
  Literal *body;

  /* Find all rules and add their heads and bodies Lambda */
  hashInitIterator(&i, &ruleTable);
  while ((r = hashGetDatum(&i))) {
    initLiteral(r->head, r->head_neg, Lambda);
    for (body = r->body; body; body = body->next)
      initLiteral(body->atom, body->neg, Lambda);
  }
#endif

#ifdef DL_DEBUG
  /* Print out what's initally in Lambda */
  fprintf(stderr, "Lambda = ");
  fprintLiteralList(stderr, *Lambda, offsetof(Literal, next));
  fputc('\n', stderr);
#endif
}


/*----------------------------------------------------------------------------
Function: inferInitializeS
Purpose : initializes a list of literals for use with infer()
Args    : pointer to the list of literals
Returns : nothing
Source  : "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
void inferInitializeS(Literal **S) {
  hashIterator i;
  Atom *a;

  /*
  ** For every atom in the atom table, check to see if it should be inserted
  ** into S:
  */
  hashInitIterator(&i, &atomTable);
  while ((a = hashGetDatum(&i))) {
    /* We can ignore the boolean constants since they'll be added later */
    if (!strcmp(a->id, "true") || !strcmp(a->id, "false"))
      continue;
    
    if (a->plus_DELTA)                               /* Add a if a is a fact */
      initLiteral(a, false, S);
    else if (!a->unknown) {
      _Bool found_strict = false, found_rule = false;
      RuleList *rl;
      for (rl = a->rule_heads; rl; rl = rl->next)   /* Check for rules for a */
        if (rl->rule->arrow_type == SARROW)
          found_strict = true;
        else
          found_rule = true;
      if (!found_strict) {                      /* Set -D if no strict rules */
        a->minus_DELTA = true;
        initLiteral(a, false, S);
      }
      if (!found_rule && !found_strict) {              /* Set -d if no rules */
        a->minus_sigma = true;
        initLiteral(a, false, S);
      }
    }
    
    if (a->plus_DELTA_neg)                         /* Add ~a if ~a is a fact */
      initLiteral(a, true, S);
    else if (!a->unknown_neg) {
      _Bool found_strict = false, found_rule = false;
      RuleList *rl;
      for (rl = a->rule_heads_neg; rl; rl = rl->next)     /* check for rules */
        if (rl->rule->arrow_type == SARROW)
          found_strict = true;
        else
          found_rule = true;
      if (!found_strict) {                      /* Set -D if no strict rules */
        a->minus_DELTA_neg = true;
        initLiteral(a, true, S);
      }
      if (!found_rule && !found_strict) {            /* Set -d~a if no rules */
        a->minus_sigma_neg = true;
        initLiteral(a, true, S);
      }
    }
  }     

  /*
  ** We want to make sure that the boolean constants are at the beginning of
  ** S so that they are processed first in the main loop
  */
  a = hashLookup("true", &atomTable);
  initLiteral(a, false, S);                                 /* Add true to S */
  initLiteral(a, true, S);                              /* Add neg true to S */
  
  a = hashLookup("false", &atomTable);
  initLiteral(a, false, S);                                /* Add false to S */
  initLiteral(a, true, S);                             /* Add neg false to S */
  
#ifdef DL_DEBUG
  /* Print out what's initally in S */
  fprintf(stderr, "S = ");
  fprintLiteralList(stderr, *S, offsetof(Literal, next));
  fputc('\n', stderr);
#endif
}


/*----------------------------------------------------------------------------
Function: checkInferenceStandard
Purpose : Examines given literal and sets +d or -d if appropriate; if +d,
          removes literal from list Lambda, if given
Args    : a, neg -- atom/boolean pair specifying the literal to examine
          Lambda -- pointer to beginning of list from which to delete
Returns : true if +d or -d was set, otherwise false
Notes   : Lambda should be NULL in regular defeasible logic, but should point
          to a list of literals for well-founded defeasible logic.
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
_Bool checkInferenceStandard(Atom *a, _Bool neg, Literal **Lambda) {
  _Bool *plus_DELTA = neg ? &(a->plus_DELTA_neg) : &(a->plus_DELTA);
  _Bool *plus_sigma = neg ? &(a->plus_sigma_neg) : &(a->plus_sigma);
  _Bool *minus_sigma_not = neg ? &(a->minus_sigma) : &(a->minus_sigma_neg);
  _Bool *minus_DELTA_not = neg ? &(a->minus_DELTA) : &(a->minus_DELTA_neg);
  
  if (*plus_DELTA || (*plus_sigma && *minus_DELTA_not && *minus_sigma_not)) {
    _Bool *plus_delta = neg ? &(a->plus_delta_neg) : &(a->plus_delta);
#ifdef DL_DEBUG
    fprintf(stderr, "\t\tsetting +d %s%s\n", neg ? "~" : "", a->id);
#endif
    if (Lambda)
      deleteLiteral(a, neg, Lambda);
    return (*plus_delta = true);
  }
  else {
    _Bool *unknown = neg ? &(a->unknown_neg) : &(a->unknown);
    _Bool *minus_DELTA = neg ? &(a->minus_DELTA_neg) : &(a->minus_DELTA);
    _Bool *minus_sigma = neg ? &(a->minus_sigma_neg) : &(a->minus_sigma);
    _Bool *plus_DELTA_not = neg ? &(a->plus_DELTA) : &(a->plus_DELTA_neg);
    _Bool *plus_sigma_not = neg ? &(a->plus_sigma) : &(a->plus_sigma_neg);
  
    if (*minus_DELTA && (*minus_sigma || *plus_DELTA_not || *plus_sigma_not) &&
        !*unknown) {
      _Bool *minus_delta = neg ? &(a->minus_delta_neg) : &(a->minus_delta);
#ifdef DL_DEBUG
      fprintf(stderr, "\t\tsetting -d %s%s\n", neg ? "~" : "", a->id);
#endif
      return (*minus_delta = true);
    }
  }

  return false;
}


/*----------------------------------------------------------------------------
Function: checkInferenceRevised
Purpose : Examines given literal and sets +d if appropriate
Args    : a, neg -- atom/boolean pair specifying the literal to examine
Returns : true if +d was set, otherwise false
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57; and
          "Efficient Defeasible Reasoning Systems" by G. Antoniou,
          D. Billington, M.J. Maher, A. Rock
----------------------------------------------------------------------------*/
_Bool checkInferenceRevised(Atom *a, _Bool neg, Literal **Lambda) {
  _Bool *plus_DELTA = neg ? &(a->plus_DELTA_neg) : &(a->plus_DELTA);
  _Bool *plus_sigma = neg ? &(a->plus_sigma_neg) : &(a->plus_sigma);
  _Bool *minus_DELTA_not = neg ? &(a->minus_DELTA) : &(a->minus_DELTA_neg);
  
  if (*plus_DELTA || (*plus_sigma && *minus_DELTA_not)) {
    _Bool *plus_delta = neg ? &(a->plus_delta_neg) : &(a->plus_delta);
#ifdef DL_DEBUG
    fprintf(stderr, "\t\tsetting +d %s%s\n", neg ? "~" : "", a->id);
#endif

    /* Mark all occurrences of this literal in Lambda */
    if (*Lambda)
      for (; *Lambda; *Lambda = (*Lambda)->next)
        if ((*Lambda)->atom == a && (*Lambda)->neg == neg)
          (*Lambda)->marked = true;

    return (*plus_delta = true);
  }

  return false;
}


/*----------------------------------------------------------------------------
Function: resetRules
Purpose : resets rules to the state they were in before executing 
          inferRevisedAlgorithm() for the first time
Args    : ?
Returns : ?
Source  : E-mail from M.J. Maher to T. Miller, 10/12/99 19:25:57
----------------------------------------------------------------------------*/
void resetRules(void) {

  /* *** NOT IMPLEMENTED YET *** */

  yyerror("This undocumented feature has not yet been implemented!");
  exit(EXIT_FAILURE);

}


/*----------------------------------------------------------------------------
Function: deleteRule
Purpose : deletes the specified rule
Args    : r - pointer to rule to delete
Returns : pointer to literal specifying head of the deleted rule (note that
          this is static local variable -- the calling function needn't
          (indeed shouldn't) free the returned pointer)
----------------------------------------------------------------------------*/
Literal *deleteRule(Rule *r) {
  static Literal l;
  RuleList **prl = r->head_neg ? 
    &(r->head->rule_heads_neg) : &(r->head->rule_heads);
  RuleList *rl;
  
  /* Remove r from its head's rule_heads list */
  for (rl = *prl; rl && rl->rule != r; rl = rl->next);
  if (rl) {
    if (rl->prev)
      rl->prev->next = rl->next;
    if (rl->next)
      rl->next->prev = rl->prev;
    if (rl == *prl)
      (*prl) = rl->next;
  }
  
  /* Remove body */
  while (deleteFirstLiteral(&(r->body)));
  
  /* Fill l with data on the rule's head */
  l.atom = r->head;
  l.neg = r->head_neg;
  
  /* Remove r from ruleTable and free its memory */
  rl = hashDelete(r->id, &ruleTable);
  bfree(r->id);
  bfree(rl);
  
  return &l;
}


/*----------------------------------------------------------------------------
Function: deleteLiteral
Purpose : deletes all occurrences of the specified literal from the specified
          list L
Args    : a, neg -- atom/boolean pair specifying the literal to examine
          L -- pointer to beginning of list from which to delete
Returns : nothing
----------------------------------------------------------------------------*/
void deleteLiteral(Atom *a, _Bool neg, Literal **L) {
  Literal *l;

  while (*L && (*L)->atom == a && (*L)->neg == neg)
    deleteFirstLiteral(L);

  for (l = *L; l; l = l->next)
    while (l && l->atom == a && l->neg == neg)
      deleteFirstLiteral(&l);
}


/*----------------------------------------------------------------------------
Function: deleteFirstLiteral
Purpose : deletes the first literal in the list pointed to by occ
Args    : l -- pointer to beginning of list of literals
Returns : pointer to the rule of the deleted literal, or NULL if there is no
          rule or if the list pointed to by occ is empty
----------------------------------------------------------------------------*/
Rule *deleteFirstLiteral(Literal **L) {
  Literal *l = *L;
  Rule *r;

  /* List is already empty */
  if (!l)
    return NULL;

  if ((r = l->rule)) {
    /*
    ** If literal is part of a rule body, and is the first in its atom's list
    ** of rule occurrences, remove the literal from its atom's list of rule
    ** occurrences
    */
    if (l->atom->defeasible_occ_neg == l)
      l->atom->defeasible_occ_neg = l->down;
    else if (l->atom->defeasible_occ == l)
      l->atom->defeasible_occ = l->down;
    else if (l->atom->strict_occ_neg == l)
      l->atom->strict_occ_neg = l->down;
    else if (l->atom->strict_occ == l)
      l->atom->strict_occ = l->down;
    else if (l->atom->defeater_occ_neg == l)
      l->atom->defeater_occ_neg = l->down;
    else if (l->atom->defeater_occ == l)
      l->atom->defeater_occ = l->down;

    /*
    ** If literal is the first in a rule body, modify the rule's body pointer
    ** so that it points to the next literal in the body
    */
    if (r->body == l)
      r->body = l->next;
  }

  /* Modify the list L so that it now points to the next literal */
  *L = l->next;

  /* Tie together the ends of the two linked lists the literal is part of */
  if (l->prev)
    l->prev->next = l->next;
  if (l->next)
    l->next->prev = l->prev;
  if (l->up)
    l->up->down = l->down;
  if (l->down)
    l->down->up = l->up;

  bfree(l);
  return r;
}


/*---------------------------------------------------------------------------
Function: yyerror
Purpose : display error messages returned by the parser or lexer
Args    : errmsg -- message to be displayed
Returns : nothing
----------------------------------------------------------------------------*/
void yyerror(const char *errmsg) {
  fprintf(stderr, "%s:%d at `%s': %s\n", yy_current_file, yylineno, 
          yytext, errmsg);
}


/*----------------------------------------------------------------------------
Function: setRuleName
Purpose : generates a new rule name of the form "/rN", where N is a hex number
Args    : r -- pointer to rule whose name you want set
Returns : nothing
Notes   : 1. As this function uses the rule's memory address in its name, it
             is guaranteed not to produce a name already used for a rule.
          2. Provided the format string has only the one conversion specifier, 
             id[] should have dimension (log_16(2^(CHAR_BIT*sizeof(Rule *))) + 
             strlen(format)) to avoid overflowing its buffer. Assuming 8
             bits per byte, the above expression collapses to
             (2*(sizeof Rule *) + strlen(format)).
          3. This function does not check that r->id is already set, so make
             sure any existing id is freed before calling.
----------------------------------------------------------------------------*/
void setRuleName(Rule *r) {
  static const char *format="/r%#p";
  r->id = balloc(2 * sizeof(Rule *) + strlen(format));
  sprintf(r->id, format, r);
}


/*-----------------------------------------------------------------------------
Function: fprintRule
Purpose : Prints out information on the given rule.  If no rule is given, 
          prints out information for all rules.
Args    : r -- pointer to rule to print, or NULL for all rules
          fp -- file pointer for output
Returns : nothing
-----------------------------------------------------------------------------*/
void fprintRule(FILE *fp, const Rule *r) {
  Rule const **ruleList, **ptr;

  /* Build an array of pointers to rules to print */
  if (r) {
    ruleList = balloc(2 * sizeof(Rule *));
    ruleList[0] = r;
    ruleList[1] = NULL;
  }
  else {
    hashIterator i;
    size_t pop = hashPopulation(&ruleTable);
    ruleList = balloc((pop + 1) * sizeof *ruleList);
    hashInitIterator(&i, &ruleTable);
    for (ptr = ruleList; (*ptr = hashGetDatum(&i)); ptr++);
    qsort(ruleList, pop, sizeof(Rule *), cmpRuleOrder);
  }

  /* Actually print the rules */
  for (ptr = ruleList; *ptr; ptr++) {
    if ((*ptr)->head) {
      fprintf(fp, "%s: %s%s ", (*ptr)->id, (*ptr)->head_neg ? "~" : "", 
              (*ptr)->head->id);
      switch ((*ptr)->arrow_type) {
      case SARROW:
        fprintf(fp, "<- ");
        break;
      case DARROW:
        fprintf(fp, "<= ");
        break;
      case DEFARROW:
        fprintf(fp, "<~ ");
        break;
      }
      fprintLiteralList(fp, (*ptr)->body, offsetof(Literal, next));
    }
    else
      fprintf(fp, "%s: undefined", (*ptr)->id);
    if (*(ptr + 1))
      fputc('\n', fp);
  }

  bfree(ruleList);
}


/*-----------------------------------------------------------------------------
Function: cmpRuleOrder
Purpose : Compares the `ordinal' field of two rules
Args    : r1 -- pointer to first rule
          r2 -- pointer to second rule
Returns : -1 if r1->ordinal < r2->ordinal
           0 if r1->ordinal = r2->ordinal
           1 if r1->ordinal > r2->ordinal
-----------------------------------------------------------------------------*/
int cmpRuleOrder(const void *r1, const void *r2) {
  size_t o1 = ((*(const Rule **)r1))->ordinal, 
         o2 = ((*(const Rule **)r2))->ordinal;

  return (o1 < o2) ? -1 : (o1 > o2);
}


/*-----------------------------------------------------------------------------
Function: fprintAtom
Purpose : Prints out information on the given atom.
Args    : a -- pointer to atom to print
          fp -- file pointer for output
Returns : nothing
-----------------------------------------------------------------------------*/
void fprintAtom(FILE *fp, const Atom *a) {
  RuleList *r;
#ifdef DL_DEBUG
  Literal *l;
#endif
  fprintf(fp,   "+d %s=%c", a->id, a->plus_delta?'T':'F');
  fprintf(fp, "\t-d %s=%c", a->id, a->minus_delta?'T':'F');
  fprintf(fp, "\t+D %s=%c", a->id, a->plus_DELTA?'T':'F');
  fprintf(fp, "\t-D %s=%c", a->id, a->minus_DELTA?'T':'F');
#ifdef DL_DEBUG
  fprintf(fp, "\t+s %s=%c", a->id, a->plus_sigma?'T':'F');
  fprintf(fp, "\t-s %s=%c", a->id, a->minus_sigma?'T':'F');
#endif
  fprintf(fp, "\n+d~%s=%c", a->id, a->plus_delta_neg ? 'T':'F');
  fprintf(fp, "\t-d~%s=%c", a->id, a->minus_delta_neg ? 'T':'F');
  fprintf(fp, "\t+D~%s=%c", a->id, a->plus_DELTA_neg ? 'T':'F');
  fprintf(fp, "\t-D~%s=%c", a->id, a->minus_DELTA_neg ? 'T':'F');
#ifdef DL_DEBUG
  fprintf(fp, "\t+s~%s=%c", a->id, a->plus_sigma_neg ? 'T':'F');
  fprintf(fp, "\t-s~%s=%c", a->id, a->minus_sigma_neg ? 'T':'F');
  fprintf(fp, "\n%s strict_occ: ", a->id);
  for (l = a->strict_occ; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
  fprintf(fp, "\n~%s strict_occ: ", a->id);
  for (l = a->strict_occ_neg; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
  fprintf(fp, "\n %s defeasible_occ: ", a->id);
  for (l = a->defeasible_occ; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
  fprintf(fp, "\n~%s defeasible_occ: ", a->id);
  for (l = a->defeasible_occ_neg; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
  fprintf(fp, "\n %s defeater_occ: ", a->id);
  for (l = a->defeater_occ; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
  fprintf(fp, "\n~%s defeater_occ: ", a->id);
  for (l = a->defeater_occ_neg; l; l = l->down)
    fprintf(fp, "%s%s", l->rule->id, l->down ? "," : "");
#endif
  fprintf(fp, "\nrules for %s: ", a->id);
  for (r = a->rule_heads; r; r = r->next)
    fprintf(fp, "%s%s", r->rule->id, r->next ? "," : "");
  fprintf(fp, "\nrules for not %s: ", a->id);
  for (r = a->rule_heads_neg; r; r = r->next)
    fprintf(fp, "%s%s", r->rule->id, r->next ? "," : "");
  fputc('\n', fp);
}


/*-----------------------------------------------------------------------------
Function: fprintLiteralList
Purpose : Prints out a list of literals
Args    : l -- pointer to the first literal to print
          next -- offset in the Literal struct of the pointer to the next node
            (by variously setting this to next, prev, up, and down, you can
            print out the list in any of its four 'directions')
          fp -- file pointer for output
Returns : nothing
-----------------------------------------------------------------------------*/
void fprintLiteralList(FILE *fp, const Literal *l, size_t next) {
  for (; l; l = *(Literal **)((char *)l + next))
    fprintf(fp, "%s%s%s", l->neg ? "~" : "", l->atom->id, 
            *(Literal **)((char *)l + next) ? "," : "");
}


/*-----------------------------------------------------------------------------
Function: dupRule
Purpose : creates a duplicate of the given rule
Args    : r -- pointer to the rule to duplicate
          idfmt -- format string of new rule's id, or NULL to use the standard
          method of naming new rules
Returns : pointer to the new rule
Note    : 1. idfmt must be a printf-like format string containing exactly one
          conversion, %s, which will be replaced with the old rule's id
          2. this function does not insert the duplicate into the rule table, 
          allowing its attributes to be changed first by the calling function
-----------------------------------------------------------------------------*/
Rule *dupRule(const Rule *r, const char *idfmt) {
  Rule *newRule = balloc(sizeof *newRule);
  Literal *body, *first, *prev;

  /* Set the new rule id */
  if (idfmt) {
    newRule->id = balloc(strlen(idfmt) + strlen(r->id) - 1);
    sprintf(newRule->id, idfmt, r->id);
  }
  else
    setRuleName(newRule);
  
  /* Duplicate the body */
  for (body = r->body; body; body = body->next) {
    Literal *newLit = balloc(sizeof *newLit);
    newLit->atom = body->atom;
    newLit->neg = body->neg;
    if (body == r->body) {
      first = newLit;
      newLit->prev = NULL;
    }
    else {
      prev->next = newLit;
      newLit->prev = prev;
    }
    if (body->next)
      prev = newLit;
    else
      newLit->next = NULL;
  }
  newRule->body = first;
  
  /* Set the remaining rule attributes */
  newRule->head = r->head;
  newRule->head_neg = r->head_neg;
  newRule->arrow_type = r->arrow_type;

  newRule->ordinal=0;
  
  return newRule;
}


/*-----------------------------------------------------------------------------
Function: addRule
Purpose : adds rule to rule table (if it's not already there) and to the
          program's data structure as a whole
Args    : r -- pointer to the rule to add
          func -- dummy argument for compatibility with elimSupRel/elimDef
            chaining; if calling this function directly, pass NULL here
Returns : pointer to the added rule
-----------------------------------------------------------------------------*/
Rule *addRule(Rule *r, Rule *(*func)(Rule *, Rule *(*)())) {
  Literal *body;
  Rule *temp;
  RuleList **headList, *ruleNode;

  /* Add rule to rule table */
  if ((temp = hashLookup(r->id, &ruleTable))) {
    static const char msg[] = "warning: ignoring redefinition of `%s'";
    char *err = balloc(strlen(r->id) + strlen(msg));
    sprintf(err, msg, r->id);
    yyerror(err);
    bfree(err);
    return temp;
  }
  else if (!hashInsert(r->id, r, &ruleTable)) {
      yyerror("cannot insert into rule table");
      exit(EXIT_FAILURE);
  }

  /*
  ** All literals in rule's body are part of a linked list of the same literal
  ** among the same type of rule
  */
  for (body = r->body ; body; body = body->next) {
    Literal **occList;
    
    /* Determine which linked list of body occurrences body belongs to */
    switch (r->arrow_type) {
    case SARROW:
      occList = body->neg ? &(body->atom->strict_occ_neg):
        &(body->atom->strict_occ);
      break;
    case DARROW:
      occList = body->neg ? &(body->atom->defeasible_occ_neg):
        &(body->atom->defeasible_occ);
      break;
    case DEFARROW:
      occList = body->neg ? &(body->atom->defeater_occ_neg):
        &(body->atom->defeater_occ);
      break;
    }
    
    /* Add body to the end of occList */
    /* *** is this necessary?  can it be added to the beginning? *** */
    body->down = NULL;
    for (; *occList && (*occList)->down; occList = &((*occList)->down));
    if (!*occList) {
      body->up = NULL;
      *occList = body;
    }
    else {
      body->up = *occList;
      (*occList)->down = body;
    }
  }
  
  /* All atoms keep track of rules for which they serve as heads */
  ruleNode = balloc(sizeof *ruleNode);
  ruleNode->rule = r;
  ruleNode->next = NULL;
  for (headList = r->head_neg ? 
         &(r->head->rule_heads_neg) : &(r->head->rule_heads);
       *headList && (*headList)->next;
       headList = &((*headList)->next));
  if (!*headList) {
    ruleNode->prev = NULL;
    *headList = ruleNode;
  }
  else {
    ruleNode->prev = *headList;
    (*headList)->next = ruleNode;
  }

  return r;
}


/*-----------------------------------------------------------------------------
Function: initRule
Purpose : initializes a rule with given parameters
Args    : r -- pointer to the rule to initialize, or NULL to create a new rule
          id -- pointer to the rule's id, or NULL to automatically generate one
          head -- pointer to head atom
          head_neg -- is head negated
          arrow -- must be one of SARROW, DARROW, or DEFARROW
          body -- pointer to body literal list
Returns : pointer to the initialized rule
Notes   : unlike initAtom(), id is not strdup'ed. id should point to something
          dynamically-allocated, and shouldn't be freed by the calling function
-----------------------------------------------------------------------------*/
Rule *initRule(Rule *r, const char *id, const Atom *head, _Bool head_neg, 
               int arrow, Literal *body) {

  /*
  ** The following variable keeps track of 
  ** the order in which rules are initialized
  */
  static uintmax_t ordinal = 0;
  
  if (!r)
    r = balloc(sizeof *r);
  if (id)
    r->id = (char *)id;
  else
    setRuleName(r);
  r->head = (Atom *)head;
  r->head_neg = head_neg;
  r->arrow_type = arrow;
  r->body = body;
  r->ordinal = ordinal++;
  
  if (ordinal == 0)
    /*
    ** The following is only a warning, since the program will still
    ** work except that rules will not be properly sorted when listed
    */
    yyerror("warning: rule count overflow");

  for (; body; body = body->next)
    body->rule = r;         /* Link back to rule */
  
  return r;
}


/*-----------------------------------------------------------------------------
Function: initLiteral
Purpose : creates and initializes a literal with given parameters, and adds
          to the beginning of the literal list pointed to by list
Args    : a -- the atom to which the literal refers
          neg -- is the atom negated?
          list -- pointer to literal list to append the literal to, 
          or NULL to simply initialize the literal without adding it to a list
Returns : pointer to the the new literal (i.e. the beginning of the updated
          list, if any)
-----------------------------------------------------------------------------*/
Literal *initLiteral(const Atom *a, _Bool neg, Literal **list) {
  Literal *l = balloc(sizeof *l);
  l->atom = (Atom *)a;
  l->neg = neg;
  l->rule = NULL;
  l->down = NULL;
  l->up = NULL;
  l->prev = NULL;
  if (list) {
    l->next = *list;
    if (*list)
      (*list)->prev = l;
    *list = l;
  }
  else
    l->next = NULL;
  return l;
}


/*-----------------------------------------------------------------------------
Function: initAtom
Purpose : returns the named atom, or creates and initializes a new atom with
          the given id
Args    : id -- string identifying the atom
Returns : pointer to new atom (or pointer to old atom if it already exists in
          the atom table)
Notes   : id is strdup'ed, so there's no need pass initAtom a dynamically-
          allocated string
-----------------------------------------------------------------------------*/
Atom *initAtom(const char *id) {
  Atom *a = hashLookup(id, &atomTable);
  if (!a) {
    a = balloc(sizeof *a);
    a->id = dl_strdup(id);
    a->plus_delta = false;
    a->minus_delta = false;
    a->plus_DELTA = false;
    a->minus_DELTA = false;
    a->plus_sigma = false;
    a->minus_sigma = false;
    a->plus_delta_neg = false;
    a->minus_delta_neg = false;
    a->plus_DELTA_neg = false;
    a->minus_DELTA_neg = false;
    a->plus_sigma_neg = false;
    a->minus_sigma_neg = false;
    a->unknown = false;
    a->unknown_neg = false;
    a->strict_occ = NULL;
    a->defeasible_occ = NULL;
    a->defeater_occ = NULL;
    a->strict_occ_neg = NULL;
    a->defeasible_occ_neg = NULL;   
    a->defeater_occ_neg = NULL;
    a->rule_heads = NULL;
    a->rule_heads_neg = NULL;
    if (!hashInsert(a->id, a, &atomTable)) {
      yyerror("cannot insert into atom table");
      exit(EXIT_FAILURE);
    }
  }
  return a;
}


/*-----------------------------------------------------------------------------
Function: elimSupRel
Purpose : eliminates a superiority relation
Args    : ids of two rules, r1 and r2, denoting the superiority relationship
          r1 > r2
          func -- function to call on the products of decomposition
Returns : NULL
Source  : Secs. 5.2 ("Simulating the Superiority Relation") and 5.3 (Simulating
          the Defeaters) of an unknown paper provided by M.J. Maher
-----------------------------------------------------------------------------*/
Rule *elimSupRel(const char *r1, const char *r2, 
                 Rule *(*func)(Rule *, Rule *(*)())) {
  Atom *inf_plus_r1, *inf_minus_r1, *inf_plus_r2, *inf_minus_r2;
  char *buf = balloc(strlen(r1) + strlen(r2) + 11);
  
  /* Find (or create) the inf +  and inf- atoms for r1  */
  sprintf(buf, "%s/inf_plus", r1);
  inf_plus_r1 = initAtom(buf);
  sprintf(buf, "%s/inf_minus", r1);
  inf_minus_r1 = initAtom(buf);

  /* Find (or create) the inf +  and inf- atoms for r2  */
  sprintf(buf, "%s/inf_plus", r2);
  inf_plus_r2 = initAtom(buf);
  sprintf(buf, "%s/inf_minus", r2);
  inf_minus_r2 = initAtom(buf);
  
  /* Create inf + (r2) <= ~inf + (r1) */
  sprintf(buf, "%s/gt_%s_0", r1, r2);
  if (!hashLookup(buf, &ruleTable))
    func(initRule(NULL, dl_strdup(buf), inf_plus_r2, false, DARROW, 
               initLiteral(inf_plus_r1, true, NULL)), addRule);
  
  /* Create inf-(r2) <= ~inf-(r1) */
  sprintf(buf, "%s/gt_%s_1", r1, r2);
  if (!hashLookup(buf, &ruleTable)) {
    func(initRule(NULL, dl_strdup(buf), inf_minus_r2, false, DARROW, 
             initLiteral(inf_minus_r1, true, NULL)), addRule);
  }

  bfree(buf);
  return NULL;
}


/*-----------------------------------------------------------------------------
Function: elimSupRel_r
Purpose : decomposes a rule to facilitate superiority relation elimination
Args    : r1 -- the rule to decompose
          func -- function to call on the products of decomposition
Returns : NULL
Notes   : deletes the rule r1
-----------------------------------------------------------------------------*/
Rule *elimSupRel_r(Rule *r1, Rule *(*func)(Rule *, Rule *(*)())) {
  Atom *inf_plus_r1, *inf_minus_r1, *r1_body;
  char *buf = balloc(strlen(r1->id) + 11);
  Rule *r;

  /* Find (or create) the inf +  and inf- atoms for r1  */
  sprintf(buf, "%s/inf_plus", r1->id);
  inf_plus_r1 = initAtom(buf);
  sprintf(buf, "%s/inf_minus", r1->id);
  inf_minus_r1 = initAtom(buf);

  /*
  ** Find (or create) the alias, r1/body, for r1's body.
  ** This avoids unnecessary duplication of r1's body.
  */
  sprintf(buf, "%s/body", r1->id);
  if (!(r1_body = hashLookup(buf, &atomTable))) {
    r1_body = initAtom(buf);
    r1_body->minus_delta_neg = true;
    r1_body->minus_DELTA_neg = true;
    r = dupRule(r1, "%s/bodyrule");
    addRule(initRule(r, r->id, r1_body, false, DARROW, r->body), NULL);
  }

  switch (r1->arrow_type) {

  case DARROW:

    /* Create ~inf + (r1) <= r1_body */
    sprintf(buf, "%s/es0", r1->id);
    func(initRule(NULL, dl_strdup(buf), inf_plus_r1, true, DARROW, 
                  initLiteral(r1_body, false, NULL)), addRule);
    
    /* Create r1.head <= ~inf + (r1) */
    sprintf(buf, "%s/es1", r1->id);
    func(initRule(NULL, dl_strdup(buf), r1->head, r1->head_neg, DARROW, 
             initLiteral(inf_plus_r1, true, NULL)), addRule);
    
    /* Create ~inf-(r1) <= r1_body */
    sprintf(buf, "%s/es2", r1->id);
    func(initRule(NULL, dl_strdup(buf), inf_minus_r1, true, DARROW, 
             initLiteral(r1_body, false, NULL)), addRule);
    
    /* Create r1.head <= ~inf-(r1) */
    sprintf(buf, "%s/es3", r1->id);
    func(initRule(NULL, dl_strdup(buf), r1->head, r1->head_neg, DARROW, 
             initLiteral(inf_minus_r1, true, NULL)), addRule);
    
    break;

  case DEFARROW:
    
    /* Create ~inf-(r1) <= r1_body */
    sprintf(buf, "%s/es4", r1->id);
    func(initRule(NULL, dl_strdup(buf), inf_minus_r1, true, DARROW, 
             initLiteral(r1_body, false, NULL)), addRule);
    
    /* Create r1.head <~ ~inf-(r1) */
    sprintf(buf, "%s/es5", r1->id);
    func(initRule(NULL, dl_strdup(buf), r1->head, r1->head_neg, DEFARROW, 
             initLiteral(inf_minus_r1, true, NULL)), addRule);
    
  }

  deleteRule(r1);
  bfree(buf);
  return NULL;
}


/*-----------------------------------------------------------------------------
Function: elimDef
Purpose : decomposes a rule to facilitate defeater elimination
Args    : r1 -- the rule to decompose
          func -- function to call on the products of decomposition
Returns : NULL
Notes   : changes the rule r1
-----------------------------------------------------------------------------*/
Rule *elimDef(Rule *r1, Rule *(*func)(Rule *, Rule *(*)())) {
  Atom *r1_body, *head_plus, *head_minus;
  int l1 = strlen(r1->id) + 7, l2 = strlen(r1->head->id) + 12;
  char *buf = balloc(l1>l2 ? l1 : l2);

  /* Find (or create) head+ and head- */
  sprintf(buf, "%s/head_plus", r1->head->id);
  head_plus = initAtom(buf);
  sprintf(buf, "%s/head_minus", r1->head->id);
  head_minus = initAtom(buf);

  switch (r1->arrow_type) {

  case DARROW:

    /*
    ** Find (or create) the alias, r1_body, for r1's body.
    ** This avoids unnecessary duplication of r1's body.
    */
    sprintf(buf, "%s/body", r1->id);
    if (!(r1_body = hashLookup(buf, &atomTable))) {
      Rule *r = dupRule(r1, "%s/bodyrule");
      r1_body = initAtom(buf);
      r1_body->minus_delta_neg = true;
      r1_body->minus_DELTA_neg = true;
      addRule(initRule(r, r->id, r1_body, false, DARROW, r->body), NULL);
    }

    if (r1->head_neg) {

      /* Create r1 + : ~head+ <= r1_body */
      sprintf(buf, "%s/plus", r1->id);
      func(initRule(NULL, dl_strdup(buf), head_plus, true, DARROW, 
                       initLiteral(r1_body, false, NULL)), addRule);

      /* Create r1-: head- <= r1_body */
      sprintf(buf, "%s/minus", r1->id);
      func(initRule(NULL, dl_strdup(buf), head_minus, false, DARROW, 
                       initLiteral(r1_body, false, NULL)), addRule);

      /* Create ~head <= r1_body, head- */
      func(initRule(r1, r1->id, r1->head, true, DARROW, 
                       initLiteral(head_minus, false, &(r1->body))), addRule);

    }
    else {

      /* Create r1 + : head+ <= r1_body */
      sprintf(buf, "%s/plus", r1->id);
      func(initRule(NULL, dl_strdup(buf), head_plus, false, DARROW, 
                       initLiteral(r1_body, false, NULL)), addRule);

      /* Create r1-: ~head- <= r1_body */
      sprintf(buf, "%s/minus", r1->id);
      func(initRule(NULL, dl_strdup(buf), head_minus, true, DARROW, 
                       initLiteral(r1_body, false, NULL)), addRule);

      /* Create head <= r1_body, head+ */
      func(initRule(r1, r1->id, r1->head, false, DARROW, 
                       initLiteral(head_plus, false, &(r1->body))), addRule);

    }   

    break;

  case DEFARROW:

    if (r1->head_neg)
      func(initRule(r1, r1->id, head_plus, true, DARROW, r1->body), addRule);
    else
      func(initRule(r1, r1->id, head_minus, true, DARROW, r1->body), addRule);
    break;
  }
  
  bfree(buf);
  return NULL;
}
