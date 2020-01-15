/*
 * Copyright (c) 2016 Apple Computer, Inc. All rights reserved.
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <xpc/private.h>
#include <System/sys/csr.h>
#include <System/machine/cpu_capabilities.h>

#include <os/assumes.h>
#include <os/variant_private.h>

enum variant_property {
	VP_ALL = 0,
	VP_CONTENT,
	VP_DIAGNOSTICS,
	VP_UI,
	VP_SECURITY,
	VP_MAX
};

enum check_status {
	S_UNKNOWN = 0,
	S_NO = 2,
	S_YES = 3
};

static bool
status2bool(enum check_status status) {
	switch (status) {
	case S_NO:
		return false;
	case S_YES:
		return true;
	default:
		os_crash("os_variant had unexpected status");
	}
}

#define VAR_FILE_LEGACY "/var/db/disableAppleInternal"

#if TARGET_OS_OSX
#define VAR_FILE_OVERRIDE "/var/db/os_variant_override"
#else
#define VAR_FILE_OVERRIDE "/usr/share/misc/os_variant_override"
#endif

#if !TARGET_OS_SIMULATOR
#define INTERNAL_CONTENT_PATH "/System/Library/CoreServices/AppleInternalVariant.plist"
#else
#define INTERNAL_CONTENT_PATH "/AppleInternal"
#endif

#define SYSTEM_VERSION_PLIST_PATH "/System/Library/CoreServices/SystemVersion.plist"
#define SYSTEM_VERSION_PLIST_KEY "ReleaseType"

#if TARGET_OS_IPHONE
#define INTERNAL_SETTINGS_PATH "/AppleInternal/Library/PreferenceBundles/Internal Settings.bundle"
#else
#define INTERNAL_DIAGS_PROFILE_PATH "/var/db/ConfigurationProfiles/Settings/com.apple.InternalDiagnostics.plist"
#endif

#if !TARGET_OS_SIMULATOR
#define CACHE_SYSCTL_NAME "kern.osvariant_status"

static void _restore_cached_check_status(uint64_t status);
static uint64_t _get_cached_check_status(void);

static char * _read_file(const char *path, size_t *size_out)
{
	char *buf = NULL;

	int fd = open(path, O_RDONLY);
	if (fd == -1) return NULL;

	struct stat sb;
	int rc = fstat(fd, &sb);
	if (rc != 0 || sb.st_size == 0) {
		goto error;
	}

	size_t size_limit = (size_out && *size_out != 0) ? *size_out : 1024;
	size_t size = (size_t)sb.st_size;
	if (size_out) *size_out = (size_t)sb.st_size;
	if (size > size_limit) {
		goto error;
	}

	buf = malloc(size + 1);
	if (!buf) {
		goto error;
	}

	ssize_t bytes_read = read(fd, buf, size);
	buf[size] = '\0';


	if (bytes_read == (ssize_t)size) {
		close(fd);
		return buf;
	}

error:
	close(fd);
	free(buf);
	return NULL;
}

static xpc_object_t read_plist(const char *path)
{
	size_t size = 16 * 1024;
	uint8_t *buf = (uint8_t*)_read_file(path, &size);
	if (!buf) return NULL;

	xpc_object_t plist = xpc_create_from_plist(buf, size);
	if (plist && xpc_get_type(plist) != XPC_TYPE_DICTIONARY) {
		xpc_release(plist);
		plist = NULL;
	}

	free(buf);

	return plist;
}
#endif

#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
static enum check_status internal_content = S_UNKNOWN;
#endif
#if !TARGET_OS_SIMULATOR
static enum check_status can_has_debugger = S_UNKNOWN;
#if TARGET_OS_IPHONE
static enum check_status internal_release_type = S_UNKNOWN;
#else // TARGET_OS_IPHONE
static enum check_status internal_diags_profile = S_UNKNOWN;
#endif // TARGET_OS_IPHONE
#endif // !TARGET_OS_SIMULATOR

static bool disabled_status[VP_MAX] = {};

static void _parse_disabled_status(char *test_string)
{
#if TARGET_OS_SIMULATOR
#pragma unused(test_string)
#else // TARGET_OS_SIMULATOR
	char *override_str = NULL;

	bzero(disabled_status, sizeof(disabled_status));

	if (test_string != NULL) {
		/* used for unit tests */
		override_str = strdup(test_string);
	} else {
		if (access(VAR_FILE_LEGACY, F_OK) == 0) {
			goto disable_all;
		} else if (access(VAR_FILE_OVERRIDE, F_OK) != 0) {
			return;
		}

		override_str = _read_file(VAR_FILE_OVERRIDE, NULL);
	}

	if (override_str == NULL) goto disable_all;

	char *token, *string = override_str;
	while ((token = strsep(&string, ",\n")) != NULL) {
		if (strcmp(token, "content") == 0) {
			disabled_status[VP_CONTENT] = true;
		} else if (strcmp(token, "diagnostics") == 0) {
			disabled_status[VP_DIAGNOSTICS] = true;
		} else if (strcmp(token, "ui") == 0) {
			disabled_status[VP_UI] = true;
		} else if (strcmp(token, "security") == 0) {
			disabled_status[VP_SECURITY] = true;
		}
	}

	free(override_str);
	return;

