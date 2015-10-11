#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#define COPYBUFSIZ 1024

void close_msg(int f) {
	if (close(f == -1))
		fprintf(stderr, "Failed to close file descriptor %d: %s\n", f, strerror(errno));
}

void dup2_msg(int f1, int f2) {
	if (dup2(f1, f2) == -1) {
		fprintf(stderr, "Failed to dup2() fd %d to fd %d: %s\n", f1, f2, strerror(errno));
		exit(-1);
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

	int greppid;
	switch (greppid = fork()) {
		case 0:
			dup2_msg(pipefd1[0], STDIN_FILENO);
			dup2_msg(pipefd2[1], STDOUT_FILENO);
			close_msg(pipefd1[0]);
			close_msg(pipefd1[1]);
			close_msg(pipefd2[0]);
			close_msg(pipefd2[1]);
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
			dup2_msg(pipefd2[0], STDIN_FILENO);
			close_msg(pipefd1[0]);
			close_msg(pipefd1[1]);
			close_msg(pipefd2[0]);
			close_msg(pipefd2[1]);
			execlp("more", "more", (char *) NULL);
			fprintf(stderr, "Failed to open more: %s\n", strerror(errno));
			exit(-1);
		case -1:
			fprintf(stderr, "Failed to fork() for more: %s\n", strerror(errno));
			exit(-1);
		default:
			break;
	}
	close_msg(pipefd1[0]);
	close_msg(pipefd2[0]);
	close_msg(pipefd2[1]);
	int fi;
	char buf [COPYBUFSIZ];
	for (int i = 2; i < argc; i++) {
		if ((fi = open(argv[i], O_RDONLY)) == -1) {
			fprintf(stderr, "Failed to open %s: %s\n", argv[i], strerror(errno));
			continue;
		}
		copy_file(fi, pipefd1[1], buf, COPYBUFSIZ);
	}
	close_msg(pipefd1[1]);
	return 0;
}