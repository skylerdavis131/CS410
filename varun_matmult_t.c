#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "readMat.h"
ucontext_t uct_main;
ucontext_t* uct_func;
int ** mat1;
int ** mat2;
int ** result;
//This Function is for Matrix Multiplication
int varun_vectorMult(int* vec1, int* vec2, int numArgs1, int numArgs2)
{
	int result = 0;
	int i;

	if(numArgs1 != numArgs2){
		fprintf(stderr, "-vectorMult: cant multiply arrays of different length\n");
		exit(0);
	}

	for(i = 0; i < numArgs1; i++){
		//printf("mult: %d x %d \n", mat1[i], mat2[i]);
		result = result + (vec1[i] * vec2[i]);
	}
	//printf("\n");

	return result;
}

void varun_thread(int n, int m, int p, int i)
{
	int k;
	int vec[m];
	for(k = 0; k <m;k++)
		vec[k] = mat2[k][i%p];
	if(i < ((n*p)-1))
	{
		if(swapcontext(&uct_func[i],&uct_func[i+1]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
	  if(swapcontext(&uct_func[i],&uct_func[0]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
  result[i/p][i%p] = varun_vectorMult(mat1[i/p],vec,m,m);
  //  printf("excuse me\n");
}

int main(int argc, char const *argv[])
{
  getcontext(&uct_main);
	int i;
  // FIND THIS PART DYNAMICALLY FROM COMMAND LINE ///
  /* (n x m) * (m x p) = (n x p) */
  //const int n = 2;
  //const int m = 3;
  //const int p = 2;
  /*mat 1 with nrows and m=3 columns*/
	/*mat1 = (int**)malloc(n*sizeof(int*));
	mat2 = (int**)malloc(m*sizeof(int*));
	for(i = 0; i<n;i++)
		mat1[i] = (int*)malloc(m*sizeof(int));
	for(i = 0; i<m;i++)
		mat2[i] = (int*)malloc(p*sizeof(int));
  for(i = 0; i<n*m;i++)
		mat1[i/m][i%m] = i+1;
  for(i = 0; i<m*p;i++)
		mat2[i/p][i%p] = i + 1 + (n*m);*/
		int n = 0;
		int* np = &n;
		int m1 = 0;
		int* m1p = &m1;
		mat1 = getMat(np,m1p);

		int m2 = 0;
		int* m2p = &m2;
		int p = 0;
		int* pp = &p;
		mat2 = getMat(m2p,pp);

		if(m1 != m2)
		{
			fprintf(stderr, "-matmult_t: mat1 number of columns must equal mat2 number of rows\n");
			exit(EXIT_FAILURE);
		}
		int m = m1;

	/////////////////////////////////////////////////////

  result = (int **)malloc(n*sizeof(int *));
  for(i = 0; i < n; i++)
    result[i] = (int*)malloc(p*sizeof(int));
	uct_func = (ucontext_t*)malloc(n*p*sizeof(ucontext_t));

  /////////////////////////////////////////////////////

  char func_stack[n*p][16384];
  for(i = 0; i < (n*p); i++)
  {
    if(getcontext(&(uct_func[i])) != -1)
    {
      (uct_func[i]).uc_stack.ss_sp = func_stack[i];
      (uct_func[i]).uc_stack.ss_size = sizeof(func_stack[i]);
      if(i != ((n*p) - 1))
        (uct_func[i]).uc_link = &uct_func[i+1];
      else
        (uct_func[i]).uc_link = &uct_main;
      makecontext(&(uct_func[i]), (void(*)(void))varun_thread ,4,n,m,p,i);
    }
    else
    {
      perror("getcontext fault\n");
      exit(0);
    }
  }
	if(swapcontext(&uct_main,&(uct_func[0])) == -1)
	{
		perror("Error with swapcontext :(");
		exit(EXIT_FAILURE);
	}
  int j;
  for(i = 0; i < n; i++)
  {
    for(j = 0; j < p ; j++)
    {
      printf("%d ",result[i][j]);
    }
    printf("\n");
  }
  free(result);
  free(uct_func);
	free(mat1);
	free(mat2);
	free(result);
  return 0;
}
