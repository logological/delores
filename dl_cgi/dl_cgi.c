/*----------------------------------------------------------------------------
Filename: dl_cgi.c
What    : CGI interface for DELORES
Author  : Tristan Miller
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "cgic.h"

#define DL_HTML_FILE "dl_cgi.html"
#define DL_COMMAND "/home/xerxes/www/cgi-bin/dl.cgi -q"
#define BUFSIZE 1024
#define ACTION_LEN 16

/*----------------------------------------------------------------------------
Local function prototypes
----------------------------------------------------------------------------*/
static void displayForm(void);
static void submitTheory(void);
static void printHeader(void);
static void printError(char *msg);
static void printFooter(void);

/*----------------------------------------------------------------------------
Global variables
----------------------------------------------------------------------------*/
/*
** Programs using CGIC do not have the luxury of using the exit() function,
** so instead functions which "abort" due to errors simply set the error
** status in the following variable, and immediately return to cgiMain().
*/
static int ErrorCode = 0;


/*----------------------------------------------------------------------------
Function: cgiMain
Purpose : Retrieves the value of the submit button from the CGI form and
          executes the appropriate function
Args    : "submit" -- field from CGI indicating what the interface is to do
Returns : nothing
----------------------------------------------------------------------------*/
int cgiMain(void) {
  char action[ACTION_LEN];

  cgiFormStringNoNewlines("submit", action, ACTION_LEN);

  if (!strcmp(action, "Submit theory"))
	submitTheory();
  /*
  ** Other possible actions would go here -- for example:
  **
  ** ...
  ** else if (!strcmp(action, "Save theory"))
  **    saveTheory();
  ** else if (!strcmp(action, "Load theory"))
  **    loadTheory();
  ** ...
  **   
  */
  else
	/* By default, just print out the standard "Enter your theory here" form */
	displayForm();

  return ErrorCode;
}


/*----------------------------------------------------------------------------
Function: submitTheory
Purpose : Reads in the theory from the CGI form, calls DELORES to
          interpret it, and sends DELORES's output back to the web as an HTML
          page.
Args    : "theory" -- field from CGI form containing the user's theory
Returns : nothing
----------------------------------------------------------------------------*/
void submitTheory(void) {
  int length;
  FILE *f;
  char *theory, tmpFile[L_tmpnam], *pipeCmd, buf[BUFSIZE];

  /* Print the top of the resulting web page */
  cgiHeaderContentType("text/html");
  printHeader();

  /* Does the "theory" field exist? */
  if (cgiFormStringSpaceNeeded("theory", &length) != cgiFormSuccess) {
	printError("Can't find theory field.");
	return;
  }

  /* Allocate a character buffer large enough to contain the theory */
  if (!(theory = malloc(length))) {
	printError("Out of memory.");
	return;
  }

  /* Copy the theory into the buffer */
  if (cgiFormString("theory", theory, length) != cgiFormSuccess) {
	printError("Can't get theory data.");
	return;
  }

  /* Copy the buffer into a temporary file */
  if (!(f = fopen(tmpnam(tmpFile), "w"))) {
	printError("Can't open temp file.");
	return;
  }
  fputs(theory, f);
  fclose(f);
  free(theory);

  /* Construct the command line to call DELORES */
  if (!(pipeCmd = malloc(strlen(DL_COMMAND) + L_tmpnam + 1))) {
	printError("Out of memory.");
	return;
  }
  sprintf(pipeCmd, "%s %s", DL_COMMAND, tmpFile);

  /* Open a pipe to DELORES */
  if (!(f = popen(pipeCmd, "r"))) {
	printError("Can't open pipe.");
	free(pipeCmd);
	return;
  }
  free(pipeCmd);

  /* Pipe the output of DELORES to the web */
  fprintf(cgiOut, "<h1>DELORES output</h1>\n<pre>");
  while (fgets(buf, BUFSIZE, f))
	fputs(buf, cgiOut);
  fprintf(cgiOut, "</pre>\n");

  /* Close the pipe and delete the temporary file */
  pclose(f);
  remove(tmpFile);

  /* Print the bottom of the web page */
  printFooter();
}


/*----------------------------------------------------------------------------
Function: displayForm
Purpose : prints out the default "Enter your theory here" form
Args    : none
Returns : nothing
----------------------------------------------------------------------------*/
void displayForm(void) {
  FILE *htmlFile;
  char buf[BUFSIZE];

  cgiHeaderContentType("text/html");

  if (!(htmlFile = fopen(DL_HTML_FILE, "r"))) {
	printError("Can't open form file.");
	return;
  }

  while (fgets(buf, BUFSIZE, htmlFile))
	fputs(buf, cgiOut);
  
  fclose(htmlFile);

  return;
}


/*----------------------------------------------------------------------------
Function: printHeader
Purpose : prints out the document type and first few HTML tags of the output
          page
Args    : none
Returns : nothing
----------------------------------------------------------------------------*/
void printHeader(void) {
  fprintf(cgiOut, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">"
		  "<HTML><HEAD><TITLE>DELORES</TITLE></HEAD><BODY bgcolor=#ffffff>\n");
}


/*----------------------------------------------------------------------------
Function: printFooter
Purpose : prints out the end of the HTML document
Args    : none
Returns : nothing
----------------------------------------------------------------------------*/
void printFooter(void) {
  fprintf(cgiOut, "</body></html>\n");
}


/*----------------------------------------------------------------------------
Function: printError
Purpose : prints out the given error message and a footer ending the HTML
          docmuent
Args    : msg -- the error message to print
Returns : nothing
----------------------------------------------------------------------------*/
void printError(char *msg) {
  fprintf(cgiOut, "<h1>DELORES error</h1><p>%s</p>\n", msg);
  printFooter();
  ErrorCode = EXIT_FAILURE;
}
