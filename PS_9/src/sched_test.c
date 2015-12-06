#include "sched.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void child() {
	printf("In child with pid %d, ppid %d\n", sched_getpid(), sched_getppid());
	unsigned long max = 1e8;
	for (unsigned long i = 0; i < max; i++) {
		getpid();
	}
	sched_exit(sched_getpid());
}

void init() {
	printf("Hello\n");
	int pid;
	for (int i = 0; i < 5; i++) {
		switch(pid = sched_fork()) {
			case -1:
				printf("sched_fork() error\n");
				sched_exit(0);
			case 0:
				child();
			default: 
				printf("Created pid %d\n", pid);
				break;
		}	
	}
	printf("In parent with pid %d, child pid %d\n", sched_getpid(), pid);
	int code;
	for (int i = 0; i < 5; i++) {
		sched_wait(&code);
		printf("Child returned with code %d\n", code);
	}
	sched_exit(0);
	printf("Ending init process...\n");
	exit(0);
}

int main(int argc, char **argv) {
	sched_init(init);
}