#include "sched.h"

#include <stdio.h>

void init() {
	printf("Hello\n");
}

int main(int argc, char **argv) {
	sched_init(init);
}