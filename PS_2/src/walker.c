#include "walker.h"

#include <getopt.h>    // getopt

int main(int argc, char* argv[]) {
    int c;
    while ((c = getopt(argc, argv, "+b:m:o:vh")) != -1) {
        switch (c) {
            case '?':
                break;
        }
    }
    init_walk(argv[optind]);
}