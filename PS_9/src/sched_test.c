#include "sched.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void child() {
	// priority: 6, 4, 5, 3, 2
	switch (sched_getpid()) {
		case 2:
			sched_nice(10);
			break;
		case 3:
			sched_nice(8);
			break;
		case 4:
			sched_nice(4);
			break;
		case 5:
			sched_nice(6);
			break;
		case 6:
			sched_nice(2);
			break;
	}
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
	printf("In parent with pid %d\n", sched_getpid());
	int code;
	// unsigned long max = 1e9;
	// for (unsigned long i = 0; i < max; i++) {
	// 	getpid();
	// }
	int order [5];
	for (int i = 0; i < 5; i++) {
		sched_wait(&code);
		order[i] = code;
		printf("Child returned with code %d\n", code);
	}
	printf("Children returned in order:");
	for (int i = 0; i < 5; i++)
		printf(" %d", order[i]);
	printf("\n");
	sched_exit(0);
	printf("Ending init process...\n");
	exit(0);
}

int main(int argc, char **argv) {
	sched_init(init);
}