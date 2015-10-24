#include <dispatch/dispatch.h>
#include <uuid/uuid.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/arch.h>
#include <mach-o/getsect.h>
#include <pthread.h>
#include <sys/types.h>
#include <execinfo.h>
#include <stdio.h>
#include <dlfcn.h>
#include <asl.h>
#include <errno.h>
#include <pthread.h>
#include "assumes.h"

#define OSX_ASSUME_LOG_REDIRECT_SECT_NAME "__osx_log_func"
#define osx_atomic_cmpxchg(p, o, n) __sync_bool_compare_and_swap((p), (o), (n))

static bool _osx_should_abort_on_assumes = false;

static const char *
_osx_basename(const char *p)
{
	return ((strrchr(p, '/') ? : p - 1) + 1);
}

static void
_osx_get_build(char *build, size_t sz)
{
	/* Get the build every time. We used to cache it, but if PID 1 experiences
	 * an assumes() failure before the build has been set, that would mean that
	 * all future failures would get bad build info. So we fetch it every time.
	 * Since assumes() failures are on the slow path anyway, not a huge deal.
	 */
	int mib[] = { CTL_KERN, KERN_OSVERSION };

	size_t oldsz = sz;
	int r = sysctl(mib, 2, build, &sz, NULL, 0);
	if (r == 0 && sz == 1) {
		(void)strlcpy(build, "99Z999", oldsz);
	}
}

static void
_osx_get_image_uuid(void *hdr, uuid_t uuid)
{
#if __LP64__
	struct mach_header_64 *hdr32or64 = (struct mach_header_64 *)hdr;
#else
	struct mach_header *hdr32or64 = (struct mach_header *)hdr;
#endif /* __LP64__ */

	size_t i = 0;
	size_t next = sizeof(*hdr32or64);
	struct load_command *cur = NULL;
	for (i = 0; i < hdr32or64->ncmds; i++) {
		cur = (struct load_command *)((uintptr_t)hdr32or64 + next);
		if (cur->cmd == LC_UUID) {
			struct uuid_command *cmd = (struct uuid_command *)cur;
			uuid_copy(uuid, cmd->uuid);
			break;
		}

		next += cur->cmdsize;
	}

	if (i == hdr32or64->ncmds) {
		uuid_clear(uuid);
	}
}

static void
_osx_abort_on_assumes_once(void)
{
	/* Embedded boot-args can get pretty long. Let's just hope this is big
	 * enough.
	 */
	char bootargs[2048];
	size_t len = sizeof(bootargs) - 1;

	if (sysctlbyname("kern.bootargs", bootargs, &len, NULL, 0) == 0) {
		if (strnstr(bootargs, "-osx_assumes_fatal", len)) {
			_osx_should_abort_on_assumes = true;
		}
	}
}

static bool
_osx_abort_on_assumes(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	bool result = false;

	if (getpid() != 1) {
		if (getenv("OSX_ASSUMES_FATAL")) {
			result = true;
		} else {
			pthread_once(&once, _osx_abort_on_assumes_once);
			result = _osx_should_abort_on_assumes;
		}
	} else {
		if (getenv("OSX_ASSUMES_FATAL_PID1")) {
			result = true;
		}
	}

	return result;
}

#if __LP64__
static osx_redirect_t
_osx_find_log_redirect_func(struct mach_header_64 *hdr)
{
	osx_redirect_t result = NULL;

	char name[128];
	unsigned long size = 0;
	uint8_t *data = getsectiondata(hdr, "__TEXT", OSX_ASSUME_LOG_REDIRECT_SECT_NAME, &size);
	if (data && size < sizeof(name) - 2) {
		/* Ensure NUL termination. */
		(void)strlcpy(name, (const char *)data, size + 1);
		result = dlsym(RTLD_DEFAULT, name);
	}

	return result;
}
#else
static osx_redirect_t
_osx_find_log_redirect_func(struct mach_header *hdr)
{
	osx_redirect_t result = NULL;

	char name[128];
	unsigned long size = 0;
	uint8_t *data = getsectiondata(hdr, "__TEXT", OSX_ASSUME_LOG_REDIRECT_SECT_NAME, &size);
	if (data && size < sizeof(name) - 2) {
		(void)strlcpy(name, (const char *)data, size + 1);
		result = dlsym(RTLD_DEFAULT, name);
	}

	return result;
}
#endif

static bool
_osx_log_redirect(void *hdr, const char *msg)
{
	bool result = false;

	osx_redirect_t redirect_func = _osx_find_log_redirect_func(hdr);
	if (redirect_func) {
		result = redirect_func(msg);
	}

	return result;
}

