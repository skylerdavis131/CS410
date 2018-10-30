/*
	Code by Jack Belmont & Skyler Davis
	Class: CS410
	Professor: Rich West
	Project: a2

	This file:
		This file conains the main entry point for the myshell program. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellFuncs.h"

/*global variables to keep track of childID's*/
int* childID;
int numChildren;


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
	int executeStat;


	/*set up the golobal childID array to hold the 
	  process id's of the child processes*/
	numChildren = 512;
	childID = malloc(sizeof(int) * numChildren);
	memset(childID, 0, sizeof(int)* numChildren);



	/*shell loop | exits when user types ctrl+d*/
	while(1){
		input = getCommand();

		//simple exit check. Should I delete this?
		if( strcmp(input, "exit") == 0 ){
			free(input);
			free(childID);
			exit(0);
		}
		else if ( strcmp( input, "clear" ) == 0 ){
			system("clear");
			free(input);
			continue;
		}

		/*make a pointer to numArgs so it can 
		  be incremented inside of the myParse Function*/
		numArgs = 0;
		numArgs_p = &numArgs;


		args = myParse(input, numArgs_p);

		/*echo the arguments*/
/*		int i, j;
		int lineLen = strlen(input) + 1;
		for(i = 0;i < numArgs; i++ ){
			printf("%s with null at index: ", args[i]);
			for(j = 0; j < lineLen; j++ ){
				if (args[i][j] == '\0'){
					printf("%d\n", j);
					break;
				}
			}
		}
		printf("\n");
		
*/
		executeStat = myExec(args, numArgs);



		/*free the dynamically allocated memory 
		  to avoid memory leaks*/
		free(input);
		free(args);
	}

	exit(0); //return successfully	
}
