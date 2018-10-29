/*
	Code by Jack Belmont & Skyler Davis
	Class: CS410
	Professor: Rich West
	Project: a2

	This File:
		This file contains the functions that are used
		in myshell.c to perform command opperations.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shellFuncs.h"

#define NORMAL_BUFFSIZE 1024;


/*
README: getCommand()
	+ This function gets and returns a line entered by the command line.
	+ The function will terminate the process when ctrl+d is pressed.
	+ Make sure you free the returned string from this function when
		it is no longer being used in main()
*/
char* getCommand(void)
{
	int c;
	int index = 0;
	int buffsize = NORMAL_BUFFSIZE;
	char* lineBuff;

	/*alocate space for the buffer*/
	if(!(lineBuff = malloc(buffsize))){
		fprintf(stderr, "Allocation for lineBuff failed\n");
		exit(1);
	}
	memset(lineBuff, 0, buffsize);

	/*Shell prompt*/
	printf("myshell> ");

	/*get the line and check for termination & buffer overflow*/
	while(1){
		c = getchar();

		if (c == EOF){ 
			/*ctrl-d was detected so we should free the 
			  buffer and exit successfully*/
			printf("\n");
			free(lineBuff);
			exit(1);
		}
		else if (c == '\n'){
			lineBuff[index] = '\0';
			return lineBuff;
		}
		else{
			lineBuff[index] = c;
		}

		index++;

		/*Check if we must allocate more space for the buffer*/
		if(index >= buffsize){
			buffsize = buffsize + NORMAL_BUFFSIZE;
			if(!(lineBuff = realloc(lineBuff, buffsize))){
				fprintf(stderr, "Reallocation of lineBuff failed\n");
				exit(1);
			}
		}
	}


	return lineBuff;
}


/*
README: myparse()
	+ This function parses the line that was input by the user
	+ The line will be split into arguments(args)
		- each argument will be determined based on white space
	+ Function will return the number of tokens
		(character blocks separated by white space)

*/
char** myParse(char* line, int* numArgs){
	char** args;
	int lineLen, i, j, position;

	/*dynamically allocate the args variable for the tokens
	  with space that is more than enough to avoid any memory coruption:
	  each token must be <= lineLen,and there must be <= lineLen tokens,
	  based on the limits of a line size*/
	lineLen = strlen(line);
	args = (char**)malloc(lineLen * sizeof(char*));
	memset(args, 0, (lineLen * sizeof(char*)));
	for(i=0; i<lineLen; i++){
		args[i] = (char*)malloc(lineLen * sizeof(char*));
		memset(args[i], 0, (lineLen * sizeof(char*)));
	}

	position = 0;
	while(line[position] == ' '){
		/*trim off any leading white space*/
		position++;
	}

	i = 0; //token string
	j = 0; //which token
	while(line[position] != '\0'){
		(*numArgs)++;
		/*get and store the token*/
		while( line[position] != ' ' && line[position] != '\0'){
			args[i][j] = line[position];
			position++;
			j++;
		}

		/*make sure we are not at the end of the line*/
		if( line[position] == '\0' ){
			break;
		}

		j = 0; /*done with first token. reset token offset to zero*/
		i++; /*next token*/
		while( line[position] == ' ' && line[position != '\0' ]){
			/*move to next token*/
			position++;
		}

		/*check again to make sure we arent at the end of te line*/
		if ( line[position] == '\0' ){
			break;
		}
	}

	return args;
}




