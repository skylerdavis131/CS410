# LAB 4: FORK, EXEC, AND SIGNAL

- relationship between process and program binary (executable)
	- proccess = given address space, needs to be cooked

# system calls

- fork(), execv(), wait()
	- signal(), kill()


## fork = creates a new process by cloning the caller
## pid_t fork(void);

- child does exact same job, unless execution path is separated using the returned value of fork
    - 0 = returned in the child process
    - child's PID = returned in the parent process
    - way to differentiate child from parent
- what does the child NOT inherit from the parent
    - unique PID and return values
- copy-on-write policy = implicit sharing or shadowing, a resource-management technique used in computer programming to efficiently implement a duplicate without wasting memory

## exec = assigns a program to a process

- example:
    - write a program that clones its process
    - parent sleeps for 5 seconds, prints message and exits
        - child opens a file with vi
	- what happens when the parent process exits?

## wait = waits for state changes (e.g. when a child process exits) in a child of the calling process
## pid_t wait(int * wstatus);

- if a child has already changed state, then these calls return immediately
- otherwise, they block until either a child changes state or a signal handler interrupts the call
	- example: open a file with vi and wait until it ends executing


## kill = can be used to send any signal to any process group or process
## int kill(pid_t pid, int sig);

- if:
    - pid > 0, sig is sent to the specified process
    - pid = 0, sig is sent to every process in the process group of the calling process
    - pid = -1, sig is sent to every process for which the calling process has permission to send signals

## signal = sets the disposition of the signal signum to handler
## typedef void (* sighandler_t)(int);
## sighandler_t signal(int signum, sighandler_t handler);

- handler is either:
    - SIG_IGN (ignore)
    - SIG_DFL (default behavior)
    - address of a programmer-defined fn (a "signal handler")
- example: send SIGUSR1 to the child process and should acknowledge the receipt
