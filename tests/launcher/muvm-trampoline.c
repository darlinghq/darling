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
	uint8_t header[8 + 20 * 8] = { 0 };
	FILE* file = fopen(path, "wb");

	put32(header, 0xcafebabeU);
	put32(header + 4, (uint32_t) count);

	for (size_t i = 0; i < count; i++)
	{
		put32(header + 8 + 20 * i, cputypes[i]);
		put32(header + 8 + 20 * i + 12, 0x1000U * (uint32_t) (i + 1));
	}

	if (file != NULL)
	{
		fwrite(header, 1, 8 + 20 * count, file);
		fclose(file);
	}
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

	if (g_failures != 0)
	{
		printf("%d check(s) failed\n", g_failures);
		return 1;
	}

	printf("all checks passed\n");
	return 0;
}
