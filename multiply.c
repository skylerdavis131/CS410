#include <stdio.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#include "readMat.h"

int vectorMult(int* mat1, int* mat2, int numArgs1, int numArgs2)
{
	int result = 0;
	int i;

	if(numArgs1 != numArgs2){
		fprintf(stderr, "-vectorMult: cant multiply arrays of different length\n");
		exit(0);
	}

	for(i = 0; i < numArgs1; i++){
		//printf("mult: %d x %d \n", mat1[i], mat2[i]);
		result = result + (mat1[i] * mat2[i]);
	}
	//printf("\n");

	return result;
}

/*for use with shared memory pass on command line:
	+ shmRetMat_id
	+ shmMat1_id
	+ shmMat2_id
	+ mat1_row_number
	+ mat2_column_number
	+ n
	+ m
	+ p

	in the following format:
	./multiply shmRetMat_id shmMat1_id shmMat2_id mat1_row_number mat2_column_number n m p
*/
int main(int argc, char** args){

	if(argc <= 1){
		int numArgs1 = 0;
		int* numArgs1_p = &numArgs1;
		int numArgs2 = 0;
		int* numArgs2_p = &numArgs2;

		int* vec1 = getVec(numArgs1_p);
		int* vec2 = getVec(numArgs2_p);

		int result;

		if(numArgs1 != numArgs2){
			fprintf(stderr, "-multiply: cant multiply arrays of different length\n");
			exit(0);
		}
		else{
			result = vectorMult(vec1, vec2, numArgs1, numArgs2);
		}

		write(1, intToString(result), strlen(intToString(result)));
	}
	else if (argc == 9){
		int i;

		//convert the shm_ids to int equivalents
		int shmRetMat_id = stringToInt(args[1]);
		int shmMat1_id = stringToInt(args[2]);
		int shmMat2_id = stringToInt(args[3]);

		//convert row number and column numbers to integers
		int mat1_r = stringToInt(args[4]);
		int mat2_c = stringToInt(args[5]);

		//convert n m p
		int n = stringToInt(args[6]);
		int m = stringToInt(args[7]);
		int p = stringToInt(args[8]);

		//attach the shared memory segments:
		int* retMat;
		int* mat1;
		int* mat2;
		retMat = (int*)shmat(shmRetMat_id, 0, 0);
		if(retMat == (void*)-1){
			fprintf(stderr, "-multiply: error attaching shared memory | %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		mat1 = (int*)shmat(shmMat1_id, 0, 0);
		if (mat1 == (void*)-1){
			fprintf(stderr, "-multiply: error attaching shared memory | %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		mat2 = (int*)shmat(shmMat2_id, 0, 0);
		if(mat2 == (void*)-1){
			fprintf(stderr, "-multiply: error attaching shared memory | %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		//get the collumn from mat2 in a single array
		int* mat2col = (int*)malloc(m * sizeof(int));
		memset(mat2col, 0, m * sizeof(int));
		for (i = 0; i < m; i++){
			mat2col[i] = *(mat2 + p*i + mat2_c);
		}

		//get the row from mat1 in a single array
		int* mat1row = (int*)malloc(m * sizeof(int));
		memset(mat1row, 0, m * sizeof(int));
		for(i = 0; i < m; i++){
			mat1row[i] = *(mat1 + m*mat1_r + i);
		}

		int multResult = vectorMult(mat1row, mat2col, m, m);

		*(retMat + n*mat1_r + mat2_c) = multResult;



		//detatch shared memory segments
		shmdt( (void*)retMat );
		shmdt( (void*)mat1 );
		shmdt( (void*)mat2 );
	}


	return 0;
}
