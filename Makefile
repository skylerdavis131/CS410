all: myshell multiply jack_matmult_p matformatter helloworld adder varun_matmult_t

myshell: myshell.c shellFuncs.c
	gcc myshell.c shellFuncs.c -o myshell

multiply: multiply.c
	gcc multiply.c -o multiply

matformatter: matformatter.c
	gcc matformatter.c -o matformatter

jack_matmult_p: jack_matmult_p.c
	gcc jack_matmult_p.c -o jack_matmult_p

helloworld: helloworld.c
	gcc helloworld.c -o helloworld

adder: adder.cpp
	g++ adder.cpp -o adder

varun_matmult_t: varun_matmult_t.c
	gcc varun_matmult_t.c -o varun_matmult_t




clean:
	rm myshell multiply jack_matmult_p matformatter helloworld adder
