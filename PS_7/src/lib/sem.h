#include <sys/types.h>

#define MAX_PROCESSES 64

// Mutex circular buffer containing pids waiting on semaphore
struct wait_list {
	unsigned list[MAX_PROCESSES];
	pid_t pids[MAX_PROCESSES];
	size_t start;
	size_t len;
};

// Semaphore structure
struct sem {
	int lock;
	int count;
	sigset_t sig;
	struct wait_list procs;
};

extern unsigned my_procnum;

void empty_handler(int sig);

void kill_waiting(struct sem *s);

int  sem_init(struct sem *s, int count);
int  sem_try (struct sem *s);
void sem_wait(struct sem *s);
void sem_inc (struct sem *s);