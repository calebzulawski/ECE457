#include "walker.h"

#include <getopt.h>    // getopt

int main(int argc, char* argv[]) {
    int c;
    ino_t target = 0;
    int stayOnDev = 0;
    struct stat t;

    while ((c = getopt(argc, argv, "+l:x")) != -1) {
        switch (c) {
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
        init_walk(argv[optind], stayOnDev, target);
    else
        init_walk(".", stayOnDev, target);
}