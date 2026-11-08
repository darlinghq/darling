// Every exec records the code signature of the program it loads, and csops()
// reports it, for the process itself and for others. Run it as root (the
// `darling shell` default) with two signed programs that have an x86_64 slice,
// darlingserver's tests/fixtures/codesign/fat-adhoc (ad-hoc) and fat-runtime
// (ad-hoc, hardened runtime):
//
//   codesign_exec_register /path/to/fat-adhoc /path/to/fat-runtime
//
// Under the policy in effect (`off` by default):
// - this test program is not signed: CS_OPS_CDHASH fails with EBADEXEC and
//   CS_OPS_PIDOFFSET is 0; CS_OPS_STATUS still reports CS_VALID, as it did
//   before images were recorded (under `enforce`: no CS_VALID, as on macOS);
// - a fork child has the same state as its parent, seen from both sides;
// - a child that execs a signed program (posix_spawn, started suspended so it
//   stays alive) reports the CDHash, identifier and slice offset computed here
//   from the file, and the status flags macOS reports (0x22000001 ad-hoc,
//   0x22011311 hardened), except that without enforcement CS_ENFORCEMENT and
//   CS_KILL are not reported (a deliberate Darling difference);
// - the parent's own state does not change when its child execs.
// Then, with the `enforce` policy pushed (and put back afterwards), children
// started from now on report exactly macOS's flags: the hardened program
// with CS_ENFORCEMENT and CS_KILL, this unsigned program without CS_VALID.
// Nothing is refused here: refusing invalid images is a separate change.
//
// Built for i386, the checks on this program cover what mldr32 records.
// Exits by itself.
#include <CommonCrypto/CommonDigest.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/dyld.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <signal.h>
#include <spawn.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/codesign.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <kern/cs_blobs.h>

#include <darlingserver/rpc-supplement.h>

extern char** environ;

#define MAX_FILE_SIZE (16 * 1024 * 1024)
#define REGISTER_TIMEOUT_MS 5000
#define BLOB_HEADER_SIZE 8

// what macOS 27 reports for x86_64 programs signed ad-hoc, without and with
// the hardened runtime (CS_DEBUGGED aside, which Darling adds to every answer)
#define ADHOC_FLAGS (CS_VALID | CS_SIGNED | CS_DYLD_PLATFORM)
#define RUNTIME_FLAGS (ADHOC_FLAGS | CS_RUNTIME | CS_HARD | CS_KILL | CS_ENFORCEMENT | CS_FORCED_LV)
// left out without enforcement
#define REFUSING_FLAGS (CS_ENFORCEMENT | CS_KILL)

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);

// the expected identity of a signed program, from its file
struct signature {
	uint64_t slice_offset;
	uint8_t cdhash[CS_CDHASH_LEN];
	char identifier[256];
};

static int failures;
static pid_t spawned = -1;
static get_fn get_config;
static set_fn set_config;
static dserver_system_config_t saved;
static int pushed;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// get_system_config reports the effective values, defaults included; a push may only carry the
// values whose `present` bit it sets, so the others are cleared here
static dserver_system_config_t pushable(const dserver_system_config_t* config)
{
	dserver_system_config_t push = *config;
	push.generation = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY))
		push.code_signing_policy = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_CSR_ACTIVE_CONFIG))
		push.csr_active_config = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES))
		push.seal_images = 0;
	if (!(push.present & DSERVER_SYSTEM_CONFIG_HAS_TRUST_STORE_PATH))
		memset(push.trust_store_path, 0, sizeof(push.trust_store_path));
	return push;
}

static void restore_policy(void)
{
	if (!pushed)
		return;
	pushed = 0;

	dserver_system_config_t push = pushable(&saved);
	int err = set_config(&push);
	if (err != 0)
		printf("FAIL: could not put the system configuration back: %d (%s)\n", err, strerror(err));
}

static void kill_spawned(void)
{
	if (spawned > 0) {
		kill(spawned, SIGKILL);
		waitpid(spawned, NULL, 0);
		spawned = -1;
	}
}

