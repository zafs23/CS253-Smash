
//----------------------------------------------------------------------------------------------------------------------
//
// Runs a smash 
// Implements some of the prototypes and symbols defined by smash.h
//
// Author: Sajia Zafreen
//
//------------------------------------------------------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#ifndef HISTORY_H
#define HISTORY_H 
#include "history.h"
#endif

#ifndef SMASH_H
#define SMASH_H 
#include "smash.h"
#endif


#define MAXLINE 4096

char bfr[MAXLINE];


int main(int argc, char **argv) {
  signal(SIGINT, SIG_IGN);
  fputs("$ ",stderr); //Output the first prompt

  int bfrlen=0;
  bfrlen = strlen(bfr);
  //Loop reading commands until EOF or error 
  init_history();
  while (fgets(bfr, MAXLINE, stdin) != NULL) { // reads at most MAXLINE-1 chars 
    setvbuf(stdout,NULL,_IONBF,0);
    bfr[bfrlen - 1] = '\0'; //Replace newline with NUL 
    if (flushEmptyLine(bfr) != 0){ // taking care of only newline or empty line charecters
      executeCommand(bfr);
    }
    fputs("$ ",stderr);
  }
  clear_history();
  return 0;
}

int flushEmptyLine (char *buffer){

	char sourceBuffer[MAXLINE-1];
	strncpy(sourceBuffer, buffer, MAXLINE);
	int charecters =0;
	for (int i = 0; i < strlen(sourceBuffer); i++){
	 	if (sourceBuffer[i] != '\n' && sourceBuffer[i] != ' ' && sourceBuffer[i] != '\0'){
	 		charecters++;
	 	}
	}
	return charecters;
}
