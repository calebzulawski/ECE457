#include "copycat.h"

/* Library functions */
#include <stdlib.h>    // malloc
#include <string.h>    // strcmp

/* Linux system calls */
#include <unistd.h>    // read
#include <errno.h>     // errno
#include <sys/types.h> // open, write
#include <sys/stat.h>  // open, write
#include <fcntl.h>     // open, write

void cc_copy(Options* options) {
    int /*fee*/ fi, fo /*fum*/;
    const int w_flags = O_WRONLY // Read only
                      | O_CREAT  // Create file if it doesn't exist
                      | O_TRUNC; // Truncate the file if it does

    const int r_flags = O_RDONLY; // Read only

    // Open output file
    if (options->outfile_index != -1) {
        fo = open(options->argv[options->outfile_index], w_flags, (mode_t) options->mode);
        if (fo == -1)
            cc_error_f(CC_F_OPEN_WR, errno, options->argv[options->outfile_index]);
    } else {
        fo = STDOUT_FILENO;
    }

    // Cycle through input files;
    char* buffer = malloc(options->buffersize);
    if (buffer == NULL)
        cc_error(CC_MALLOC_FAIL);

    if (options->infiles_index >= options->argc) {
        fi = STDIN_FILENO;
        int err = 0;
        cc_file_error_t status = cc_copy_file(fi, fo, options->buffersize, buffer, &err);
        if (status == CC_F_READ) {
            cc_error_f(status, err, "STDIN");
        } else if (status == CC_F_WRITE) {
            cc_error_f(status, err, options->argv[options->outfile_index]);
        }
    } else {
        for (int i = options->infiles_index; i < options->argc; i++) {
            if (!strcmp(options->argv[i], "-")) {
                fi = STDIN_FILENO;
            } else {
                fi = open(options->argv[i], r_flags);
                if (fi == -1)
                    cc_error_f(CC_F_OPEN_RD, errno, options->argv[i]);
            }

            int err = 0;
            cc_file_error_t status = cc_copy_file(fi, fo, options->buffersize, buffer, &err);
            if (status == CC_F_READ) {
                cc_error_f(status, err, options->argv[i]);
            } else if (status == CC_F_WRITE) {
                cc_error_f(status, err, options->argv[options->outfile_index]);
            }
        }
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