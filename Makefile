all: myshell multiply matmult_p matmult_t matformatter

myshell: myshell.c shellFuncs.c
	gcc myshell.c shellFuncs.c -o myshell

multiply: multiply.c
	gcc multiply.c -o multiply

matformatter: matformatter.c
	gcc matformatter.c -o matformatter

matmult_p: matmult_p.c
	gcc matmult_p.c -o matmult_p

matmult_t: matmult_t.c
	gcc matmult_t.c -o matmult_t




clean:
	rm myshell multiply matmult_p matformatter matmult_t
	