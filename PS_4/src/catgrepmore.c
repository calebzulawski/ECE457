#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define COPYBUFSIZ 1024

void set_descriptors(int in, int out) {
	if (in != -1) {
		if (close(STDIN_FILENO) == -1) {
			fprintf(stderr, "Failed to close standard input: %s\n", strerror(errno));
			exit(-1);
		}
		if (dup2(in, STDIN_FILENO) == -1) {
			fprintf(stderr, "Failed to set standard input to pipe: %s\n", strerror(errno));
			exit(-1);
		}
		if (close(in) == -1)
			fprintf(stderr, "Failed to close duplicate file descriptor: %s\n", strerror(errno));
	}

	if (out != -1) {
		if (close(STDOUT_FILENO) == -1) {
			fprintf(stderr, "Failed to close standard out: %s\n", strerror(errno));
			exit(-1);
		}
		if (dup2(out, STDOUT_FILENO) == -1) {
			fprintf(stderr, "Failed to set standard output to pipe: %s\n", strerror(errno));
			exit(-1);
		}
		if (close(out) == -1)
			fprintf(stderr, "Failed to close duplicate file descriptor: %s\n", strerror(errno));
	}
}

void copy_file(const int    fi,
               const int    fo,
               char*        buf,
               const int    buf_len)
{
    int bytes_read, bytes_written;

    while ((bytes_read = read(fi, buf, buf_len))) {
        if (bytes_read == -1) {
            fprintf(stderr, "Error calling read(): %s\n", strerror(errno));
            return;
        }

        do {
            if ((bytes_written = write(fo, buf, bytes_read)) == -1) {
                fprintf(stderr, "Error calling write(): %s\n", strerror(errno));
                return;
            }
        } while (bytes_written < bytes_read); // Handle partial write
    }
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "Not enough arguments\n");
		exit(-1);
	}

	int pipefd1[2], pipefd2[2];

	if ( pipe(pipefd1) == -1 || pipe(pipefd2) == -1){
		fprintf(stderr, "Failed to open pipe: %s\n", strerror(errno));
		exit(-1);
	}

	switch (fork()){
		case 0:
			set_descriptors(pipefd1[0], pipefd2[1]);
			execlp("grep", "grep", argv[1], (char *) NULL);
			fprintf(stderr, "Failed to open grep: %s\n", strerror(errno));
			exit(-1);
		case -1:
			fprintf(stderr, "Failed to fork() for grep: %s\n", strerror(errno));
			exit(-1);
		default:
			break;
	}

	switch (fork()) {
		case 0:
			set_descriptors(pipefd1[0], -1);
			execlp("more", "more", (char *) NULL);
			fprintf(stderr, "Failed to open more: %s\n", strerror(errno));
			exit(-1);
		case -1:
			fprintf(stderr, "Failed to fork() for more: %s\n", strerror(errno));
			exit(-1);
		default:
			break;
	}

	int fi;
	char buf [COPYBUFSIZ];
	for (int i = 2; i < argc; i++) {
		if ((fi = open(argv[i], O_RDONLY)) == -1) {
			fprintf(stderr, "Failed to open %s: %s\n", argv[i], strerror(errno));
			continue;
		}
		copy_file(fi, pipefd1[1], buf, COPYBUFSIZ);
	}
	return 0;
}