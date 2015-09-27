#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

#define STRSIZ 1024

void bail(const char * msg) {
	perror(msg);
	exit(-1);
}

int safe_open(const char * file, int flags) {
	int f = open(file, flags);
	if (f == -1) {
		fprintf(stderr, "Could not open %s: %s\n", file, strerror(errno));
		exit(-1);
	}
	return f;
}

int main( int argc, char* argv[], char *envp[] ) {

	FILE * instream = stdin;

	while(1) {
		char   command   [STRSIZ] = {0};
		char * tokenized [STRSIZ] = {0};

		int stdin_fork  = -1;
		int stdout_fork = -1;
		int stderr_fork = -1;

		int status = 0;
		struct rusage u;
		clock_t time1, time2;
		float realt, usert, syst;

		printf("$ ");
		if (!fgets(command, STRSIZ, instream)) {
			printf("\n");
			exit(0);
		}

		if ((command[0] == '\0')) {
			printf("end of file");
			exit(0);
		}

		int ind = 0;
		int whitespace = 1;
		for (int i = 0; i < STRSIZ; i++) {
			switch (command[i]) {
				case ' ':
				case '\t':
				case '\n':
					command[i] = '\0';
					whitespace = 1;
					break;
				case '\0':
					i = STRSIZ; // better than a goto?
					break;
				default:
					if (whitespace) {
						tokenized[ind++] = &command[i];
						whitespace = 0;
					}
			}
		}

		if (tokenized[0][0] == '#')
			continue;

		int argsDone = 0;
		for(int i = 1; tokenized[i] != 0; i++) {
			if (!strncmp(tokenized[i], "2>>", 3)) {
				stderr_fork = safe_open(&tokenized[i][3], O_WRONLY | O_CREAT | O_APPEND);
				argsDone = 1;
			} else if (!strncmp(tokenized[i], "2>", 2)) {
				stderr_fork = safe_open(&tokenized[i][2], O_WRONLY | O_CREAT | O_TRUNC);
				argsDone = 1;
			} else if (!strncmp(tokenized[i], ">>", 2)) {
				stdout_fork = safe_open(&tokenized[i][2], O_WRONLY | O_CREAT | O_APPEND);
				argsDone = 1;
			} else if (!strncmp(tokenized[i], ">", 1)) {
				stdout_fork = safe_open(&tokenized[i][1], O_WRONLY | O_CREAT | O_TRUNC);
				argsDone = 1;
			} else if (!strncmp(tokenized[i], "<", 1)) {
				stdin_fork = safe_open(&tokenized[i][1], O_RDONLY);
				argsDone = 1;
			} else if (argsDone) {
				char error [STRSIZ];
				fprintf(stderr, "Unrecognized stream redirect: %s", tokenized[i]);
				bail(error);
			}

			if (argsDone)
				tokenized[i] = 0;
		}

		int pid = fork();
		switch (pid) {
			case -1:
				bail("fork() failed");
				break;
			case 0:
				if (stdin_fork != -1) {
					close(0);
					dup2(stdin_fork,  0);
				}

				if (stdout_fork != -1) {
					close(1);
					dup2(stdout_fork, 1);
				}

				if (stderr_fork != -1) {
					close(2);
					dup2(stderr_fork, 2);
				}

				if (execvpe(tokenized[0], tokenized, envp) != 0)
					bail("execvpe()");

				break;
			default:
				time1 = clock();
				wait(&status);
				time2 = clock();

				getrusage(pid, &u);

				realt = (float)(time2 - time1)/CLOCKS_PER_SEC;
				usert = (float)u.ru_utime.tv_usec/1e6;
				syst  = (float)u.ru_stime.tv_usec/1e6;

				printf("\nCommand returned with return code %d\n", status);
				printf("Consuming %f real seconds, %f user, %f system\n", realt, usert, syst);
				break;
		}
	}
}