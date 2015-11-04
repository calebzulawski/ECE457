#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define _GNU_SOURCE

const char *smallfile = "small.file";

void sig_handle(int s) {
	psignal(s, "Caught signal");
	exit(1);
}

void quit(const char *msg) {
	perror(msg);
	exit(1);
}

size_t file_size(int fd) {
	struct stat s;
	if (fstat(fd, &s) == -1)
		quit("fstat() failed");
	return s.st_size;
}

void* _mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
	void *m;
	if ((m = mmap(addr, length, prot, flags, fd, offset)) == MAP_FAILED)
		quit("mmap() failed");
	return m;
}

int _open(const char *pathname, int flags) {
	int fd;
	if ((fd = open(pathname, flags)) == -1)
		quit("open() failed");
	return fd;
}

void test_A() {
	printf("Opening small file\n");
	int fd = _open(smallfile, O_RDONLY);
	size_t length = file_size(fd);
	printf("File has length %zuB\n", length);
	void *m = _mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
	printf("Writing to map...\n");
	sprintf(m, "a");
}

int main( int argc, const char* argv[] ) {
	if (signal(SIGSEGV,sig_handle) == SIG_ERR)
		quit("signal() failed");
	switch (argv[1][0]) {
		case 'A':
			test_A();
			break;
	}
}