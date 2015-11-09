#include "sem.h"
#include "tas.h"
#include <signal.h>

unsigned my_procnum;

void sem_init(struct sem *s, int count) {
	s->lock = 0;
	s->count = count;
	s->procs.start = 0;
	s->procs.len = 0;
	sigemptyset(&s->sig);
	sigaddset(&s->sig, SIGUSR1);
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
		if (s->count == 0) {
			// Add to suspend list
			s->procs.list[(s->procs.start + s->procs.len) % MAX_PROCESSES] = my_procnum;
			s->procs.len++;
		} else {
			// Decrement and return
			s->count--;
			s->lock = 0;
			return;
		}
		// Wake up when called and try again
		sigsuspend(&s->sig);
	}
}

void sem_inc (struct sem *s) {
	while(tas((char *)&s->lock));
	kill(s->procs.pids[s->procs.list[s->procs.start]], SIGUSR1);
	s->procs.start = (s->procs.start + 1) % MAX_PROCESSES;
	s->procs.len--;
	s->count++;
	s->lock = 0;
}