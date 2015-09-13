#ifndef WALKER_H
#define WALKER_H

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#define WALK_FLAGS      O_RDONLY | O_DIRECTORY
#define WALK_BUFFERSIZE 1024
#define WALK_MAXDEPTH   1024
#define WALK_PATHLEN    1024

struct linux_dirent {
    unsigned long  d_ino;     /* Inode number */
    unsigned long  d_off;     /* Offset to next linux_dirent */
    unsigned short d_reclen;  // Length of this linux_dirent 
    char           d_name[];  /* Filename (null-terminated) */
    /*
    char           pad;       // Zero padding byte
    char           d_type;    // File type
    */
};


/* Error handling wrappers for resource acquisition */

void * safe_malloc(size_t size);

void   safe_fstat(int fd, struct stat *buf);

int    safe_open(const char *pathname);

int    safe_openat(int f_base, const char *pathname);

/* Path walk functions */

void init_walk(char* filename);

void recursive_walk(const char* dirname, ino_t thisino, ino_t parentino, int f, unsigned depth);

void printstat(const char* filename, int f_next);

#endif /* WALKER_H */