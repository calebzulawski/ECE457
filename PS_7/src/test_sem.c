#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "lib/sem.h"

#define NUM_PROC 8u
#define ITER 1000000ul

struct shared {
	struct sem sem;
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
		printf("Supply an input option:\n\t1 for normal\n\t2 for mutex\n");
		exit(0);
	}
	int mutex = 0;
	if (argv[1][0] == '1')
		mutex = 0;
	else if (argv[1][0] == '2')
		mutex = 1;
	else
		printf("Not a valid option.  Assuming no mutex.\n");

	struct shared *s = make_mem();
	if (!sem_init(&s->sem, 1)) {
		perror("Could not initialize semaphore");
		exit(-1);
	}
	s->data = 0;

	printf("Spawning %u processes.\n", NUM_PROC);
	for(size_t i = 0; i < NUM_PROC; i++) {
		my_procnum = i;
		pid_t pid = fork();
		switch (pid) {
			case -1:
				perror("fork()");
				exit(-1);
			case 0:
				if (mutex) {
					for (unsigned long count = 0; count < ITER; count++) {
						sem_wait(&s->sem);
						s->data++;
						sem_inc(&s->sem);
					}
				} else {
					for (unsigned long count = 0; count < 1e6; count++) {
						s->data++;
					}
				}
				printf("Exiting child.\n");
				exit(0);
			default:
				continue;
		}

	}
	for (size_t i = 0; i < NUM_PROC; i++)
		waitpid(s->sem.pids[i], NULL, 0);
	printf("Data has value: %lu\n", s->data);
	printf("Data should be: %lu\n", ITER*NUM_PROC);
	printf("Exiting parent process.\n");
	exit(0);
}