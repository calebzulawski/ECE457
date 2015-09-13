#include "walker.h"

#include <stdlib.h>      // malloc
#include <sys/types.h>   // fstat, open
#include <sys/stat.h>    // fstat, open
#include <unistd.h>      // fstat
#include <fcntl.h>       // open
#include <sys/syscall.h> // syscall
#include <stdio.h>       // perror

void * safe_malloc(size_t size) {
    void * x = malloc(size);
    if (x == NULL) {
        perror("malloc()");
        exit(-1);
    }
    return x;
}

void safe_fstat(int fd, struct stat *buf) {
    if ( fstat(fd, buf) != 0 ) {
        perror("fstat()");
        exit(-1);
    }
}

int safe_open(const char *pathname) {
    int f = open(pathname, WALK_INIT_FLAGS);
    if (f == -1) {
        perror(pathname);
        exit(-1);
    }
    return f;
}

int safe_openat(int f_base, const char *pathname) {
    int f = openat(f_base, pathname, WALK_FLAGS);
    if (f == -1) {
        perror(pathname);
        exit(-1);
    }
    return f;
}

int safe_getdents(int f, char* buffer) {
    int ret = syscall(SYS_getdents, f, buffer, WALK_BUFFERSIZE);
    if (ret == -1) {
        perror("getdents()");
        exit(-1);
    }
    return ret;
}