#include "walker.h"

#include <stdio.h>  // printf
#include <string.h> // strlen
#include <unistd.h> // close
#include <stdlib.h> // free
#include <dirent.h> // DT_ defines
#include <time.h>   // ctime

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

    // inode list
    ino_t ino_list[WALK_MAXDEPTH+1] = {0};

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
    recursive_walk(filename, this_ino, parent_ino, f, 0, ino_list);

    // Close directory
    close(f);
}

void recursive_walk(const char* dirname, ino_t this_ino, ino_t parent_ino, int f, unsigned depth, ino_t * ino_list) {
    if (depth > WALK_MAXDEPTH)
        return;

    ino_list[depth]   = this_ino;
    ino_list[depth+1] = 0;

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
                if (f_next != -1) {
                    char* nextfile = safe_malloc(WALK_PATHLEN);
                    sprintf(nextfile, "%s/%s", dirname, d->d_name);
                    printstat(nextfile, f_next);
                    if ( (((char*)d)[d->d_reclen-1] == DT_DIR) && !is_loop(ino_list, d->d_ino) )
                        recursive_walk(nextfile, d->d_ino, this_ino, f_next, depth + 1, ino_list);
                    free(nextfile);
                    close(f_next);
                }
            }
            index += d->d_reclen;
        }
    } while (ret > 0);
    free(buffer);
}

int is_loop(ino_t *ino_list, ino_t this_ino) {
    for (size_t i = 0; i < WALK_MAXDEPTH; i++) {
        if (ino_list[i] == this_ino) {
            fprintf(stderr, "Found filesystem loop...\n");
            return 1;
        }
        if (ino_list[i] == 0)
            continue;
    }
    return 0;
}

void printstat(const char* filename, int f_next) {
    struct stat s;
    safe_lstat(filename, &s);
    char perms    [12];
    char user     [17];
    char group    [17];
    char size     [16];
    char linkpath [WALK_PATHLEN];

    strmode(s.st_mode, perms);
    getusrgrp(s.st_uid, s.st_gid, user, group);
    getsizeid(&s, size);
    char* mtime = ctime(&s.st_mtime);
    mtime[24] = '\0';

    getlinkcontents(&s, filename, linkpath, WALK_PATHLEN);

    printf( "%04o/%-10d %s  %-5d %-12s %-12s %-16s %s %s %s\n", 
        (unsigned) s.st_dev,
        (int)      s.st_ino,
                   perms,
        (int)      s.st_nlink,
                   user,
                   group,
                   size,
                   mtime,
                   filename,
                   linkpath );
}