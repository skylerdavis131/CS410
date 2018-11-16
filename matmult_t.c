#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include "readMat.h"
ucontext_t uct_main;
ucontext_t* uct_func;
int ** mat1;
int ** mat2;
int ** result;
char** func_stack;
int current_id,n,m,p;
int * thread_id;
//This Function is for Matrix Multiplication
int varun_vectorMult(int* vec1, int* vec2, int numArgs1, int numArgs2)
{
	int result1 = 0;
	int i;

	if(numArgs1 != numArgs2){
		fprintf(stderr, "-vectorMult: cant multiply arrays of different length\n");
		exit(0);
	}
	// This for loop is for doing dot product of vectors
	for(i = 0; i < numArgs1; i++){
		result1 = result1 + (vec1[i] * vec2[i]);
	}

	return result1;
}


void varun_thread(int i)
{
	int k;
	int vec[m];
	// This For loop is for getting the getting the row vector in the Matrix
	for(k = 0; k <m;k++)
	{
		vec[k] = mat2[k][i%p];
	}
	// This condition checks that is this is not the last thread
	if(i < ((n*p)-1))
	{
		// Save the current context in the uct_func array and jumps to next thread
		if(swapcontext(&uct_func[i],&uct_func[i+1]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		// Goes back to the first thread if it is the last thread
	  if(swapcontext(&uct_func[i],&uct_func[0]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
	// Saves the result in the ouptut Matrix
  result[i/p][i%p] = varun_vectorMult(mat1[i/p],vec,m,m);
	// Swap context to get the result from every thread
	if(i < ((n*p)-1))
	{
		// Save the current context in the uct_func array and jumps to next thread
		if(swapcontext(&uct_func[i],&uct_func[i+1]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		// Goes back to the first thread if it is the last thread
	  if(swapcontext(&uct_func[i],&uct_func[0]) == -1)
		{
			perror("Error with swapcontext :(");
			exit(EXIT_FAILURE);
		}
	}
}

// The function used for creating Thread
void my_thr_create(void (*func)(int), int thr_id)
{
	// Initializes a context
	if(getcontext(&(uct_func[current_id])) != -1)
	{
		int truth = 1;
		int iter;
		// Checks if the thread id is repeated or not
		for(iter = 0; iter < current_id; iter++)
		{
			if(thread_id[iter] == thr_id)
				truth = 0;
		}
		// If the thread is not repeated then true
		if(truth)
		{
			// Gives the context a stack to work with
			(uct_func[current_id]).uc_stack.ss_sp = func_stack[current_id];
			// Stores the size of the stack
			(uct_func[current_id]).uc_stack.ss_size = sizeof(func_stack[current_id]);
			// Sets the uc_link to the main thread
			(uct_func[current_id]).uc_link = &uct_main;
			// Saves the id in a ID array
			thread_id[current_id] = thr_id;
			// Makes context to assign the starting of thread to the start of
			// the function
			makecontext(&(uct_func[current_id]), (void(*)(void))func ,1,current_id);
			// Increments the number of threads
			current_id++;
		}
		else//If Thread Id is repeated then this consition
			printf("The Thread ID %d is already in use :(\n",thr_id);
	}
	else// If context cannot be initialized.
	{
		perror("getcontext fault\n");
		exit(0);
	}
}


int main(int argc, char const *argv[])
{
  getcontext(&uct_main);
	int i;
  // FIND THIS PART DYNAMICALLY FROM COMMAND LINE ///

	///////////////////////////////////////////////////
	// Gets the first Matrix
	n = 0;
	int* np = &n;
	int m1 = 0;
	int* m1p = &m1;
	mat1 = getMat(np,m1p);

	// Gets the Second Matrix
	int m2 = 0;
	int* m2p = &m2;
	//int p = 0;
	p = 0;
	int* pp = &p;
	mat2 = getMat(m2p,pp);

	if(m1 != m2)
	{
		fprintf(stderr, "-matmult_t: mat1 number of columns must equal mat2 number of rows\n");
		exit(EXIT_FAILURE);
	}
	//int m = m1;
	m = m1;

	/////////////////////////////////////////////////////
	// Initializes an array for result
  result = (int **)malloc(n*sizeof(int *));
  for(i = 0; i < n; i++)
    result[i] = (int*)malloc(p*sizeof(int));
	// Initializes an arrat of contexts for threads
	uct_func = (ucontext_t*)malloc(n*p*sizeof(ucontext_t));

  /////////////////////////////////////////////////////
	// Initializes an array for IDs
	thread_id = (int*)malloc(n*p*sizeof(int));
	// Initializes the cuurent number of threads
	current_id = 0;
  //char func_stack[n*p][16384];
	// Initializes an array of stacks for the Thread
	func_stack = (char**)malloc(n*p*sizeof(char*));
	for(i = 0; i < n*p; i++)
		func_stack[i] = (char*)malloc(16384*sizeof(char));

	/////////////////////////////////////////////////////
	// Calls my_thr_create function for every thread
  for(i = 0; i < (n*p); i++)
  {
		my_thr_create(varun_thread,i);
  }
	// Starts the fun of threadding
	if(swapcontext(&uct_main,&(uct_func[0])) == -1)
	{
		perror("Error with swapcontext :(");
		exit(EXIT_FAILURE);
	}
	// Prints the result
  int j;
  for(i = 0; i < n; i++)
  {
    for(j = 0; j < p ; j++)
    {
      printf("%d ",result[i][j]);
    }
    printf("\n");
  }
  //free(func_stack);
  //free(uct_func);
	free(mat1);
	free(mat2);
	free(result);
  return 0;
}
