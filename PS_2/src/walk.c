#include "walker.h"

#include <sys/types.h>   // open, stat
#include <sys/stat.h>    // open, stat
#include <fcntl.h>       // open
#include <unistd.h>      // syscall, stat
#include <sys/syscall.h> // syscall
#include <dirent.h>      // getdents

#include <stdio.h>       // printf, perror
#include <stdlib.h>      // malloc
#include <errno.h>       // errno
#include <string.h>      // strlen

void * safe_malloc(size_t size) {
    void * x = malloc(size);
    if (x == NULL) {
        perror("malloc()");
        exit(-1);
    }
    return x;
}

void safe_fstat(int fd, struct stat *buf) {
    buf = malloc(sizeof(struct stat));
    if (buf == NULL) {
        perror("fstat(): malloc()");
        exit(-1);
    }
    if ( fstat(fd, buf) == -1 ) {
        perror("fstat()");
        exit(-1);
    }
}

int safe_open(const char *pathname) {
    int f = open(pathname, WALK_FLAGS);
    if (!f) {
        perror(pathname);
        exit(-1);
    }
    return f;
}

int safe_openat(int f_base, const char *pathname) {
    int f = openat(f_base, pathname, WALK_FLAGS);
    if (!f) {
        perror(pathname);
        exit(-1);
    }
    return f;
}

void init_walk(const char* filename) {
    int f = safe_open(filename);
    int f_parent = safe_openat(f,"..");
    struct stat * parentstat = NULL;
    struct stat * thisstat = NULL;

    safe_fstat(f_parent, parentstat);

    close(f_parent);

    safe_fstat(f, thisstat);

    recursive_walk(filename, thisstat->st_ino, parentstat->st_ino, f, 0);

    close(f);
}

void recursive_walk(const char* dirname, ino_t this_ino, ino_t parent_ino, int f, unsigned depth) {
    if (depth > WALK_MAXDEPTH)
        return;

    char* buffer = safe_malloc(WALK_BUFFERSIZE);

    int ret;
    struct linux_dirent * d;
    size_t index;
    do {
        ret = syscall(SYS_getdents, f, buffer, WALK_BUFFERSIZE);
        for (index = 0; index < ret; ) {
            d = (struct linux_dirent *) (buffer + index);
            if ( d->d_ino != this_ino && d->d_ino != parent_ino ) {
                int f_next = safe_openat(f, d->d_name);
                char* nextfile = safe_malloc(WALK_PATHLEN);
                sprintf(nextfile, "%s/%s", dirname, d->d_name);
                printstat(nextfile, f_next);
                if ( *((char*)(d+d->d_reclen-1)) == DT_DIR )
                    recursive_walk(nextfile, d->d_ino, this_ino, f_next, depth + 1);
                free(nextfile);
                close(f_next);
            }
            index += d->d_reclen;
        }
    } while (ret > 0);
    if (ret == -1) {
        perror("getdents()");
        exit(-1);
    }
    free(buffer);
}

void printstat(const char* filename, int f_next) {
    printf("%s\n", filename);
}