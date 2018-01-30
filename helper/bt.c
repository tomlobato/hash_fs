/* Licensed as Public Domain */
#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>

static void dump_trace() {
	void * buffer[255];
	const int calls = backtrace(buffer,
		sizeof(buffer) / sizeof(void *));
	backtrace_symbols_fd(buffer, calls, 1);
	exit(EXIT_FAILURE);
}

void boom() {
	int a[10];
	printf("%d", a[-100000]);
}

void two() { boom(); }
void one() { two(); }

int main() {
	signal(SIGSEGV, dump_trace);
	one();
	return 0;
}