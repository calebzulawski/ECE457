/* copycat_copy.c
 * Caleb Zulawski
 * 
 * Contains the functions for copying data from
 * the specified files.  Reports errors.
 */

#include "copycat.h"

/* Library functions */
#include <stdlib.h>    // malloc
#include <string.h>    // strcmp

/* Linux system calls */
#include <unistd.h>    // read, close
#include <errno.h>     // errno
#include <sys/types.h> // open, write
#include <sys/stat.h>  // open, write
#include <fcntl.h>     // open, write

/* Calls appropriate copy functions and handles errors */
void cc_copy(Options* options) {
    int  /*fee*/ fi, fo /*fum*/;
    char**       argv          = options->argv;
    int          argc          = options->argc;
    int          outfile_index = options->outfile_index;
    int          infiles_index = options->infiles_index;
    mode_t       mode          = options->mode;
    unsigned int buffersize    = options->buffersize;

    const int w_flags = O_WRONLY // Read only
                      | O_CREAT  // Create file if it doesn't exist
                      | O_TRUNC; // Truncate the file if it does

    const int r_flags = O_RDONLY; // Read only

    // Open output file
    if (outfile_index != -1) {
        fo = open(argv[outfile_index], w_flags, (mode_t) mode);
        if (fo == -1)
            cc_error_f(CC_F_OPEN_WR, errno, argv[outfile_index]);
    } else {
        fo = STDOUT_FILENO;
    }

    // Acquire buffer for copying    
    char* buffer = malloc(buffersize);
    if (buffer == NULL)
        cc_error(CC_MALLOC_FAIL);

    if (infiles_index >= argc) {
        // If no files were supplied, use STDIN
        fi = STDIN_FILENO;
        int err = 0;
        cc_file_error_t status = cc_copy_file(fi, fo, buffersize, buffer, &err);
        close(fi);
        if (status == CC_F_READ) {
            close(fo);
            cc_error_f(status, err, "STDIN");
        } else if (status == CC_F_WRITE) {
            close(fo);
            cc_error_f(status, err, argv[outfile_index]);
        }
    } else {
        // If files were supplied, copy each in order
        for (int i = infiles_index; i < argc; i++) {
            if (!strcmp(argv[i], "-")) {
                fi = STDIN_FILENO;
            } else {
                fi = open(argv[i], r_flags);
                if (fi == -1)
                    cc_error_f(CC_F_OPEN_RD, errno, argv[i]);
            }

            int err = 0;
            cc_file_error_t status = cc_copy_file(fi, fo, buffersize, buffer, &err);
            close(fi);
            if (status == CC_F_READ) {
                close(fo);
                cc_error_f(status, err, argv[i]);
            } else if (status == CC_F_WRITE) {
                close(fo);
                cc_error_f(status, err, argv[outfile_index]);
            }
        }
    }
    close(fo);
}

cc_file_error_t cc_copy_file(const int          fi,
                             const int          fo,
                             const unsigned int buf_len,
                             char*              buf,
                             int*               err)
{
    ssize_t bytes_read, bytes_written;

    while ((bytes_read = read(fi, buf, (size_t) buf_len))) {
        if (bytes_read == -1) {
            *err = errno;
            return CC_F_READ;
        }

        do {
            if (!(bytes_written = write(fo, buf, bytes_read))) {
                *err = errno;
                return CC_F_WRITE;
            }
        } while (bytes_written < bytes_read); // Handle partial write
    }
    return CC_F_NONE;
}