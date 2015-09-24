#include "walker.h"

#include <stdio.h>  // printf
#include <string.h> // strlen
#include <unistd.h> // close
#include <stdlib.h> // free
#include <dirent.h> // DT_ defines
#include <time.h>   // ctime

void init_walk(char* filename, int stayOnDev, ino_t target, uid_t uid) {    
    // Remove unnecessary slash
    size_t filelen = strlen(filename);
    if (filename[filelen-1] == '/')
        filename[filelen-1] = '\0';

    // Open directory and parents
    int f        = safe_open(filename);

    // Stat
    struct stat * thisstat   = safe_malloc(sizeof(struct stat));

    // inode list
    ino_t ino_list[WALK_MAXDEPTH+1] = {0};

    // Get inodes
    safe_fstat(f, thisstat);
    ino_t this_ino = thisstat->st_ino;

    // For option -x
    dev_t this_dev;
    if (stayOnDev)
        this_dev = thisstat->st_dev;
    else
        this_dev = 0;

    // Free stat structs
    free(thisstat);

    // Walk!
    recursive_walk(filename, this_ino, this_dev, uid, target, f, 0, ino_list);

    // Close directory
    close(f);
}

void recursive_walk(const char* dirname,
                    ino_t       this_ino,
                    dev_t       this_dev,
                    uid_t       uid,
                    ino_t       target,
                    int         f,
                    unsigned    depth,
                    ino_t*      ino_list ) {

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
            if ( strcmp(d->d_name, "..") && strcmp(d->d_name, ".") ) {
                int f_next = safe_openat(f, d->d_name);
                if (f_next != -1) {
                    char* nextfile = safe_malloc(WALK_PATHLEN);
                    sprintf(nextfile, "%s/%s", dirname, d->d_name);
                    
                    int ret   = stat_file(nextfile, f_next, this_dev, target, uid);
                    int loop  = is_loop(ino_list, d->d_ino);
                    int isdir = (((char*)d)[d->d_reclen-1] == DT_DIR);
                    
                    if (loop)
                        fprintf(stderr, "walker: Found loop in filesystem.  Skipping already walked directory %s\n", d->d_name);
                    
                    if ( !loop && ret && isdir)
                        recursive_walk(nextfile, d->d_ino, this_dev, uid, target, f_next, depth + 1, ino_list);
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
            return 1;
        }
        if (ino_list[i] == 0)
            continue;
    }
    return 0;
}

int stat_file(const char* filename, int f_next, dev_t this_dev, ino_t target, uid_t uid) {
    struct stat s, t;
    safe_lstat(filename, &s);
    safe_stat(filename, &t);
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

    if ( ( target == 0 || (t.st_ino == target && t.st_ino != s.st_ino) ) && ( uid == -1 || (s.st_uid == uid ) ) )
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
    
    if ( (this_dev == 0) || (this_dev == s.st_dev) ) {
        return 1;
    } else {
        printf( "walker (-x): not crossing mountpoint at %s\n", filename);
        return 0;
    }
}