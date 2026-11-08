// Copyright (C) 2026 Lubos Dolezel
//
// dyld and Darling's code signing configuration: DYLD_* variables in hardened
// programs, image snapshots and image sealing.
//
// Usage: dyld_hardened_runtime <companion dylib>
// The companion is this file built with -DDYLD_TEST_COMPANION -dynamiclib. Run as
// root (the default in `darling shell`) for the enforce and SealImages parts:
// they push a configuration through libsystem_kernel's Darling-private
// __darling_{get,set}_system_config and put the previous one back (also when
// the alarm fires). Exits by itself.
//
// The program ad-hoc signs copies of itself (SHA-256 CodeDirectory, special
// slots for the requirements and entitlements, as described by
// <kern/cs_blobs.h>):
//   plain   no hardened runtime, no entitlements
//   rt      hardened runtime, disable-library-validation
//   rt_env  hardened runtime, disable-library-validation,
//           allow-dyld-environment-variables
//   rt_gta  hardened runtime, disable-library-validation, get-task-allow
// (disable-library-validation keeps Darling's unsigned libraries loadable
// under enforce). Each copy is started with DYLD_INSERT_LIBRARIES naming the
// companion, and reports whether the companion was inserted, whether
// DYLD_INSERT_LIBRARIES is still in its environment, and whether the __TEXT
// of a dlopen()ed copy of the companion can still be made writable (not
// sealed). Whether an image was mapped from a snapshot cannot be observed
// from the guest (writes to an image's file did not show in its mapping
// even without a snapshot), so snapshots are not checked here; the fcntl
// itself is tested with libsystem_kernel.
//
// Expected:
// - default configuration (policy off, SealImages off): every copy honours
//   the variable, nothing is sealed (as before this change; macOS would
//   ignore the variable in `rt`, a deliberate difference);
// - CodeSigningPolicy enforce: as on macOS 27, `rt` ignores and loses the
//   DYLD_* variables, `rt_env` and `rt_gta` keep them;
// - SealImages on: only `rt` (hardened, no get-task-allow) has its __TEXT
//   sealed, and only in 64-bit processes (Linux has no mseal() for 32-bit
//   ones).
// Checks that need the hardened runtime reported by csops() are skipped when
// it is not (mldr registers the signature of what it starts), and the sealing
// check is skipped when the host kernel has no mseal().
#include <CommonCrypto/CommonDigest.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <kern/cs_blobs.h>
#include <libkern/OSByteOrder.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <signal.h>
#include <spawn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/codesign.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef DYLD_TEST_COMPANION

__attribute__((constructor))
static void companion_loaded(void)
{
	setenv("P93_INSERTED", "1", 1);
}

int p93_companion_marker(void)
{
	return 93;
}

#else // !DYLD_TEST_COMPANION

#include <darlingserver/rpc-supplement.h>

extern char** environ;

#define SIGN_PAGE_SHIFT 12
#define SIGN_PAGE_SIZE (1u << SIGN_PAGE_SHIFT)
#define ENT_ALLOW_DYLD_ENV "com.apple.security.cs.allow-dyld-environment-variables"
#define ENT_DISABLE_LV "com.apple.security.cs.disable-library-validation"
#define ENT_GET_TASK_ALLOW "com.apple.security.get-task-allow"

typedef int (*get_fn)(dserver_system_config_t*);
typedef int (*set_fn)(const dserver_system_config_t*);

static get_fn get_config;
static set_fn set_config;
static dserver_system_config_t saved;
static int pushed;
static int failures;
static char tmp_dir[64];

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

// a configuration as read back, made into a push: the fields that were not
// pushed (their `present` bit is clear) and the generation must be zero
static dserver_system_config_t as_push(const dserver_system_config_t* config)
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

static int push_config(uint32_t policy, uint32_t seal)
{
	dserver_system_config_t push = as_push(&saved);
	push.code_signing_policy = policy;
	push.seal_images = seal;
	push.present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY | DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES;
	int err = set_config(&push);
	if (err == 0)
		pushed = 1;
	return err;
}

