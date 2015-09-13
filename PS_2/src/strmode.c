#include "walker.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void strmode(mode_t mode, char* p) {

    /* Type */
    switch (mode & S_IFMT) {
        case S_IFREG: // Regular
            *p++ = '-';
            break;
        case S_IFDIR:  // Directory
            *p++ = 'd';
            break;
        case S_IFCHR:  // Character device
            *p++ = 'c';
            break;
        case S_IFBLK:  // Block Device
            *p++ = 'b';
            break;
        case S_IFIFO:  // Named pipe
            *p++ = 'p';
            break;
        case S_IFLNK:  // Symlink
            *p++ = 'l';
            break;
        case S_IFSOCK: // Socket
            *p++ = 's';
            break;
        default:       // Otherwise...
            *p++ = '?';
            break;
    }

    /* OWNER */
    *p++ = (mode & S_IRUSR) ? 'r' : '-';

    *p++ = (mode & S_IWUSR) ? 'w' : '-';

    switch (mode & (S_IXUSR | S_ISUID)) {
        case S_IXUSR:
            *p++ = 'x';
            break;
        case 0:
            *p++ = '-';
            break;
        case S_ISUID:
            *p++ = 'S';
            break;
        case S_IXUSR | S_ISUID:
            *p++ = 's';
            break;
    }

    /* GROUP */
    *p++ = (mode & S_IRGRP) ? 'r' : '-';

    *p++ = (mode & S_IWGRP) ? 'w' : '-';

    switch (mode & (S_IXGRP | S_ISGID)) {
        case S_IXGRP:
            *p++ = 'x';
            break;
        case 0:
            *p++ = '-';
            break;
        case S_ISGID:
            *p++ = 'S';
            break;
        case S_IXGRP | S_ISGID:
            *p++ = 's';
            break;
    }

    /* OTHER */
    *p++ = (mode & S_IROTH) ? 'r' : '-';

    *p++ = (mode & S_IWOTH) ? 'w' : '-';

    switch (mode & (S_IXOTH | S_ISVTX)) {
        case S_IXOTH:
            *p++ = 'x';
            break;
        case 0:
            *p++ = '-';
            break;
        case S_ISVTX:
            *p++ = 'T';
            break;
        case S_IXOTH | S_ISVTX:
            *p++ = 't';
            break;
    }
    *p = '\0';
}