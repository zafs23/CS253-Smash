
//----------------------------------------------------------------------------
//
// Sajia Zafreen
// commands.c -- Implements most of the prototypes and symbols defined by smash.h
//
//----------------------------------------------------------------------------



#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include<sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <signal.h>



#ifndef HISTORY_H
#define HISTORY_H
#include "history.h"
#endif

#ifndef SMASH_H
#define SMASH_H 
#include "smash.h"
#endif

#define externalExit 127
#define MAXLINE 4096 
#define MAXARGS 2048 //the highest chars enetered in the args is 4096 with \0 and \n and thus highest number of 
// args will be when there are max number of spaces(delimeters), that is why highest args will be (4096-2)/2


int exitStatus = EXIT_SUCCESS; // in general normal exit
int firstSequenceNumber =0;// first time execute of command.c


void executeCommand(char *str ){

    int pipePresent = 0 ; 
    char *argsCopy [MAXARGS]; //highest MAXLINE -1 was read, thus there is no way the args array need to store more
    char *token;

    char *fullCommand =malloc(MAXLINE*sizeof(char));
    strncpy(fullCommand, str, MAXLINE);// later used for history

    token = strtok(str, " "); // first call parse the string with delimeter with a null-terminated string
    int tokenNumber = 0;
   /* parsing spaces*/
    while( token != NULL ) {
        argsCopy[tokenNumber] = token;
        token = strtok(NULL, " \n");// do not use just delimeter otherwise cd won't work
        tokenNumber++;
    }

    //num of pipes
    for (int inumP=0;inumP< tokenNumber ;inumP++){
        for (int jnumP=0; jnumP<strlen(argsCopy[inumP]); jnumP++){
    	   if(argsCopy[inumP][jnumP] =='|'){
    		  pipePresent++;
    	    }
        }
    }

    if (pipePresent >0){

        struct pipeCmd { 
            int num; 
            int pIn;
            int pOut;
            char *pipein;
            char *pipeout;
            char *argsP[MAXARGS]; 
        };

        //copy of struct to parse
        struct pipecmdcopy{
            char *argspCopy[MAXARGS];
            int numcopy;
        };

	// initialize variables
	int commandNum = pipePresent+1;
        int noError = 1; // for error checking
        //struct initialization
        struct pipeCmd pipecmd[commandNum]; // voyonkor ekta vul cilo
        struct pipecmdcopy pipecmdCP[commandNum];

        for (int iPP=0; iPP< pipePresent+1; iPP++){ // initialize more than 1 as there will be more than one command
            pipecmd[iPP].num = 0;
            pipecmd[iPP].pIn = 0;
            pipecmd[iPP].pOut = 0;
        }

	    // parsing pipe
	    int jP = 0;// number of arrays in one struct
	    int jS = 0; // number of structs
	    for (int iparse=0; iparse<tokenNumber ;iparse++){ // parsing pipes
	    	if (strchr(argsCopy[iparse], '|') != NULL){
    			if ( strlen(argsCopy[iparse]) > 1){
                    char *commPi = argsCopy[iparse];
    				int index = strchr(argsCopy[iparse],'|')-argsCopy[iparse];
    			    int length = strlen(argsCopy[iparse]);
                    //fprintf(stderr, "%d %d\n",index, length );
    				char *argsPip[strlen(argsCopy[iparse])];
    				char *pipeToken = strtok(argsCopy[iparse], "|");
    				int piJ =0;// track tokens
	    	        while( pipeToken != NULL){
                        argsPip[piJ] = pipeToken;
                        
                        if (index == 0){
                            jS++;
                            jP=0;

                            commandEnd (argsPip[piJ]);
                            pipecmdCP[jS].argspCopy[jP] = argsPip[piJ];
                            jP++;
                            pipecmdCP[jS].numcopy = jP;
                        }// first index
                        if ((index>0) && (index <(length-1))){
                            if(piJ==0){// starts the new one
                                pipecmdCP[jS].argspCopy[jP] = argsPip[piJ];
                                jP++;
                                pipecmdCP[jS].argspCopy[jP] = NULL;
                                pipecmdCP[jS].numcopy = jP;
                            }
                            if(piJ>0) {
                                jS++;
                                jP=0;
                                commandEnd (argsPip[piJ]);
                                pipecmdCP[jS].argspCopy[jP] = argsPip[piJ];
                                jP++;
                                pipecmdCP[jS].numcopy = jP;
                            }
                        }// middle index

                        if(index == (length-1)){
                            pipecmdCP[jS].argspCopy[jP] = argsPip[piJ];
                            jP++;
                            
                            pipecmdCP[jS].argspCopy[jP] = NULL;
                            pipecmdCP[jS].numcopy = jP;
                            jS++;
                            jP=0;
                        }// end index
                        
                        // change the index
                        for (int m=index; m<length ; m++){
                            if (commPi[m] =='|'){
                                index = m;
                                break;
                            }
                        }// end of change in index
                        
                        pipeToken = strtok(NULL, "|");
                        piJ++; 
                    }// end of while loop of strtok of pipe
    			}else{// else of if only pipe present
    				pipecmdCP[jS].argspCopy[jP] = NULL;
                    jS++;
    		        jP=0;
    		    }// end of pipe present
    		}else {// else of no pipe present
	    		if (jP==0){
    		    	commandEnd (argsCopy[iparse]);
    		      }
	    		pipecmdCP[jS].argspCopy[jP] = argsCopy[iparse];
                        jP++;
	    		pipecmdCP[jS].numcopy = jP;
	    	}// end of no pipe
	    	if (iparse == tokenNumber -1){ // only for the forloop
    	    	pipecmdCP[jS].argspCopy[jP] = NULL;
    	    	pipecmdCP[jS].numcopy = jP;
    	    }
	    }// end of for loop parsing pipes

	    //printing first commands
	    //for (int ifcomm=0; ifcomm<commandNum ; ifcomm++){
       	//    fprintf(stderr, "%s %d", pipecmdCP[ifcomm].argspCopy[0], pipecmdCP[ifcomm].numcopy);
       	//    fputs("\n", stderr);
        //}

        int pipeinprev =0;
        int pipeoutprev =0;
        int track = 0;
        // parsing commands if pipe commands they have '<' or '>'
	    for (int iinout =0; iinout< commandNum; iinout++){
            track =0;
	    	for(int jinout=0; jinout< pipecmdCP[iinout].numcopy ;jinout++){
	    		if (strchr(pipecmdCP[iinout].argspCopy[jinout],'<') != NULL){
    			    pipecmd[iinout].pIn++;
    			    if ( strlen(pipecmdCP[iinout].argspCopy[jinout]) > 1){// < and other command parsing
    			    	int index = strchr(pipecmdCP[iinout].argspCopy[jinout],'<')- pipecmdCP[iinout].argspCopy[jinout];
    			        int length = strlen(pipecmdCP[iinout].argspCopy[jinout]);
    				    char *argsIn[length];

    			        char *tokenIn = strtok (pipecmdCP[iinout].argspCopy[jinout], "<");
    			        int ipIn =0;// to track loop
    			        while (tokenIn != NULL){
    			    	    argsIn[ipIn] = tokenIn;
    			    	    tokenIn = strtok(NULL, "<");
    			    	    ipIn ++;
                        }// end of token
                        if (index == 0){
                            commandEnd(argsIn[0]);
    			    		pipecmd[iinout].pipein = argsIn[0];
    			    	}//end of first index
    			        if ((index>0) && (index <(length-1))){
                            commandEnd(argsIn[0]);
    			    		pipecmd[iinout].argsP[track] = argsIn[0];
                            track++;
                            pipecmd[iinout].num++;

                            commandEnd(argsIn[1]);
    			    		pipecmd[iinout].pipein = argsIn[1];
    			    	}// end of middle index
    			        if(index == (length-1)){
                            pipeinprev = 1;
                            commandEnd(argsIn[0]);
    			    	    pipecmd[iinout].argsP[track] = argsIn[0];
                            track++;
                            pipecmd[iinout].num++;
    			    	}// end of last index
    			    } else {// < only present no other command
                        pipeinprev = 1;
    			    }// end of < parsing
    		    }else if (strchr(pipecmdCP[iinout].argspCopy[jinout],'>') != NULL){// start of > parsing
    			    pipecmd[iinout].pOut++;
    			    if ( strlen(pipecmdCP[iinout].argspCopy[jinout]) > 1){// start of > and other command
    			    	int index = strchr(pipecmdCP[iinout].argspCopy[jinout],'>')- pipecmdCP[iinout].argspCopy[jinout];
    			        int length = strlen(pipecmdCP[iinout].argspCopy[jinout]);
    				    char *argsOut[length];

    			        char *tokenOut = strtok (pipecmdCP[iinout].argspCopy[jinout], ">");
    			        int ipOut =0;
    			        while (tokenOut != NULL){
    			    	    argsOut[ipOut] = tokenOut;
    			    	    tokenOut = strtok(NULL, ">");
    			    	    ipOut ++;
                        }
                        if (index == 0){
                            commandEnd(argsOut[0]);
    			    		pipecmd[iinout].pipeout = argsOut[0]; 
    			    	}// end of first index
    			        if ((index>0) && (index <(length-1))){
                            commandEnd(argsOut[0]);
    			    		pipecmd[iinout].argsP[track] = argsOut[0];
                            pipecmd[iinout].num++;
                            commandEnd(argsOut[1]);
    			    		pipecmd[iinout].pipeout = argsOut[1];
    			    	}// end of middle index
    			        if(index == (length-1)){
                            pipeoutprev = 1;
                            commandEnd(argsOut[0]);
    			    	    pipecmd[iinout].argsP[track] = argsOut[0];
                            pipecmd[iinout].num++;
    			    	}// end of last index
    			    } else { // only > present , and no other command
                        pipeoutprev = 1;
    			    }// end of parsing > when only > present 
    		    }else {
                    commandEnd(pipecmdCP[iinout].argspCopy[jinout]);
                    if (pipeinprev){
                        pipecmd[iinout].pipein = pipecmdCP[iinout].argspCopy[jinout];
                        pipeinprev =0;
                    }else if (pipeoutprev){
                        pipecmd[iinout].pipeout = pipecmdCP[iinout].argspCopy[jinout];
                        pipeoutprev =0;
                    }else {
                        pipecmd[iinout].argsP[track] = pipecmdCP[iinout].argspCopy[jinout];
                        track++;
                        pipecmd[iinout].num++;
                    }
                }// end of > parsing
	    	}// end of jinout forloop
	    }// end of iinout forloop

        //printing commands, delete later
        //fputs("\n", stderr);
        //for (int ik=0; ik<commandNum ; ik++){
        //    for (int jk=0; jk<pipecmd[ik].num; jk++){
        //        fprintf(stderr, "%s ", pipecmd[ik].argsP[jk]); 
        //    }
        //    fprintf(stderr, "i:%d o:%d %d", pipecmd[ik].pIn, pipecmd[ik].pOut, pipecmd[ik].num);
        //    if (pipecmd[ik].pIn >0){
        //        fprintf(stderr, "in:%s ", pipecmd[ik].pipein);
        //    }
        //    if (pipecmd[ik].pOut >0){
        //        fprintf(stderr, "out:%s ", pipecmd[ik].pipeout);
        //    }
        //    fputs("\n", stderr);
        //}

        //checking for error
        // I/O redirection to a pipe other than pipe is not supported
        for(int icheck =0; icheck<commandNum; icheck++){
            if(icheck !=0){
                if(pipecmd[icheck].pIn>0){
                    noError = 0;
                }
            }
            if(icheck != (commandNum -1)){
                if(pipecmd[icheck].pOut>0){
                    noError=0;
                }
            }
        }
        // return if unsupported command is entered
        if(!noError){
            exitStatus = externalExit;
            add_history(fullCommand, exitStatus, firstSequenceNumber);
            fputs("Error: I/O redirection to a pipe is not supported\n", stderr);
            return;
        }

        // reassuring args end
        for(int r=0; r<commandNum; r++){
            pipecmd[r].argsP[pipecmd[r].num]= NULL;
        }

        //starting to setting up pipes to run the commands
        // initialize each pipe
        int pipefd[2*pipePresent];
        for (int ifd=0; ifd< pipePresent ; ifd++){
       	    if (pipe(pipefd + ifd*2)<0){
       	    	perror("Error Pipe");
       	    	exit(EXIT_FAILURE);
       	    }
        }

        // initialize variables
        int pid;
        int iP= 0 ;
        //int commandNum = pipePresent+1; // already copied above
        int pipestatus;
        int inP = dup(0);
        int outP = dup(1);
        int fdpin;
        int fdpout;
        int historyCalled = 0;

        fflush(stdin);
	    fflush(stdout);
        for (int ipipe=0; ipipe< commandNum ; ipipe++){
        	pid = fork();
        	if (pid == 0){ // childs
        		signal(SIGINT, SIG_DFL); // signal handling
        		//if first command has in redirection
        	    if (pipecmd[ipipe].pIn >0){
                    if ((fdpin = open(pipecmd[ipipe].pipein, O_RDONLY, 0)) < 0) {
                        perror("1.Error: ");
                        exit(externalExit);
                    }
                    dup2(fdpin, 0);
                    close (fdpin);
                }
                //if the last command has out redirection
                if (pipecmd[ipipe].pOut >0){
                    if ((fdpout = open( pipecmd[ipipe].pipeout, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
        	            perror("2.Error: ");
                        exit(externalExit);
                    }
                    dup2(fdpout, 1); // 1 here can be replaced by STDOUT_FILENO
                    close(fdpout);
                }

        		if (ipipe != (commandNum -1)){ // if not last cmd
        			if (dup2(pipefd[iP+1], 1)<0){
        				perror("dup2");
        				exit(EXIT_FAILURE);
        			}
        		}
        		// if not first command 
        		if (iP != 0){
        			if (dup2(pipefd[iP -2], 0) <0){
        				perror("dup2");
        				exit(EXIT_FAILURE);
        			}
        		}

        		for (int iC =0; iC< 2*pipePresent; iC++){
        			close(pipefd[iC]);
        		}

        		if((ifCommand(pipecmd[ipipe].argsP[0],"history")==0)){
        		    exitStatus = EXIT_SUCCESS;
                    add_history(fullCommand, exitStatus,firstSequenceNumber);
                    print_history(firstSequenceNumber);
                    historyCalled = 1;
        		} else if(execvp(pipecmd[ipipe].argsP[0], pipecmd[ipipe].argsP) <0){
        			    perror("Error");
		            }
		        _exit(externalExit);// failed command
		        
        	}else if (pid <0){ // if fork failed
        		perror("Error");
        	    exit (EXIT_FAILURE);
        	}
        	    
        	iP+=2; // increment pipe numbers by two
        }// end of pipe for loop

        //in the parent now
        // close if I/O redirection occured
        dup2(inP, 0);
        dup2(outP, 1);
        close(inP);
        close(outP);

        for(int iD = 0; iD < 2 * pipePresent; iD++){//close all pipe descriptors
            close(pipefd[iD]);
        }
        for(int iE = 0; iE < commandNum; iE++){//wait for all child to return
            wait(&pipestatus);
        }
        if (pipestatus != 0){ //if pipe status is non zero then make it as externalExit = 127
            pipestatus = externalExit ;
        }
        //update pipe status
        exitStatus = pipestatus;
        //add to the history
        if (!historyCalled){
            add_history(fullCommand, exitStatus, firstSequenceNumber);
        }

    }else {// end of pipe present
        // starting without pipe
        // varibale initialization
        int inRedirect = 0; 
        int outRedirect = 0;
        char *indexIn ;
        char *indexOut ;
        int argsi = 0;
        char *args [MAXARGS];
        int inPrevious = 0;
        int outPrevious = 0;

        //looping through
        for(int l=0; l< tokenNumber; l++){
            if ((strchr(argsCopy[l], '<') != NULL) && (strchr(argsCopy[l], '>')!= NULL)){//start of commands where < and > present w/o space
                inRedirect++;
                outRedirect++;

                int indexI = strchr(argsCopy[l],'<')-argsCopy[l];
                int lengthI = strlen(argsCopy[l]);
                char *outargs;
                char *argsIn[strlen(argsCopy[l])];
                char *tokenIn = strtok (argsCopy[l], "<");
                int iinLoop = 0;
                while (tokenIn != NULL){
                    argsIn[iinLoop] = tokenIn;
                    tokenIn = strtok(NULL, "<");
                    iinLoop++; 
                }
                //parsed first command 
                if (indexI == 0){// starting of first index
                        commandEnd(argsIn[0]);
                        outargs = argsIn[0];
                        inPrevious = 1;
                }
                if ((indexI>0) && (indexI <(lengthI-1))){ // starting of middle index
                    if (strchr(argsIn[0], '>') != NULL){
                        commandEnd(argsIn[1]);
                        indexIn = argsIn[1];
                        outargs = argsIn[0];
                    }else {
                        commandEnd(argsIn[0]);
                        args[argsi] = argsIn[0];
                        argsi++;
                        outargs = argsIn[1];
                        inPrevious = 1;
                    }
                }
                if(indexI == (lengthI-1)){// last index
                        inPrevious = 1;
                        outargs = argsIn[0];
                }

                //parse second token which has >
                int indexO = strchr(outargs,'>')-outargs;
                int lengthO = strlen(outargs);
                char *argsOut[strlen(outargs)];
                char *tokenOut = strtok (outargs, ">");
                int ioutloop = 0;
                while (tokenOut != NULL){
                    argsOut[ioutloop] = tokenOut;
                    tokenOut = strtok(NULL, ">");
                    ioutloop++; 
                }

                if (indexO == 0){
                    commandEnd(argsOut[0]);
                    indexOut = argsOut[0]; 
                }
                if((indexO > 0) && (indexO < (lengthO-1))){
                    if (inPrevious){
                        commandEnd(argsOut[0]);
                        indexIn = argsOut[0];

                        commandEnd(argsOut[1]);
                        indexOut = argsOut[1];

                        inPrevious = 0;
                    }else {
                        commandEnd(argsOut[0]);
                        args[argsi] = argsOut[0];
                        argsi++;

                        commandEnd(argsOut[1]);
                        indexOut = argsOut[1];
                    }
                }
                if (indexO == (lengthO -1)){
                    outPrevious = 1;
                    if (inPrevious){
                        commandEnd(argsOut[0]);
                        indexIn = argsOut[0];
                        inPrevious = 0;
                    }
                }
            } else if (strchr(argsCopy[l],'<') != NULL){ // start of checking commands with only < and no space
                inRedirect++;
                int index = strchr(argsCopy[l],'<')-argsCopy[l];
                int length = strlen(argsCopy[l]);
                if ( strlen(argsCopy[l]) > 1){
                    char *argsIn[strlen(argsCopy[l])];
                    char *tokenIn = strtok (argsCopy[l], "<");
                    int iinLoop = 0;
                    while (tokenIn != NULL){
                        argsIn[iinLoop] = tokenIn;
                        tokenIn = strtok(NULL, "<");
                        iinLoop++; 
                    }
                    if (index == 0){// starting of first index
                        commandEnd(argsIn[0]);
                        indexIn = argsIn[0]; 
                    }
                    if ((index>0) && (index <(length-1))){ // starting of middle index
                        commandEnd(argsIn[0]);
                        args[argsi] = argsIn[0];
                        argsi++;

                        commandEnd(argsIn[1]);
                        indexIn = argsIn[1];
                    }
                    if(index == (length-1)){// last index
                        inPrevious = 1;
                        commandEnd(argsIn[0]);
                        args[argsi] = argsIn[0];
                        argsi++;
                    }
                } else {
                    inPrevious = 1; // extra < or > where no other command is present
                }
            }else if (strchr(argsCopy[l],'>') != NULL){ // check for > presents
                outRedirect++;
                int index = strchr(argsCopy[l],'>')-argsCopy[l];
                int length = strlen(argsCopy[l]);
                if ( strlen(argsCopy[l]) > 1){
                    char *argsOut[strlen(argsCopy[l])];
                    char *tokenOut = strtok (argsCopy[l], ">");
                    int ioutloop = 0;
                    while (tokenOut != NULL){
                        argsOut[ioutloop] = tokenOut;
                        tokenOut = strtok(NULL, ">");
                        ioutloop++; 
                    }
                    if (index == 0){
                        commandEnd(argsOut[0]);
                        indexOut = argsOut[0]; 
                    }
                    if((index > 0) && (index < (length-1))){
                        commandEnd(argsOut[0]);
                        args[argsi] = argsOut[0];
                        argsi++;
                        commandEnd(argsOut[1]);
                        indexOut = argsOut[1]; 
                    }
                    if (index == (length -1)){
                        outPrevious = 1;
                        commandEnd(argsOut[0]);
                        args[argsi] = argsOut[0];
                        argsi++;
                    }
                    // end of > with commands
                }else {// only > present
                    outPrevious = 1;
                }
            }else {// nothing present, only command
                commandEnd(argsCopy[l]);
                if(inPrevious){
                    indexIn = argsCopy[l];
                    inPrevious =0;
                }else if (outPrevious){
                    indexOut = argsCopy[l];
                    outPrevious =0;
                }else {
                    args[argsi] = argsCopy[l];
                    argsi++;
                }
            }
        }// end of l forloop

        //print in/out parsing
        //for (int t=0; t<=argsi;t++){
        //    fprintf(stderr, "%s ",args[t]);
        //}
        //fprintf(stderr, "in:%s out:%s ", indexIn,indexOut );
        //fputs ("\n", stderr);

        // walking through the tokens
        if (ifCommand(args[0] , "exit")==0){
            exitStatus = EXIT_SUCCESS;
            free(fullCommand);
            clear_history();
            exit (exitStatus);
        }else if (ifCommand( args[0], "cd") == 0){
            if (tokenNumber > 1){
                exitStatus = cd(args[1]);
            } else {
                exitStatus = EXIT_FAILURE;// add an exit status
            }
            add_history(fullCommand, exitStatus, firstSequenceNumber);
        }else if ((ifCommand(args[0], "history") == 0)){
            int inhis = dup(0);
            int outhis = dup(1);
            fflush(stdout);
            if (inRedirect >0){
                int fd0;
                if ((fd0 = open(indexIn, O_RDONLY, 0)) < 0) {
                    perror("Error: ");
                    exit(externalExit);
                }
                // dup2() copies content of fdo in input of preceeding file
                dup2(fd0, 0); // STDIN_FILENO here can be replaced by 0 
                close(fd0);
            }
            if (outRedirect >0){
                int fd1 ;
                if ((fd1 = open( indexOut, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
                    perror("Error: ");
                    exit(externalExit);
                }
                dup2(fd1, 1); // 1 here can be replaced by STDOUT_FILENO
                close(fd1);
            }// end of checking out was redirected
            exitStatus = EXIT_SUCCESS;
            add_history(fullCommand, exitStatus,firstSequenceNumber);
            print_history(firstSequenceNumber);

            // if stdin or stdout was redirected 
            dup2(inhis, 0);
            dup2(outhis, 1);
            close(inhis);
            close(outhis);
        }else {
            args[argsi]=NULL; // reassuring ars end
            exitStatus = executeExternalCommand(args[0], args, inRedirect, outRedirect, indexIn, indexOut);
            add_history(fullCommand, exitStatus, firstSequenceNumber);
        }// end of checking external command
    }// end of else of everything other than pipe
    
    // freeing memory
    firstSequenceNumber ++;
    free(fullCommand); 
}


int executeExternalCommand(char *arg0, char *args[], int inRedirect, int outRedirect, char *indexIn,char *indexOut){
	int exitResult;
    int inEx = dup(0);
    int outEx = dup(1);
	int fdInEx;
	int fdOutEx;

    fflush(stdin);
	fflush(stdout);
    int childPid = fork();
	if (childPid == 0){
		signal(SIGINT, SIG_DFL); // crtl+c handling
	    if (inRedirect >0){
            if ((fdInEx = open(indexIn, O_RDONLY, 0)) < 0) {
                perror("Error: ");
                exit(externalExit);
            }
            dup2(fdInEx, 0);
            close (fdInEx);
        }
        if (outRedirect >0){
            if ((fdOutEx = open( indexOut, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
        	    perror("Error: ");
                exit(externalExit);
            }
            dup2(fdOutEx, 1); // 1 here can be replaced by STDOUT_FILENO
            close(fdOutEx);
        }
		if (execvp(arg0, args) <0 ){
			perror("Error");
		}
		_exit(externalExit);
	}
	dup2(inEx, 0);
    dup2(outEx, 1);
    close(inEx);
    close(outEx);

	if (childPid >0){
		fflush(stdout);// calling before waitpid
		if (waitpid(childPid, &exitResult, 0) >0){  
			return WEXITSTATUS(exitResult);
		} else { //waitpid failed
			perror ("Error");
			return -1;
		}
	} else {// fork failed
		perror("Error");
		return -1;
	}
}

// cd command
int cd (char *pth){
	char cwd[MAXLINE -1]; // check later again
	char path[MAXLINE -1];
    strcpy(path,pth);
    int currentDirectory =0;
    if (path == NULL ) {
        //fprintf(stderr, "Error: expected argument to \"cd\"\n"); // okay to print in stderr
        return EXIT_FAILURE;
    } else {
    	// add current folder behavior
    	// first case if current directory is given
        if (chdir(path) != 0) {
            perror(path); // directed to print in perror
            return EXIT_FAILURE;
        }
    }
    //printing the cwd
    if (!currentDirectory){
        if ((strchr(path, '/'))==NULL){
    	    //fputs(path, stdout);
    	    getcwd (cwd, sizeof(cwd));

    	    char *tokenCwd;
    	    const char delimeterCwd[2] = "/";
    	    char *argsCwd [MAXLINE -1];

    	    tokenCwd = strtok(cwd, delimeterCwd);

            int tokenNumberCwd = 0;
            /* walk through other tokens */
            while( tokenCwd != NULL ) {
        	    argsCwd[tokenNumberCwd] = tokenCwd;
                tokenCwd = strtok(NULL, delimeterCwd);// do not use delimeter otherwise cd won't work
                tokenNumberCwd++;
            }
            fprintf(stdout, "%s\n",argsCwd[tokenNumberCwd -1]);

        }else {
    	    getcwd (cwd, sizeof(cwd));
            fputs(cwd, stdout);// stdout directed 
            fputs("\n", stdout); // stdout directed
        }
    } else {
    	fprintf(stdout, "%s\n",path);
    }
    return EXIT_SUCCESS;
}

/// take care of new line at the end of the command tokens
void commandEnd (char *arg){
	for(int comE=0; comE< strlen(arg); comE++){ // args[0] was here
    	if(arg[comE] == '\n'){ // here there is no space only a new line
    		arg[comE]= '\0';
    	}
    }
}

//if a command is equal to other command
int ifCommand(char const *p, char const *q){
    int iComm = 0;
    for(iComm = 0;q[iComm];iComm++) {
        if(p[iComm] != q[iComm]){
            return -1;
        }
    }
    return 0;
}
