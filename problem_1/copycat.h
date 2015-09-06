#ifndef COPYCAT_H
#define COPYCAT_H

#define DEFAULT_BUFFER_SIZE 1024 // 1KB

#define POSIXLY_CORRECT

typedef struct {
	unsigned int buffersize;
	int          outfile_index;
	int          infiles_index;
	int          verbose;
} Options;

typedef enum {
	NONE,
	USAGE
} cc_error_t;

cc_error_t cc_parse_args(int argc, char* argv[], Options* options);

void cc_error(cc_error_t e);

#endif /* COPYCAT_H */