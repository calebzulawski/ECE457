#include "walker.h"

#include <sys/types.h>   // open
#include <sys/stat.h>    // open
#include <fcntl.h>       // open
#include <unistd.h>      // syscall
#include <sys/syscall.h> // syscall
#include <dirent.h>      // getdents64

#include <stdio.h>       // printf
#include <stdlib.h>      // malloc



void walk(char filename[], unsigned int depth) {
    int f = open(filename, WALK_FLAGS);
    if (!f)
        0;

    struct linux_dirent64 * buffer = (struct linux_dirent64 *) malloc(WALK_BUFFERSIZE);
    if (buffer == NULL)
        0;

    int ret;
    do {
        ret = syscall(SYS_getdents64, f, buffer, WALK_BUFFERSIZE);
        for (int i = 0; i < ret; i++) {
            printdir(&buffer[i]);
            if (buffer[i].d_type == DT_DIR && depth != WALK_MAXDEPTH)
                walk(buffer[i].d_name, depth + 1);
        }
    } while (ret > 0);
    if (ret == -1)
        0;
}

void printdir(struct linux_dirent64 * d) {
    printf("%s\n", d->d_name);
}