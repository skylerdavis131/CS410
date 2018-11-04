#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char* intToString(int input)
{
  char intLookup[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
  int inputN = input;
  int rem;
  int n;

  if (inputN == 0){
    n = 1;
  }
  else{
    while(inputN != 0)
    {
      inputN = inputN / 10;
      n = n + 1;
    }
  }
  char *intStr = (char *) malloc(sizeof(char) * n);

  if(input == 0){
    intStr[0] = '0';
  }
  else {
  int i = 0;
    while(input != 0)
    {
      rem = input % 10;
      intStr[i++] = intLookup[rem];
      input = input / 10;
    }
  }

  int size = strlen(intStr);
  char *returnString = (char *) malloc(sizeof(char) * size);
  int j;
  for (j = 0; j < size; j++){
    returnString[size - 1 - j] = intStr[j];
  }

  return returnString;
}

int stringToInt(char* line)
{
	int num = 0;
	int i;
	for(i = 0; i < strlen(line); i++){
		num = (num*10) + (line[i] -'0');
	}

	return num;
}



int vectorMult(int* mat1, int* mat2, int vectorLen)
{
	int result = 0;
	int i;

	for(i = 0; i < vectorLen; i++){
		//printf("mult: %d x %d \n", mat1[i], mat2[i]);
		result = result + (mat1[i] * mat2[i]);
	}
	//printf("\n");

	return result;
}

int main()
{
	int errnum;

	/* (n x m) * (m x p) = (n x p) */
	const int n = 2;
	const int m = 3;
	const int p = 2;

// DO THIS PART DYNAMICALLY FROM COMMAND LINE ///////
	/*mat 1 with n=2 rows and m=3 columns*/
	int mat1[n][m] = 	{{1 , 2 , 3},
						 {4 , 5 , 6}};
	/*mat 2 with m=3 rows and p=2 columns*/
	int mat2[m][p] =	{{7 , 8},
						 {9 , 10},
						 {11 , 12}};
/////////////////////////////////////////////////////

	//initialize a 2d array for the result of n x p size
	int* resultMat[n];
	int i;
	for(i = 0; i<n; i++){
		//p collumns in every row
		resultMat[i] = (int*)malloc(p * sizeof(int));
		memset(resultMat[i], 0, p * sizeof(int));
	}

//parallel Processing time: /////////////////////////////////////////////////////////////////////////

	pid_t pid[n][p]; // we will be running n*p processes to fill in result matrix
	int pipeEnds[n][p][2]; // we will be needing n*p pipes to communicate with n*p processes
	int j;
	int k;
	int mat2j[m]; //vector to hold column of mat2
	int childResult;
	char parentResult[2048];
	for(i = 0; i < n; i++){ //create one process for each 
		for(j = 0; j < p; j++){

			/*open pipe for proccess communication*/
			if( pipe(pipeEnds[i][j]) < 0){
				errnum = errno;
				fprintf(stderr, "-matmult_p: error making pipe | %s\n", strerror(errnum));
			}

			if((pid[i][j] = fork()) < 0){
				errnum = errno;
				fprintf(stderr,"-matmult_p: error forking | %s\n", strerror(errnum));
			}
			else if (pid[i][j] == 0){ //child
				//close the reading end of the pipe
				close(pipeEnds[i][j][0]);

				//create the column vector of mat2
				for (k = 0; k < m; k++){
					mat2j[k] = mat2[k][j];
				}

				childResult = vectorMult(mat1[i], mat2j, m);
				write(pipeEnds[i][j][1], intToString(childResult), strlen(intToString(childResult)));

				//close the pipe;
				close(pipeEnds[i][j][1]);
				exit(1);
			}
			else{//parent
				//close the writing end of the pipe
				close(pipeEnds[i][j][1]);

				//read from the pipe
				read(pipeEnds[i][j][0], parentResult, 2048);
				resultMat[i][j] = stringToInt(parentResult);

				//close the reading end of the pipe
				close(pipeEnds[i][j][0]);
			}

		}
	}

	pid_t waitID;
	int status;
	//check to make sure all of our child processes have finished before we go on:
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



	for(i = 0; i < n; i++){ 
		for(j = 0; j < p; j++){
			printf("%d ", resultMat[i][j]);
		}
		printf("\n");
	}


	return 0;
}