__attribute__((always_inline))
static void
_osx_construct_message(const char *prefix, uint64_t code, aslmsg asl_message, Dl_info *info, char *buff, size_t sz)
{
	const char *image_name = NULL;
	uintptr_t offset = 0;
	uuid_string_t uuid_str;

	void *ret = __builtin_return_address(0);
	if (dladdr(ret, info)) {
		uuid_t uuid;
		_osx_get_image_uuid(info->dli_fbase, uuid);
		
		uuid_unparse(uuid, uuid_str);
		image_name = _osx_basename(info->dli_fname);
		
		offset = ret - info->dli_fbase;
	}

	char name[256];
	(void)snprintf(name, sizeof(name), "com.apple.assumes.%s", image_name);

	char sig[64];
	(void)snprintf(sig, sizeof(sig), "%s:%lu", uuid_str, offset);

	char result[24];
	(void)snprintf(result, sizeof(result), "0x%llx", code);

	char build[16];
	size_t bsz = sizeof(build);
	_osx_get_build(build, bsz);

	(void)snprintf(buff, sz, "%s: %s: %s + %lu [%s]: %s", prefix, build, image_name, offset, uuid_str, result);

	(void)asl_set(asl_message, "com.apple.message.domain", name);
	(void)asl_set(asl_message, "com.apple.message.signature", sig);
	(void)asl_set(asl_message, "com.apple.message.value", result);
}

void
_osx_assumes_log(uint64_t code)
{
	aslmsg asl_message = asl_new(ASL_TYPE_MSG);
	if (asl_message) {
		Dl_info info;
		char message[256];
		_osx_construct_message("Bug", code, asl_message, &info, message, sizeof(message));
		if (!_osx_log_redirect(info.dli_fbase, message)) {
			/* MessageTracer messages aren't logged to the regular syslog store,
			 * so we log the message without any MessageTracer attributes so
			 * that we can see it in our regular syslog.
			 */
			(void)asl_log(NULL, NULL, ASL_LEVEL_ERR, "%s", message);
			(void)asl_log(NULL, asl_message, ASL_LEVEL_ERR, "%s", message);
		}

		asl_free(asl_message);
	}

	if (_osx_abort_on_assumes()) {
		__builtin_trap();
	}
}

char *
_osx_assert_log(uint64_t code)
{
	char *result = NULL;

	aslmsg asl_message = asl_new(ASL_TYPE_MSG);
	if (asl_message) {
		Dl_info info;
		char message[256];
		_osx_construct_message("Fatal bug", code, asl_message, &info, message, sizeof(message));
		if (!_osx_log_redirect(info.dli_fbase, message)) {
			(void)asl_log(NULL, NULL, ASL_LEVEL_ERR, "%s", message);
			(void)asl_log(NULL, asl_message, ASL_LEVEL_ERR, "%s", message);
		}
		
		asl_free(asl_message);
		result = strdup(message);
	}

#if LIBC_NO_LIBCRASHREPORTERCLIENT
	/* There is no crash report information facility on embedded, which is
	 * really regrettable. Fortunately, all we need to capture is the value
	 * which tripped up the assertion. We can just stuff that into the thread's 
	 * name.
	 */
	char name[64];
	(void)pthread_getname_np(pthread_self(), name, sizeof(name));

	char newname[64];
	if (strlen(name) == 0) {
		(void)snprintf(newname, sizeof(newname), "[Fatal bug: 0x%llx]", code);
	} else {
		(void)snprintf(newname, sizeof(newname), "%s [Fatal bug: 0x%llx]", name, code);
	}

	(void)pthread_setname_np(newname);
#endif

	return result;
}

void
_osx_assumes_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code)
{
	aslmsg asl_message = asl_new(ASL_TYPE_MSG);
	if (asl_message) {
		Dl_info info;
		char message[256];
		_osx_construct_message("Bug", code, asl_message, &info, message, sizeof(message));

		(void)callout(asl_message, ctx, message);
		asl_free(asl_message);
	}

	if (_osx_abort_on_assumes()) {
		__builtin_trap();
	}
}

char *
_osx_assert_log_ctx(osx_log_callout_t callout, void *ctx, uint64_t code)
{
	char *result = NULL;

	aslmsg asl_message = asl_new(ASL_TYPE_MSG);
	if (asl_message) {
		Dl_info info;
		char message[256];
		_osx_construct_message("Fatal bug", code, asl_message, &info, message, sizeof(message));

		(void)callout(asl_message, ctx, message);
		asl_free(asl_message);
		result = strdup(message);
	}
}

extern void
_osx_avoid_tail_call(void)
{
	// no-op
}
