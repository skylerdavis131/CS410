#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "readMat.h"

extern char** environ;

int main()
{
	int errnum;
	int i, j;

	//read in first matrix
	int n = 0;
	int* np = &n;
	int m1 = 0;
	int* m1p = &m1;
	int** mat1 = getMat(np, m1p);

	//read in second matrix
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

	//vector to hold column of mat2
	//Dynamically Alocated: int mat2col[m]; 
	int* mat2col = (int*)malloc(m * sizeof(int));
	memset(mat2col, 0, m * sizeof(int));

	//Shared Memory: ///////////////////////////////////////////////////////////

	int shmRetMat_id;
	int* shmRetMat;
	int shmRetMatNumElements = n*p*sizeof(int);
	shmRetMat_id = shmget(IPC_PRIVATE, shmRetMatNumElements, 0600);
	if(shmRetMat_id < 0){
		fprintf(stderr,"-matmult_p: error with shmget() for output matrix | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	shmRetMat = (int*)shmat(shmRetMat_id, 0, 0);
	if(shmRetMat == (void*)-1){
		fprintf(stderr, "-matmult_p: error with shmat() for output matrix | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < n*p; i++){
		*(shmRetMat + i) = 0;
	}

	//copy mat1 and mat2 into shared memory
	int shmMat1_id, shmMat2_id;
	int* shmMat1;
	int* shmMat2;
	shmMat1_id = shmget(IPC_PRIVATE, n*m, 0600);
	shmMat2_id = shmget(IPC_PRIVATE, m*p, 0600);
	if(shmMat1_id < 0 || shmMat2_id < 0){
		fprintf(stderr, "-matmult_p: error with shmget() for input matrices | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	shmMat1 = (int*)shmat(shmMat1_id, 0, 0);
	if (shmMat1 == (void*)-1){
		fprintf(stderr, "-matmult_p: error with shmat() for input matrix1 | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	shmMat2 = (int*)shmat(shmMat2_id, 0, 0);
	if (shmMat2 == (void*)-1){
		fprintf(stderr, "-matmult_p: error with shmat() for input matrix2 | %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//copy mat1 into shared memory
	for(i = 0; i < n; i++){
		for(j = 0; j < m; j++){
			*(shmMat1 + i*m + j) = mat1[i][j];
		}
	}
	//copy mat2 into shared memory
	for(i = 0; i < m; i++){
		for(j = 0; j < p; j++){
			*(shmMat2 + i*p + j) = mat2[i][j];
		}
	}


	///////////////////////////////////////////////////////////////////////////
	//arguments for the exec call
	char* arg1 = "multiply\0";
	char* arg10 = NULL;
	char* execArgs[10];
	execArgs[0] = arg1;
	execArgs[1] = intToString(shmRetMat_id);
	execArgs[2] = intToString(shmMat1_id);
	execArgs[3] = intToString(shmMat2_id);
	//args[4] filled in by loop | mat1_row to multiply
	//args[5] filled in by loop | mat2_column to multiply
	execArgs[6] = intToString(n);
	execArgs[7] = intToString(m);
	execArgs[8] = intToString(p);
	execArgs[9] = arg10;

	////////////////////////////////


	///////////////////////////////////////////////////////////////////////////

	int k;
	int childResult;
	char parentResult[2048];
	for(i = 0; i < n; i++){ //create one process for each 
		for(j = 0; j < p; j++){

			//row of mat1 to multiply
			execArgs[4] = intToString(i);
			//column of mat2 to multiply
			execArgs[5] = intToString(j);

			if((pid[i][j] = fork()) < 0){
				errnum = errno;
				fprintf(stderr,"-matmult_p: error forking | %s\n", strerror(errnum));
			}
			else if (pid[i][j] == 0){ //child
				if(execve(&execArgs[0][0], execArgs, environ) < 0){
					fprintf(stderr, "-matmult_p: multiply exec error: | %s\n", strerror(errno));
				}
				
				exit(1);
			}
			else{//parent
				//we dont really have to do anything in the parent

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
				fprintf(stderr, "-matmult_p: error waiting for children to finish | %s\n", strerror(errnum));
			}
		}
	}

//Done Parallel Processing///////////////////////////////////////////////////////////////////////////////

//print the resultant matrix to stdout: 
	for(i = 0; i < n; i++){
		for(j = 0; j < p; j++)
			printf("%d ", *(shmRetMat + i*n + j));
		//printf("%d ",*(shmRetMat + i));
		printf("\n");
	}

//free dynamically allocated memory: /////////////////////////////////////////////////////////////////////
	for(i = 0; i < n; i++){
		free(pid[i]);
	}
	free(pid);
	free(mat2col);

//free shared memory: /////////////////////////////////////////////////////////////////////////////////////
	shmctl(shmRetMat_id, IPC_RMID, 0);
	shmctl(shmMat1_id, IPC_RMID, 0);
	shmctl(shmMat2_id, IPC_RMID, 0);

	shmdt( (void*)shmRetMat );
	shmdt( (void*)shmMat1 );
	shmdt( (void*)shmMat2 );

//End the program execution ///////////////////////////////////////////////////////////////////////////////


	return 0;
}