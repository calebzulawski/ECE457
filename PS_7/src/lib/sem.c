#include "sem.h"
#include "tas.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

unsigned my_procnum;

void empty_handler(int sig) {
	if (sig == SIGINT)
		exit(0);
	return;
}

int sem_init(struct sem *s, int count) {
	s->lock = 0;
	s->count = count;
	memset(s->suspended, 0, sizeof s->suspended);
	return (sigfillset(&s->sig) == 0)
	     & (sigdelset(&s->sig, SIGUSR1) == 0)
	     & (sigdelset(&s->sig, SIGINT) == 0)
	     & (signal(SIGUSR1, empty_handler) != SIG_ERR);
}

int sem_try (struct sem *s) {
	int ret = 0;
	while(tas((char *)&s->lock));
	if(s->count > 0) {
		s->count--;
		ret = 1;
	}
	s->lock = 0;
	return ret;
}
void sem_wait(struct sem *s) {
	while (1) {
		// Wait for lock
		while(tas((char *)&s->lock));
		if (s->count > 0) {
			// Decrement and return
			s->count--;
			s->lock = 0;
			return;
		}

		// Add to suspend list
		s->suspended[my_procnum] = 1;
		s->pids[my_procnum] = getpid();
		s->lock = 0;

		// Wake up when called and try again
		// printf("Sleeping: %u\n", my_procnum);
		sigsuspend(&s->sig);
		s->suspended[my_procnum] = 0;
		// printf("Waking  : %u\n", my_procnum);
	}
}

void sem_inc (struct sem *s) {
	while(tas((char *)&s->lock));
	s->count++;
	for (size_t i = 0; i < MAX_PROCESSES; i++) {
		if (s->suspended[i]) {
			kill(s->pids[i], SIGUSR1);
			break;
		}
	}
	s->lock = 0;
}