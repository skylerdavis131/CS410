#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "readMat.h"

extern char** environ;

// char* intToString(int input)
// {
//   char intLookup[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
//   int inputN = input;
//   int rem;
//   int n;

//   if (inputN == 0){
//     n = 1;
//   }
//   else{
//     while(inputN != 0)
//     {
//       inputN = inputN / 10;
//       n = n + 1;
//     }
//   }
//   char *intStr = (char *) malloc(sizeof(char) * n);

//   if(input == 0){
//     intStr[0] = '0';
//   }
//   else {
//   int i = 0;
//     while(input != 0)
//     {
//       rem = input % 10;
//       intStr[i++] = intLookup[rem];
//       input = input / 10;
//     }
//   }

//   int size = strlen(intStr);
//   char *returnString = (char *) malloc(sizeof(char) * size);
//   int j;
//   for (j = 0; j < size; j++){
//     returnString[size - 1 - j] = intStr[j];
//   }
//   free(intStr);

//   return returnString;
// }

// int stringToInt(char* line)
// {
// 	int num = 0;
// 	int i;
// 	for(i = 0; i < strlen(line); i++){
// 		num = (num*10) + (line[i] -'0');
// 	}

// 	return num;
// }



// int vectorMult(int* mat1, int* mat2, int vectorLen)
// {
// 	int result = 0;
// 	int i;

// 	for(i = 0; i < vectorLen; i++){
// 		//printf("mult: %d x %d \n", mat1[i], mat2[i]);
// 		result = result + (mat1[i] * mat2[i]);
// 	}
// 	//printf("\n");

// 	return result;
// }