static void finish(void)
{
	kill_spawned();
	restore_policy();
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	fflush(stdout);
	_exit(failures ? 1 : 0);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	failures++;
	finish();
}

static uint32_t be32(const uint8_t* p)
{
	return OSReadBigInt32(p, 0);
}

static int in_bounds(uint64_t offset, uint64_t length, uint64_t size)
{
	return offset <= size && length <= size - offset;
}

// a higher rank wins when a signature has more than one CodeDirectory
static int hash_rank(uint8_t type)
{
	switch (type) {
		case CS_HASHTYPE_SHA384: return 4;
		case CS_HASHTYPE_SHA256: return 3;
		case CS_HASHTYPE_SHA256_TRUNCATED: return 2;
		case CS_HASHTYPE_SHA1: return 1;
		default: return 0;
	}
}

static void cdhash_of(const uint8_t* cd, uint32_t length, uint8_t type, uint8_t out[CS_CDHASH_LEN])
{
	uint8_t digest[CC_SHA384_DIGEST_LENGTH];

	switch (type) {
		case CS_HASHTYPE_SHA384: CC_SHA384(cd, length, digest); break;
		case CS_HASHTYPE_SHA256:
		case CS_HASHTYPE_SHA256_TRUNCATED: CC_SHA256(cd, length, digest); break;
		default: CC_SHA1(cd, length, digest); break;
	}
	memcpy(out, digest, CS_CDHASH_LEN);
}

// Finds the x86_64 slice of `path` and the CodeDirectory csops() reports,
// following the layout in <mach-o/fat.h>, <mach-o/loader.h> and
// <kern/cs_blobs.h>. Returns 0 when it is signed.
static int read_signature(const char* path, struct signature* sig)
{
	struct stat st;
	uint8_t* file;
	uint64_t size, slice = 0, slice_size;
	int fd, ret = -1;

	fd = open(path, O_RDONLY);
	if (fd < 0 || fstat(fd, &st) != 0 || st.st_size <= 0 || st.st_size > MAX_FILE_SIZE)
		return -1;
	size = (uint64_t) st.st_size;
	file = malloc(size);
	if (file == NULL || pread(fd, file, size, 0) != (ssize_t) size)
		goto out;

	slice_size = size;
	if (be32(file) == FAT_MAGIC) {
		uint32_t count = be32(file + 4);
		int found = 0;
		for (uint32_t i = 0; i < count && !found; i++) {
			uint64_t entry = sizeof(struct fat_header) + (uint64_t) i * sizeof(struct fat_arch);
			if (!in_bounds(entry, sizeof(struct fat_arch), size))
				goto out;
			const uint8_t* arch = file + entry;
			if ((cpu_type_t) be32(arch + offsetof(struct fat_arch, cputype)) == CPU_TYPE_X86_64) {
				slice = be32(arch + offsetof(struct fat_arch, offset));
				slice_size = be32(arch + offsetof(struct fat_arch, size));
				found = 1;
			}
		}
		if (!found || !in_bounds(slice, slice_size, size))
			goto out;
	}
	sig->slice_offset = slice;

	const uint8_t* image = file + slice;
	struct mach_header_64 mh;
	if (slice_size < sizeof(mh))
		goto out;
	memcpy(&mh, image, sizeof(mh));
	if (mh.magic != MH_MAGIC_64 || mh.cputype != CPU_TYPE_X86_64)
		goto out;

	uint64_t lc = sizeof(mh);
	struct linkedit_data_command cs = { 0 };
	for (uint32_t i = 0; i < mh.ncmds; i++) {
		struct load_command cmd;
		if (!in_bounds(lc, sizeof(cmd), slice_size))
			goto out;
		memcpy(&cmd, image + lc, sizeof(cmd));
		if (cmd.cmdsize < sizeof(cmd) || !in_bounds(lc, cmd.cmdsize, slice_size))
			goto out;
		if (cmd.cmd == LC_CODE_SIGNATURE && cmd.cmdsize >= sizeof(cs))
			memcpy(&cs, image + lc, sizeof(cs));
		lc += cmd.cmdsize;
	}
	if (cs.cmd != LC_CODE_SIGNATURE || !in_bounds(cs.dataoff, cs.datasize, slice_size) || cs.datasize < sizeof(CS_SuperBlob))
		goto out;

	const uint8_t* sb = image + cs.dataoff;
	if (be32(sb) != CSMAGIC_EMBEDDED_SIGNATURE)
		goto out;
	uint32_t sb_length = be32(sb + offsetof(CS_SuperBlob, length));
	uint32_t count = be32(sb + offsetof(CS_SuperBlob, count));
	if (sb_length > cs.datasize)
		goto out;

	int best = 0;
	for (uint32_t i = 0; i < count; i++) {
		uint64_t entry = sizeof(CS_SuperBlob) + (uint64_t) i * sizeof(CS_BlobIndex);
		if (!in_bounds(entry, sizeof(CS_BlobIndex), sb_length))
			goto out;
		uint32_t type = be32(sb + entry);
		uint32_t offset = be32(sb + entry + 4);
		if (type != CSSLOT_CODEDIRECTORY && (type < CSSLOT_ALTERNATE_CODEDIRECTORIES || type >= CSSLOT_ALTERNATE_CODEDIRECTORY_LIMIT))
			continue;
		if (!in_bounds(offset, sizeof(CS_CodeDirectory), sb_length))
			goto out;

		const uint8_t* cd = sb + offset;
		uint32_t cd_length = be32(cd + offsetof(CS_CodeDirectory, length));
		uint32_t ident = be32(cd + offsetof(CS_CodeDirectory, identOffset));
		uint8_t hash_type = cd[offsetof(CS_CodeDirectory, hashType)];
		if (be32(cd) != CSMAGIC_CODEDIRECTORY || !in_bounds(offset, cd_length, sb_length) || ident >= cd_length)
			goto out;
		if (hash_rank(hash_type) <= best)
			continue;
		best = hash_rank(hash_type);

		size_t ident_length = strnlen((const char*) cd + ident, cd_length - ident);
		if (ident_length == cd_length - ident || ident_length >= sizeof(sig->identifier))
			goto out;
		memcpy(sig->identifier, cd + ident, ident_length + 1);
		cdhash_of(cd, cd_length, hash_type, sig->cdhash);
	}
	if (best > 0)
		ret = 0;

out:
	free(file);
	close(fd);
	return ret;
}


