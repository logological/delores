%{
/*----------------------------------------------------------------------------
File    : $Id: parser.y,v 1.3 2003-12-13 14:52:02 psy Exp $
What    : Defeasible logic parser

Copyright (C) 1999, 2000 Michael Maher <mjm@math.luc.edu>
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dl_malloc.h"
#include "dl_stdint.h"
#include "dl_stdbool.h"
#include "dl.h"

#define YYERROR_VERBOSE
#define ERROR_LIMIT 16

#ifdef DL_PARSER_DEBUG
#define DL_PRINT_RULE(r,s) { \
        fprintf(stderr,"\tDL_PARSER: `"); \
        fprintRule(stderr,(r)); \
        fprintf(stderr,"' is a %s\n",(s)); \
 }
#endif

static uintmax_t yy_error_count = 0;

%}

%union {
    Atom *a;
    Rule *r;
    Literal *l;
    char *id;
}

%token UNKNOWN NOT SARROW DARROW DEFARROW LISTING PRINT INFER END FAIL_d FAIL_D
%token INFERWF
%token <a> NAME VARIABLE DL_FALSE DL_TRUE
%token <id> LABEL
%type <a> atom term truefalse
%type <l> lliteral literal litlist simple_literal
%type <r> srule drule defeater rule

%left '>' '<'
%nonassoc NOT UNKNOWN FAIL_D FAIL_d

%%
program:    statement   {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: program recognized\n");
                #endif
            }
    |       program statement   {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: program recognized\n");
                #endif
            }
    ;

statement:  rule    {
                elimSupRel_r($1,elimDef);
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: statement recognized\n");
                #endif
            }
    |       fact    {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: statement recognized\n");
                #endif
            }
    |       suprel  {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: statement recognized\n");
                #endif
            }
    |       query {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: statement recognized\n");
                #endif
            }
    |       END '.' {
                if (!InteractiveMode && yy_error_count)
                  YYABORT;
                YYACCEPT;
            }
    |       error '.' {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: error #%" PRIuMAX "\n",
                            yy_error_count);
                #endif
                if (!InteractiveMode && ++yy_error_count > ERROR_LIMIT)
                  YYABORT;
            }
    ;

query:      LISTING '.' {
                printRule(NULL);
                fputc('\n',stdout);
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: query recognized\n");
                #endif
            }
    |       LISTING '(' LABEL ')' '.' {
                Rule *r=hashLookup($3,&ruleTable);
                if (r) {
                  printRule(r);
                  fputc('\n',stdout);
                }
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: query recognized\n");
                #endif
            }
    |       PRINT '(' atom ')' '.' {
                if ($3)
                    printAtom($3);
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: query recognized\n");
                #endif
            }
    |       INFER '.' {
                infer();
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: query recognized\n");
                #endif
            }
    |       INFERWF '.' {
                inferWellFounded();
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: query recognized\n");
                #endif
            }
    ;
suprel:     LABEL '<' LABEL '.' {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s<%s' is a suprel\n",$1,$3);
                #endif
                if (!strcmp($1,$3))
                  yyerror("warning: same rule appears on both sides "
                          "of superiority relation");
                else
                  elimSupRel($3,$1,elimDef);
                bfree($1);
                bfree($3);
            }
    |       LABEL '>' LABEL '.' {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s>%s' is a suprel\n",$1,$3);
                #endif
                if (!strcmp($1,$3))
                  yyerror("warning: same rule appears on both sides "
                          "of superiority relation");
                else
                  elimSupRel($1,$3,elimDef);
                bfree($1);
                bfree($3);
            }
    ;

atom:       NAME    {
                #ifdef DL_PARSER_DEBUG
                    if ($1)
                      fprintf(stderr,"\tDL_PARSER: `%s' is an atom\n",$1->id);
                #endif
                $$=$1;
            }
    |       NAME '(' termlist ')'   {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    if ($1)
                      fprintf(stderr,"\tDL_PARSER: `%s({list})' is an atom\n",
                              $1->id);
                #endif
                $$=$1;
            }
    ;

truefalse:  DL_TRUE {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s' is a true/false atom\n",
                            $$->id);
                #endif
            }
    |       DL_FALSE    {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s' is a true/false atom\n",
                            $$->id);
                #endif
            }
    ;           

term:       NAME    {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s' is a term\n",$1->id);
                #endif
                $$=$1;
            }
    |       VARIABLE    {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s' is a term\n",$1->id);
                #endif
                $$=$1;
            }
    |       NAME '(' termlist ')'   {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s({list})' is a term\n",
                            $1->id);
                #endif
                $$=$1;
            }
    ;

termlist:   term    {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: termlist recognized\n");
                #endif
            }
    |       termlist ',' term   {
                /* *** NOT IMPLEMENTED YET *** */
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: termlist recognized\n");
                #endif
            }
    ;

