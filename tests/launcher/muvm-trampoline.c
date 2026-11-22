// Host-side test of the launcher's muvm trampoline (src/startup/muvm-trampoline.c).
//
// The Mach-O headers are written here byte by byte, so the sniffing is checked
// against the on-disk layout rather than against the code that reads it. The
// re-exec is run in a child with a stub muvm on PATH, which records the argv it
// was handed.
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "muvm-trampoline.h"

#define MH_MAGIC_64 0xfeedfacfU
#define MH_MAGIC 0xfeedfaceU
#define CPU_TYPE_ARM 12U
#define CPU_TYPE_X86 7U
#define CPU_ARCH_ABI64 0x01000000U
#define CPU_TYPE_ARM64 (CPU_TYPE_ARM | CPU_ARCH_ABI64)
#define CPU_TYPE_X86_64 (CPU_TYPE_X86 | CPU_ARCH_ABI64)

static int g_failures;
static char g_directory[PATH_MAX];

static void check(bool ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		g_failures++;
}

static const char* inDirectory(const char* name)
{
	static char path[2][PATH_MAX];
	static int slot;

	slot = (slot + 1) % 2;
	snprintf(path[slot], sizeof(path[slot]), "%s/%s", g_directory, name);
	return path[slot];
}

static void put32(uint8_t* at, uint32_t value)
{
	at[0] = (uint8_t) (value >> 24);
	at[1] = (uint8_t) (value >> 16);
	at[2] = (uint8_t) (value >> 8);
	at[3] = (uint8_t) value;
}

// A 64-bit thin Mach-O: magic, cputype, and the rest of the fixed header.
static void writeThin(const char* path, uint32_t cputype)
{
	uint8_t header[32] = { 0 };
	FILE* file = fopen(path, "wb");

	header[0] = 0xcf;
	header[1] = 0xfa;
	header[2] = 0xed;
	header[3] = 0xfe;
	header[4] = (uint8_t) cputype;
	header[5] = (uint8_t) (cputype >> 8);
	header[6] = (uint8_t) (cputype >> 16);
	header[7] = (uint8_t) (cputype >> 24);

	if (file != NULL)
	{
		fwrite(header, 1, sizeof(header), file);
		fclose(file);
	}
}

// A universal binary, which is stored big-endian.
static void writeFat(const char* path, const uint32_t* cputypes, size_t count)
{
	size_t sz = 8 + 20 * count;
	uint8_t* header = malloc(sz);
	if (!header)
		return;
	memset(header, 0, sz);
	FILE* file = fopen(path, "wb");

	put32(header, 0xcafebabeU);
	put32(header + 4, (uint32_t) count);

	for (size_t i = 0; i < count; i++)
	{
		put32(header + 8 + 20 * i, cputypes[i]);
		put32(header + 8 + 20 * i + 8, 0x1000U * (uint32_t) (i + 1));
		put32(header + 8 + 20 * i + 12, 0x1000U * (uint32_t) (i + 1));
	}

	if (file != NULL)
	{
		fwrite(header, 1, sz, file);
		fclose(file);
	}
	free(header);
}

static void writeFat64(const char* path, const uint32_t* cputypes, size_t count)
{
	size_t sz = 8 + 32 * count;
	uint8_t* header = malloc(sz);
	if (!header)
		return;
	memset(header, 0, sz);
	FILE* file = fopen(path, "wb");

	put32(header, 0xcafebabfU);
	put32(header + 4, (uint32_t) count);

	for (size_t i = 0; i < count; i++)
	{
		put32(header + 8 + 32 * i, cputypes[i]);
		put32(header + 8 + 32 * i + 12, 0x1000U * (uint32_t) (i + 1));
		put32(header + 8 + 32 * i + 20, 0x1000U * (uint32_t) (i + 1));
	}

	if (file != NULL)
	{
		fwrite(header, 1, sz, file);
		fclose(file);
	}
	free(header);
}

