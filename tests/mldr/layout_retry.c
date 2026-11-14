/* SPDX-License-Identifier: GPL-3.0-or-later */
// Exercise the actual i386 retry path, replacing only its host system calls.
// Unused loader sections are discarded so no darlingserver is needed.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <unistd.h>

#ifndef __i386__
#error This test must be built with -m32
#endif

static void* retry_mmap(void*, size_t, int, int, int, off_t);
static int retry_munmap(void*, size_t);
static ssize_t retry_readlink(const char*, char*, size_t);
static int retry_setenv(const char*, const char*, int);
static int retry_unsetenv(const char*);
static int retry_execv(const char*, char* const[]);
static int retry_uname(struct utsname*);

#define main mldr_main
#define mmap retry_mmap
#define munmap retry_munmap
#define readlink retry_readlink
#define setenv retry_setenv
#define unsetenv retry_unsetenv
#define execv retry_execv
#define uname retry_uname
#define commpage_address retry_commpage_address
#define commpage_length retry_commpage_length
#ifndef MLDR_SOURCE
#define MLDR_SOURCE "mldr.c"
#endif
#include MLDR_SOURCE
#undef main
#undef mmap
#undef munmap
#undef readlink
#undef setenv
#undef unsetenv
#undef execv
#undef uname
#undef commpage_address
#undef commpage_length

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
		exit(1); \
	} \
} while (0)

static int maps, unmaps, links, sets, execs, unsets;
static int map_mode, map_error, fail_readlink, fail_setenv;
static int chain_mode, chain_fd;
static char next_count[16];
static char** expected_argv;

unsigned long retry_commpage_address(bool wide) { (void)wide; return 0xffff0000UL; }
unsigned long retry_commpage_length(bool wide) { (void)wide; return 0x1000; }

// Keep compatible_mmap's legacy-kernel fallback outside this retry-path test.
static int retry_uname(struct utsname* name)
{
	memset(name, 0, sizeof(*name));
	strcpy(name->release, "6.0.0");
	return 0;
}

static void* retry_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	++maps;
	CHECK(addr == (void*)0xfffe0000UL && length == 0x11000);
	CHECK(prot == PROT_NONE && (flags & MAP_ANONYMOUS) && fd == -1 && offset == 0);
	if (map_mode == 1) return addr;
	if (map_mode == 2) return (char*)addr - 0x10000;
	errno = map_error;
	return MAP_FAILED;
}

static int retry_munmap(void* addr, size_t length)
{
	++unmaps;
	CHECK(length == 0x11000);
	CHECK(addr == (void*)(map_mode == 1 ? 0xfffe0000UL : 0xfffd0000UL));
	return 0;
}

static ssize_t retry_readlink(const char* path, char* buffer, size_t size)
{
	++links;
	CHECK(strcmp(path, "/proc/self/exe") == 0);
	if (chain_mode) return readlink(path, buffer, size);
	if (fail_readlink) { errno = ENOENT; return -1; }
	const char self[] = "/synthetic/mldr32";
	CHECK(size >= sizeof(self) - 1);
	memcpy(buffer, self, sizeof(self) - 1); // readlink does not append NUL
	return sizeof(self) - 1;
}

static int retry_setenv(const char* name, const char* value, int overwrite)
{
	++sets;
	CHECK(strcmp(name, LAYOUT_RETRY_ENV) == 0 && overwrite == 1);
	CHECK(strlen(value) < sizeof(next_count));
	strcpy(next_count, value);
	if (fail_setenv) { errno = ENOMEM; return -1; }
	return setenv(name, value, overwrite);
}

static int retry_unsetenv(const char* name)
{
	++unsets;
	CHECK(strcmp(name, LAYOUT_RETRY_ENV) == 0);
	return unsetenv(name);
}

static int retry_execv(const char* path, char* const argv[])
{
	++execs;
	CHECK(argv == expected_argv);
	CHECK(getenv(LAYOUT_RETRY_ENV) != NULL);
	CHECK(strcmp(getenv(LAYOUT_RETRY_ENV), next_count) == 0);
	if (chain_mode) {
		CHECK(strlen(next_count) == 1 && next_count[0] >= '1' && next_count[0] <= '8');
		CHECK(write(chain_fd, next_count, 1) == 1);
		return execv(path, argv);
	}
	CHECK(strcmp(path, "/synthetic/mldr32") == 0);
	errno = ENOENT;
	return -1;
}

static void reset(const char* value)
{
	maps = unmaps = links = sets = execs = unsets = 0;
	map_mode = fail_readlink = fail_setenv = 0;
	map_error = EEXIST;
	next_count[0] = '\0';
	CHECK(unsetenv(LAYOUT_RETRY_ENV) == 0);
	if (value) CHECK(setenv(LAYOUT_RETRY_ENV, value, 1) == 0);
}

