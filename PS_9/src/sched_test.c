#include "sched.h"

#include <stdio.h>

void init() {
	printf("Hello\n");
	int pid;
	switch(pid = sched_fork()) {
		case -1:
			printf("sched_fork() error\n");
			sched_exit(0);
		case 0:
			printf("In child with pid %d, ppid %d\n", sched_getpid(), sched_getppid());
			sched_exit(7);
		default:
			printf("In parent with pid %d, child pid %d\n", sched_getpid(), pid);
			int code;
			sched_wait(&code);
			printf("Child returned with code %d\n", code);
			sched_exit(0);
	}
	printf("Ending init process... bad things may happen\n");
}

int main(int argc, char **argv) {
	sched_init(init);
}