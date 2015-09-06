#include "copycat.h"

/* Library functions */
#include <stdio.h>     // sscanf, printf
#include <getopt.h>    // getopt
#include <stdlib.h>    // exit
#include <string.h>    // strerror

cc_error_t cc_parse_args(int argc, char* argv[], Options* options) {
    options->buffersize    = DEFAULT_BUFFER_SIZE;
    options->outfile_index = -1;
    options->argc          = argc;
    options->argv          = argv;
    options->mode          = DEFAULT_FILE_PERM;

    int c;
    while ((c = getopt(argc, argv, "+b:m:o:vh")) != -1) {
        switch (c) {
            case 'b':
                if ( !sscanf(optarg, "%u", &options->buffersize) )
                    cc_error(CC_USAGE);
                break;
            case 'm':
                if ( !sscanf(optarg, "%o", &options->mode) )
                    cc_error(CC_USAGE);
                break;
            case 'o':
                options->outfile_index = optind-1;
                break;
            case 'v':
                options->verbose = 1;
                break;
            case 'h':
                cc_error(CC_USAGE);
                break;
            case '?':
                cc_error(CC_USAGE);
                break;
        }
    }

    options->infiles_index = optind;

    return CC_NONE;
}

void cc_log(Options* options) {
    if (options->verbose) {
        // OUTPUT FILE
        if (options->outfile_index != -1)
            printf("Output file:\t%s\n", options->argv[options->outfile_index]);
        else
            printf("Printing to standard output stream\n");

        // INPUT FILES
        for (int i = options->infiles_index; i < options->argc; i++) {
            printf("Input file:\t%s\n", options->argv[i]);
        }
        if (options->infiles_index == options->argc)
            printf("Reading from standard input stream\n");

        // BUFFER SIZE
        printf("Buffer size:\t%u\n", options->buffersize);

        // OUTPUT FILE PERMISSIONS
        printf("Output mode:\t%o\n", options->mode);
    }
}

void cc_error(cc_error_t e) {
    switch (e) {
        case CC_NONE:
            return;
        case CC_USAGE:
            printf("Usage: copycat [OPTION]... [FILE]...\n");
            printf("Concatenate FILE(s), or standard input, to standard output.\n");
            printf("Similar to GNU cat.\n\n");
            printf("  -v           print diagnostic messages to standard error\n");
            printf("  -b SIZE      size of internal copy buffer, in bytes\n");
            printf("  -m MODE      file mode, in octal\n");
            printf("  -o FILE      output to FILE instead of standard output\n");
            printf("  -h           display this help and exit\n");
            exit(-1);
        case CC_MALLOC_FAIL:
            printf("Could not allocate buffer!\n");
            exit(-1);
    }
}

void cc_error_f(cc_file_error_t e, int err, char filename[]) {
    switch (e) {
        case CC_F_NONE:
            return;
        case CC_F_OPEN_RD:
        case CC_F_OPEN_WR:
            fprintf(stderr, "Error opening file %s: %s\n", filename, strerror(err));
            exit(-1);
        case CC_F_READ:
            fprintf(stderr, "Error reading from file %s: %s\n", filename, strerror(err));
            exit(-1);
        case CC_F_WRITE:
            fprintf(stderr, "Error writing to file %s: %s\n", filename, strerror(err));
    }
}