static void restore_config(void)
{
	if (!pushed)
		return;
	pushed = 0;
	dserver_system_config_t push = as_push(&saved);
	int err = set_config(&push);
	if (err != 0)
		printf("FAIL: could not put the system configuration back: %d (%s)\n", err, strerror(err));
}

static const char* const copy_names[] = { "plain", "rt", "rt_env", "rt_gta" };

// the signed copies, and the companion copies their children may have left
static void remove_tmp(void)
{
	char path[256];

	if (!tmp_dir[0])
		return;
	for (size_t i = 0; i < sizeof(copy_names) / sizeof(copy_names[0]); i++) {
		snprintf(path, sizeof(path), "%s/%s", tmp_dir, copy_names[i]);
		unlink(path);
		snprintf(path, sizeof(path), "%s/companion-%s.dylib", tmp_dir, copy_names[i]);
		unlink(path);
	}
	rmdir(tmp_dir);
}

static void finish(void)
{
	restore_config();
	remove_tmp();
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

//
// ad-hoc signing
//

struct buffer {
	uint8_t* data;
	size_t length;
};

static int append(struct buffer* b, const void* data, size_t length)
{
	uint8_t* grown = realloc(b->data, b->length + length);
	if (!grown)
		return -1;
	b->data = grown;
	if (data)
		memcpy(b->data + b->length, data, length);
	else
		memset(b->data + b->length, 0, length);
	b->length += length;
	return 0;
}

static void put32(uint8_t* p, uint32_t v)
{
	OSWriteBigInt32(p, 0, v);
}

static void put64(uint8_t* p, uint64_t v)
{
	OSWriteBigInt64(p, 0, v);
}

static size_t round_up(size_t value, size_t to)
{
	return (value + to - 1) / to * to;
}

// The entitlements blob for the given keys (all set to true), or an empty
// buffer when there are none.
static struct buffer entitlements_blob(const char* const* keys)
{
	struct buffer xml = { NULL, 0 }, blob = { NULL, 0 };
	const char* head = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
		"<plist version=\"1.0\">\n<dict>\n";
	const char* tail = "</dict>\n</plist>\n";

	if (!keys || !keys[0])
		return blob;

	append(&xml, head, strlen(head));
	for (size_t i = 0; keys[i]; i++) {
		char line[256];
		snprintf(line, sizeof(line), "\t<key>%s</key>\n\t<true/>\n", keys[i]);
		append(&xml, line, strlen(line));
	}
	append(&xml, tail, strlen(tail));

	uint8_t header[8];
	put32(header, CSMAGIC_EMBEDDED_ENTITLEMENTS);
	put32(header + 4, (uint32_t)(sizeof(header) + xml.length));
	append(&blob, header, sizeof(header));
	append(&blob, xml.data, xml.length);
	free(xml.data);
	return blob;
}

// Writes an ad-hoc signed copy of the thin Mach-O file `source` to `target`.
static int adhoc_sign(const char* source, const char* target, int runtime, const char* const* entitlement_keys)
{
	struct buffer file = { NULL, 0 };
	int fd = open(source, O_RDONLY);
	if (fd < 0)
		return -1;
	struct stat st;
	if (fstat(fd, &st) != 0 || append(&file, NULL, (size_t)st.st_size) != 0 ||
			pread(fd, file.data, file.length, 0) != (ssize_t)file.length) {
		close(fd);
		return -1;
	}
	close(fd);

	const struct mach_header* mh = (const struct mach_header*)file.data;
	int is64 = (mh->magic == MH_MAGIC_64);
	if (!is64 && mh->magic != MH_MAGIC)
		return -1;
	size_t header_size = is64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header);

	// find __TEXT, __LINKEDIT, the first section and any old signature
	uint8_t* linkedit = NULL;
	uint64_t text_offset = 0, text_size = 0, first_section = file.length;
	uint8_t* cmd = file.data + header_size;
	for (uint32_t i = 0; i < mh->ncmds; i++) {
		const struct load_command* lc = (const struct load_command*)cmd;
		if (lc->cmd == LC_SEGMENT_64) {
			const struct segment_command_64* seg = (const void*)cmd;
			const struct section_64* sect = (const void*)(seg + 1);
			if (strcmp(seg->segname, SEG_TEXT) == 0) {
				text_offset = seg->fileoff;
				text_size = seg->filesize;
			} else if (strcmp(seg->segname, SEG_LINKEDIT) == 0) {
				linkedit = cmd;
			}
			for (uint32_t s = 0; s < seg->nsects; s++)
				if (sect[s].offset != 0 && sect[s].offset < first_section)
					first_section = sect[s].offset;
		} else if (lc->cmd == LC_SEGMENT) {
			const struct segment_command* seg = (const void*)cmd;
			const struct section* sect = (const void*)(seg + 1);
			if (strcmp(seg->segname, SEG_TEXT) == 0) {
				text_offset = seg->fileoff;
				text_size = seg->filesize;
			} else if (strcmp(seg->segname, SEG_LINKEDIT) == 0) {
				linkedit = cmd;
			}
			for (uint32_t s = 0; s < seg->nsects; s++)
				if (sect[s].offset != 0 && sect[s].offset < first_section)
					first_section = sect[s].offset;
		} else if (lc->cmd == LC_CODE_SIGNATURE) {
			// re-signing is not needed for this test
			return -1;
		}
		cmd += lc->cmdsize;
	}
	if (!linkedit || header_size + mh->sizeofcmds + sizeof(struct linkedit_data_command) > first_section)
		return -1;

	// the signature goes at the end of the file, 16-byte aligned
	uint32_t code_limit = (uint32_t)round_up(file.length, 16);
	append(&file, NULL, code_limit - file.length);

	// blobs
	const char* identifier = "org.darlinghq.test.dyld-hardened-runtime";
	struct buffer requirements = { NULL, 0 };
	uint8_t req[12];
	put32(req, CSMAGIC_REQUIREMENTS);
	put32(req + 4, sizeof(req));
	put32(req + 8, 0);
	append(&requirements, req, sizeof(req));
	struct buffer entitlements = entitlements_blob(entitlement_keys);

	uint32_t special_slots = entitlements.length ? CSSLOT_ENTITLEMENTS : CSSLOT_REQUIREMENTS;
	uint32_t code_slots = (code_limit + SIGN_PAGE_SIZE - 1) / SIGN_PAGE_SIZE;
	size_t cd_header = offsetof(CS_CodeDirectory, end_withPreEncryptOffset);
	size_t ident_offset = cd_header;
	size_t hash_offset = round_up(ident_offset + strlen(identifier) + 1, 4) + special_slots * CS_SHA256_LEN;
	size_t cd_length = hash_offset + code_slots * CS_SHA256_LEN;

	uint32_t blob_count = entitlements.length ? 3 : 2;
	size_t super_header = sizeof(CS_SuperBlob) + blob_count * sizeof(CS_BlobIndex);
	size_t signature_length = super_header + cd_length + requirements.length + entitlements.length;
	uint32_t data_size = (uint32_t)round_up(signature_length, 16);

	// load command and __LINKEDIT first: the code pages include the header
	struct mach_header* wmh = (struct mach_header*)file.data;
	struct linkedit_data_command* sig = (struct linkedit_data_command*)(file.data + header_size + wmh->sizeofcmds);
	sig->cmd = LC_CODE_SIGNATURE;
	sig->cmdsize = sizeof(*sig);
	sig->dataoff = code_limit;
	sig->datasize = data_size;
	wmh->ncmds++;
	wmh->sizeofcmds += sizeof(*sig);

	cmd = file.data + header_size;
	for (uint32_t i = 0; i < wmh->ncmds; i++) {
		struct load_command* lc = (struct load_command*)cmd;
		if (lc->cmd == LC_SEGMENT_64 && strcmp(((struct segment_command_64*)cmd)->segname, SEG_LINKEDIT) == 0) {
			struct segment_command_64* seg = (void*)cmd;
			seg->filesize = code_limit + data_size - seg->fileoff;
			if (seg->vmsize < round_up(seg->filesize, SIGN_PAGE_SIZE))
				seg->vmsize = round_up(seg->filesize, SIGN_PAGE_SIZE);
		} else if (lc->cmd == LC_SEGMENT && strcmp(((struct segment_command*)cmd)->segname, SEG_LINKEDIT) == 0) {
			struct segment_command* seg = (void*)cmd;
			seg->filesize = code_limit + data_size - seg->fileoff;
			if (seg->vmsize < round_up(seg->filesize, SIGN_PAGE_SIZE))
				seg->vmsize = (uint32_t)round_up(seg->filesize, SIGN_PAGE_SIZE);
		}
		cmd += lc->cmdsize;
	}

	// CodeDirectory
	uint8_t* cd = calloc(1, cd_length);
	if (!cd)
		return -1;
	put32(cd + offsetof(CS_CodeDirectory, magic), CSMAGIC_CODEDIRECTORY);
	put32(cd + offsetof(CS_CodeDirectory, length), (uint32_t)cd_length);
	put32(cd + offsetof(CS_CodeDirectory, version), CS_SUPPORTSRUNTIME);
	put32(cd + offsetof(CS_CodeDirectory, flags), CS_ADHOC | (runtime ? CS_RUNTIME : 0));
	put32(cd + offsetof(CS_CodeDirectory, hashOffset), (uint32_t)hash_offset);
	put32(cd + offsetof(CS_CodeDirectory, identOffset), (uint32_t)ident_offset);
	put32(cd + offsetof(CS_CodeDirectory, nSpecialSlots), special_slots);
	put32(cd + offsetof(CS_CodeDirectory, nCodeSlots), code_slots);
	put32(cd + offsetof(CS_CodeDirectory, codeLimit), code_limit);
	cd[offsetof(CS_CodeDirectory, hashSize)] = CS_SHA256_LEN;
	cd[offsetof(CS_CodeDirectory, hashType)] = CS_HASHTYPE_SHA256;
	cd[offsetof(CS_CodeDirectory, pageSize)] = SIGN_PAGE_SHIFT;
	put64(cd + offsetof(CS_CodeDirectory, execSegBase), text_offset);
	put64(cd + offsetof(CS_CodeDirectory, execSegLimit), text_size);
	put64(cd + offsetof(CS_CodeDirectory, execSegFlags), CS_EXECSEG_MAIN_BINARY);
	if (runtime)
		put32(cd + offsetof(CS_CodeDirectory, runtime), 0x000b0000); // hardened runtime version: macOS 11.0
	memcpy(cd + ident_offset, identifier, strlen(identifier) + 1);
	CC_SHA256(requirements.data, (CC_LONG)requirements.length, cd + hash_offset - CSSLOT_REQUIREMENTS * CS_SHA256_LEN);
	if (entitlements.length)
		CC_SHA256(entitlements.data, (CC_LONG)entitlements.length, cd + hash_offset - CSSLOT_ENTITLEMENTS * CS_SHA256_LEN);
	for (uint32_t i = 0; i < code_slots; i++) {
		size_t start = (size_t)i * SIGN_PAGE_SIZE;
		size_t length = (start + SIGN_PAGE_SIZE <= code_limit) ? SIGN_PAGE_SIZE : code_limit - start;
		CC_SHA256(file.data + start, (CC_LONG)length, cd + hash_offset + i * CS_SHA256_LEN);
	}

	// SuperBlob
	struct buffer signature = { NULL, 0 };
	uint8_t head[sizeof(CS_SuperBlob) + 3 * sizeof(CS_BlobIndex)];
	size_t offset = super_header;
	put32(head, CSMAGIC_EMBEDDED_SIGNATURE);
	put32(head + 4, (uint32_t)signature_length);
	put32(head + 8, blob_count);
	put32(head + 12, CSSLOT_CODEDIRECTORY);
	put32(head + 16, (uint32_t)offset);
	offset += cd_length;
	put32(head + 20, CSSLOT_REQUIREMENTS);
	put32(head + 24, (uint32_t)offset);
	offset += requirements.length;
	if (entitlements.length) {
		put32(head + 28, CSSLOT_ENTITLEMENTS);
		put32(head + 32, (uint32_t)offset);
	}
	append(&signature, head, super_header);
	append(&signature, cd, cd_length);
	append(&signature, requirements.data, requirements.length);
	append(&signature, entitlements.data, entitlements.length);
	append(&signature, NULL, data_size - signature.length);
	free(cd);
	free(requirements.data);
	free(entitlements.data);

	fd = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0755);
	int ok = fd >= 0 && write(fd, file.data, file.length) == (ssize_t)file.length &&
		write(fd, signature.data, signature.length) == (ssize_t)signature.length;
	if (fd >= 0)
		close(fd);
	free(file.data);
	free(signature.data);
	return ok ? 0 : -1;
}

