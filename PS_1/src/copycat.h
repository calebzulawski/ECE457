/* copycat.h
 * Caleb Zulawski
 * 
 * Function and struct declarations, constants.
 */

#ifndef COPYCAT_H
#define COPYCAT_H

#define DEFAULT_BUFFER_SIZE 1024 // 1KB
#define DEFAULT_FILE_PERM   0664 // RW-RW-R--

// Command line options
typedef struct {
    char**       argv;
    int          argc;
    unsigned int buffersize;
    int          outfile_index;
    int          infiles_index;
    int          verbose;
    unsigned int mode;
} Options;

// Non-read/write error conditions
typedef enum {
    CC_NONE,
    CC_USAGE,
    CC_MALLOC_FAIL
} cc_error_t;

// Read/write error conditions
typedef enum {
    CC_F_NONE,
    CC_F_OPEN_RD,
    CC_F_OPEN_WR,
    CC_F_READ,
    CC_F_WRITE
} cc_file_error_t;

// Function declarations
cc_error_t      cc_parse_args(int argc, char* argv[], Options* options);

void            cc_error(cc_error_t e);

void            cc_error_f(cc_file_error_t e, int err, char filename[]);

void            cc_log(Options* options);

void            cc_copy(Options* options);

cc_file_error_t cc_copy_file(const int          fo,
                             const int          fi,
                             const unsigned int buf_len,
                             char*              buf,
                             int*               err
                            );

#endif /* COPYCAT_H */