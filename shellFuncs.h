#ifndef SHELLFUNCS_H
#define SHELLFUNCS_H

	#define NORMAL_BUFFSIZE 1024;

	char* getCommand(void);

	char** myParse(char* line, int* numArgs);

	int executeCommands(char** args, int numArgs);

	int	myExec(char** args, char* command, char* commandParam);

#endif