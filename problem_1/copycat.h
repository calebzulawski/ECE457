#ifndef COPYCAT_H
#define COPYCAT_H

#define DEFAULT_BUFFER_SIZE 1024 // 1KB

typedef struct {
    char**       argv;
    int          argc;
    unsigned int buffersize;
    int          outfile_index;
    int          infiles_index;
    int          verbose;
} Options;

typedef enum {
    CC_NONE,
    CC_USAGE,
    CC_MALLOC_FAIL
} cc_error_t;

typedef enum {
    CC_F_NONE,
    CC_F_OPEN_RD,
    CC_F_OPEN_WR,
    CC_F_READ,
    CC_F_WRITE
} cc_file_error_t;



cc_error_t      cc_parse_args(int argc, char* argv[], Options* options);

void            cc_error(cc_error_t e);

void            cc_error_f(cc_file_error_t e);

void            cc_log(Options* options);

void            cc_copy(Options* options);

cc_file_error_t cc_copy_file(const int fo, const int fi, const unsigned int buf_len, char* buf, int* err);

#endif /* COPYCAT_H */