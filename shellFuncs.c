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
#include <signal.h>
#include <setjmp.h>

#include "shellFuncs.h"

extern int* 			childID;
extern int 				numChildren, childNum;

extern char** 			environ;

extern sigjmp_buf		jmpbuf;
extern sig_atomic_t		canjump;

static int SAVED_IN = 0;
static int SAVED_OUT = 1;
static int SAVED_ERR = 1;




/////////////////////////////////////////////////////////////////////////////////////
/*Signal Handler functions*/

void SIGINT_handler(int sig){
	int i;

	if (sig == SIGINT){
		for (i = 0; i < childNum; i++){
			kill( childID[i], SIGINT );
		}
	}
	else{
		fprintf(stderr, "-SIGINT_handler: error handling SIGINT\n");
		exit(EXIT_FAILURE);
	}


	printf("\n");

	if(canjump)
		siglongjmp(jmpbuf, 1);


	return;
}

void SIGCHLD_handler(int sig){
	pid_t pid;

	/*Stop the child from being a zombie by calling wait*/
	pid = wait(NULL);
	removeChildID(pid);



	// if(canjump)
	// 	siglongjmp(jmpbuf, 1);

	return;
}




/////////////////////////////////////////////////////////////////////////////////////

/*README: removeChildID()
	+ Called to remove the terminated background child from the childID array
*/
void removeChildID(int id)
{
	int i;


	// printf("trying to remove: %d\n", id);
	// printf("before remove: ");
	// for(i = 0; i < childNum; i++)
	// 	printf("%d, ", childID[i]);
	// printf("\n");


	for(i = 0; i < childNum; i++){
		if(childID[i] == id){
			for(; i+1 < childNum; i++){
				childID[i] = childID[i+1];
			}
			childID[i++] = 0;
		}
	}
	childNum--;


	// printf("after remove: ");
	// for(i = 0; i < childNum; i++)
	// 	printf("%d, ", childID[i]);
	// printf("\n");

	return;
}


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
					// free(childID);
					exit(EXIT_SUCCESS);
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
			exit(EXIT_SUCCESS);
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

/*README setUpAndExecute()
	+ performs some basic error checking on the input
	+ calls the recursive function powerExec() to execute the commands
*/
int setUpAndExecute(char** args, int numArgs)
{
	int background;
	int i;

	/*Error check the command line arguments*/
	for(i=0; i < numArgs; i++){
		if( strcmp(args[i], ">")  == 0  || strcmp(args[i], "1>") == 0 ||
			strcmp(args[i], "2>") == 0  || strcmp(args[i], "&>") == 0 ||
			strcmp(args[i], "<")  == 0  || strcmp(args[i], "|")  == 0 ||
			strcmp(args[i], ";")  == 0  || strcmp(args[i], "&")  == 0 ){
				/*Can not have two shell commands next to each other*/
				/*& symbol must be last character*/
				if(strcmp(args[i], "&") == 0){
					if(i < numArgs - 1){
						fprintf(stderr,"-setUpAndExecute: & character must be last character\n");
						return 1;
					}
				}
				else if (i < numArgs -1){
					if( strcmp(args[i+1], ">")  == 0  || strcmp(args[i+1], "1>") == 0 ||
						strcmp(args[i+1], "2>") == 0  || strcmp(args[i+1], "&>") == 0 ||
						strcmp(args[i+1], "<")  == 0  || strcmp(args[i+1], "|")  == 0 ||
						strcmp(args[i+1], ";")  == 0 ){

							fprintf(stderr, "-setUpAndExecute: can not have two commands next to each other\n");
							return 1;
					}
				}

		}
	}

	if( strcmp(args[numArgs-1], "&") == 0){
		background = 1;
		numArgs--;
	}
	else
		background = 0;

	if (strcmp(args[numArgs-1], ";") == 0){
		/*ignore the last ;*/
		numArgs--;
	}
	else if (strcmp(args[numArgs-1], ">")  == 0  || strcmp(args[numArgs-1], "1>") == 0 ||
			 strcmp(args[numArgs-1], "2>") == 0  || strcmp(args[numArgs-1], "&>") == 0 ||
			 strcmp(args[numArgs-1], "<")  == 0  || strcmp(args[numArgs-1], "|")  == 0){
		fprintf(stderr, "-setUpAndExecute: final argument can not be a command\n");
		return 1;
	}


	powerExec(args, 0, numArgs, background, STDIN_FILENO);

	return 0;
}


