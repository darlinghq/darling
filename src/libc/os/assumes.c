/*
 * Copyright (c) 2011, 2012 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include <TargetConditionals.h>
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
#include <_simple.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include "os/assumes.h"
#include <os/debug_private.h>

#define OSX_ASSUMES_LOG_REDIRECT_SECT_NAME "__osx_log_func"
#define os_atomic_cmpxchg(p, o, n) __sync_bool_compare_and_swap((p), (o), (n))

static bool _os_should_abort_on_assumes = false;

static const char *
_os_basename(const char *p)
{
	return ((strrchr(p, '/') ? : p - 1) + 1);
}

static void
_os_get_build(char *build, size_t sz)
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
#if TARGET_IPHONE_SIMULATOR
        char *simVersion = getenv("SIMULATOR_RUNTIME_BUILD_VERSION");
        if (simVersion) {
            strlcat(build, " ", oldsz);
            strlcat(build, simVersion, oldsz);
        }
#endif
}

static void
_os_get_image_uuid(void *hdr, uuid_t uuid)
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
_os_abort_on_assumes_once(void)
{
	/* Embedded boot-args can get pretty long. Let's just hope this is big
	 * enough.
	 */
	char bootargs[2048];
	size_t len = sizeof(bootargs) - 1;

	if (sysctlbyname("kern.bootargs", bootargs, &len, NULL, 0) == 0) {
		if (strnstr(bootargs, "-os_assumes_fatal", len)) {
			_os_should_abort_on_assumes = true;
		}
	}
}

static bool
_os_abort_on_assumes(void)
{
	static pthread_once_t once = PTHREAD_ONCE_INIT;
	bool result = false;

	if (getpid() != 1) {
		if (getenv("OS_ASSUMES_FATAL")) {
			result = true;
		} else {
			pthread_once(&once, _os_abort_on_assumes_once);
			result = _os_should_abort_on_assumes;
		}
	} else {
		if (getenv("OS_ASSUMES_FATAL_PID1")) {
			result = true;
		}
	}

	return result;
}

#if __LP64__
typedef struct mach_header_64 os_mach_header;
#else
typedef struct mach_header os_mach_header;
#endif

static os_redirect_t
_os_find_log_redirect_func(os_mach_header *hdr)
{
	os_redirect_t result = NULL;

	char name[128];
	unsigned long size = 0;
	uint8_t *data = getsectiondata(hdr, OS_ASSUMES_REDIRECT_SEG, OS_ASSUMES_REDIRECT_SECT, &size);
	if (!data) {
		data = getsectiondata(hdr, "__TEXT", OSX_ASSUMES_LOG_REDIRECT_SECT_NAME, &size);

		if (data && size < sizeof(name) - 2) {
			(void)strlcpy(name, (const char *)data, size + 1);
			result = dlsym(RTLD_DEFAULT, name);
		}
	} else if (size == sizeof(struct _os_redirect_assumes_s)) {
		struct _os_redirect_assumes_s *redirect = (struct _os_redirect_assumes_s *)data;
		result = redirect->redirect;
	}

	return result;
}

static bool
_os_log_redirect(void *hdr, const char *msg)
{
	bool result = false;

	os_redirect_t redirect_func = _os_find_log_redirect_func(hdr);
	if (redirect_func) {
		result = redirect_func(msg);
	}

	return result;
}

__attribute__((always_inline))
static void
_os_construct_message(uint64_t code, _SIMPLE_STRING asl_message, Dl_info *info, char *buff, size_t sz)
{
	const char *image_name = NULL;
	uintptr_t offset = 0;
	uuid_string_t uuid_str;

	void *ret = __builtin_return_address(0);
	if (dladdr(ret, info)) {
		uuid_t uuid;
		_os_get_image_uuid(info->dli_fbase, uuid);
		
		uuid_unparse(uuid, uuid_str);
		image_name = _os_basename(info->dli_fname);
		
		offset = ret - info->dli_fbase;
	}

	char sig[64];
	(void)snprintf(sig, sizeof(sig), "%s:%lu", uuid_str, offset);

	char result[24];
	(void)snprintf(result, sizeof(result), "0x%llx", code);

	char build[32];
	size_t bsz = sizeof(build);
	_os_get_build(build, bsz);

	(void)snprintf(buff, sz, "assertion failed: %s: %s + %lu [%s]: %s", build, image_name, offset, uuid_str, result);

	_simple_asl_msg_set(asl_message, "com.apple.message.domain", "com.apple.assumes.failure");
	_simple_asl_msg_set(asl_message, "com.apple.message.signature", sig);
	_simple_asl_msg_set(asl_message, "com.apple.message.signature2", result);
	_simple_asl_msg_set(asl_message, "com.apple.message.signature3", image_name);
	_simple_asl_msg_set(asl_message, "com.apple.message.summarize", "YES");
}