//
// the started copies
//

// Runs in each signed copy. Prints one RESULT line.
static int child(const char* self, const char* companion)
{
	uint32_t flags = 0;
	csops(0, CS_OPS_STATUS, &flags, sizeof(flags));

	// before the companion is loaded here: only an inserted one has set it
	int inserted = getenv("P93_INSERTED") != NULL;
	int env_kept = getenv("DYLD_INSERT_LIBRARIES") != NULL;

	// a private copy of the companion, so that sealing it affects nothing else
	const char* name = strrchr(self, '/') ? strrchr(self, '/') + 1 : self;
	char copy[256];
	snprintf(copy, sizeof(copy), "%s/companion-%s.dylib", tmp_dir, name);
	int in = open(companion, O_RDONLY), out = open(copy, O_RDWR | O_CREAT | O_TRUNC, 0755);
	char chunk[4096];
	ssize_t n;
	while (in >= 0 && out >= 0 && (n = read(in, chunk, sizeof(chunk))) > 0)
		write(out, chunk, (size_t)n);
	if (in >= 0)
		close(in);

	int sealed = -1;
	void* handle = dlopen(copy, RTLD_NOW);
	void* marker = handle ? dlsym(handle, "p93_companion_marker") : NULL;
	Dl_info info;
	if (marker && dladdr(marker, &info) && info.dli_fbase) {
		const struct mach_header* mh = info.dli_fbase;
		long page = sysconf(_SC_PAGESIZE);
		void* text = (void*)((uintptr_t)mh & ~(uintptr_t)(page - 1));
		if (mprotect(text, (size_t)page, PROT_READ | PROT_WRITE) == 0) {
			sealed = 0;
			mprotect(text, (size_t)page, PROT_READ | PROT_EXEC);
		} else {
			sealed = (errno == EPERM) ? 1 : -1;
		}
	}
	if (out >= 0)
		close(out);
	unlink(copy);

	printf("RESULT runtime=%d gta=%d inserted=%d env_kept=%d sealed=%d\n",
		(flags & CS_RUNTIME) ? 1 : 0, (flags & CS_GET_TASK_ALLOW) ? 1 : 0,
		inserted, env_kept, sealed);
	fflush(stdout);
	return 0;
}

