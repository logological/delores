%{
/*----------------------------------------------------------------------------
File    : $Id: lexer.l,v 1.3 2003-12-12 19:30:00 psy Exp $
What    : Defeasible logic lexer

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

static int include_stack_index = 0;

#if HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <string.h>
#include "dl_strdup.h"
#include <limits.h>
#include "dl.h"
#include "parser.h"
#include "dl_malloc.h"
#include "dl_stdint.h"

#define MAX_INCLUDE_DEPTH 15

struct {
  YY_BUFFER_STATE buffer;
  char *filename;
} include_stack[MAX_INCLUDE_DEPTH];

char *yy_current_file = "stdin";
extern uintmax_t yy_error_count;
%}

%option yylineno
%option nounput
%s L_SUPREL
%s L_PRINT
%x L_COMMENT
%s L_LISTING
%x L_INCLUDE

space           [ \t\n]
name            [a-z][A-Za-z0-9_]*
definablelabel  [A-Za-z][A-Za-z0-9_]*
label           [A-Za-z/][A-Za-z0-9_/]*

%%

{space}+    ;                               /* ignore whitespace */

unknown     return UNKNOWN;

neg|not     return NOT;

:-|\<-      return SARROW;

:=|\<=      return DARROW;

:^|\<~      return DEFARROW;

faild       return FAIL_d;

failD       return FAIL_D;

print   {
  BEGIN L_PRINT;
  return PRINT;
}
    
listing/{space}*\(  {
  BEGIN L_LISTING;
  return LISTING;
}

listing     return LISTING;

infer       return INFER;

inferwf     return INFERWF;

end         {
  /* The following line has no purpose other than to suppress lint warnings */
  (void)yy_flex_realloc;
  return END;
}

true    {                                   /* true */
  yylval.a=hashLookup("true",&atomTable);
  return DL_TRUE;
}

false   {                                   /* false */
  yylval.a=hashLookup("false",&atomTable);
  return DL_FALSE;
}

{definablelabel}/{space}*:[^-=^]    {       /* Rule definition */
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a LABEL\n", yytext);
#endif
  yylval.id=dl_strdup(yytext);
  return LABEL;
}

{label}/{space}*[><][^-=~]  {               /* LHS of superiority relation */
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a LABEL\n", yytext);
#endif
  BEGIN L_SUPREL;
  yylval.id=dl_strdup(yytext);
  return LABEL;
}
    
<L_SUPREL>{label}   {                       /* RHS of superiority relation */
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a LABEL\n", yytext);
#endif
  BEGIN INITIAL;
  yylval.id=dl_strdup(yytext);
  return LABEL;
}
    
<L_LISTING>{label}  {                       /* listing() statement */
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a LABEL\n", yytext);
#endif
  BEGIN INITIAL;
  yylval.id=dl_strdup(yytext);
  return LABEL;
}

<INITIAL>{name} {                           /* Atom */
  Atom *a=initAtom(yytext);
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a NAME (%p)\n", yytext, a);
#endif
  yylval.a=a;
  return NAME;
}

<L_PRINT>{name} {                           /* print() statement */
  yylval.a=hashLookup(yytext, &atomTable);
  if (!yylval.a) {
    char *err=balloc(yyleng+19);
    sprintf(err,"`%s' is not an atom",yytext);
    yyerror(err);
    bfree(err);
  }
  BEGIN INITIAL;
  return NAME;
}
    
{label}|_   {                               /* Variable */
  /* *** NOT IMPLEMENTED YET *** */
#ifdef DL_LEXER_DEBUG
  fprintf(stderr, "\tDL_LEXER: `%s' is a VARIABLE (%p)\n",
          yytext, (void *)NULL);
#endif
  return VARIABLE;
}

<INITIAL>include{space}*\(       {
  BEGIN L_INCLUDE;
}

<L_INCLUDE>.*\){space}*\. {       /* Include file */
  if (include_stack_index>=MAX_INCLUDE_DEPTH) {
    yyerror("too many nested includes");
    exit(EXIT_FAILURE);
  }
  
  /* Remember the current file so we can resume processing it later */
  *strrchr(yytext,')')='\0';
  include_stack[include_stack_index].buffer=YY_CURRENT_BUFFER;
  if (include_stack_index==0)
    include_stack[include_stack_index].filename="stdin";
  include_stack[++include_stack_index].filename=dl_strdup(yytext);
  yy_current_file=include_stack[include_stack_index].filename;

  /* Open the include file */
  if (!(yyin=fopen(yy_current_file,"r"))) {
    char *buf=balloc(strlen(yy_current_file)+28);
    sprintf(buf,"%s: no such file or directory",yy_current_file);
    yyerror(buf);
    bfree(buf);
    exit(EXIT_FAILURE);
  }

#ifdef DL_LEXER_DEBUG
  fprintf(stderr,"\tDL_LEXER: opening %s\n",yy_current_file);
#endif

  /* Begin processing the included file */
  yy_switch_to_buffer(yy_create_buffer(yyin,YY_BUF_SIZE));
  BEGIN INITIAL;
}

<<EOF>> {                                   /* End include file */
  if (include_stack_index==0)
    yyterminate();
  else {
#ifdef DL_LEXER_DEBUG
  fprintf(stderr,"\tDL_LEXER: closing %s\n",yy_current_file);
#endif
    bfree(include_stack[include_stack_index--].filename);
    yy_current_file=include_stack[include_stack_index].filename;
    yy_delete_buffer(YY_CURRENT_BUFFER);
#ifdef DL_LEXER_DEBUG
  fprintf(stderr,"\tDL_LEXER: resuming %s\n",yy_current_file);
#endif
    yy_switch_to_buffer(include_stack[include_stack_index].buffer);
  }
}

%               BEGIN L_COMMENT;            /* Comment marker */
<L_COMMENT>.    ;                           /* Ignore comments */
<L_COMMENT>\n   BEGIN INITIAL;              /* End of comments */

.       return yytext[0];                   /* Default action */

{name}{space}*\?    {                       /* Debugging tool */
  Atom *a;
  Rule *r;
  yytext[yyleng-1]='\0';
  if ((a=hashLookup(yytext,&atomTable)))
    fprintAtom(stderr,a);
  if ((r=hashLookup(yytext,&ruleTable)))
    fprintRule(stderr,r);
}

%%
