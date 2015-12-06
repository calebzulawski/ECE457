#ifndef _SCHED_H_
#define _SCHED_H_

#include "context.h"

#define STACK_SIZE 0xffff

#define SCHED_NPROC    512
#define SCHED_READY    0
#define SCHED_RUNNING  1
#define SCHED_SLEEPING 2
#define SCHED_ZOMBIE   3

struct sched_proc {
    int state;
    int pid;
    int ppid;
    int nice;
    unsigned accumulated;
    unsigned cpu_time;
    void *stack;
    struct savectx context;
    int exit_code;
    int priority;
};

struct sched_waitq {
    struct sched_proc * procs [SCHED_NPROC];
    unsigned nprocs;
};

int create_pid();
void * create_stack();
struct sched_proc * create_proc(int ppid);
int sched_init(void (*init_fn)());
int sched_fork();
void sched_exit(int code);
int sched_wait(int *exit_code);
void sched_nice(int val);
int sched_getpid();
int sched_getppid();
int sched_gettick();
void sched_ps();
int getPriority(int pid);
void sched_switch();
void sched_tick();


#endif /* _SCHED_H_ */