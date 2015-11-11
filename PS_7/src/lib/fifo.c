#include "fifo.h"

int fifo_init(struct fifo *f) {
	f->readptr = 0;
	f->writeptr = 0;
	return sem_init(&f->writeAvailable, MYFIFO_BUFSIZ)
	     & sem_init(&f->readAvailable, 0)
	     & sem_init(&f->mutex, 1);
}

void fifo_wr(struct fifo *f, unsigned long d) {
	while (1) {
		sem_wait(&f->writeAvailable);
		if (sem_try(&f->mutex)) {
			f->buffer[f->writeptr] = d;
			f->writeptr = (f->writeptr + 1) % MYFIFO_BUFSIZ;
			sem_inc(&f->mutex);
			sem_inc(&f->readAvailable);
			return;
		} else {
			sem_inc(&f->writeAvailable);
		}
	}
}

unsigned long fifo_rd(struct fifo *f) {
	unsigned long val;
	while (1) {
		sem_wait(&f->readAvailable);
		if (sem_try(&f->mutex)) {
			val = f->buffer[f->readptr];
			f->readptr = (f->readptr + 1) % MYFIFO_BUFSIZ;
			sem_inc(&f->mutex);
			sem_inc(&f->writeAvailable);
			return val;
		} else {
			sem_inc(&f->readAvailable);
		}
	}
}