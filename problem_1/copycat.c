#include "copycat.h"

/* Linux system calls */
#include <unistd.h>    // read
#include <errno.h>     // errno
#include <sys/types.h> // open, write
#include <sys/stat.h>  // open, write
#include <fcntl.h>     // open, write

/* Library functions */
#include <stdio.h>     // sscanf, printf
#include <stdlib.h>    // exit
#include <getopt.h>    // getopt
#include <string.h>    // strerror

int main(int argc, char* argv[]) {
    Options options;
    options.buffersize = DEFAULT_BUFFER_SIZE;
    options.outfile_index = -1;
    options.argc = argc;
    options.argv = argv;
    
    cc_error( cc_parse_args(argc, argv, &options) );
    cc_log(&options);
    cc_copy(&options);

    return 0;
}

cc_error_t cc_parse_args(int argc, char* argv[], Options* options) {
    int c;
    while ((c = getopt(argc, argv, "+b:o:vh")) != -1) {
        switch (c) {
            case 'b':
                if ( !sscanf(optarg, "%u", &options->buffersize) )
                    return CC_USAGE;
                break;
            case 'o':
                options->outfile_index = optind-1;
                break;
            case 'v':
                options->verbose = 1;
                break;
            case 'h':
                return CC_USAGE;
                break;
            case '?':
                return CC_USAGE;
                break;
        }
    }

    options->infiles_index = optind;

    return CC_NONE;
}

void cc_error(cc_error_t e) {
    switch (e) {
        case CC_NONE:
            return;
        case CC_USAGE:
            printf("copycat [-v] [-b BUFFER_SIZE] [-o OUTFILE] INFILE11 [...INFILE2...]\n");
            printf("copycat [-v] [-b BUFFER_SIZE] [-o OUTFILE]\n");
            printf("copycat -h\n");
            exit(-1);
        case CC_MALLOC_FAIL:
            printf("Could not allocate buffer!\n");
            exit(-1);
    }
}

void cc_error_f(cc_file_error_t e) {

}

void cc_log(Options* options) {
    if (options->verbose) {
        // OUTPUT FILE
        if (options->outfile_index)
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
    }
}

void cc_copy(Options* options) {
    int /*fee*/ fi, fo /*fum*/;
    const int w_flags = O_WRONLY // Read only
                     || O_CREAT  // Create file if it doesn't exist
                     || O_TRUNC; // Truncate the file if it does

    const int r_flags = O_RDONLY; // Read only

    // Open output file
    if (options->outfile_index != -1) {
        fo = open(options->argv[options->outfile_index], w_flags);
        if (fo == -1)
            cc_error_f(CC_F_OPEN_WR);
    } else {
        fo = STDOUT_FILENO;
    }

    // Cycle through input files;
    char* buffer = malloc(options->buffersize);
    if (buffer == NULL)
        cc_error(CC_MALLOC_FAIL);

    for (int i = options->infiles_index; i < options->argc; i++) {
        if (!strcmp(options->argv[i], "-")) {
            fi = STDERR_FILENO;
        } else {
            fi = open(options->argv[i], r_flags);
            if (fi == -1)
                cc_error_f(CC_F_NONE);
        }

        int err = 0;
        cc_copy_file(fi, fo, options->buffersize, buffer, &err);
    }

}

cc_file_error_t cc_copy_file(const int fi, const int fo, const unsigned int buf_len, char* buf, int* err) {
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(fi, buf, (size_t) buf_len))) {
        if (bytes_read == -1) {
            *err = errno;
            return CC_F_READ;
        }

        if (!(bytes_written = write(fo, buf, bytes_read))) {
            *err = errno;
            return CC_F_WRITE;
        }
    }
    return CC_F_NONE;
}