#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "readMat.h"

int main()
{
	int numArgs = 0;
	int* numArgs_p = &numArgs;
	int* vec = getVec(numArgs_p);

	int i;
	for(i = 0; i<numArgs; i++){
		printf("%d\n", vec[i]);
	}
	return 0;
}