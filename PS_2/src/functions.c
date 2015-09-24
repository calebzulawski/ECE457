#include "walker.h"

#include <stdlib.h>      // malloc
#include <sys/types.h>   // fstat, open
#include <sys/stat.h>    // fstat, open
#include <unistd.h>      // fstat
#include <fcntl.h>       // open
#include <sys/syscall.h> // syscall
#include <stdio.h>       // perror, sprintf
#include <pwd.h>         // getpwuid
#include <grp.h>         // getgrgid

void * safe_malloc(size_t size) {
    void * x = malloc(size);
    if (x == NULL) {
        perror("malloc()");
        exit(-1);
    }
    return x;
}

void safe_stat(const char* filename, struct stat *buf) {
    if ( stat(filename, buf) != 0 ) {
        perror("stat()");
        exit(-1);
    }
}

void safe_fstat(int fd, struct stat *buf) {
    if ( fstat(fd, buf) != 0 ) {
        perror("fstat()");
        exit(-1);
    }
}

void safe_lstat(const char* filename, struct stat *buf) {
    if ( lstat(filename, buf) != 0 ) {
        perror("lstat()");
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
        // Don't exit
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

void getusrgrp(uid_t uid, gid_t gid, char* usr, char* grp) {
    struct passwd * p = getpwuid(uid);
    struct group * g  = getgrgid(gid);

    if (p != NULL) {
        sprintf(usr, "%s", p->pw_name);
    } else {
        sprintf(usr, "%d", uid);
    }

    if (g != NULL) {
        sprintf(grp, "%s", g->gr_name);
    } else {
        sprintf(grp, "%d", gid);
    }
}

void getsizeid(struct stat * s, char* sizeid) {
    switch (s->st_mode & S_IFMT) {
        case S_IFCHR:
        case S_IFBLK:
            sprintf(sizeid, "0x%x", (unsigned) s->st_rdev);
            break;
        default:
            sprintf(sizeid, "%d", (int) s->st_size);
            break;
    }
}

void getlinkcontents(struct stat * s, const char* path, char* buf, size_t bufsiz) {
    if ((s->st_mode & S_IFMT) == S_IFLNK) {
        sprintf(buf, "-> ");
        size_t ret = readlink(path, &buf[3], bufsiz-3);
        if (ret == -1) {
            perror("readlink()");
            exit(-1);
        }
        if (ret + 3 < bufsiz)
            buf[ret + 3] = '\0';
        else
            buf[bufsiz-1] = '\0';
    } else {
        buf[0] = '\0';
    }
}