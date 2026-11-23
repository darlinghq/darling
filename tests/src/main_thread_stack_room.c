/*
 Copyright (C) 2026 Lubos Dolezel

 Licensed under the GPL, version 3 or (at your option) any later version.
*/
// The main thread of a Mach-O process has to be able to use a deep stack
// (a few hundred kilobytes here; macOS gives it 8 MB) without damaging
// anything else. The strings of argv and envp live in the host's initial
// stack, which the loader maps at a random place in a 32-bit address space; if
// the main thread's stack runs into that mapping the strings are overwritten
// without any fault. The test uses a deep stack and then checks that argv[0]
// and every environment string are unchanged.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

#define FRAME_BYTES 8192
#define DEPTH 96 // about 768 KB

static int bad_frames;
static volatile char sink;

static __attribute__((noinline)) void use_stack(int depth) {
	volatile char frame[FRAME_BYTES];
	unsigned char pattern = (unsigned char) (0xA5 ^ depth);
	for (int i = 0; i < FRAME_BYTES; i++)
		frame[i] = (char) pattern;
	if (depth > 0)
		use_stack(depth - 1);
	for (int i = 0; i < FRAME_BYTES; i++) {
		if ((unsigned char) frame[i] != pattern) {
			bad_frames++;
			break;
		}
	}
	sink = frame[depth];
}

int main(int argc, char **argv) {
	size_t total = strlen(argv[0]) + 1;
	for (char **e = environ; *e != NULL; e++)
		total += strlen(*e) + 1;

	char *copy = malloc(total);
	char *p = copy;
	p = stpcpy(p, argv[0]) + 1;
	for (char **e = environ; *e != NULL; e++)
		p = stpcpy(p, *e) + 1;

	use_stack(DEPTH);

	int failures = 0;
	p = copy;
	if (strcmp(argv[0], p) != 0) {
		printf("FAIL argv[0] changed after using a deep main-thread stack\n");
		failures++;
	}
	p += strlen(p) + 1;
	for (char **e = environ; *e != NULL; e++) {
		if (strcmp(*e, p) != 0) {
			printf("FAIL environment string changed after using a deep main-thread stack: %.30s\n", *e);
			failures++;
			break;
		}
		p += strlen(p) + 1;
	}
	if (bad_frames != 0) {
		printf("FAIL %d stack frames changed under the program\n", bad_frames);
		failures++;
	}
	puts(failures ? "FAIL" : "PASS");
	return failures ? 1 : 0;
}
