// Copyright (C) 2026 Lubos Dolezel
//
// The software key store of libaks (aks_wrap_key and aks_unwrap_key, what securityd wraps the
// per-item keys of the data protection keychain with): a key wraps into 8 more bytes and back
// (RFC 3394 key wrap), a changed wrapped key is refused and leaves nothing of the key behind,
// only the device key bag (handle 0) exists, lengths near the top of the range are refused
// instead of overflowing, a leftover temporary file does not keep a class key from being made,
// and a class key file that is not exactly one key long is refused.
//
// libaks is a static library: link the libaks.a of the build, with its include directory on the
// include path. It uses classes 240 to 247 of the store, whose files it removes again. Run it
// as the owner of the store (root in the guest). Prints PASS or FAIL lines.
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "libaks.h"

#define STORE "/var/db/org.darlinghq.libaks"
#define FIRST_CLASS 240
#define LAST_CLASS 247

static int failures;
static void expect(int ok, const char *what) {
	printf("%s %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

static void class_path(char *buf, size_t size, int keyclass) {
	snprintf(buf, size, STORE "/bag0-class%d.key", keyclass);
}

static void remove_class(int keyclass) {
	char path[256];
	class_path(path, sizeof(path), keyclass);
	unlink(path);
}

static int is_zero(const uint8_t *p, size_t n) {
	for (size_t i = 0; i < n; i++)
		if (p[i])
			return 0;
	return 1;
}

// Runs fn in a child process, so that a crash is a failure and not the end of the test.
static int in_child(int (*fn)(void)) {
	fflush(stdout);
	pid_t pid = fork();
	if (pid == 0)
		_exit(fn());
	int status = 0;
	if (pid < 0 || waitpid(pid, &status, 0) != pid)
		return -1;
	return WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
}

static uint8_t small_key[32];

static int wrap_huge(void) {
	// a source of almost 4 GiB that is not there: the length check has to refuse it before it is read
	uint8_t out[64];
	int out_len = INT_MAX;
	kern_return_t r = aks_wrap_key(small_key, UINT32_MAX - 7, FIRST_CLASS, device_keybag_handle, out, &out_len, NULL);
	return r == kAKSReturnBadArgument ? 0 : 1;
}

static int unwrap_huge(void) {
	uint8_t out[64];
	int out_len = INT_MAX;
	kern_return_t r = aks_unwrap_key(small_key, UINT32_MAX - 7, FIRST_CLASS, device_keybag_handle, out, &out_len);
	return r == kAKSReturnBadArgument ? 0 : 1;
}

int main(void) {
	uint8_t key[32], wrapped[40], unwrapped[32];
	int wrapped_len, unwrapped_len;
	keyclass_t actual = 0;
	char path[256], what[256];

	for (int c = FIRST_CLASS; c <= LAST_CLASS; c++)
		remove_class(c);
	for (size_t i = 0; i < sizeof(key); i++)
		key[i] = (uint8_t)(i * 7 + 1);

	// a key wraps into 8 more bytes, and back
	wrapped_len = sizeof(wrapped);
	kern_return_t r = aks_wrap_key(key, sizeof(key), FIRST_CLASS, device_keybag_handle, wrapped, &wrapped_len, &actual);
	expect(r == kAKSReturnSuccess && wrapped_len == 40 && actual == FIRST_CLASS, "a 256-bit key is wrapped into 40 bytes");
	expect(memcmp(wrapped + 8, key, sizeof(key)) != 0, "the wrapped key is not the key");
	unwrapped_len = sizeof(unwrapped);
	r = aks_unwrap_key(wrapped, 40, FIRST_CLASS, device_keybag_handle, unwrapped, &unwrapped_len);
	expect(r == kAKSReturnSuccess && unwrapped_len == 32 && memcmp(unwrapped, key, 32) == 0, "and unwrapped back");
	class_path(path, sizeof(path), FIRST_CLASS);
	struct stat st;
	expect(stat(path, &st) == 0 && S_ISREG(st.st_mode) && st.st_size == 32 && (st.st_mode & 077) == 0,
	       "the class key is a file of 32 bytes only its owner can read");

	// another class has another key
	r = aks_unwrap_key(wrapped, 40, FIRST_CLASS + 1, device_keybag_handle, unwrapped, &unwrapped_len);
	expect(r != kAKSReturnSuccess, "a key wrapped for one class does not unwrap with another");

	// a changed wrapped key fails the integrity check, and nothing of it is handed out
	wrapped[20] ^= 1;
	memset(unwrapped, 0xa5, sizeof(unwrapped));
	unwrapped_len = sizeof(unwrapped);
	r = aks_unwrap_key(wrapped, 40, FIRST_CLASS, device_keybag_handle, unwrapped, &unwrapped_len);
	expect(r == kAKSReturnDecodeError, "a changed wrapped key is refused");
	expect(is_zero(unwrapped, sizeof(unwrapped)), "and the output buffer is cleared");
	wrapped[20] ^= 1;

	// the device key bag is the only one
	wrapped_len = sizeof(wrapped);
	expect(aks_wrap_key(key, sizeof(key), FIRST_CLASS, 1, wrapped, &wrapped_len, NULL) == kAKSReturnBadArgument,
	       "wrapping with key bag 1 is refused");
	expect(aks_wrap_key(key, sizeof(key), FIRST_CLASS, bad_keybag_handle, wrapped, &wrapped_len, NULL) == kAKSReturnBadArgument,
	       "wrapping with key bag -1 is refused");
	unwrapped_len = sizeof(unwrapped);
	expect(aks_unwrap_key(wrapped, 40, FIRST_CLASS, 1, unwrapped, &unwrapped_len) == kAKSReturnBadArgument,
	       "unwrapping with key bag 1 is refused");

	// lengths whose wrapped size does not fit
	expect(in_child(wrap_huge) == 0, "wrapping a key of UINT32_MAX - 7 bytes is refused, not overflowed");
	expect(in_child(unwrap_huge) == 0, "unwrapping a key of UINT32_MAX - 7 bytes is refused");
	wrapped_len = 39;
	expect(aks_wrap_key(key, sizeof(key), FIRST_CLASS, device_keybag_handle, wrapped, &wrapped_len, NULL) == kAKSReturnBadArgument,
	       "an output buffer that is too small is refused");

	// a leftover temporary file of another process (named after its pid) does not keep a class key from being made
	int pipefd[2];
	if (pipe(pipefd) != 0)
		return 2;
	fflush(stdout);
	pid_t child = fork();
	if (child == 0) {
		char go;
		close(pipefd[1]);
		if (read(pipefd[0], &go, 1) != 1)
			_exit(3);
		uint8_t w[40];
		int wl = sizeof(w);
		_exit(aks_wrap_key(key, sizeof(key), FIRST_CLASS + 2, device_keybag_handle, w, &wl, NULL) == kAKSReturnSuccess ? 0 : 1);
	}
	close(pipefd[0]);
	class_path(path, sizeof(path), FIRST_CLASS + 2);
	char stale[300];
	snprintf(stale, sizeof(stale), "%s.%d", path, (int)child);
	int fd = open(stale, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd >= 0)
		close(fd);
	write(pipefd[1], "g", 1);
	close(pipefd[1]);
	int status = 0;
	waitpid(child, &status, 0);
	expect(fd >= 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0, "a stale temporary file does not block making a class key");
	unlink(stale);

	// a class key file of the wrong size is refused, and not replaced
	for (int n = 0; n < 2; n++) {
		size_t sizes[] = { 33, 31 };
		int c = FIRST_CLASS + 3 + n;
		class_path(path, sizeof(path), c);
		fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
		uint8_t junk[33];
		memset(junk, 0x5a, sizeof(junk));
		int written = fd >= 0 && write(fd, junk, sizes[n]) == (ssize_t)sizes[n];
		if (fd >= 0)
			close(fd);
		wrapped_len = sizeof(wrapped);
		r = aks_wrap_key(key, sizeof(key), c, device_keybag_handle, wrapped, &wrapped_len, NULL);
		snprintf(what, sizeof(what), "a class key file of %zu bytes is refused", sizes[n]);
		expect(written && r != kAKSReturnSuccess, what);
		snprintf(what, sizeof(what), "and kept as it is (%zu bytes)", sizes[n]);
		expect(stat(path, &st) == 0 && st.st_size == (off_t)sizes[n], what);
	}

	// no temporary files of the test's classes are left in the store
	int leftovers = 0;
	DIR *dir = opendir(STORE);
	struct dirent *entry;
	while (dir != NULL && (entry = readdir(dir)) != NULL) {
		int c = 0;
		char rest[8] = "";
		if (sscanf(entry->d_name, "bag0-class%d.key%7s", &c, rest) == 2 && c >= FIRST_CLASS && c <= LAST_CLASS && rest[0] == '.')
			leftovers++;
	}
	if (dir != NULL)
		closedir(dir);
	expect(dir != NULL && leftovers == 0, "no temporary files are left behind");

	for (int c = FIRST_CLASS; c <= LAST_CLASS; c++)
		remove_class(c);
	printf(failures ? "FAILED %d\n" : "ALL PASSED\n", failures);
	return failures != 0;
}