struct result {
	int ok;
	int runtime, gta, inserted, env_kept, sealed;
	int no_mseal;
};

// Starts a signed copy with DYLD_INSERT_LIBRARIES set and collects its result.
static struct result run_copy(const char* name, const char* companion)
{
	struct result r;
	memset(&r, 0, sizeof(r));

	char path[128], insert[1100], tmp_env[96];
	snprintf(path, sizeof(path), "%s/%s", tmp_dir, name);
	snprintf(insert, sizeof(insert), "DYLD_INSERT_LIBRARIES=%s", companion);
	snprintf(tmp_env, sizeof(tmp_env), "P93_TMP=%s", tmp_dir);
	char* argv[] = { path, "child", (char*)companion, NULL };
	char* envp[] = { insert, tmp_env, "PATH=/usr/bin:/bin", NULL };

	int out[2], err[2];
	if (pipe(out) != 0 || pipe(err) != 0)
		return r;
	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	posix_spawn_file_actions_adddup2(&actions, out[1], 1);
	posix_spawn_file_actions_adddup2(&actions, err[1], 2);
	posix_spawn_file_actions_addclose(&actions, out[0]);
	posix_spawn_file_actions_addclose(&actions, err[0]);
	pid_t pid;
	int rc = posix_spawn(&pid, path, &actions, NULL, argv, envp);
	posix_spawn_file_actions_destroy(&actions);
	close(out[1]);
	close(err[1]);
	if (rc != 0) {
		close(out[0]);
		close(err[0]);
		printf("  %s: posix_spawn failed: %d\n", name, rc);
		return r;
	}

