# LAB 5: INTERPROCESS COMMUNICATION AND PTHREADS

## PIPE

- a unidirectional data channel for interprocess communication

- pipe() 
	- creates two pipe endpoints (one for reading, one for writing)
	- kernel buffers data written to the write endpoint until it's read from the read endpoint
	- returns the two file descriptors
	- treated like a file, therefore we can use read(), write() and close()
	- usually used before calling fork()
		- what you want to be shared should be done before fork
		- what you don't want, have done after fork

- popen()
	- opens a process by creating a pipe, forking, invoking the shell
		- executing a command and binding the stdout of the new process to a file that is shard between the child and parent process

- example:
	$ ./P1 < f1 | ./P2
	- stdout is shared between P1 and P2
	- stdin of P1 is not shared

## PTHREAD

- pthread_create()
	- starts a new thread in the calling process
	- the newly created thread can be either a kernel (PTHREAD_SCOPE_SYSTEM) or a user (PTHREAD_SCOPE_PROCESS) thread

- pthread_join()
	- blocks the caller until a specified thread terminates
	- example: a simple thread creation and termination

- user-level threads (two reccomended solutions)
	- system V user-level context switching
	- nonlocal jumps (sigsetjump, siglongjump, sigaltstack)