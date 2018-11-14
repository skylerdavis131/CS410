#include <stdio.h>

#include "readMat.h"

int main(){
	int m = 0;
	int n = 0;
	int* np = &n;
	int* mp = &m;

	//int* vec = getVec(np);

	int** mat = getMat(np, mp);

}