	char text[4096] = "", errors[4096] = "";
	size_t used = 0, err_used = 0;
	ssize_t n;
	while ((n = read(out[0], text + used, sizeof(text) - 1 - used)) > 0)
		used += (size_t)n;
	while ((n = read(err[0], errors + err_used, sizeof(errors) - 1 - err_used)) > 0)
		err_used += (size_t)n;
	text[used] = errors[err_used] = '\0';
	close(out[0]);
	close(err[0]);
	int status = 0;
	waitpid(pid, &status, 0);

	const char* line = strstr(text, "RESULT ");
	r.ok = line && sscanf(line, "RESULT runtime=%d gta=%d inserted=%d env_kept=%d sealed=%d",
		&r.runtime, &r.gta, &r.inserted, &r.env_kept, &r.sealed) == 5 &&
		WIFEXITED(status) && WEXITSTATUS(status) == 0;
	r.no_mseal = strstr(errors, "does not support mseal") != NULL;
	printf("  %s: %s", name, line ? line : "(no result)\n");
	if (err_used)
		printf("  %s stderr: %s%s", name, errors, errors[err_used - 1] == '\n' ? "" : "\n");
	return r;
}

static void expect(const char* scenario, const char* name, const struct result* r,
	int want_inserted, int want_env, int want_sealed)
{
	char what[256];
	snprintf(what, sizeof(what), "%s, %s: started and reported", scenario, name);
	check(r->ok, what);
	if (!r->ok)
		return;
	snprintf(what, sizeof(what), "%s, %s: companion %s", scenario, name, want_inserted ? "inserted" : "not inserted");
	check(r->inserted == want_inserted, what);
	snprintf(what, sizeof(what), "%s, %s: DYLD_INSERT_LIBRARIES %s the environment", scenario, name, want_env ? "kept in" : "removed from");
	check(r->env_kept == want_env, what);
	if (want_sealed && r->no_mseal) {
		printf("SKIP: %s, %s: the host kernel has no mseal()\n", scenario, name);
		return;
	}
	snprintf(what, sizeof(what), "%s, %s: __TEXT %s", scenario, name, want_sealed ? "sealed (mprotect EPERM)" : "not sealed");
	check(r->sealed == want_sealed, what);
}