/*README: powerExec()
	+ recursive function to execute the command line arguments based on IO manipulation given

*/
int powerExec(char** args, int execArgIndex, int numArgs, int background, int inputFd)
{
	char** execArgs;
	int numExecArgs;

	pid_t pid, waitingpid;
	int status;

	int fd;
	int pipefd[2];

	int i;


	// /*set up and get the execArgs*/
	execArgs = (char**)malloc(numArgs*2 * sizeof(char*));
	memset(execArgs, 0, numArgs*2*sizeof(char*));

	/*special case*/
	if (numArgs == 1){
		doPowerExec(args, background, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, pipefd, 0);
		return 0;
	}

	// /*get the execArgs*/
	numExecArgs = 0;
	i = execArgIndex;
	while(i < numArgs){
		if( strcmp(args[i], ";")  != 0 && strcmp(args[i], ">")  != 0 &&
			strcmp(args[i], "1>") != 0 && strcmp(args[i], "2>") != 0 && 
			strcmp(args[i], "&>") != 0 && strcmp(args[i], "<")  != 0 &&
			strcmp(args[i], "|")  != 0){
				/*Fill in the execArgs array*/
				execArgs[numExecArgs] = args[i];
				numExecArgs++;
				i++;
		}
		else{
			break;
		}
	}
	execArgs[i] =NULL;
	if (i >= numArgs){
		/*Base Case reached: Last exec argument*/
		doPowerExec(execArgs, background, inputFd, STDOUT_FILENO, STDERR_FILENO, pipefd, 0);
	}
	else{
		if ( strcmp(args[i], ";") == 0 ){
			/*We are not redirecting any input/output*/
			dup2(SAVED_IN, 0);
			dup2(SAVED_OUT, 1);
			dup2(SAVED_ERR, 2);
			//do the exec with input from inputFd
			doPowerExec(execArgs, background, inputFd, STDOUT_FILENO, STDERR_FILENO, pipefd, 0);

			//we are not piping so recurse with input from stdin for next arg
			powerExec(args, i+1, numArgs, background, STDIN_FILENO);
		}
		else if ( strcmp(args[i], ">") == 0 ){
			/*Redirect stdout to next file*/


			/*move to file name and open it*/
			i++;
			if((fd = open(args[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "-myshell: error opening file for execution output| %s\n", strerror(errno));
				return 1;
			}

			//do exec with input from recursion, output to fd
			doPowerExec(execArgs, background, inputFd, fd, STDERR_FILENO, pipefd, 0);

			//make sure we are not at the end of the string
			if (i + 1 >= numArgs){
				return 0;
			}

			i++;
			if (strcmp(args[i], ";") == 0){
				if(i + 1 >= numArgs)
					return 0;
				else
					powerExec(args, i+1, numArgs, background, STDIN_FILENO);
			}
			else if (strcmp(args[i], "|") == 0){
				/*we are piping the output*/
			}
			else{
				fprintf(stderr, "-myshell: error seperating commands\n");
			}
			

		}
		else if ( strcmp(args[i], "1>") == 0 ){
			/*Redirect stdout to next file*/

			/*move to file name and open it*/
			i++;
			if((fd = open(args[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "-myshell: error opening file for execution output| %s\n", strerror(errno));
				return 1;
			}

			//do exec with input from recursion, output to fd
			doPowerExec(execArgs, background, inputFd, fd, STDERR_FILENO, pipefd, 0);

			//make sure we are not at the end of the string
			if (i + 1 >= numArgs){
				return 0;
			}

			i++;
			if (strcmp(args[i], ";") == 0){
				if(i + 1 >= numArgs)
					return 0;
				else
					powerExec(args, i+1, numArgs, background, STDIN_FILENO);
			}
			else if (strcmp(args[i], "|") == 0){
				/*we are piping the output*/
			}
			else{
				fprintf(stderr, "-myshell: error seperating commands\n");
			}


		}
		else if ( strcmp(args[i], "2>") == 0 ){
			/*Redirect stderr to next file*/

			/*move to file name and open it*/
			i++;
			if((fd = open(args[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "-myshell: error opening file for execution errors | %s\n", strerror(errno));
				return 1;
			}

			//do exec with input from recursion, stderr to fd
			doPowerExec(execArgs, background, inputFd, STDOUT_FILENO, fd, pipefd, 0);

			//make sure we are not at the end of the string
			if (i + 1 >= numArgs){
				return 0;
			}

			i++;
			if (strcmp(args[i], ";") == 0){
				if(i + 1 >= numArgs)
					return 0;
				else
					powerExec(args, i+1, numArgs, background, STDIN_FILENO);
			}
			else if (strcmp(args[i], "|") == 0){
				/*we are piping the output*/
			}
			else{
				fprintf(stderr, "-myshell: error seperating commands\n");
			}

		}
		else if ( strcmp(args[i], "&>") == 0 ){
			/*Redirect stdout and stderr to next file*/

			/*move to file name and open it*/
			i++;
			if((fd = open(args[i], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "-myshell: error opening file for execution output | %s\n", strerror(errno));
				return 1;
			}

			//do exec with input from recursion, stderr and stdout to fd
			doPowerExec(execArgs, background, inputFd, fd, fd, pipefd, 0);

			//make sure we are not at the end of the string
			if (i + 1 >= numArgs){
				return 0;
			}

			i++;
			if (strcmp(args[i], ";") == 0){
				if(i + 1 >= numArgs)
					return 0;
				else
					powerExec(args, i+1, numArgs, background, STDIN_FILENO);
			}
			else if (strcmp(args[i], "|") == 0){
				/*we are piping the output*/
			}
			else{
				fprintf(stderr, "-myshell: error seperating commands\n");
			}


		}
		else if ( strcmp(args[i], "<") == 0 ){
			/*Redirect stdin from next file*/

			/*move to file name and open it*/
			i++;
			if((fd = open(args[i], O_RDWR, S_IRUSR | S_IWUSR)) < 0){
				fprintf(stderr, "-myshell: error opening file for execution input | %s\n", strerror(errno));
				return 1;
			}

			if (i + 1 < numArgs){ /*there are more arguments*/
				if (strcmp(args[i+1], "|") == 0){
					i++;
					doPowerExec(execArgs, background, fd, STDOUT_FILENO, STDERR_FILENO, pipefd, 1);
					powerExec(args, i+1, numArgs, background, pipefd[0]);
				}
				else if (strcmp(args[i+1], ";") == 0) {
					doPowerExec(execArgs, background, fd, STDOUT_FILENO, STDERR_FILENO, pipefd, 0);
					powerExec(args, i+1, numArgs, background, STDIN_FILENO);
				}
				else{
					fprintf(stderr, "-myshell: error seperating commands\n");
				}

			}
			else{ /*end of arguments*/
				doPowerExec(execArgs, background, fd, STDOUT_FILENO, STDERR_FILENO, pipefd, 0);
			}


		}
		else if ( strcmp(args[i], "|") == 0 ){
			/*Redirect stdout to stdin of next command*/
			doPowerExec(execArgs, background, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO, pipefd, 1);
			powerExec(args, i+1, numArgs, background, pipefd[0]);

		}
	}

 	


	free(execArgs);


	return 0;
}


/*README: doPowerExec()
	+ function that executes a single command from the recursive function
		+ uses the filedescriptors passed by the inFd, outFd, errFd instead of stdin, stdout, stderr
			if they are specified as different than stdi/o
		+ if do pipe is high (~0) then we pipe the output to pipefd[]
*/
void doPowerExec(char** args, int background, int inFd, int outFd, int errFd, int pipefd[2], int doPipe)
{
	int status;
	pid_t pid, wpid;

	struct stat statbuff;

	if(stat(args[0], &statbuff) < 0){
		/*not an existing file*/
		fprintf(stderr, "- myshell: %s: %s\n", args[0], strerror(errno));
	}
	else if(statbuff.st_mode & S_IXUSR){
		/*we can execute the file*/

		if(doPipe){
			if (pipe(pipefd) == -1){
				fprintf(stderr, "-myshell: failed to make pipe | %s\n", strerror(errno));
				return;
			}
		}

		/*fork and exec*/
		if ((pid = fork()) < 0){
			fprintf(stderr, "- myshell: error forking | %s\n", strerror(errno));
			return;
		}
		else if (pid == 0){/*child*/

			/*set up input and output*/
			if (inFd != STDIN_FILENO){
				//printf("setting input\n");
				dup2(inFd, 0);
				close(inFd);
			}
			if (outFd != STDOUT_FILENO){
				//printf("setting output\n");
				dup2(outFd, 1);
				close(outFd);
			}
			if (errFd != STDERR_FILENO){
				//printf("setting errput\n");
				dup2(errFd, 2);
				close(errFd);
			}

			if(background){
				/*move the process out of the forground*/
				setpgid(0,0);
			}

			if(doPipe){
				close(pipefd[0]);
				dup2(pipefd[1], 1);
			}

			/*execute the program*/
			if(execve(&args[0][0], args, environ) < 0){
				fprintf(stderr, "- myshell: %s: %s\n", args[0], strerror(errno));
				free(args);
				exit(EXIT_FAILURE);
			}
		}
		else{/*parent*/
			/*add the child ID to keep track of it*/	

			if (doPipe){
				close(pipefd[1]);
			}


			if(!background){
				childID[childNum] = pid;
				childNum++;
				if(childNum > numChildren){
					numChildren = numChildren * 2;
					if( !(childID = realloc(childID, numChildren)) ){
						fprintf(stderr, "-myshell: realloc for background children | %s\n", strerror(errno));
						exit(EXIT_FAILURE);
					}
				}
				do{
					wpid = waitpid(pid, &status, WUNTRACED);
				} while( !WIFEXITED(status) && !WIFSIGNALED(status));
			}
		}
	}
	else{
		fprintf(stderr, "- myshell: %s: cant execute this file\n", args[0]);
	}



	return;
}






