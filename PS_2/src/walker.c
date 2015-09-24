#include "walker.h"

#include <getopt.h>    // getopt
#include <errno.h>
#include <stdlib.h>
#include <pwd.h>       // getpwuid
#include <string.h>
#include <limits.h>

int main(int argc, char* argv[]) {
    int c;
    ino_t target = 0;
    int stayOnDev = 0;
    struct stat t;
    uid_t uid = -1;  // unsigned, but still invalid uid
    long uid_temp;
    char end = 'c';
    char* endp = &end;
    long int modTime;

    while ((c = getopt(argc, argv, "+l:xu:m:")) != -1) {
        switch (c) {
            case 'u':
                uid_temp = strtol(optarg, &endp, 10);
                if (*endp == '\0') {
                    uid = uid_temp;
                    break;
                }
                struct passwd * p = getpwnam(optarg);
                if (p != NULL) {
                    uid = p->pw_uid;
                } else {
                    exit(-1);
                }
                break;
            case 'm':
                modTime = strtol(optarg, &endp, 10);
                if (*endp != '\0')
                    exit(-1);
                break;
            case 'l':
                safe_stat(optarg, &t);
                target = t.st_ino;
                break;
            case 'x':
                stayOnDev = 1;
                break;
            case '?':
                break;
        }
    }
    if (optind < argc)
        init_walk(argv[optind], stayOnDev, target, uid, modTime);
    else
        init_walk(".", stayOnDev, target, uid, modTime);
}