#include <sys/types.h>

#define MAX_PROCESSES 64

// Semaphore structure
struct sem {
	char lock;
	int count;
	sigset_t sig;
	char suspended[MAX_PROCESSES];
	pid_t pids[MAX_PROCESSES];
};

extern unsigned my_procnum;

void empty_handler(int sig);

void wake_waiting(struct sem *s);

int  sem_init(struct sem *s, int count);
int  sem_try (struct sem *s);
void sem_wait(struct sem *s);
void sem_inc (struct sem *s);