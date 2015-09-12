#include "walker.h"

#include <sys/types.h>   // open
#include <sys/stat.h>    // open
#include <fcntl.h>       // open
#include <unistd.h>      // syscall
#include <sys/syscall.h> // syscall
#include <dirent.h>      // getdents64

#include <stdio.h>       // printf, perror
#include <stdlib.h>      // malloc
#include <errno.h>       // errno
#include <string.h>      // strcmp



void walk(char filename[], unsigned int depth) {
    int f = open(filename, WALK_FLAGS);
    if (!f) {
        perror(filename);
        exit(-1);
    }

    char* buffer = malloc(WALK_BUFFERSIZE);
    if (buffer == NULL) {
        perror(NULL);
        exit(-1);
    }

    int ret;
    struct linux_dirent64 * d;
    size_t index;
    do {
        ret = syscall(SYS_getdents64, f, buffer, WALK_BUFFERSIZE);
        close(f);
        for (index = 0; index < ret; ) {
            d = (struct linux_dirent64 *) (buffer + index);
            printdir(d);
            if ( strcmp(d->d_name, "..")
              && strcmp(d->d_name, ".")
              && d->d_type == DT_DIR
              && depth != WALK_MAXDEPTH ) {
                char nextfile [WALK_PATHLEN] = {'\0'};
                sprintf(nextfile, "%s/%s", filename, d->d_name);
                walk(nextfile, depth + 1);
            }
            index += d->d_reclen;
        }
    } while (ret > 0);
    if (ret == -1) {
        perror("getdents64()");
        exit(-1);
    }
}

void printdir(struct linux_dirent64 * d) {
    printf("%s\n", d->d_name);
}