disable_all:
	for (int i = 0; i < VP_MAX; i++) {
		disabled_status[i] = true;
	}
#endif //!TARGET_OS_SIMULATOR
}

#if !TARGET_OS_SIMULATOR
static bool _load_cached_status(void)
{
	uint64_t status = 0;
	size_t status_size = sizeof(status);
	int ret = sysctlbyname(CACHE_SYSCTL_NAME, &status, &status_size, NULL, 0);
	if (ret != 0) {
		return false;
	}

	if (status) {
		_restore_cached_check_status(status);
		return true;
	}

	if (status == 0 && getpid() == 1) {
		/*
		 * Looks like we are in launchd; try to set the status.
		 *
		 * We don't actually care if this works because we'll have warmed our state.
		 */
		status = _get_cached_check_status();
		sysctlbyname(CACHE_SYSCTL_NAME, NULL, 0, &status, status_size);
		return true;
	}

	return false;
}
#endif

static void _initialize_status(void * __unused ctx)
{
#if !TARGET_OS_SIMULATOR
	if (!_load_cached_status()) {
		_parse_disabled_status(NULL);
	}
#else
	_parse_disabled_status(NULL);
#endif
}

static bool _check_disabled(enum variant_property variant_property)
{
	static dispatch_once_t disabled_status_pred;
	dispatch_once_f(&disabled_status_pred, NULL, _initialize_status);

	return disabled_status[variant_property];
}

#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
static bool _check_internal_content(void)
{
	if (internal_content == S_UNKNOWN) {
#if !TARGET_OS_SIMULATOR
		const char * path = INTERNAL_CONTENT_PATH;
#else
		char *simulator_root = getenv("IPHONE_SIMULATOR_ROOT");
		char *to_free = NULL, *path = NULL;
		if (simulator_root) {
			asprintf(&path, "%s/%s", simulator_root, INTERNAL_CONTENT_PATH);
			if (path == NULL) {
				return false;
			}
			to_free = path;
		}
#endif
		internal_content = (access(path, F_OK) == 0) ? S_YES : S_NO;
#if TARGET_OS_SIMULATOR
		free(to_free);
#endif
	}
	return status2bool(internal_content);
}
#endif // !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#if TARGET_OS_IPHONE

/*
 * This set of criteria was taken from copyInternalBuild in MobileGestalt.c
 */
static bool _check_internal_release_type(void)
{
#if TARGET_OS_SIMULATOR
	return _check_internal_content();
#else // TARGET_OS_SIMULATOR
	if (internal_release_type == S_UNKNOWN) {
		xpc_object_t system_version_plist = read_plist(SYSTEM_VERSION_PLIST_PATH);
		if (system_version_plist) {
			const char *release_type =
					xpc_dictionary_get_string(system_version_plist,
							SYSTEM_VERSION_PLIST_KEY);

			if (release_type == NULL) {
				/*
				 * Confusingly, customer images are just completely missing this key.
				 */
				internal_release_type = S_NO;
			} else if (strcmp(release_type, "Internal") == 0 ||
					strcmp(release_type, "Lite Internal") == 0 ||
					strcmp(release_type, "NonUI") == 0) {
				internal_release_type = S_YES;
			} else {
				internal_release_type = S_NO;
			}

			xpc_release(system_version_plist);
		} else {
			internal_release_type = (access(INTERNAL_SETTINGS_PATH, F_OK) == 0) ? S_YES : S_NO;
		}
	}

	return status2bool(internal_release_type);
#endif // TARGET_OS_SIMULATOR
}

#else

static bool _check_internal_diags_profile() {
	static enum check_status internal_diags_profile = S_UNKNOWN;

	if (internal_diags_profile == S_UNKNOWN) {
		xpc_object_t profile_settings = read_plist(INTERNAL_DIAGS_PROFILE_PATH);
		if (profile_settings) {
			internal_diags_profile = xpc_dictionary_get_bool(profile_settings, "AppleInternal") ? S_YES : S_NO;
			xpc_release(profile_settings);
		} else {
			internal_diags_profile = S_NO;
		}
	}

	return status2bool(internal_diags_profile);
}

#endif

