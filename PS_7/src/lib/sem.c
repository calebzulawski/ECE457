#include "sem.h"
#include "tas.h"
#include <signal.h>
#include <stdio.h>

unsigned my_procnum;

void empty_handler(int sig) {
	return;
}

int sem_init(struct sem *s, int count) {
	s->lock = 0;
	s->count = count;
	s->procs.start = 0;
	s->procs.len = 0;
	return (sigemptyset(&s->sig) == 0)
	     & (sigaddset(&s->sig, SIGUSR1) == 0)
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
		s->procs.list[(s->procs.start + s->procs.len) % MAX_PROCESSES] = my_procnum;
		s->procs.len++;
		s->lock = 0;

		// Wake up when called and try again
		printf("Start waiting\n");
		sigsuspend(&s->sig);
		printf("SIGUSR1 received\n");
	}
}

void sem_inc (struct sem *s) {
	while(tas((char *)&s->lock));
	if (s->procs.len > 0) {
		printf("Sending SIGUSR1 to %ld\n", s->procs.pids[s->procs.list[s->procs.start]]);
		kill(s->procs.pids[s->procs.list[s->procs.start]], SIGUSR1);
		s->procs.start = (s->procs.start + 1) % MAX_PROCESSES;
		s->procs.len--;
	}
	s->count++;
	s->lock = 0;
}