#ifndef SHELLFUNCS_H
#define SHELLFUNCS_H

	#define NORMAL_BUFFSIZE 1024;

	char* getCommand(void);

	char** myParse(char* line, int* numArgs);

	int	myExec(char** args, int numArgs);

#endif