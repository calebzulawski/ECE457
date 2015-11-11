#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "lib/fifo.h"

#define WRITE_SIZE 100
#define NUM_PROC 8

struct fifo * make_mem() {
	void *m = mmap(0, sizeof(struct fifo), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	if (m == (void *)-1){
		perror("mmap()");
		exit(-1);
	}
	return (struct fifo *) m;
}

int main(void) {
	struct fifo *f = make_mem();
	if (!fifo_init(f)) {
		perror("Could not initialize fifo");
		exit(-1);
	}

	pid_t readpid;
	for(size_t i = 0; i < NUM_PROC; i++) {
		my_procnum = i;
		pid_t pid = fork();
		switch (pid) {
			case -1:
				perror("fork()");
				exit(-1);
			case 0:
				if (my_procnum == 0) {
					unsigned long val;
					for (size_t j = 0; j < WRITE_SIZE*(NUM_PROC-1); j++) {
						val = fifo_rd(f);
						printf("Got %lx\n", val);
					}
					printf("Exiting reader.\n");
				} else {
					unsigned long val = (my_procnum << 16);
					for (unsigned long j = 0; j < WRITE_SIZE; j++) {
						printf("Sent %lx\n", val + j);
						fifo_wr(f, val + j);
					}
					printf("Exiting writer.\n");
				}
				exit(0);
			default:
				if (my_procnum == 0)
					readpid = pid;
				continue;
		}

	}
	waitpid(readpid, NULL, 0);
	exit(0);
}