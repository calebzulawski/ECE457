#include "copycat.h"

#include "stdio.h"  // sscanf, printf
#include "stdlib.h" // exit
#include "getopt.h" // getopt

int main(int argc, char* argv[]) {
	Options options;
	options.buffersize = DEFAULT_BUFFER_SIZE;
	
	cc_error( cc_parse_args(argc, argv, &options) );
	
	if (options.verbose) {
		// OUTPUT FILE
		if (options.outfile_index)
			printf("Output file:\t%s\n", argv[options.outfile_index]);
		else
			printf("Printing to standard output stream\n");

		// INPUT FILES
		for (int i = options.infiles_index; i < argc; i++) {
			printf("Input file:\t%s\n", argv[i]);
		}
		if (options.infiles_index == argc)
			printf("Reading from standard input stream\n");

		// BUFFER SIZE
		printf("Buffer size:\t%u\n", options.buffersize);
	}
	return 0;
}

cc_error_t cc_parse_args(int argc, char* argv[], Options* options) {
	int c;
	while ((c = getopt(argc, argv, "+b:o:vh")) != -1) {
		switch (c) {
			case 'b':
				if ( !sscanf(optarg, "%u", &options->buffersize) )
					return USAGE;
				break;
			case 'o':
				options->outfile_index = optind-1;
				break;
			case 'v':
				options->verbose = 1;
				break;
			case 'h':
				return USAGE;
				break;
			case '?':
				return USAGE;
				break;
		}
	}

	options->infiles_index = optind;

	return NONE;
}

void cc_error(cc_error_t e) {
	switch (e) {
		case NONE:
			return;
		case USAGE:
			printf("copycat [-v] [-b BUFFER_SIZE] [-o OUTFILE] INFILE11 [...INFILE2...]\n");
			printf("copycat [-v] [-b BUFFER_SIZE] [-o OUTFILE]\n");
			printf("copycat -h");
			exit(-1);
			break;
	}
}