#ifndef SHELLFUNCS_H
#define SHELLFUNCS_H

	#define NORMAL_BUFFSIZE 1024;

	void SIGINT_handler(int sig);

	void SIGCHLD_handler(int sig);

	void removeChildID(int id);

	char* getCommand(void);

	char** myParse(char* line, int* numArgs);

	int setUpAndExecute(char** args, int numArgs);

	int powerExec(char** args, int execArgIndex, int numArgs, int background, int inputFd);
	
	void doPowerExec(char** args, int background, int inFd, int outFd, int errFd, int pipefd[2], int doPipe);

#endif