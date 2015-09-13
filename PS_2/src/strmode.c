#include "walker.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void strmode(mode_t mode, char* modestr) {

    /* Type */
    switch (mode & S_IFMT) {
        case S_IFREG: // Regular
            *modestr++ = '-';
            break;
        case S_IFDIR:  // Directory
            *modestr++ = 'd';
            break;
        case S_IFCHR:  // Character device
            *modestr++ = 'c';
            break;
        case S_IFBLK:  // Block Device
            *modestr++ = 'b';
            break;
        case S_IFIFO:  // Named pipe
            *modestr++ = 'p';
            break;
        case S_IFLNK:  // Symlink
            *modestr++ = 'l';
            break;
        case S_IFSOCK: // Socket
            *modestr++ = 's';
            break;
        default:       // Otherwise...
            *modestr++ = '?';
            break;
    }

    /* Owner */
    *modestr++ = (mode & S_IRUSR) ? 'r' : '-';

    *modestr++ = (mode & S_IWUSR) ? 'w' : '-';

    switch (mode & (S_IXUSR | S_ISUID)) {
        case S_IXUSR:
            *modestr++ = 'x';
            break;
        case 0:
            *modestr++ = '-';
            break;
        case S_ISUID:
            *modestr++ = 'S';
            break;
        case S_IXUSR | S_ISUID:
            *modestr++ = 's';
            break;
    }

    /* Group */
    *modestr++ = (mode & S_IRGRP) ? 'r' : '-';

    *modestr++ = (mode & S_IWGRP) ? 'w' : '-';

    switch (mode & (S_IXGRP | S_ISGID)) {
        case S_IXGRP:
            *modestr++ = 'x';
            break;
        case 0:
            *modestr++ = '-';
            break;
        case S_ISGID:
            *modestr++ = 'S';
            break;
        case S_IXGRP | S_ISGID:
            *modestr++ = 's';
            break;
    }

    /* Other */
    *modestr++ = (mode & S_IROTH) ? 'r' : '-';

    *modestr++ = (mode & S_IWOTH) ? 'w' : '-';

    switch (mode & (S_IXOTH | S_ISVTX)) {
        case S_IXOTH:
            *modestr++ = 'x';
            break;
        case 0:
            *modestr++ = '-';
            break;
        case S_ISVTX:
            *modestr++ = 'T';
            break;
        case S_IXOTH | S_ISVTX:
            *modestr++ = 't';
            break;
    }
    *modestr = '\0';
}