int main()
{
	int errnum;

// FIND THIS PART DYNAMICALLY FROM COMMAND LINE ///////
	/* (n x m) * (m x p) = (n x p) */
	// const int n = 2;
	// const int m = 3;
	// const int p = 2;

	// /*mat 1 with n=2 rows and m=3 columns*/
	// int mat1[n][m] = 	{{1 , 2 , 3},
	// 					 {4 , 5 , 6}};
	// /*mat 2 with m=3 rows and p=2 columns*/
	// int mat2[m][p] =	{{7 , 8},
	// 					 {9 , 10},
	// 					 {11 , 12}};

	int n = 0;
	int* np = &n;
	int m1 = 0;
	int* m1p = &m1;
	int** mat1 = getMat(np, m1p);

	int m2 = 0;
	int* m2p = &m2;
	int p = 0;
	int* pp = &p;
	int** mat2 = getMat(m2p, pp);

	if(m1 != m2){
		fprintf(stderr, "-matmult_p: mat1 number of columns must equal mat2 number of rows\n");
		exit(EXIT_FAILURE);
	}

	//program would have exited if mat1 columns != mat2 rows
	//so we can assign m = mat1 number of columns
	int m = m1;
	


/////////////////////////////////////////////////////
	int i;

	//initialize a 2d array for the result of n x p size
	//dynamically allocated because we dont know the size
	//before compile time
	int** resultMat = (int**)malloc(n * sizeof(int*));
	memset(resultMat, 0, (n * sizeof(int*)));
	for(i = 0; i<n; i++){
		//p collumns in every row
		resultMat[i] = (int*)malloc(p * sizeof(int));
		memset(resultMat[i], 0, p * sizeof(int));
	}

//parallel Processing time: /////////////////////////////////////////////////////////////////////////

	// we will be running n*p processes to fill in result matrix
	// so we need n*p pid's 
	// Dynamically Allocated: pid[n][p]
	pid_t** pid = (pid_t**) malloc(n * sizeof(pid_t*));
	memset(pid, 0, (n * sizeof(pid_t*)));
	for(i = 0; i < n; i++){
		pid[i] = (pid_t*)malloc(p * sizeof(pid_t));
		memset(pid[i], 0, p* sizeof(pid_t));
	}

	// we will be needing n*p pipes to communicate with n*p processes
	//we need 2 of these. one for child->parent communication
	//and one for parent->child communication

	// dynamically allocated child->parent communiction: pipeCtoP[n][p][2] 
	int j;
	int*** pipeCtoP = (int***) malloc(n * sizeof(int**));
	memset(pipeCtoP, 0, (n * sizeof(int**)));
	for(i = 0; i < n; i++){
		pipeCtoP[i] = (int**)malloc(p * sizeof(int*));
		memset(pipeCtoP[i], 0, (p * sizeof(int*)));
		for(j = 0; j < 2; j++){
			pipeCtoP[i][j] = (int*)malloc(2 * sizeof(int));
			memset(pipeCtoP[i][j], 0, (2 * sizeof(int)));
		}
	}

	//dynamically allocated parent->child communication: pipePtoC[n][p][2]
	int*** pipePtoC = (int***) malloc(n * sizeof(int**));
	memset(pipePtoC, 0, (n * sizeof(int**)));
	for(i = 0; i < n; i++){
		pipePtoC[i] = (int**)malloc(p * sizeof(int*));
		memset(pipePtoC[i], 0, (p * sizeof(int*)));
		for(j = 0; j < 2; j++){
			pipePtoC[i][j] = (int*)malloc(2 * sizeof(int));
			memset(pipePtoC[i][j], 0, (2 * sizeof(int)));
		}
	}

	//vector to hold column of mat2
	//Dynamically Alocated: int mat2col[m]; 
	int* mat2col = (int*)malloc(m * sizeof(int));
	memset(mat2col, 0, m * sizeof(int));

	//for the exec call:
	char* arg1 = "multiply\0";
	char* arg2 = NULL;
	char* execArgs[2] = {arg1, arg2};

	//Shared Memory: 
	// key_t key = 'm';
	// int shmMatid;
	// int** shmMat;
	// shmMatid = shmget(key, sizeof(*int[n]), IPC_CREAT);
	// if(shmMatid == -1){
	// 	fprintf(stderr,"-matmult_p: error with shmget() | %s\n", strerror(errno));
	// 	exit(EXIT_FAILURE);
	// }
	// else{ /*create the shared memory segment*/
	// 	shmMat = shmat(shmMatid, 0, 0);
	// 	shmMat[0] = 1;
	// 	shmMat[1] = 2;
	// 	shmMat[2] = 3;
	// 	shmMat[3] = 4;
	// }

	int k;
	int childResult;
	char parentResult[2048];
	for(i = 0; i < n; i++){ //create one process for each 
		for(j = 0; j < p; j++){

			// /*open pipe for proccess communication*/
			// if( pipe(pipeCtoP[i][j]) < 0){
			// 	errnum = errno;
			// 	fprintf(stderr, "-matmult_p: error making pipe | %s\n", strerror(errnum));
			// }

			// if((pid[i][j] = fork()) < 0){
			// 	errnum = errno;
			// 	fprintf(stderr,"-matmult_p: error forking | %s\n", strerror(errnum));
			// }
			// else if (pid[i][j] == 0){ //child
			// 	//close the reading end of the pipe
			// 	close(pipeCtoP[i][j][0]);

			// 	//create the column vector of mat2
			// 	for (k = 0; k < m; k++){
			// 		mat2col[k] = mat2[k][j];
			// 	}

			// 	childResult = vectorMult(mat1[i], mat2col, m);
			// 	write(pipeCtoP[i][j][1], intToString(childResult), strlen(intToString(childResult)));

			// 	//close the pipe;
			// 	close(pipeCtoP[i][j][1]);
			// 	exit(1);
			// }
			// else{//parent
			// 	//close the writing end of the pipe
			// 	close(pipeCtoP[i][j][1]);

			// 	//read from the pipe
			// 	read(pipeCtoP[i][j][0], parentResult, 2048);
			// 	resultMat[i][j] = stringToInt(parentResult);

			// 	//close the reading end of the pipe
			// 	close(pipeCtoP[i][j][0]);
			// }




			/*open pipes for proccess communication*/
			if( pipe(pipeCtoP[i][j]) < 0){
				errnum = errno;
				fprintf(stderr, "-matmult_p: error making pipe | %s\n", strerror(errnum));
			}
			if( pipe(pipePtoC[i][j]) < 0 ){
				errnum = errno;
				fprintf(stderr, "-matmult_p: error making pipe | %s\n", strerror(errnum));
			}


			if((pid[i][j] = fork()) < 0){
				errnum = errno;
				fprintf(stderr,"-matmult_p: error forking | %s\n", strerror(errnum));
			}
			else if (pid[i][j] == 0){ //child
				//close the reading end of pipeCtoP
				close(pipeCtoP[i][j][0]);

				//close the writing end of pipePtoC
				close(pipePtoC[i][j][1]);

				//duplicate stdout onto the pipeCtoP so 
				//stdout of child communicates directly with the parent
				dup2(pipeCtoP[i][j][1], 1);

				//duplicate stdin onto the pipePtoC so
				//stdout of parent communicates directly with the child.
				dup2(pipePtoC[i][j][0], 0);

				if(execve(&execArgs[0][0], execArgs, environ) < 0){
					fprintf(stderr, "-matmult_p: multiply exec error: | %s\n", strerror(errno));
				}

				// childResult = vectorMult(mat1[i], mat2col, m);
				// write(pipeCtoP[i][j][1], intToString(childResult), strlen(intToString(childResult)));

				//close the pipes;
				close(pipePtoC[i][j][0]);
				close(pipeCtoP[i][j][1]);
				exit(EXIT_SUCCESS);
			}
			else{//parent
				//close the writing end of pipeCtoP
				close(pipeCtoP[i][j][1]);

				//close the reading end of pipePtoC
				close(pipePtoC[i][j][0]);

				//create the column vector of mat2
				for (k = 0; k < m; k++){
					mat2col[k] = mat2[k][j];
				}

				//send the first array
				for(k = 0; k < m; k++){
					printf("%s ", intToString(mat1[i][k]));
					write(pipePtoC[i][j][1], intToString(mat1[i][k]), strlen(intToString(mat1[i][k])));
					write(pipePtoC[i][j][1], " ", 1);
				}
				printf("x ");
				write(pipePtoC[i][j][1], "\n", 1);

				//send the second array
				for(k = 0; k < m; k++){
					printf("%s ", intToString(mat2col[k]));
					write(pipePtoC[i][j][1], intToString(mat2col[k]), strlen(intToString(mat2col[k])));
					write(pipePtoC[i][j][1], " ", 1);
				}
				printf("\n");
				write(pipePtoC[i][j][1], "\n", 1);

				//read from the pipe
				read(pipeCtoP[i][j][0], parentResult, 2048);
				resultMat[i][j] = stringToInt(parentResult);

				//close the pipes
				close(pipeCtoP[i][j][0]);
				close(pipePtoC[i][j][1]);
			}



		}
	}

	
	//check to make sure all of our child processes have finished before we go on:
	pid_t waitID;
	int status;
	for(i = 0; i<n; i++){
		for(j = 0; j<p; j++){
			waitID = waitpid(pid[i][j], &status, WNOHANG);
			if(waitID == 0){
				//child hasnt exited yet; lets wait for it
				do{
					waitID = waitpid(pid[i][j], &status, WUNTRACED);
				} while( !WIFEXITED(status) && !WIFSIGNALED(status));
			}
			else if (waitID == -1){
				//error with waiting for child
				errnum = errno;
				fprintf(stderr, "-matmult_p: error waiting for children to finish | %s", strerror(errnum));
			}
		}
	}

//Done Parallel Processing///////////////////////////////////////////////////////////////////////////////

//print the resultant matrix to stdout: 
	for(i = 0; i < n; i++){ 
		for(j = 0; j < p; j++){
			printf("%d ", resultMat[i][j]);
		}
		printf("\n");
	}

//free dynamically allocated memory: /////////////////////////////////////////////////////////////////////
	for(i = 0; i < n; i++){
		free(resultMat[i]);
		free(pid[i]);
		for(j = 0; j < 2; j++){
			free(pipeCtoP[i][j]);
			free(pipePtoC[i][j]);
		}
		free(pipeCtoP[i]);
		free(pipePtoC[i]);
	}
	free(resultMat);
	free(pid);
	free(pipeCtoP);
	free(pipePtoC);
	free(mat2col);

//End the program execution ///////////////////////////////////////////////////////////////////////////////


	return 0;
}