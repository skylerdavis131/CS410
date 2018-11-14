/*
	Jack Belmont, Skyler Davis, Varun

Important Functions:
	getVec(int* numArgs)
		+ reads a vector from the command line
		+ pass a pointer to an integer that has the value of 0 to return the number of arguments

	getMat(int* n, int* m)
		+ reads a matrix from the command line
		+ pass pointer to n and m which will hold the dimensions of the matrix after program execution
			n rows by m collumns

*/

#ifndef READMAT_H
#define READMAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define NORMAL_BUFFSIZE 1024;

int* getVec(int* numArgs);
//int** getMat(int* n, int* m);

int stringToInt(char* line);
char* myGetLine(void);
char* intToString(int input);

/*README stringToInt()
	+ converts a string integer to an int
*/
int stringToInt(char* line)
{
	int num = 0;
	int sign = 1;
	int i = 0;
	if (line[0] == '-'){
		sign = -1;
		i++;
	}
	while(line[i] != '\0'){
		num = (num*10) + (line[i] - '0');
		i++;
	}
	num = sign * num;

	return num;
}

/*README: intToString()
	+ converts an integer to a string for printing

*/
char* intToString(int input)
{
	char intLookup[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
	int remainder;
	int sign = 1;
	int index = 0;

	char* revStr = malloc(sizeof(char) * 200);
	memset(revStr, 0, sizeof(char) * 200);

	if(input == 0){
		revStr[index] = '0';
		index++;
		revStr[index] = '\0';
		return revStr;
	}

	if(input < 0){
		sign = -1;
		input = input * -1;
	}

	while(input != 0){
		remainder = input % 10;
		revStr[index] = intLookup[remainder];
		index++;
		input = input / 10;
	}

	if(sign == -1){
		revStr[index] = '-';
		index++;
	}
	char* str = malloc(sizeof(char) * 200);
	memset(str, 0, sizeof(char) * 200);

	int i;
	for(i = 0; i < index; i++){
		str[i] = revStr[index - 1 -i];
	}
	str[i] = '\0';

	free(revStr);

	return str;

}


int** getMat(int* ni, int* mi)
{
	(*ni) = 0;
	(*mi) = 0;

	int space;

	int** retMat;

	//get the first matrix vector row to find m
	int m = 0;
	int* mp = &m;
	int* vec;
	if(!(vec = getVec(mp))){
		while(!(vec = getVec(mp))){
			//keep getting input
		}
	}
	int n = m * 2;

	//(*mi) value should not change after this point;
	(*mi) = m;

	//allocate space for the return matrix with  m*2 rows and m collumns
	space = n * 10;
	if( (retMat = (int**)malloc(space * sizeof(int*))) < 0 ){
		fprintf(stderr, "-getMat: error with malloc | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(retMat, 0, space * sizeof(int*));
	int i;
	for(i = 0; i < space; i++){
		if ( (retMat[i] = (int*)malloc(m * sizeof(int))) < 0 ){
			fprintf(stderr, "-getMat: error with malloc | %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		memset(retMat[i], 0, m * sizeof(int));
	}

	i = 0;
	retMat[i] = vec;
	(*ni)++;
	i++;


	while((vec = getVec(mp))){ //will continue until getVec() returns a nullptr
		if( (*mp) != (*mi) ){
			//check if same number of collumns as first vector inputted
			fprintf(stderr, "-getMat: matrix dimensions do not match up.\n");
			for(i = 0; i < n; i++){
				free(retMat[i]);
			}
			free(retMat);
			exit(EXIT_FAILURE);
		}

		retMat[i] = vec;
		(*ni)++;
		i++;

		if(i >= space){
			space = space * 2;
			if ( !(retMat = (int**)realloc(retMat, space)) ){
				fprintf(stderr, "-getMat: error reallocating matrix space | %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}

	}



	return retMat;
}


/* README: getVec()
	+ reads a line of a matrix in from the command line as a array of ints

*/
int* getVec(int* numArgs){
	if((*numArgs) != 0){
		(*numArgs) = 0;
	}
	char* line = myGetLine();
	char** args;
	int lineLen, i, j, position;
	lineLen = strlen(line) + 1;

	//initialize the vector
	int* vec;
	if ( (vec = malloc(lineLen * sizeof(int))) <0 ){
		fprintf(stderr, "-getVec: error with malloc for vector | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(strcmp(line, "\0") == 0){
		/*return a nullptr if myGetLine() returns a char* == "\0" */
		vec = NULL;
		return vec;
	}

	//////////////////////////////////////////////////////////////////////////////////////
	/*before getting the vec array we must first split the input line into
		separate string int tokens*/
	lineLen = strlen(line) + 1;
	if ( (args = (char**)malloc(lineLen * sizeof(char*))) < 0){
		fprintf(stderr, "-getVec: error with malloc | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(args, 0, (lineLen * sizeof(char*)));
	for(i=0; i<lineLen; i++){
		if ( (args[i] = (char*)malloc(lineLen * sizeof(char*))) < 0){
			fprintf(stderr, "-getVec: error with malloc | %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
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
//////////////////////////////////////////////////////
	//convert string array to integer vector
	for(i = 0; i < *numArgs; i++){
		vec[i] = stringToInt(args[i]);
	}
	
	free(args);
	return vec;
}


char* myGetLine(void)
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


	/*get the line and check for termination & buffer overflow*/
	while(1){
		c = getchar();
		if (c == '\n' || c == EOF){
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
	//////no we have a line in lineBuff


	return lineBuff;
}



#endif