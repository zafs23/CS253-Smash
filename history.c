//----------------------------------------------------------------------------
//
// Sajia Zafreen
// history.c -- Implements the prototypes and symbols defined by history.h
//
//----------------------------------------------------------------------------


//Private structures for that file should go in the .c file, 
//with a declaration in the .h file if they are used by any functions in the .h
//Public structures should go in the .h file.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef HISTORY_H
#define HISTORY_H
#include "history.h"
#endif

#define MAXLINE 4096


struct Cmd *cmdPointer[MAXHISTORY]; // here MAXHISTORY = 10

//Define the maximum number of entries in the history array 
//Smash will save the history of the last 10 commands

//Function prototypes for the command history feature  
void init_history(void) {
	for (int i=0; i< MAXHISTORY ; i++){
		cmdPointer[i] = malloc (sizeof(struct Cmd));
		cmdPointer[i]->cmd = malloc(MAXLINE); // full string with the '\0' at the end
		cmdPointer[i]->exitStatus =0;

	}
}

void add_history(char *cmd, int exitStatus, int firstSequenceNumber){ //Adds an entry to the history
	if (firstSequenceNumber > MAXHISTORY -1){
		int trackSequence = MAXHISTORY -1;

		// copying the last 9 commands to the start of the array
		for (int i=0; i< MAXHISTORY -1; i++){
			strncpy(cmdPointer[i]->cmd, cmdPointer[i+1]->cmd, MAXLINE); // MAXLINE cz there is a '\0' at the end
	        cmdPointer[i]->exitStatus = cmdPointer[i+1]->exitStatus;
		}

	    // assigning value to the most recent command in the end of the array
	    strncpy(cmdPointer[trackSequence]->cmd ,cmd, MAXLINE);  // nothing the last memory
	    cmdPointer[trackSequence]->exitStatus = exitStatus;// nothing
	}else {
	    strncpy(cmdPointer[firstSequenceNumber]->cmd ,cmd, MAXLINE); 
	    cmdPointer[firstSequenceNumber]->exitStatus = exitStatus;
    } 
}

void clear_history(void){ // have to call it
	for (int i=0; i<MAXHISTORY ; i++){
		free(cmdPointer[i]->cmd);
		free(cmdPointer[i]);
	}
} //Frees all malloc’d memory in the history 


void print_history(int firstSequenceNumber) {
	if(firstSequenceNumber <= (MAXHISTORY -1)){
		for (int i=0; i<=firstSequenceNumber ; i++ ){
			fprintf(stdout, "%d  ", (i+1));
			fprintf(stdout, "[%d]  ", cmdPointer[i]->exitStatus);
	        fprintf(stdout, "%s", cmdPointer[i]->cmd);
		}
	}else {
		int track = firstSequenceNumber - MAXHISTORY + 1;
		for (int i=0; i<MAXHISTORY; i++){
			fprintf(stdout, "%d  ", track+1);
			fprintf(stdout, "[%d]  ", cmdPointer[i]->exitStatus);
	        fprintf(stdout, "%s", cmdPointer[i]->cmd);
	        track++;
		}
	}
}