// does the image this process runs carry a code signature load command?
static int self_has_signature(void)
{
	const struct mach_header* mh = _dyld_get_image_header(0);
	const uint8_t* lc = (const uint8_t*) mh + (mh->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header));

	for (uint32_t i = 0; i < mh->ncmds; i++) {
		const struct load_command* cmd = (const struct load_command*) lc;
		if (cmd->cmd == LC_CODE_SIGNATURE)
			return 1;
		lc += cmd->cmdsize;
	}
	return 0;
}

static uint32_t status_of(pid_t pid, int* rc)
{
	uint32_t flags = 0;
	*rc = csops(pid, CS_OPS_STATUS, &flags, sizeof(flags));
	return flags & ~(uint32_t) CS_DEBUGGED;
}

static int cdhash_errno(pid_t pid)
{
	uint8_t cdhash[CS_CDHASH_LEN];
	return csops(pid, CS_OPS_CDHASH, cdhash, sizeof(cdhash)) == 0 ? 0 : errno;
}

static void sleep_ms(long ms)
{
	struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
	nanosleep(&ts, NULL);
}

static const char* policy_name(uint32_t policy)
{
	return policy == dserver_code_signing_policy_enforce ? "enforce" : policy == dserver_code_signing_policy_warn ? "warn" : "off";
}

// starts `path` suspended (it stops in dyld, after mldr has registered it)
static int spawn_suspended(const char* path)
{
	posix_spawnattr_t attr;
	char* const argv[] = { (char*) path, NULL };

	posix_spawnattr_init(&attr);
	posix_spawnattr_setflags(&attr, POSIX_SPAWN_START_SUSPENDED);
	int err = posix_spawn(&spawned, path, NULL, &attr, argv, environ);
	posix_spawnattr_destroy(&attr);
	if (err != 0)
		spawned = -1;
	return err;
}

