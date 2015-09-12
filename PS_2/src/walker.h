#ifndef WALKER_H
#define WALKER_H

#define _GNU_SOURCE // Enable use of O_PATH

#include <sys/types.h>

#define WALK_FLAGS      O_PATH
#define WALK_BUFFERSIZE 1024
#define WALK_MAXDEPTH   1024

struct linux_dirent64 {
    ino64_t        d_ino;    /* 64-bit inode number */
    off64_t        d_off;    /* 64-bit offset to next structure */
    unsigned short d_reclen; /* Size of this dirent */
    unsigned char  d_type;   /* File type */
    char           d_name[]; /* Filename (null-terminated) */
};

void walk(char filename[], unsigned int depth);

void printdir(struct linux_dirent64 * d);

#endif /* WALKER_H */