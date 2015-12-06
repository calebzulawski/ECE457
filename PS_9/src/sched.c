#include "sched.h"
#include "adjstack.h"
#include "context.h"

#include <sys/time.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#define HOLD_SIGNALS(x) \
    sigprocmask(SIG_BLOCK, &all_signals, NULL); \
    x \
    sigprocmask(SIG_UNBLOCK, &all_signals, NULL);

#define BLOCK_SIGNALS()   sigprocmask(SIG_BLOCK, &all_signals, NULL)
#define UNBLOCK_SIGNALS() sigprocmask(SIG_UNBLOCK, &all_signals, NULL)

struct sched_proc * current = NULL;
struct sched_waitq * queue = NULL;
unsigned nticks;
sigset_t all_signals;

int create_pid() {
    if (queue == NULL) {
        fprintf(stderr, "Running process list has not been allocated!");
        return -1;
    }
    // Skip PID 0, since scheduler should probably have that
    for (unsigned i = 0; i < SCHED_NPROC; i++){
        if (queue->procs[i] == NULL)
            return i;
    }
    fprintf(stderr, "Ran out of PIDs!\n");
    return -1;
}

void * create_stack() {
    void * newsp;
    if ((newsp = mmap(0, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED) {
        perror("mmap()");
        return MAP_FAILED;
    }
    return newsp;
}

struct sched_proc * create_proc(int ppid) {
    int pid = create_pid();
    if (pid == -1) {

        return 0;
    }
    struct sched_proc * proc = (struct sched_proc *) malloc(sizeof(struct sched_proc));
    if (proc == NULL) {
        perror("malloc()");
        return NULL;
    }
    void * newsp = create_stack();
    if (newsp == -1) {
        free(proc);
        return NULL;
    }

    // Set contents
    proc->state       = SCHED_READY;
    proc->pid         = pid;
    proc->ppid        = ppid;
    proc->nice        = 0;
    proc->accumulated = 0;
    proc->cpu_time    = 0;
    proc->stack       = newsp;

    queue->procs[pid] = proc;
    queue->nprocs++;

    return proc;
}

int sched_init(void (*init_fn)()) {
    // Initialize process queue
    queue = (struct sched_waitq *) calloc(1, sizeof(struct sched_waitq));
    if (queue == NULL) {
        perror("malloc()");
        return -1;
    }

    // Set up ticks
    sigfillset(&all_signals);
    nticks = 0;
    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 1e6;
    struct itimerval it;
    it.it_interval = t;
    it.it_value = t;
    if(setitimer(ITIMER_REAL, &it, NULL) == -1) {
        perror("Failed to set timer");
        return -1;
    }
    signal(SIGVTALRM, sched_tick);

    // Create init process and switch control
    current = create_proc(0);
    struct savectx ctx;
    ctx.regs[JB_BP] = current->stack + STACK_SIZE;
    ctx.regs[JB_SP] = current->stack + STACK_SIZE;
    ctx.regs[JB_PC] = init_fn;
    restorectx(&ctx, 0);

    return 0;
}

int sched_fork() {
    HOLD_SIGNALS(
        struct sched_proc * newproc = create_proc(current->pid);
        newproc->cpu_time = current->cpu_time;
        memcpy(newproc->stack, current->stack, STACK_SIZE);
        adjstack(newproc->stack, newproc->stack + STACK_SIZE, newproc->stack - current->stack);

        int return_pid = newproc->pid;
        if (!savectx(&newproc->context)) {
            newproc->context.regs[JB_BP] += newproc->stack - current->stack;
            newproc->context.regs[JB_SP] += newproc->stack - current->stack;
        } else {
            return_pid = 0;
        }
    );
    return return_pid;
}

void sched_exit(int code) {
    HOLD_SIGNALS(
        current->state = SCHED_ZOMBIE;
        current->exit_code = code;
        queue->nprocs--;

        // Check for wait()
        if ((queue->procs[current->ppid] != NULL)
         && (queue->procs[current->ppid]->state == SCHED_SLEEPING)) {
            queue->procs[current->ppid]->state = SCHED_READY;
        }
    );
    sched_switch();
}

int sched_wait(int *exit_code) {
    while(1) {
        BLOCK_SIGNALS();
        int found = 0;
        int zombie = 0;
        found = 0;
        zombie = 0;
        for (int i = 0; i < SCHED_NPROC; i++) {
            if (queue->procs[i] != NULL
             && queue->procs[i]->ppid == current->pid) {
                found = 1;
                if (queue->procs[i]->state == SCHED_ZOMBIE) {
                    zombie = 1;
                    current->state = SCHED_READY;
                    *exit_code = queue->procs[i]->exit_code;
                    free(queue->procs[i]);
                    queue->procs[i] = NULL;
                    break;
                }
            }

        }

        if (found == 0) {
            UNBLOCK_SIGNALS();
            return -1;
        } else if (found == 1 && zombie == 1) {
            UNBLOCK_SIGNALS();
            return 0;
        } else {
            UNBLOCK_SIGNALS();
            sched_switch();
        }
    }
}

void sched_nice(int val) {
    val = val >  19 ? val :  19;
    val = val < -20 ? val : -20;
    current->nice = val;
}

int sched_getpid() {
    return current->pid;
}

int sched_getppid() {
    return current->ppid;
}

int sched_gettick() {
    return nticks;
}

void sched_ps() {
    printf("PID       PPID      STATE     STACK     STATIC    DYNAMIC   TIME      \n");
    for (int i = 0; i < SCHED_NPROC; i++) {
        if (queue->procs[i] != NULL) {
            printf("%9d %9d %9d %9x %9d %9d %9d", 
                queue->procs[i]->pid,
                queue->procs[i]->ppid,
                queue->procs[i]->state,
                (unsigned)queue->procs[i]->stack,
                20 - queue->procs[i]->nice,
                queue->procs[i]->priority,
                queue->procs[i]->accumulated);
        }
    }

}

int getPriority(int pid) {
    if (queue->procs[pid] != NULL) {
        int priority = 20 - queue->procs[pid]->nice - queue->procs[pid]->cpu_time/2;
        priority = priority > 39 ? 39 : priority;
        priority = priority < 0  ? 0  : priority;
        queue->procs[pid]->priority = priority;
        return priority;
    } else {
        return -1;
    }
}

void sched_switch() {
    BLOCK_SIGNALS();
    if (current->state != SCHED_ZOMBIE && current->state != SCHED_SLEEPING)
        current->state = SCHED_READY;

    int best = -1;
    int best_pid = -1;
    int p;
    for (int pid = 0; pid < SCHED_NPROC; pid++) {
        if ((queue->procs[pid] != NULL)
         && (queue->procs[pid] == SCHED_READY)
         && ((p = getPriority(pid)) > best)) {
            best = p;
            best_pid = pid;
        }
    }

    if (best_pid == current->pid) {
        current->state = SCHED_RUNNING;
        current->cpu_time = 0;
        UNBLOCK_SIGNALS();
        return;
    }

    if(savectx(&current->context) == 0) {
        current = queue->procs[best_pid];
        current->cpu_time = 0;
        current->state = SCHED_RUNNING;
        UNBLOCK_SIGNALS();
        restorectx(&current->context, 1);
    }
}

void sched_tick() {
    nticks++;
    current->accumulated++;
    current->cpu_time++;
    sched_switch();
    return;
}