static void test_self_unsigned(uint32_t policy)
{
	int64_t offset = -1;
	int rc;
	char what[160];

	if (self_has_signature()) {
		printf("SKIP: this test program is signed; build it without a signature for the unsigned checks\n");
		return;
	}

	uint32_t flags = status_of(0, &rc);
	if (policy == dserver_code_signing_policy_enforce) {
		check(rc == 0 && !(flags & CS_VALID), "unsigned, enforce: no CS_VALID");
	} else {
		snprintf(what, sizeof(what), "unsigned, %s: CS_VALID, as before images were recorded", policy_name(policy));
		check(rc == 0 && flags == CS_VALID, what);
	}
	uint32_t by_pid = status_of(getpid(), &rc);
	check(rc == 0 && by_pid == flags, "the same, asked by pid");
	check(cdhash_errno(0) == EBADEXEC, "unsigned: no CDHash (EBADEXEC)");
	check(csops(0, CS_OPS_PIDOFFSET, &offset, sizeof(offset)) == 0 && offset == 0,
		"a thin program's slice offset is 0");
}

static void test_fork_inherits(void)
{
	int ready[2], go[2];
	char c = 0;

	if (pipe(ready) != 0 || pipe(go) != 0) {
		check(0, "pipe");
		return;
	}

	int parent_rc;
	uint32_t parent_flags = status_of(0, &parent_rc);
	int parent_cdhash = cdhash_errno(0);

	fflush(stdout);
	pid_t child = fork();
	if (child < 0) {
		check(0, "fork");
		return;
	}
	if (child == 0) {
		int rc;
		uint32_t flags = status_of(0, &rc);
		int same = rc == parent_rc && flags == parent_flags && cdhash_errno(0) == parent_cdhash;
		write(ready[1], &c, 1);
		read(go[0], &c, 1);
		_exit(same ? 0 : 1);
	}

	close(ready[1]);
	close(go[0]);
	read(ready[0], &c, 1);

	int rc;
	uint32_t flags = status_of(child, &rc);
	check(rc == 0 && flags == parent_flags && cdhash_errno(child) == parent_cdhash,
		"the parent sees its own signing state in its fork child");

	write(go[1], &c, 1);
	int status = 0;
	waitpid(child, &status, 0);
	check(WIFEXITED(status) && WEXITSTATUS(status) == 0, "a fork child reports its parent's signing state");
	close(ready[0]);
	close(go[1]);
}