#pragma mark Internal Implementations

os_crash_callback_t _os_crash_callback = NULL;

__attribute__((always_inline))
static inline void
_os_crash_impl(const char *message) {
	os_set_crash_message(message);
	if (!_os_crash_callback) {
		_os_crash_callback = dlsym(RTLD_MAIN_ONLY, "os_crash_function");
	}
	if (_os_crash_callback) {
		_os_crash_callback(message);
	}
}

__attribute__((always_inline))
static inline void
_os_assumes_log_impl(uint64_t code)
{
	char message[256] = "";

	_SIMPLE_STRING asl_message = _simple_asl_msg_new();
	if (asl_message) {
		Dl_info info;
		_os_construct_message(code, asl_message, &info, message, sizeof(message));
		if (!_os_log_redirect(info.dli_fbase, message)) {
			_os_debug_log_error_str(message);
			_simple_asl_msg_set(asl_message, "Level", "Error");
			_simple_asl_msg_set(asl_message, "Message", "");
			_simple_asl_send(asl_message);
		}

		_simple_sfree(asl_message);
	}

	if (_os_abort_on_assumes()) {
		os_crash(message);
	}
}

__attribute__((always_inline))
static inline char *
_os_assert_log_impl(uint64_t code)
{
	char *result = NULL;

	_SIMPLE_STRING asl_message = _simple_asl_msg_new();
	if (asl_message) {
		Dl_info info;
		char message[256];
		_os_construct_message(code, asl_message, &info, message, sizeof(message));
		if (!_os_log_redirect(info.dli_fbase, message)) {
			_os_debug_log_error_str(message);
			_simple_asl_msg_set(asl_message, "Level", "Error");
			_simple_asl_msg_set(asl_message, "Message", "");
			_simple_asl_send(asl_message);
		}

		_simple_sfree(asl_message);
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

__attribute__((always_inline))
static inline void
_os_assumes_log_ctx_impl(os_log_callout_t callout, void *ctx, uint64_t code)
{
	char message[256] = "";

	_SIMPLE_STRING asl_message = _simple_asl_msg_new();
	if (asl_message) {
		Dl_info info;
		_os_construct_message(code, asl_message, &info, message, sizeof(message));

		(void)callout(asl_message, ctx, message);
		_simple_sfree(asl_message);
	}

	if (_os_abort_on_assumes()) {
		os_crash(message);
	}
}

__attribute__((always_inline))
static inline char *
_os_assert_log_ctx_impl(os_log_callout_t callout, void *ctx, uint64_t code)
{
	char *result = NULL;

	_SIMPLE_STRING asl_message = _simple_asl_msg_new();
	if (asl_message) {
		Dl_info info;
		char message[256];
		_os_construct_message(code, asl_message, &info, message, sizeof(message));

		(void)callout(asl_message, ctx, message);
		_simple_sfree(asl_message);
		result = strdup(message);
	}
	return result;
}

#pragma mark Public Interfaces
void _os_crash(const char *message)
{
	_os_crash_impl(message);
}

void
_os_assumes_log(uint64_t code)
{
	_os_assumes_log_impl(code);
}

char *
_os_assert_log(uint64_t code)
{
	return _os_assert_log_impl(code);
}

void
_os_assumes_log_ctx(os_log_callout_t callout, void *ctx, uint64_t code)
{
	_os_assumes_log_ctx_impl(callout, ctx, code);
}

char *
_os_assert_log_ctx(os_log_callout_t callout, void *ctx, uint64_t code)
{
	return _os_assert_log_ctx_impl(callout, ctx, code);
}

void
_os_avoid_tail_call(void)
{
	// no-op
}
