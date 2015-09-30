#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

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

float time_diff(const struct timeval* start, const struct timeval* stop) {
	return (stop->tv_sec - start->tv_sec) + (stop->tv_usec - start->tv_usec)/1.0e6;
}

int main( int argc, char* argv[], char *envp[] ) {
	int returnVal = 0;

	FILE * instream;
	int prompt;

	if ( argc < 2 ) {
		instream = stdin;
		prompt = 1;
	}
	else if ( argc == 2 ) {
		int f = safe_open(argv[1], O_RDONLY);
		instream = fdopen(f, "r");
		prompt = 0;
	}
	else {
		fprintf(stderr, "mysh: too many options");
	}

	while(1) {
		char   command   [STRSIZ] = {0};
		char * tokenized [STRSIZ] = {0};

		int stdin_fork  = -1;
		int stdout_fork = -1;
		int stderr_fork = -1;

		int status = 0;
		struct timeval stop, start;

		struct rusage *u1, *u2; 
		if (!(u1 = malloc(sizeof(struct rusage))))
			bail("malloc()");
		if (!(u2 = malloc(sizeof(struct rusage))))
			bail("malloc()");

		float realt, usert, syst;

		if (prompt)
			printf("$ ");
		
		if (!fgets(command, STRSIZ, instream)) {
			printf("\nend of file\n");
			free(u1);
			free(u2);
			exit(returnVal);
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

		if (tokenized[0] == '\0' || tokenized[0][0] == '#' )
			continue;

		int argsDone = 0;
		for(int i = 1; tokenized[i] != 0; i++) {
			if (!strncmp(tokenized[i], "2>>", 3)) {
				stderr_fork = open(&tokenized[i][3], O_WRONLY | O_CREAT | O_APPEND, 0644);
				if (stderr_fork == -1) {
					fprintf(stderr, "Could not open %s\n", &tokenized[i][3]);
					perror("open()");
					returnVal = -1;
					continue;
				}
				argsDone = 1;
			} else if (!strncmp(tokenized[i], "2>", 2)) {
				stderr_fork = open(&tokenized[i][2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (stderr_fork == -1) {
					fprintf(stderr, "Could not open %s\n", &tokenized[i][2]);
					perror("open()");
					returnVal = -1;
					continue;
				}
				argsDone = 1;
			} else if (!strncmp(tokenized[i], ">>", 2)) {
				stdout_fork = open(&tokenized[i][2], O_WRONLY | O_CREAT | O_APPEND, 0644);
				if (stdout_fork == -1) {
					fprintf(stderr, "Could not open %s\n", &tokenized[i][2]);
					perror("open()");
					returnVal = -1;
					continue;
				}
				argsDone = 1;
			} else if (!strncmp(tokenized[i], ">", 1)) {
				stdout_fork = open(&tokenized[i][1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if (stdout_fork == -1) {
					fprintf(stderr, "Could not open %s\n", &tokenized[i][1]);
					perror("open()");
					returnVal = -1;
					continue;
				}
				argsDone = 1;
			} else if (!strncmp(tokenized[i], "<", 1)) {
				stdin_fork = open(&tokenized[i][1], O_RDONLY, 0644);
				if (stdin_fork == -1) {
					fprintf(stderr, "Could not open %s\n", &tokenized[i][1]);
					perror("open()");
					returnVal = -1;
					continue;
				}
				argsDone = 1;
			} else if (argsDone) {
				fprintf(stderr, "Unrecognized stream redirect: %s", tokenized[i]);
				returnVal = -1;
				continue;
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
				getrusage(RUSAGE_SELF, u1);
				gettimeofday(&start, NULL);
				wait(&status);
				gettimeofday(&stop, NULL);
				getrusage(pid, u2);

				realt = time_diff(&start, &stop);
				usert = time_diff(&u2->ru_utime, &u1->ru_utime);
				syst  = time_diff(&u2->ru_stime, &u1->ru_stime);

				memset(u1, 0, sizeof(struct rusage));
				memset(u2, 0, sizeof(struct rusage));
				
				printf("\nCommand returned with return code %d\n", status);
				printf("Consuming %f real seconds, %f user, %f system\n", realt, usert, syst);
				break;
		}
	}
}