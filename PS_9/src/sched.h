#ifndef _SCHED_H_
#define _SCHED_H_

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

#endif /* _SCHED_H_ */