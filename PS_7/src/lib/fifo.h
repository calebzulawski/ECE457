#include "sem.h"

#define MYFIFO_BUFSIZ 4096

struct fifo {
	struct sem writeAvailable;
	struct sem readAvailable;
	struct sem mutex;
	unsigned long buffer[MYFIFO_BUFSIZ];
	size_t writeptr;
	size_t readptr;
};

int           fifo_init (struct fifo *f);
void          fifo_wr   (struct fifo *f, unsigned long d);
unsigned long fifo_rd   (struct fifo *f);