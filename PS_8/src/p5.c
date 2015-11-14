#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#define NANO_PER_SEC 1000000000
#define ITERATIONS 1.0e8

long long timediff(struct timespec *start, struct timespec *end) {
	return (end->tv_sec - start->tv_sec) * NANO_PER_SEC + end->tv_nsec - start->tv_nsec;
}

uid_t empty_function() { return 0; };

int main(int argc, char **argv) {
	struct timespec start;
	struct timespec end;

	clock_gettime(CLOCK_REALTIME, &start);
	for (unsigned long i = 0; i < ITERATIONS; i++);
	clock_gettime(CLOCK_REALTIME, &end);

	double loop_time = timediff(&start, &end)/(double)ITERATIONS;

	printf("Loop takes %g nanoseconds\n", loop_time);

	clock_gettime(CLOCK_REALTIME, &start);
	for (unsigned long i = 0; i < ITERATIONS; i++) {
		empty_function();
	}
	clock_gettime(CLOCK_REALTIME, &end);

	double function_time = timediff(&start, &end)/ITERATIONS - loop_time;

	printf("Function takes %g nanoseconds\n", function_time);

	clock_gettime(CLOCK_REALTIME, &start);
	for (unsigned long i = 0; i < ITERATIONS; i++) {
		getuid();
	}
	clock_gettime(CLOCK_REALTIME, &end);

	double syscall_time = timediff(&start, &end)/ITERATIONS - loop_time;

	printf("getuid() takes %g nanoseconds\n", syscall_time);

}