lliteral:   atom    {
                $$=initLiteral($1,false,NULL);
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `%s%s' is a lliteral\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    |       NOT atom    {
                $$=initLiteral($2,true,NULL);
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `%s%s' is a lliteral\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    ;

simple_literal: lliteral    {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `%s%s' is a simple_literal\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    |       truefalse {
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s%s' is a simple_literal\n",
                           $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    |       NOT truefalse {
                $$=initLiteral($2,true,NULL);
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s%s' is a simple_literal\n",
                           $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    ;

literal:    simple_literal {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `%s%s' is a literal\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    |       FAIL_D simple_literal {
                /* *** NOT IMPLEMENTED YET *** */
                $$=$2;
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `failD %s%s' is a literal\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    |       FAIL_d simple_literal {
                /* *** NOT IMPLEMENTED YET *** */
                $$=$2;
                #ifdef DL_PARSER_DEBUG
		    if ($$ && $$->atom)
                      fprintf(stderr,"\tDL_PARSER: `faild %s%s' is a literal\n",
                             $$->neg?"not ":"",$$->atom->id);
                #endif
            }
    ;

fact:       lliteral '.'        {
                if ($1->atom) {
                  if ($1->neg) {
                      $1->atom->plus_delta_neg=true;
                      $1->atom->plus_DELTA_neg=true;
                  }
                  else {
                      $1->atom->plus_delta=true;
                      $1->atom->plus_DELTA=true;
                  }
                  #ifdef DL_PARSER_DEBUG
                      fprintf(stderr,"\tDL_PARSER: `%s%s.' is a fact\n",
                             $1->neg?"not ":"",$1->atom->id);
                  #endif
                }
            }
    |       UNKNOWN lliteral '.'    {
                if ($2->atom) {
                  if ($2->neg)
                      $2->atom->unknown_neg=true;
                  else 
                      $2->atom->unknown=true;
                  #ifdef DL_PARSER_DEBUG
                      fprintf(stderr,"\tDL_PARSER: `UNKNOWN %s%s.' is a fact\n",
                             $2->neg?"not ":"",$2->atom->id);
                  #endif
                }
            }
    ;

rule:       srule   {
                Rule *r=dupRule($1,"%s/strict");
                addRule(initRule(r,r->id,r->head,r->head_neg,SARROW,r->body),
                        NULL);
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s...' is a rule\n", $$->id);
                #endif
            }
    |       drule   {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s...' is a rule\n", $$->id);
                #endif
            }
    |       defeater    {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s...' is a rule\n", $$->id);
                #endif
            }
    ;

litlist:    literal {
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    fprintf(stderr,"\tDL_PARSER: `%s' is a litlist\n",
                            $$->atom->id);
                #endif
            }
    |       litlist ',' literal {
                Literal *last;
                for (last=$$;last->next;last=last->next);
                last->next=$3;
                $3->prev=last;
                $$=$1;
                #ifdef DL_PARSER_DEBUG
                    {
                        fprintf(stderr,"\tDL_PARSER: `");
                        fprintLiteralList(stderr,$$,offsetof(Literal,next));
                        fprintf(stderr,"' is a litlist\n");
                    }
                #endif
            }
    ;

srule:      LABEL ':' lliteral SARROW litlist '.'   {
                $$=initRule(NULL,$1,$3->atom,$3->neg,DARROW,$5);
                bfree($3);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"srule");
                #endif
            }
    |       lliteral SARROW litlist '.' {
                $$=initRule(NULL,NULL,$1->atom,$1->neg,DARROW,$3);
                bfree($1);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"srule");
                #endif
            }
    ;

drule:      LABEL ':' lliteral DARROW litlist '.'   {
                $$=initRule(NULL,$1,$3->atom,$3->neg,DARROW,$5);
                bfree($3);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"drule");
                #endif
            }
    |       lliteral DARROW litlist '.' {
                $$=initRule(NULL,NULL,$1->atom,$1->neg,DARROW,$3);
                bfree($1);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"drule");
                #endif
            }
    ;

defeater:   LABEL ':' lliteral DEFARROW litlist '.' {
                $$=initRule(NULL,$1,$3->atom,$3->neg,DEFARROW,$5);
                bfree($3);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"defeater");
                #endif
            }
    |       lliteral DEFARROW litlist '.'   {
                $$=initRule(NULL,NULL,$1->atom,$1->neg,DEFARROW,$3);
                bfree($1);
                #ifdef DL_PARSER_DEBUG
                    DL_PRINT_RULE($$,"defeater");
                #endif
            }
    ;
    

%%