static void test_thin_files(void)
{
	writeThin(inDirectory("thin-x86_64"), CPU_TYPE_X86_64);
	check(darling_target_slice_arch(inDirectory("thin-x86_64"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"a thin x86_64 Mach-O is an x86_64 slice");

	writeThin(inDirectory("thin-arm64"), CPU_TYPE_ARM64);
	check(darling_target_slice_arch(inDirectory("thin-arm64"), NULL) == DARLING_TARGET_SLICE_ARM64,
		"a thin arm64 Mach-O is an arm64 slice");

	writeThin(inDirectory("thin-x86"), CPU_TYPE_X86);
	check(darling_target_slice_arch(inDirectory("thin-x86"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a 32-bit thin Mach-O is nothing the launcher hands to muvm");

	int fd = open(inDirectory("not-macho"), O_WRONLY | O_CREAT | O_TRUNC, 0644);

	if (fd >= 0)
	{
		write(fd, "\177ELF\x02\x01\x01", 7);
		close(fd);
	}
	check(darling_target_slice_arch(inDirectory("not-macho"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a file that is not Mach-O has no slice");

	check(darling_target_slice_arch(inDirectory("missing"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a missing file has no slice");
	check(darling_target_slice_arch(g_directory, NULL) == DARLING_TARGET_SLICE_NONE,
		"a directory has no slice");
	check(darling_target_slice_arch(NULL, NULL) == DARLING_TARGET_SLICE_NONE,
		"a null path has no slice");
}

static void test_fat_files(void)
{
	const uint32_t both[] = { CPU_TYPE_ARM64, CPU_TYPE_X86_64 };
	const uint32_t intel[] = { CPU_TYPE_X86_64 };
	const uint32_t arm32AndIntel[] = { CPU_TYPE_ARM, CPU_TYPE_X86_64 };
	const uint32_t bprefsIntel[] = { CPU_TYPE_X86_64, 0, 0, 0 };
	const uint32_t bprefsBothWays[] = { CPU_TYPE_X86_64, CPU_TYPE_ARM64, 0, 0 };
	const uint32_t bprefsArmSecond[] = { 0, CPU_TYPE_ARM64, 0, 0 };

	writeFat(inDirectory("fat-both"), both, 2);
	check(darling_target_slice_arch(inDirectory("fat-both"), NULL) == DARLING_TARGET_SLICE_ARM64,
		"arm64 wins a universal binary that also has x86_64");

	writeFat(inDirectory("fat-intel"), intel, 1);
	check(darling_target_slice_arch(inDirectory("fat-intel"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"an x86_64-only universal binary is an x86_64 slice");

	writeFat(inDirectory("fat-arm32-intel"), arm32AndIntel, 2);
	check(darling_target_slice_arch(inDirectory("fat-arm32-intel"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"x86_64 is preferred over a 32-bit arm slice");

	check(darling_target_slice_arch(inDirectory("fat-both"), bprefsIntel) == DARLING_TARGET_SLICE_X86_64,
		"__mldr_bprefs picks x86_64 out of a universal binary");
	check(darling_target_slice_arch(inDirectory("fat-both"), bprefsBothWays) == DARLING_TARGET_SLICE_X86_64,
		"the first matching __mldr_bprefs entry wins");
	check(darling_target_slice_arch(inDirectory("fat-both"), bprefsArmSecond) == DARLING_TARGET_SLICE_ARM64,
		"a later __mldr_bprefs entry that matches still wins over the default order");

	writeFat(inDirectory("fat-truncated"), both, 2);
	{
		int fd = open(inDirectory("fat-truncated"), O_WRONLY | O_TRUNC);

		if (fd >= 0)
		{
			uint8_t magic[8] = { 0xca, 0xfe, 0xba, 0xbe, 0, 0, 0, 3 };
			write(fd, magic, sizeof(magic));
			close(fd);
		}
	}
	check(darling_target_slice_arch(inDirectory("fat-truncated"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a truncated universal binary is not read past its end");

	uint32_t atCap[64];
	for (int i = 0; i < 63; i++)
		atCap[i] = 0x9999;
	atCap[63] = CPU_TYPE_X86_64;
	writeFat(inDirectory("fat-64-entries"), atCap, 64);
	check(darling_target_slice_arch(inDirectory("fat-64-entries"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"an x86_64 slice at entry 64 (the cap) is found by streaming");

	uint32_t aboveCap[65];
	for (int i = 0; i < 64; i++)
		aboveCap[i] = 0x9999;
	aboveCap[64] = CPU_TYPE_X86_64;
	writeFat(inDirectory("fat-65-entries"), aboveCap, 65);
	check(darling_target_slice_arch(inDirectory("fat-65-entries"), NULL) == DARLING_TARGET_SLICE_NONE,
		"an architecture count above the 64-entry cap is rejected");

	writeFat64(inDirectory("fat-64bit-header"), intel, 1);
	check(darling_target_slice_arch(inDirectory("fat-64bit-header"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"a 64-bit fat Mach-O (FAT_MAGIC_64) is recognized");

	writeFat64(inDirectory("fat-64bit-both"), arm32AndIntel, 2);
	check(darling_target_slice_arch(inDirectory("fat-64bit-both"), NULL) == DARLING_TARGET_SLICE_X86_64,
		"64-bit fat header with 2 entries selects x86_64 at 32-byte stride");

	{
		int fd = open(inDirectory("fat-zero"), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd >= 0)
		{
			uint8_t hdr[8] = { 0xca, 0xfe, 0xba, 0xbe, 0, 0, 0, 0 };
			write(fd, hdr, sizeof(hdr));
			close(fd);
		}
	}
	check(darling_target_slice_arch(inDirectory("fat-zero"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a fat table with zero entries is rejected");

	{
		int fd = open(inDirectory("fat-wrap"), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd >= 0)
		{
			uint8_t hdr[8] = { 0xca, 0xfe, 0xba, 0xbe, 0xff, 0xff, 0xff, 0xff };
			write(fd, hdr, sizeof(hdr));
			close(fd);
		}
	}
	check(darling_target_slice_arch(inDirectory("fat-wrap"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a 32-bit wrapped entry count is rejected by bounds check");

	writeFat(inDirectory("fat-exceeds-size"), both, 2);
	{
		int fd = open(inDirectory("fat-exceeds-size"), O_WRONLY);
		if (fd >= 0)
		{
			uint8_t count[4] = { 0, 0, 0, 3 };
			lseek(fd, 4, SEEK_SET);
			write(fd, count, sizeof(count));
			close(fd);
		}
	}
	check(darling_target_slice_arch(inDirectory("fat-exceeds-size"), NULL) == DARLING_TARGET_SLICE_NONE,
		"a fat table declaring more entries than file size allows is rejected");
}

static void test_bprefs_environment(void)
{
	uint32_t bprefs[4];

	setenv("__mldr_bprefs", "01000007,0100000c", 1);
	darling_read_mldr_bprefs(bprefs);
	check(bprefs[0] == CPU_TYPE_X86_64 && bprefs[1] == CPU_TYPE_ARM64
			&& bprefs[2] == 0 && bprefs[3] == 0,
		"the architecture preference comes from __mldr_bprefs");

	setenv("__mldr_bprefs", "not a preference", 1);
	darling_read_mldr_bprefs(bprefs);
	check(bprefs[0] == 0 && bprefs[1] == 0 && bprefs[2] == 0 && bprefs[3] == 0,
		"an unparsable preference selects nothing");

	unsetenv("__mldr_bprefs");
	darling_read_mldr_bprefs(bprefs);
	check(bprefs[0] == 0 && bprefs[1] == 0 && bprefs[2] == 0 && bprefs[3] == 0,
		"without the variable every preference is zero");
}

static void test_needed_matrix(void)
{
	check(!darling_muvm_needed(4096, DARLING_TARGET_SLICE_X86_64),
		"4 KB pages can load x86_64 without muvm");
	check(darling_muvm_needed(16384, DARLING_TARGET_SLICE_X86_64),
		"16 KB pages need muvm for x86_64");
	check(darling_muvm_needed(65536, DARLING_TARGET_SLICE_X86_64),
		"64 KB pages need muvm for x86_64");
	check(!darling_muvm_needed(16384, DARLING_TARGET_SLICE_ARM64),
		"arm64 runs natively on larger pages");
	check(!darling_muvm_needed(16384, DARLING_TARGET_SLICE_NONE),
		"an unknown slice never goes to muvm");
}

static int openTerminal(void)
{
	int master = posix_openpt(O_RDWR | O_NOCTTY);

	if (master < 0)
		return -1;
	if (grantpt(master) != 0 || unlockpt(master) != 0)
	{
		close(master);
		return -1;
	}

	return master;
}

static void test_stdio(void)
{
	int one = openTerminal(), two = openTerminal(), three = openTerminal();
	int pipeEnds[2] = { -1, -1 };

	if (one < 0 || two < 0 || three < 0 || pipe(pipeEnds) != 0)
	{
		printf("SKIP: no terminal available\n");
		return;
	}

	check(darling_muvm_stdio_ok(one, two, three),
		"three terminals are accepted");
	check(!darling_muvm_stdio_ok(pipeEnds[0], two, three),
		"a redirected standard input is rejected");
	check(!darling_muvm_stdio_ok(one, pipeEnds[1], three),
		"a redirected standard output is rejected");
	check(!darling_muvm_stdio_ok(one, two, pipeEnds[1]),
		"a redirected standard error is rejected");

	close(one);
	close(two);
	close(three);
	close(pipeEnds[0]);
	close(pipeEnds[1]);
}

static void writeStubMuvm(const char* directory, const char* recordPath)
{
	char path[PATH_MAX];
	FILE* file;

	snprintf(path, sizeof(path), "%s/muvm", directory);
	file = fopen(path, "w");
	if (file == NULL)
		return;
	fprintf(file,
		"#!/bin/sh\n"
		"exec > '%s'\n"
		"for argument in \"$@\"; do printf 'ARG:%%s\\n' \"$argument\"; done\n",
		recordPath);
	fclose(file);
	chmod(path, 0755);
}

// Runs the trampoline in a child and returns its exit status, or -1.
static int runReexec(char* const argv[], size_t targetIndex, const char* targetPath)
{
	pid_t child = fork();
	int status;

	if (child < 0)
		return -1;

	if (child == 0)
	{
		darling_reexec_under_muvm(argv, targetIndex, targetPath);
		_exit(3);
	}

	if (waitpid(child, &status, 0) != child || !WIFEXITED(status))
		return -1;

	return WEXITSTATUS(status);
}

static size_t readRecord(const char* path, char lines[][PATH_MAX], size_t max)
{
	FILE* file = fopen(path, "r");
	size_t count = 0;
	char line[PATH_MAX];

	if (file == NULL)
		return 0;

	while (count < max && fgets(line, sizeof(line), file) != NULL)
	{
		line[strcspn(line, "\n")] = '\0';
		if (strncmp(line, "ARG:", 4) == 0)
			snprintf(lines[count++], PATH_MAX, "%s", line + 4);
	}

	fclose(file);
	return count;
}

static bool hasPair(char lines[][PATH_MAX], size_t count, const char* name)
{
	for (size_t i = 0; i + 1 < count; i++)
	{
		if (strcmp(lines[i], "-e") == 0 && strcmp(lines[i + 1], name) == 0)
			return true;
	}

	return false;
}

static size_t indexOf(char lines[][PATH_MAX], size_t count, const char* value)
{
	for (size_t i = 0; i < count; i++)
	{
		if (strcmp(lines[i], value) == 0)
			return i;
	}

	return (size_t) -1;
}

static void test_reexec(void)
{
	char stubDirectory[PATH_MAX], recordPath[PATH_MAX], launcher[PATH_MAX];
	char lines[256][PATH_MAX];
	char* execArgv[] = { (char*) "darling", (char*) "exec", (char*) "some/app", (char*) "first", (char*) "second", NULL };
	char* plainArgv[] = { (char*) "darling", (char*) "some/app", (char*) "first", NULL };
	const char* path = getenv("PATH");
	ssize_t launcherLength;
	size_t count, dash;

	snprintf(stubDirectory, sizeof(stubDirectory), "%s/stub", g_directory);
	mkdir(stubDirectory, 0755);
	snprintf(recordPath, sizeof(recordPath), "%s/record", g_directory);
	writeStubMuvm(stubDirectory, recordPath);
	check(access(inDirectory("stub/muvm"), X_OK) == 0, "the stub muvm is executable");

	setenv("MUVM_RECORD", recordPath, 1);
	if (path != NULL)
	{
		char searchPath[PATH_MAX * 2];
		snprintf(searchPath, sizeof(searchPath), "%s:%s", stubDirectory, path);
		setenv("PATH", searchPath, 1);
	}
	else
		setenv("PATH", stubDirectory, 1);

	setenv("HOME", "/home/someone", 1);
	setenv("TERM", "xterm", 1);
	setenv("DARLING_TEST_VARIABLE", "1", 1);
	setenv("NOT_FORWARDED", "1", 1);
	unsetenv("TMPDIR");
	unsetenv("GDK_SCALE");
	unsetenv("__mldr_bprefs");

	launcherLength = readlink("/proc/self/exe", launcher, sizeof(launcher) - 1);
	if (launcherLength <= 0)
	{
		check(false, "this test can find its own executable");
		return;
	}
	launcher[launcherLength] = '\0';

	unlink(recordPath);
	check(runReexec(execArgv, 2, "/resolved/app") == 0, "the exec form re-execs muvm");
	count = readRecord(recordPath, lines, 256);
	check(count > 3 && strcmp(lines[0], "-e") == 0, "muvm receives -e before any command");

	dash = indexOf(lines, count, "--");
	check(dash != (size_t) -1 && dash > 0, "muvm receives a -- separator");

	if (dash != (size_t) -1 && dash + 6 == count)
	{
		check(strcmp(lines[dash + 1], launcher) == 0,
			"the launcher is passed by absolute path");
		check(strcmp(lines[dash + 2], "exec") == 0,
			"the exec subcommand is kept");
		check(strcmp(lines[dash + 3], "/resolved/app") == 0,
			"the target is replaced by its resolved path");
		check(strcmp(lines[dash + 4], "first") == 0 && strcmp(lines[dash + 5], "second") == 0,
			"the target's own arguments follow it");
	}
	else
		check(false, "the exec form ends after the target's arguments");

	check(hasPair(lines, count, "HOME"), "HOME is forwarded");
	check(hasPair(lines, count, "TERM"), "TERM is forwarded");
	check(hasPair(lines, count, "DARLING_TEST_VARIABLE"), "every DARLING_ variable is forwarded");
	check(!hasPair(lines, count, "NOT_FORWARDED"), "an unrelated variable is not forwarded");
	check(!hasPair(lines, count, "TMPDIR"), "an unset variable is not forwarded");
	check(!hasPair(lines, count, "__mldr_bprefs"), "an unset architecture preference is not forwarded");
	check(!hasPair(lines, count, "GDK_SCALE"), "an unset desktop scale is not forwarded");

	setenv("__mldr_bprefs", "01000007", 1);
	setenv("GDK_SCALE", "2", 1);
	unlink(recordPath);
	check(runReexec(plainArgv, 1, "/resolved/app") == 0, "the plain form re-execs muvm");
	count = readRecord(recordPath, lines, 256);
	check(hasPair(lines, count, "__mldr_bprefs"),
		"a set architecture preference reaches the guest");
	check(hasPair(lines, count, "GDK_SCALE"), "the desktop's scale reaches the guest");

	dash = indexOf(lines, count, "--");
	check(dash != (size_t) -1 && dash > 0, "the plain form reaches the separator");

	if (dash != (size_t) -1 && dash + 4 == count)
	{
		check(strcmp(lines[dash + 1], launcher) == 0,
			"the plain form passes the launcher by absolute path");
		check(strcmp(lines[dash + 2], "/resolved/app") == 0,
			"the plain form replaces the target with its resolved path");
		check(strcmp(lines[dash + 3], "first") == 0,
			"the plain form keeps the target's arguments");
	}
	else
		check(false, "the plain form ends after the target's arguments");

	unsetenv("__mldr_bprefs");
	unsetenv("GDK_SCALE");
}

static int g_probeCalls;
static bool g_probeAnswer;

static bool fakeProbe(const char* path_env)
{
	(void) path_env;
	g_probeCalls++;
	return g_probeAnswer;
}

static void test_wayland_wanted(void)
{
	check(darling_wayland_display_is_plain("wayland-0"), "a socket name is plain");
	check(!darling_wayland_display_is_plain(NULL), "unset is not plain");
	check(!darling_wayland_display_is_plain(""), "empty is not plain");
	check(!darling_wayland_display_is_plain("/run/user/1000/wayland-0"), "a path is not plain");
	check(!darling_wayland_display_is_plain("a/b"), "a name with a slash is not plain");
	check(!darling_wayland_display_is_plain(".") && !darling_wayland_display_is_plain(".."),
		"dot names are not plain");

	g_probeAnswer = true;
	g_probeCalls = 0;
	check(darling_muvm_wayland_wanted("wayland-1", "/bin", fakeProbe) && g_probeCalls == 1,
		"a session and a capable muvm give Wayland");
	g_probeAnswer = false;
	g_probeCalls = 0;
	check(!darling_muvm_wayland_wanted("wayland-1", "/bin", fakeProbe) && g_probeCalls == 1,
		"a stock muvm is left alone");

	g_probeAnswer = true;
	g_probeCalls = 0;
	check(!darling_muvm_wayland_wanted(NULL, "/bin", fakeProbe) && g_probeCalls == 0,
		"an X11 only host does not run the probe");
	check(!darling_muvm_wayland_wanted("", "/bin", fakeProbe) && g_probeCalls == 0,
		"an empty WAYLAND_DISPLAY does not run the probe");
	check(!darling_muvm_wayland_wanted("/run/user/1/wayland-1", "/bin", fakeProbe) && g_probeCalls == 0,
		"a path as WAYLAND_DISPLAY, which muvm does not serve, does not run the probe");
}

// A file of size bytes of filler with text written at offset, executable or not.
static void writeFile(const char* path, size_t size, size_t offset, const char* text, mode_t mode)
{
	FILE* file = fopen(path, "w");
	size_t length = text != NULL ? strlen(text) : 0;

	if (file == NULL)
		return;
	for (size_t at = 0; at < size; at++)
	{
		size_t into = at - offset;
		fputc(text != NULL && at >= offset && into < length ? text[into] : 'x', file);
	}
	fclose(file);
	chmod(path, mode);
}

static void makeMuvmDirectory(const char* name, const char* guestText, size_t guestOffset, bool proxy)
{
	char directory[PATH_MAX], path[PATH_MAX];

	snprintf(directory, sizeof(directory), "%s/%s", g_directory, name);
	mkdir(directory, 0755);
	snprintf(path, sizeof(path), "%s/muvm-guest", directory);
	writeFile(path, 70000, guestOffset, guestText, 0755);
	if (proxy)
	{
		snprintf(path, sizeof(path), "%s/wl-cross-domain-proxy", directory);
		writeFile(path, 16, 0, NULL, 0755);
	}
}

static void test_wayland_capable(void)
{
	char patched[PATH_MAX], stock[PATH_MAX], noProxy[PATH_MAX], straddle[PATH_MAX], both[PATH_MAX];
	char path[PATH_MAX * 2];
	char notExecutable[PATH_MAX];

	// The proxy's name sits well inside the patched guest, across the 64 KB
	// read boundary in "straddle", and nowhere in the stock one.
	makeMuvmDirectory("patched", "wl-cross-domain-proxy", 1000, true);
	makeMuvmDirectory("stock", NULL, 0, true);
	makeMuvmDirectory("noproxy", "wl-cross-domain-proxy", 1000, false);
	makeMuvmDirectory("straddle", "wl-cross-domain-proxy", 65536 - 7, true);
	makeMuvmDirectory("tail", "wl-cross-domain-proxy", 70000 - 21, true);
	makeMuvmDirectory("partial", "wl-cross-domain-proxy-", 70000 - 12, true);
	snprintf(patched, sizeof(patched), "%s/patched", g_directory);
	snprintf(stock, sizeof(stock), "%s/stock", g_directory);
	snprintf(noProxy, sizeof(noProxy), "%s/noproxy", g_directory);
	snprintf(straddle, sizeof(straddle), "%s/straddle", g_directory);

	check(darling_muvm_wayland_capable(patched), "a muvm-guest that names the proxy, with the proxy, is capable");
	check(!darling_muvm_wayland_capable(stock), "a stock muvm-guest is not capable even with the proxy installed");
	check(!darling_muvm_wayland_capable(noProxy), "a patched muvm-guest without the proxy is not capable");
	check(darling_muvm_wayland_capable(straddle), "a name across the read boundary is found");
	snprintf(both, sizeof(both), "%s/tail", g_directory);
	check(darling_muvm_wayland_capable(both), "a name at the very end of the file is found");
	snprintf(both, sizeof(both), "%s/partial", g_directory);
	check(!darling_muvm_wayland_capable(both), "half a name at the end of the file is not found");

	snprintf(path, sizeof(path), "%s:%s", stock, patched);
	check(!darling_muvm_wayland_capable(path), "the first muvm-guest in PATH is the one muvm runs");
	snprintf(path, sizeof(path), "%s:%s", patched, stock);
	check(darling_muvm_wayland_capable(path), "the first muvm-guest in PATH is the patched one");
	snprintf(path, sizeof(path), "%s:%s", noProxy, patched);
	check(darling_muvm_wayland_capable(path), "the proxy is found anywhere in PATH");
	snprintf(path, sizeof(path), "/nonexistent::%s", patched);
	check(darling_muvm_wayland_capable(path), "an empty or missing PATH element is skipped");
	check(!darling_muvm_wayland_capable(NULL), "no PATH is not capable");
	check(!darling_muvm_wayland_capable(""), "an empty PATH is not capable");

	snprintf(notExecutable, sizeof(notExecutable), "%s/patched/muvm-guest", g_directory);
	chmod(notExecutable, 0644);
	check(!darling_muvm_wayland_capable(patched), "a muvm-guest that cannot run is not capable");
	chmod(notExecutable, 0755);
}

int main(void)
{
	char template[] = "/tmp/darling-muvm-trampoline-XXXXXX";

	if (mkdtemp(template) == NULL)
	{
		printf("FAIL: cannot create a temporary directory\n");
		return 1;
	}
	snprintf(g_directory, sizeof(g_directory), "%s", template);

	test_thin_files();
	test_fat_files();
	test_bprefs_environment();
	test_needed_matrix();
	test_stdio();
	test_reexec();
	test_wayland_wanted();
	test_wayland_capable();

	if (g_failures != 0)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}

	printf("all checks passed\n");
	return 0;
}
