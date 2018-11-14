/*
	Code by Jack Belmont & Skyler Davis & Varun
	Class: CS410
	Professor: Rich West
	Project: a2

	This file:
		This file conains the main entry point for the myshell program. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>
#include <signal.h>

#include "shellFuncs.h"

/*global variables to keep track of childID's*/
int* childID;
int numChildren, childNum;

sigjmp_buf		jmpbuf;
sig_atomic_t	canjump;


int main(){
	/*main functions of the shell:
		1) Read Command from stdin
		2) Parse the Command
		3) Execute Command
	*/

	/*define variables*/
	char* input;
	char** args;
	int i, numArgs;
	int* numArgs_p;


	/*set up the golobal childID array to hold the 
	  process id's of the child processes*/
	numChildren = 512;
	childNum = 0;
	childID = malloc(sizeof(int) * numChildren);
	memset(childID, 0, sizeof(int)* numChildren);

	if ( signal(SIGINT, SIGINT_handler) == SIG_ERR ){
		fprintf(stderr, "signal(SIGINT) error\n");
	}
	if ( signal(SIGCHLD, SIGCHLD_handler) == SIG_ERR ){
		fprintf(stderr, "signal(SIGCHLD) error\n");
	}


	/*shell loop | exits when user types ctrl+d*/
	while(1){
		sigsetjmp(jmpbuf, 1);

		canjump = 1;

		/*get input from terminal command line*/
		input = getCommand();

		//simple shell commands
		if( strcmp(input, "exit") == 0 ){
			free(input);
			//free(childID);
			exit(0);
		}
		else if ( strcmp( input, "clear" ) == 0 ){
			system("clear");
			free(input);
			continue;
		}
		else if ( strcmp(input, "show child") == 0 ){
			printf("	%d running Children processes: ", childNum);
			for(i = 0; i < childNum; i++){
				printf("%d, ", childID[i]);
			}
			printf("\n");
			free(input);
			continue;
		}
		else if (strcmp(input, "\0") == 0){
			free(input);
			continue;
		}

		/*make a pointer to numArgs so it can 
		  be incremented inside of the myParse Function
		  then call the myParse function*/
		numArgs = 0;
		numArgs_p = &numArgs;
		args = myParse(input, numArgs_p);

		//executeCommands(args, numArgs);
		setUpAndExecute(args, numArgs);

		/*free the dynamically allocated memory 
		  to avoid memory leaks*/
		free(input);
		for(i = 0; i < numArgs; i++){
			free(args[i]);
		}
		free(args);

		

	}

	exit(0); //return successfully	
}