// whether csops() reports the hardened runtime for the `rt` copy at all
static int hardened_reported(const struct result* rt, const char* scenario)
{
	if (rt->ok && rt->runtime && !rt->gta)
		return 1;
	printf("SKIP: %s: csops() does not report the hardened runtime of the signed copy "
		"(the signature of a started program is not registered)\n", scenario);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc == 3 && strcmp(argv[1], "child") == 0) {
		const char* dir = getenv("P93_TMP");
		snprintf(tmp_dir, sizeof(tmp_dir), "%s", dir ? dir : "/tmp");
		alarm(20);
		return child(argv[0], argv[2]);
	}
	if (argc != 2) {
		fprintf(stderr, "usage: %s <companion dylib>\n", argv[0]);
		return 2;
	}

	signal(SIGALRM, on_alarm);
	alarm(60);
	setvbuf(stdout, NULL, _IOLBF, 0);

	get_config = (get_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	set_config = (set_fn) dlsym(RTLD_DEFAULT, "__darling_set_system_config");
	check(get_config && set_config, "libsystem_kernel has the system configuration calls");
	if (!get_config || !set_config)
		finish();
	check(get_config(&saved) == 0, "read the system configuration");

	char self[1024];
	uint32_t self_size = sizeof(self);
	check(_NSGetExecutablePath(self, &self_size) == 0, "own path");
	char companion[1024];
	check(realpath(argv[1], companion) != NULL, "companion found");

	strcpy(tmp_dir, "/tmp/dyld-hardened.XXXXXX");
	check(mkdtemp(tmp_dir) != NULL, "temporary directory");

	static const char* const ents_rt[] = { ENT_DISABLE_LV, NULL };
	static const char* const ents_env[] = { ENT_DISABLE_LV, ENT_ALLOW_DYLD_ENV, NULL };
	static const char* const ents_gta[] = { ENT_DISABLE_LV, ENT_GET_TASK_ALLOW, NULL };
	char path[128];
	snprintf(path, sizeof(path), "%s/plain", tmp_dir);
	check(adhoc_sign(self, path, 0, NULL) == 0, "signed copy plain");
	snprintf(path, sizeof(path), "%s/rt", tmp_dir);
	check(adhoc_sign(self, path, 1, ents_rt) == 0, "signed copy rt");
	snprintf(path, sizeof(path), "%s/rt_env", tmp_dir);
	check(adhoc_sign(self, path, 1, ents_env) == 0, "signed copy rt_env");
	snprintf(path, sizeof(path), "%s/rt_gta", tmp_dir);
	check(adhoc_sign(self, path, 1, ents_gta) == 0, "signed copy rt_gta");
	if (failures)
		finish();

	int root = (getuid() == 0);
	struct result plain, rt, rt_env, rt_gta;

	// 1. the default configuration
	if (root)
		check(push_config(dserver_code_signing_policy_off, 0) == 0, "default configuration pushed");
	else if (saved.code_signing_policy != dserver_code_signing_policy_off || saved.seal_images) {
		printf("SKIP: not the default configuration, and pushing one needs root\n");
		finish();
	}
	plain = run_copy("plain", companion);
	rt = run_copy("rt", companion);
	rt_env = run_copy("rt_env", companion);
	rt_gta = run_copy("rt_gta", companion);
	restore_config();
	expect("default", "plain", &plain, 1, 1, 0);
	expect("default", "rt", &rt, 1, 1, 0);
	expect("default", "rt_env", &rt_env, 1, 1, 0);
	expect("default", "rt_gta", &rt_gta, 1, 1, 0);
	if (!root) {
		printf("SKIP: the enforce and SealImages checks need root\n");
		finish();
	}

	// 2. CodeSigningPolicy enforce
	int err = push_config(dserver_code_signing_policy_enforce, 0);
	check(err == 0, "enforce pushed");
	if (err == 0) {
		plain = run_copy("plain", companion);
		rt = run_copy("rt", companion);
		rt_env = run_copy("rt_env", companion);
		rt_gta = run_copy("rt_gta", companion);
		restore_config();
		expect("enforce", "plain", &plain, 1, 1, 0);
		if (hardened_reported(&rt, "enforce")) {
			expect("enforce", "rt", &rt, 0, 0, 0);
			expect("enforce", "rt_env", &rt_env, 1, 1, 0);
			expect("enforce", "rt_gta", &rt_gta, 1, 1, 0);
		}
	}

	// 3. SealImages
	err = push_config(dserver_code_signing_policy_off, 1);
	check(err == 0, "SealImages pushed");
	if (err == 0) {
		plain = run_copy("plain", companion);
		rt = run_copy("rt", companion);
		rt_gta = run_copy("rt_gta", companion);
		restore_config();
		expect("SealImages", "plain", &plain, 1, 1, 0);
		if (hardened_reported(&rt, "SealImages")) {
#ifdef __i386__
			printf("SKIP: SealImages, rt: Linux has no mseal() for 32-bit processes\n");
			expect("SealImages", "rt", &rt, 1, 1, 0);
#else
			expect("SealImages", "rt", &rt, 1, 1, 1);
#endif
			expect("SealImages", "rt_gta", &rt_gta, 1, 1, 0);
		}
	}

	finish();
	return 0;
}

#endif // !DYLD_TEST_COMPANION
