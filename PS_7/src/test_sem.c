#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "lib/tas.h"

#define NUM_PROC 8u
#define ITER 1000000ul

struct shared {
	int lock;
	unsigned long data;
};

struct shared * make_mem() {
	void *m = mmap(0, sizeof(struct shared), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (m == (void *)-1){
		perror("mmap()");
		exit(-1);
	}
	return (struct shared *) m;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Supply an input option:\n\t1 for normal\n\t2 for spinlock\n\t3 for semaphore\n");
		exit(0);
	}
	int mutex = 0;

	if (argv[1][0] == '1')
		mutex = 0;
	else if (argv[1][0] == '2')
		mutex = 1;
	else if (argv[1][0] == '3')
		mutex = 2;
	else
		printf("Not a valid option.  Assuming no mutex.\n");

	struct shared *s = make_mem();
	s->data = 0;
	int child = 0;
	pid_t pids[NUM_PROC];

	printf("Spawning %u processes.\n", NUM_PROC);
	for(size_t i = 0; i < NUM_PROC; i++) {
		pid_t pid = fork();
		switch (pid) {
			case -1:
				perror("fork()");
				exit(-1);
			case 0:
				child = 1;
				goto endloop;
			default:
				pids[i] = pid;
				continue;
		}

	}
	endloop:

	if (child) {
		unsigned long count;

		if (mutex == 0) {
			for (count = 0; count < 1e6; count++) {
				s->data++;
			}
		} else if (mutex == 1) {
			for (count = 0; count < ITER; count++) {
				while (tas((char *)&s->lock));
				s->data++;
				s->lock = 0;
			}
		} else if (mutex == 2) {
			for (count = 0; count < ITER; count++) {
				while (tas((char *)&s->lock));
				s->data++;
				s->lock = 0;
			}
		}
		printf("Exiting child.\n");
		exit(0);
	} else {
		for (size_t i = 0; i < NUM_PROC; i++)
			waitpid(pids[i], NULL, 0);
		printf("Data has value: %lu\n", s->data);
		printf("Data should be: %lu\n", ITER*NUM_PROC);
		printf("Exiting parent process.\n");
		exit(0);
	}
}