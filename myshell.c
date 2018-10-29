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


int main(){
	/*main functions of the shell:
		1) Read Command from stdin
		2) Parse the Command
		3) Execute Command
	*/

	char* input;
	char** args;
	int i, numArgs;
	int* numArgs_p;

	while(1){
		input = getCommand();

		numArgs = 0;
		numArgs_p = &numArgs;
		args = myParse(input, numArgs_p);

		int i, j;
		for(i = 0;i < numArgs; i++ ){
			printf("%s\n", args[i]);
		}





		/*free the dynamically allocated memory 
		  to avoid memory leaks*/
		free(input);
		free(args);
	}

	exit(0); //return successfully	
}
