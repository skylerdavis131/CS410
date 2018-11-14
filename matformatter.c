#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "readMat.h"

int main()
{
	int n = 0;
	int m = 0;
	int* np = &n;
	int* mp = &m;

	int** mat = getMat(np, mp);

	int i, j;
	for(i = 0; i < m; i++){
		for(j = 0; j < n; j++){
			printf("%d ", mat[j][i]);
		}
		printf("\n");
	}
	
	return 0;
}