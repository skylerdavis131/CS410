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
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "shellFuncs.h"

extern int* childID;
extern int numChildren;

extern char** environ;


/*
README: getCommand()
	+ This function gets and returns a line entered by the command line.
	+ The function will terminate the process when ctrl+d is pressed.
	+ Make sure you free the returned string from this function when
		it is no longer being used in main()
*/
char* getCommand(void)
{
	int errnum;
	int c;
	int index = 0;
	int buffsize = NORMAL_BUFFSIZE;
	char* lineBuff;

	/*alocate space for the buffer*/
	if(!(lineBuff = malloc(buffsize))){
		errnum = errno;
		fprintf(stderr, "Allocation for lineBuff failed | %s\n", strerror(errnum));
		exit(1);
	}
	memset(lineBuff, 0, buffsize);

	/*shell prompt*/
	if(isatty(STDIN_FILENO))
		write(1, "myshell> ", strlen("myshell> ") );


	/*get the line and check for termination & buffer overflow*/
	while(1){
		c = getchar();

		if (c == EOF){ 
			/*ctrl-d was detected so we should free the 
			  buffer and exit successfully*/
			if(!isatty(STDIN_FILENO)){
				if(index == 0){
					free(lineBuff);
					free(childID);
					exit(0);
				}
				else{
					/*we must do some stuff before exiting*/
					/*can we assume that the EOF will be on its own line?*/
					lineBuff[index] = '\0';
					return lineBuff;
				}
			}
			printf("\n");
			free(lineBuff);
			free(childID);
			exit(0);
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
				errnum = errno;
				fprintf(stderr, "Reallocation of lineBuff failed | %s\n", strerror(errnum));
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
	lineLen = strlen(line) + 1;
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

	i = 0; //which token
	j = 0; //offset into token string
	while(line[position] != '\0'){
		(*numArgs)++;
		/*get and store the token*/
		while( line[position] != ' ' && line[position] != '\0'){
			args[i][j] = line[position];
			position++;
			j++;

			if(line[position] == ';'){
				/* ';' is a token */
				j = 0;
				i++;
				args[i][j] = line[position];
				j++;
				(*numArgs)++;
				position++;
				break;
			}
		}
		args[i][j] = '\0';



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

		/*check again to make sure we arent at the end of the line*/
		if ( line[position] == '\0' ){
			break;
		}
	}

	/*Last argument must be NULL termination*/
	args[*numArgs] = NULL;


	return args;
}



int executeCommands(char** args, int numArgs){

	char* command;
	char* commandParam;

	int i;
	int j;
	for(i = 0; i < numArgs; i++){
		/*Loop over every argument to execute*/

		/*comArgs holds the arguments for each command*/
		char** comArgs = (char**)malloc(numArgs * sizeof(char*));
		memset(comArgs, 0, numArgs*sizeof(char*));
		
		j = 0;
		while(i < numArgs){
			if( strcmp(args[i], ";") != 0 && strcmp(args[i], ">") != 0 &&
				strcmp(args[i], "1>") != 0 && strcmp(args[i], "2>") != 0 && 
				strcmp(args[i], "&>") != 0 && strcmp(args[i], "<") != 0){
				/*Fill in the commArgs array*/
				comArgs[j] = args[i];
			}
			else{
				/*Determine which command was found and exec accordingly*/
				if( strcmp(args[i], ";") == 0 ){
					/*if command is ';' we are executing concurrent commands' */
					myExec(comArgs, args[i], NULL);
					break;
				}
				else if( strcmp(args[i], ">") == 0  || strcmp(args[i], "1>") == 0 ||
						 strcmp(args[i], "2>") == 0 || strcmp(args[i], "&>") == 0 ||
						 strcmp(args[i], "<") == 0 ){
					/*we are redirecting input/output/error to/from next argument file (commandParam)*/
					command = args[i];
					i++;
					commandParam = args[i];
					myExec(comArgs, command, commandParam);

					/*check if there are more arguments*/
					if(i+1 < numArgs)
						if( strcmp(args[i + 1], ";") == 0 )
							i++; /*skip over the semi colon*/
						
					break;
				}

				
			}

			j++;
			i++;
			if(i >= numArgs){
				/*underscores are signals to myExec that there is
				  no redirection of stdin, stdout, or stderr*/
				myExec(comArgs, "_", "_");
				break;
			}

		}

		/*free dynamically allocated comArgs array*/
		free(comArgs);

			
	}

	return 0;
}


/*
README: myExec()
	+ This function takes in an array of string arguments and the argument 
		number that we want to execute
	+ Program will return the status of the execution.
*/
int	myExec(char** args, char* command, char* commandParam )
{
	int fd;
	int errnum;
	pid_t pid, waitingpid;
	int status;
	struct stat statbuff;

	if(stat(args[0], &statbuff) < 0){
		/*not an existing file*/
		errnum = errno;
		fprintf(stderr, "- myshell: %s: %s\n", args[0], strerror(errnum));
	}
	else if(statbuff.st_mode & S_IXUSR){
		pid = fork();
		if (pid < 0){
			errnum = errno;
			fprintf(stderr, "- myshell: error forking | %s\n", strerror(errnum));
		}
		else if (pid == 0){
			/*Child, so lets try to start the user specified process*/

			/*check for io manipulation*/
			if(strcmp(command, ">") == 0){
				/*redirect stdout to file commandParam*/
				if ( (fd = open(commandParam, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0 ){
					/*error opening the file*/
					errnum = errno;
					fprintf(stderr, "- myshell: couldnt open file: %s | %s \n", commandParam, strerror(errnum));
					exit(EXIT_FAILURE);
				}
				dup2(fd, 1);
				close(fd);
			}
			else if (strcmp(command, "1>") == 0){
				/*redirect stdout to file commandParam*/
				if ( (fd = open(commandParam, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0 ){
					/*error opening file*/
					errnum = errno;
					fprintf(stderr, "- myshell: couldnt open file: %s | %s \n", commandParam, strerror(errnum));
					exit(EXIT_FAILURE);
				}
				dup2(fd, 1);
				close(fd);
			}
			else if (strcmp(command, "2>") == 0){
				/*redirect stderr to file commandParam*/
				if ( (fd = open(commandParam, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0 ){
					/*error opening file*/
					errnum = errno;
					fprintf(stderr, "- myshell: couldnt open file: %s | %s \n", commandParam, strerror(errnum));
					exit(EXIT_FAILURE);
				}
				dup2(fd, 2);
				close (fd);
			}
			else if (strcmp(command, "<") == 0){
				/*redirect stdin from file commandParam*/
				if ( (fd = open(commandParam, O_RDWR, S_IRUSR)) < 0 ){
					/*error opening file*/
					errnum = errno;
					fprintf(stderr, "- myshell: couldnt open file: %s | %s \n", commandParam, strerror(errnum));
					exit(EXIT_FAILURE);
				}
				dup2(fd, 0);
				close (fd);
			}


			/*execute the program*/
			if(execve(&args[0][0], args, environ) < 0){
				errnum = errno;
				fprintf(stderr, "- myshell: %s: %s\n", args[0], strerror(errnum));
				free(args);
				exit(EXIT_FAILURE);
			}
		}
		else{
			/*Parent, lets wait for the child to terminate*/

			do{
				waitingpid = waitpid(pid, &status, WUNTRACED);
			} while( !WIFEXITED(status) && !WIFSIGNALED(status));

			//printf("parent: %d\n", getpid());
			
		}
	}
	else{
		fprintf(stderr, "- myshell: %s: cant execute this file\n", args[0]);
	}

	return 0;
}