// starts the signed program at `path` and checks what its child reports
static void test_exec_registers(const char* path, const char* label, uint32_t macos_flags, uint32_t policy)
{
	struct signature expected;
	char what[200];

	memset(&expected, 0, sizeof(expected));
	if (read_signature(path, &expected) != 0) {
		snprintf(what, sizeof(what), "%s has a signed x86_64 slice", label);
		check(0, what);
		return;
	}

	int self_rc;
	uint32_t self_flags = status_of(0, &self_rc);
	int self_cdhash = cdhash_errno(0);

	if (spawn_suspended(path) != 0) {
		snprintf(what, sizeof(what), "posix_spawn of %s", label);
		check(0, what);
		return;
	}

	// posix_spawn returns once the exec has started; wait until the new image
	// has been registered
	uint8_t cdhash[CS_CDHASH_LEN] = { 0 };
	int registered = 0;
	for (long waited = 0; waited < REGISTER_TIMEOUT_MS && !registered; waited += 10) {
		registered = csops(spawned, CS_OPS_CDHASH, cdhash, sizeof(cdhash)) == 0;
		if (!registered)
			sleep_ms(10);
	}
	snprintf(what, sizeof(what), "%s, %s: its CDHash is recorded after the exec", label, policy_name(policy));
	check(registered, what);
	snprintf(what, sizeof(what), "%s: the CDHash is that of its x86_64 CodeDirectory", label);
	check(registered && memcmp(cdhash, expected.cdhash, sizeof(cdhash)) == 0, what);

	int rc;
	uint32_t flags = status_of(spawned, &rc);
	uint32_t want = policy == dserver_code_signing_policy_enforce ? macos_flags : (macos_flags & ~(uint32_t) REFUSING_FLAGS);
	snprintf(what, sizeof(what), "%s, %s: status 0x%08x (expected 0x%08x)", label, policy_name(policy), flags, want);
	check(rc == 0 && flags == want, what);

	char identity[BLOB_HEADER_SIZE + sizeof(expected.identifier)];
	memset(identity, 0, sizeof(identity));
	rc = csops(spawned, CS_OPS_IDENTITY, identity, sizeof(identity));
	snprintf(what, sizeof(what), "%s: the identifier is the CodeDirectory's", label);
	check(rc == 0 && strcmp(identity + BLOB_HEADER_SIZE, expected.identifier) == 0, what);

	int64_t offset = -1;
	rc = csops(spawned, CS_OPS_PIDOFFSET, &offset, sizeof(offset));
	snprintf(what, sizeof(what), "%s: the slice offset is that of the loaded x86_64 slice", label);
	check(rc == 0 && (uint64_t) offset == expected.slice_offset, what);

	char team[BLOB_HEADER_SIZE + 64];
	errno = 0;
	snprintf(what, sizeof(what), "%s: an ad-hoc signature has no team ID (ENOENT)", label);
	check(csops(spawned, CS_OPS_TEAMID, team, sizeof(team)) == -1 && errno == ENOENT, what);

	int now_rc;
	uint32_t now_flags = status_of(0, &now_rc);
	check(now_rc == self_rc && now_flags == self_flags && cdhash_errno(0) == self_cdhash,
		"the parent's own signing state is unchanged");

	kill_spawned();
}

// under enforce, an unsigned program reports what macOS reports: no CS_VALID
static void test_unsigned_child_enforce(const char* self)
{
	if (self_has_signature())
		return;

	if (spawn_suspended(self) != 0) {
		check(0, "posix_spawn of this program");
		return;
	}

	// the child starts with this process's state (CS_VALID); wait for its own
	int rc = -1;
	uint32_t flags = CS_VALID;
	for (long waited = 0; waited < REGISTER_TIMEOUT_MS && (rc != 0 || (flags & CS_VALID)); waited += 10) {
		flags = status_of(spawned, &rc);
		if (rc != 0 || (flags & CS_VALID))
			sleep_ms(10);
	}
	check(rc == 0 && !(flags & CS_VALID), "unsigned child, enforce: no CS_VALID");
	check(cdhash_errno(spawned) == EBADEXEC, "unsigned child, enforce: no CDHash (EBADEXEC)");

	kill_spawned();
}

int main(int argc, char** argv)
{
	signal(SIGALRM, on_alarm);
	alarm(60);

	if (argc != 3) {
		printf("usage: %s <fat-adhoc> <fat-runtime>\n", argv[0]);
		failures++;
		finish();
	}

	get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	check(get_config && set_config && get_config(&saved) == 0, "the system configuration can be read");
	if (!get_config || !set_config)
		finish();
	uint32_t policy = saved.code_signing_policy;
	printf("  policy in effect: %s\n", policy_name(policy));

	test_self_unsigned(policy);
	test_fork_inherits();
	test_exec_registers(argv[1], "fat-adhoc", ADHOC_FLAGS, policy);
	test_exec_registers(argv[2], "fat-runtime", RUNTIME_FLAGS, policy);

	if (getuid() != 0) {
		printf("SKIP: pushing the enforce policy needs root\n");
		finish();
	}

	dserver_system_config_t push = pushable(&saved);
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY;
	push.code_signing_policy = dserver_code_signing_policy_enforce;
	int err = set_config(&push);
	check(err == 0, "push the enforce policy");
	if (err != 0)
		finish();
	pushed = 1;

	test_exec_registers(argv[1], "fat-adhoc", ADHOC_FLAGS, dserver_code_signing_policy_enforce);
	test_exec_registers(argv[2], "fat-runtime", RUNTIME_FLAGS, dserver_code_signing_policy_enforce);
	test_unsigned_child_enforce(argv[0]);

	finish();
}
