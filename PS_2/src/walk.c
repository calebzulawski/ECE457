#include "walker.h"

#include <stdio.h>  // printf
#include <string.h> // strlen
#include <unistd.h> // close
#include <stdlib.h> // free
#include <dirent.h> // DT_ defines

void init_walk(char* filename) {
    // Remove unnecessary slash
    size_t filelen = strlen(filename);
    if (filename[filelen-1] == '/')
        filename[filelen-1] = '\0';

    // Open directory and parents
    int f        = safe_open(filename);
    int f_parent = safe_openat(f,"..");

    // Stat
    struct stat * parentstat = safe_malloc(sizeof(struct stat));
    struct stat * thisstat   = safe_malloc(sizeof(struct stat));

    // Get inodes
    safe_fstat(f_parent, parentstat);
    close(f_parent);
    ino_t parent_ino = parentstat->st_ino; 

    safe_fstat(f, thisstat);
    ino_t this_ino = thisstat->st_ino;

    // Free stat structs
    free(parentstat);
    free(thisstat);

    // Walk!
    recursive_walk(filename, this_ino, parent_ino, f, 0);

    // Close directory
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
        ret = safe_getdents(f, buffer);
        for (index = 0; index < ret; ) {
            d = (struct linux_dirent *) (buffer + index);
            if ( d->d_ino != this_ino && d->d_ino != parent_ino ) {
                int f_next = safe_openat(f, d->d_name);
                char* nextfile = safe_malloc(WALK_PATHLEN);
                sprintf(nextfile, "%s/%s", dirname, d->d_name);
                printstat(nextfile, f_next);
                if ( ((char*)d)[d->d_reclen-1] == DT_DIR )
                    recursive_walk(nextfile, d->d_ino, this_ino, f_next, depth + 1);
                free(nextfile);
                close(f_next);
            }
            index += d->d_reclen;
        }
    } while (ret > 0);
    free(buffer);
}

void printstat(const char* filename, int f_next) {
    struct stat s;
    safe_fstat(f_next, &s);
    char perms [12];
    strmode(s.st_mode, perms);

    printf("%04o/%d\t%s\t%d\t\n", 
        (unsigned) s.st_dev,
        (int)      s.st_ino,
                   perms,
        (int)      s.st_nlink);
}

// void getperms(struct stat * s, char* perms) {
//     switch (s->st_mode) {
//         case S_ISREG:
//             perms[0] = '-';
//             break;
//         case S_ISDIR:
//             perms[0] = 'd';
//             break;
//         case S_ISLNK:
//             perms[0] = 'l';
//             break;
//         case S_ISFIFO:
//             perms[0] = 'p'
//             break;
//         case S_ISSOCK:
//             perms[0] = 's'
//             break;
//         case S_ISCHR:
//             perms[0] = 'c'
//             break;
//         case S_ISBLK:
//             perms[0] = 'b'
//             break;
//     }
// }