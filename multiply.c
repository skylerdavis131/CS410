#include <stdio.h>

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


int main(){
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

	// int j;
	// for(j = 0; j < numArgs1; j++){
	// 	fprintf(stderr, "%d    %d \n", vec1[j], vec2[j]);
	// }
	// fprintf(stderr, "\n%d\n\n", result);

	int i;
	write(1, intToString(result), strlen(intToString(result)));



	return 0;
}
