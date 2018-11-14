#include <stdio.h>

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


int main(){
	
}
