myshell: myshell.c shellFuncs.c
	gcc myshell.c shellFuncs.c -o myshell

clean:
	rm myshell
	