static void invoke(void)
{
	char* argv[] = { "mldr32", "guest", NULL };
	struct load_results lr = { .argv = argv };
	expected_argv = argv;
	ensure_top_range_free(&lr, 0x10000);
	CHECK(maps == 1);
}

static void unchanged(const char* value)
{
	const char* actual = getenv(LAYOUT_RETRY_ENV);
	CHECK(value ? actual && strcmp(actual, value) == 0 : actual == NULL);
}

static void counter(const char* value, const char* expected)
{
	reset(value);
	errno = ERANGE; // a previous error must not reject a valid conversion
	invoke();
	if (expected) {
		CHECK(links == 1 && sets == 1 && execs == 1 && unsets == 1);
		CHECK(strcmp(next_count, expected) == 0);
		CHECK(getenv(LAYOUT_RETRY_ENV) == NULL); // failed exec cleanup
	} else {
		CHECK(links == 0 && sets == 0 && execs == 0 && unsets == 0);
		unchanged(value);
	}
}

static void chain(char** argv)
{
	chain_mode = 1;
	chain_fd = atoi(argv[2]);
	map_error = EEXIST;
	expected_argv = argv;
	struct load_results lr = { .argv = argv };
	ensure_top_range_free(&lr, 0x10000);
	CHECK(getenv(LAYOUT_RETRY_ENV) && strcmp(getenv(LAYOUT_RETRY_ENV), "8") == 0);
	CHECK(write(chain_fd, "X", 1) == 1);
	_exit(0);
}

static void bounded_chain(void)
{
	int fds[2];
	CHECK(pipe(fds) == 0);
	pid_t child = fork();
	CHECK(child >= 0);
	if (child == 0) {
		close(fds[0]);
		char descriptor[16];
		snprintf(descriptor, sizeof(descriptor), "%d", fds[1]);
		char* argv[] = { "/proc/self/exe", "--retry-chain", descriptor, NULL };
		CHECK(unsetenv(LAYOUT_RETRY_ENV) == 0);
		execv(argv[0], argv);
		_exit(2);
	}
	close(fds[1]);
	char observed[10] = { 0 };
	size_t used = 0;
	bool failed = false;
	for (;;) {
		struct pollfd pfd = { .fd = fds[0], .events = POLLIN };
		if (poll(&pfd, 1, 3000) <= 0) { failed = true; break; }
		char byte;
		ssize_t n = read(fds[0], &byte, 1);
		if (n == 0) break;
		if (n != 1 || used >= sizeof(observed) - 1) { failed = true; break; }
		observed[used++] = byte;
	}
	if (failed) kill(child, SIGKILL);
	close(fds[0]);
	int status;
	CHECK(waitpid(child, &status, 0) == child);
	CHECK(!failed && WIFEXITED(status) && WEXITSTATUS(status) == 0);
	CHECK(strcmp(observed, "12345678X") == 0);
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "--retry-chain") == 0) chain(argv);
	counter(NULL, "1");
	counter("0", "1");
	counter("1", "2");
	counter("7", "8");
	counter("8", NULL);
	counter("0007", "8");
	const char* invalid[] = {
		"", "-1", "-8", "-2147483648", "+1", "abc", "1x", "7suffix",
		" 1", "1 ", "\t1", "1\t", "\n1", "1\n", "0x1", "1.0", "9",
		"2147483647", "2147483648", "4294967296", "9223372036854775808",
		"9999999999999999999999999999999999999999999999999999999999",
		"-9999999999999999999999999999999999999999999999999999999999",
	};
	for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) counter(invalid[i], NULL);
	puts("PASS: valid, exhausted and malformed counters; failed exec cleanup");

	const char* initial[] = { NULL, "0", "1", "7" };
	for (size_t i = 0; i < sizeof(initial) / sizeof(initial[0]); ++i) {
		reset(initial[i]); fail_setenv = 1; invoke();
		CHECK(links == 1 && sets == 1 && execs == 0 && unsets == 0);
		unchanged(initial[i]);
	}
	puts("PASS: setenv failure never re-execs or changes the old counter");

	reset("0"); fail_readlink = 1; invoke();
	CHECK(links == 1 && sets == 0 && execs == 0 && unsets == 0); unchanged("0");
	reset("0"); map_mode = 1; invoke();
	CHECK(unmaps == 1 && links == 0 && sets == 0 && execs == 0); unchanged("0");
	reset("0"); map_error = ENOMEM; invoke();
	CHECK(unmaps == 0 && links == 0 && sets == 0 && execs == 0); unchanged("0");
	reset("0"); map_mode = 2; invoke();
	CHECK(unmaps == 1 && links == 1 && sets == 1 && execs == 1 && unsets == 1);
	puts("PASS: free range, mapping error, misplaced probe and readlink failure");

	bounded_chain();
	puts("PASS: persistent conflict stops after exactly eight real re-execs");
	return 0;
}