static bool _check_can_has_debugger(void)
{
#if TARGET_OS_SIMULATOR
	return _check_internal_content();
#else
	if (can_has_debugger == S_UNKNOWN) {
#if TARGET_OS_IPHONE
		can_has_debugger = *((uint32_t *)_COMM_PAGE_DEV_FIRM) ? S_YES : S_NO;
#else
		/*
		 * The comm page bit does exist on macOS, but also requires kernel
		 * debugging in the CSR configuration.  We don't need to be that strict
		 * here.
		 */
		can_has_debugger = (csr_check(CSR_ALLOW_APPLE_INTERNAL) == 0) ? S_YES : S_NO;
#endif
	}
	return status2bool(can_has_debugger);
#endif // TARGET_OS_SIMULATOR
}

// For unit tests
#ifndef VARIANT_SKIP_EXPORTED

bool
os_variant_has_internal_content(const char * __unused subsystem)
{
	if (_check_disabled(VP_CONTENT)) {
		return false;
	}

#if TARGET_OS_IPHONE
	return _check_internal_release_type();
#else
	return _check_internal_content();
#endif
}


bool
os_variant_has_internal_diagnostics(const char * __unused subsystem)
{
	if (_check_disabled(VP_DIAGNOSTICS)) {
		return false;
	}

#if TARGET_OS_IPHONE
	return _check_internal_release_type();
#else
	return _check_internal_content() || _check_internal_diags_profile();
#endif
}

bool
os_variant_has_internal_ui(const char * __unused subsystem)
{
	if (_check_disabled(VP_UI)) {
		return false;
	}

#if TARGET_OS_IPHONE
	return _check_internal_release_type();
#else
	return _check_internal_content();
#endif
}

bool
os_variant_allows_internal_security_policies(const char * __unused subsystem)
{
	if (_check_disabled(VP_SECURITY)) {
		return false;
	}

	return _check_can_has_debugger();
}

#endif // VARIANT_SKIP_EXPORTED

#define STATUS_INITIAL_BITS 0x70000000F0000000ULL
#define STATUS_BIT_WIDTH 2
#define STATUS_SET 0x2
#define STATUS_MASK 0x3

enum status_flags_positions {
	SFP_INTERNAL_CONTENT = 0,
	SFP_CAN_HAS_DEBUGGER = 1,
	SFP_INTERNAL_RELEASE_TYPE = 2,
	SFP_INTERNAL_DIAGS_PROFILE = 3
};

#if !TARGET_OS_SIMULATOR
static uint64_t _get_cached_check_status(void)
{
	uint64_t res = STATUS_INITIAL_BITS;

#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
	_check_internal_content();
	if (internal_content != S_UNKNOWN)
		res |= internal_content << SFP_INTERNAL_CONTENT * STATUS_BIT_WIDTH;
#endif

	_check_can_has_debugger();
	if (can_has_debugger != S_UNKNOWN)
		res |= can_has_debugger << SFP_CAN_HAS_DEBUGGER * STATUS_BIT_WIDTH;

#if TARGET_OS_IPHONE
	_check_internal_release_type();
	if (internal_release_type != S_UNKNOWN)
		res |= internal_release_type << SFP_INTERNAL_RELEASE_TYPE * STATUS_BIT_WIDTH;
#else
	_check_internal_diags_profile();
	if (internal_diags_profile != S_UNKNOWN)
		res |= internal_diags_profile << SFP_INTERNAL_DIAGS_PROFILE * STATUS_BIT_WIDTH;
#endif

	_parse_disabled_status(NULL);
	for (int i = 0; i < VP_MAX; i++) {
		if (disabled_status[i]) {
			res |= 0x1ULL << (i + 32);
		}
	}

	return res;
}

static void _restore_cached_check_status(uint64_t status)
{
#if !TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
	if ((status >> (SFP_INTERNAL_CONTENT * STATUS_BIT_WIDTH)) & STATUS_SET)
		internal_content = (status >> (SFP_INTERNAL_CONTENT * STATUS_BIT_WIDTH)) & STATUS_MASK;
#endif
	if ((status >> (SFP_CAN_HAS_DEBUGGER * STATUS_BIT_WIDTH)) & STATUS_SET)
		can_has_debugger = (status >> (SFP_CAN_HAS_DEBUGGER * STATUS_BIT_WIDTH)) & STATUS_MASK;
#if TARGET_OS_IPHONE
	if ((status >> (SFP_INTERNAL_RELEASE_TYPE * STATUS_BIT_WIDTH)) & STATUS_SET)
		internal_release_type = (status >> (SFP_INTERNAL_RELEASE_TYPE * STATUS_BIT_WIDTH)) & STATUS_MASK;
#else
	if ((status >> (SFP_INTERNAL_DIAGS_PROFILE * STATUS_BIT_WIDTH)) & STATUS_SET)
		internal_diags_profile = (status >> (SFP_INTERNAL_DIAGS_PROFILE * STATUS_BIT_WIDTH)) & STATUS_MASK;
#endif

	for (int i = 0; i < VP_MAX; i++) {
		disabled_status[i] = (status >> (32 + i)) & 0x1;
	}
}
#endif // !TARGET_OS_SIMULATOR
