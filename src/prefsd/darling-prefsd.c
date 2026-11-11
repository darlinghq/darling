/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// darling-prefsd hands the system preferences that the layers below
// CoreFoundation depend on to darlingserver, which parses no property lists
// itself:
//   - org.darlinghq.platform: the platform identity (hw.model, gethostuuid(),
//     IOPlatformExpertDevice, ...);
//   - org.darlinghq.security: the code signing and SIP configuration, and
//     whether arm64 pointer authentication is turned off.
//
// `darling-prefsd --push-once` is run by launchctl during the system bootstrap,
// before any launchd job is loaded, and exits once both domains are pushed.
// The org.darlinghq.prefsd launchd job runs it without arguments when it is
// loaded and whenever one of the two files changes (WatchPaths); it then pushes
// each domain whose content differs from what darlingserver holds, and exits.
//
// Only root may push. The files are read only if root owns them and neither
// their group nor others can write them; a file or a value that fails the
// checks is logged and the defaults are used for that whole domain. Nothing is
// read from the environment.

#include <CoreFoundation/CoreFoundation.h>
#include <darling/emulation/linux_premigration/ext/for-platform-identity.h>
#include <darling/emulation/linux_premigration/ext/for-sysconfig.h>
#include <errno.h>
#include <fcntl.h>
#include <os/log.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/csr.h>
#include <sys/stat.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "prefsd-validate.h"

#define PREFERENCES_DIRECTORY "/Library/Preferences"
#define PLATFORM_DOMAIN "org.darlinghq.platform"
#define SECURITY_DOMAIN "org.darlinghq.security"
#define LOG_PATH "/var/log/darling-prefsd.log"

// A run never takes longer than this: --push-once runs synchronously during
// the launchd bootstrap, and the job is not started for changes while it runs.
// SIGALRM's default action ends it.
#define RUN_TIMEOUT_SECONDS 15

// How often the job reads the files at most in one run (see main())
#define MAX_ROUNDS 4

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static int g_logFD = -1;

// Messages go to os_log and, because Darling has no log store to read them back
// from, also to LOG_PATH.
static void prefsd_log(os_log_type_t type, const char* format, ...) __attribute__((format(printf, 2, 3)));
static void prefsd_log(os_log_type_t type, const char* format, ...)
{
	char message[1024];
	va_list args;

	va_start(args, format);
	vsnprintf(message, sizeof(message), format, args);
	va_end(args);

	os_log_with_type(OS_LOG_DEFAULT, type, "%{public}s", message);

	if (g_logFD == -1)
		g_logFD = open(LOG_PATH, O_WRONLY | O_APPEND | O_CREAT | O_NOFOLLOW | O_CLOEXEC, 0644);
	if (g_logFD != -1)
	{
		char stamp[32] = "";
		char line[sizeof(message) + 64];
		time_t now = time(NULL);
		struct tm tm;

		if (localtime_r(&now, &tm) != NULL)
			strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", &tm);

		int length = snprintf(line, sizeof(line), "%s darling-prefsd[%d]: %s\n", stamp, getpid(), message);
		if (length > 0)
		{
			if ((size_t) length >= sizeof(line))
				length = sizeof(line) - 1;
			(void) write(g_logFD, line, length);
		}
	}
}

//
// reading a domain
//

enum domain_state {
	DOMAIN_ABSENT,
	DOMAIN_LOADED,
	DOMAIN_UNUSABLE,
};

static void domain_path(const char* domain, char* path, size_t size)
{
	snprintf(path, size, PREFERENCES_DIRECTORY "/%s.plist", domain);
}

// Reads the domain's file in /Library/Preferences (where `defaults write
// /Library/Preferences/<domain>` and CFPreferences for kCFPreferencesAnyUser
// and kCFPreferencesAnyHost keep it). The file is opened once and checked
// through that descriptor, so the checks cover exactly the bytes that are
// parsed. CFPreferences offers no way to check the file it reads, which is why
// the bytes are parsed with CFPropertyListCreateWithData instead.
static enum domain_state load_domain(const char* domain, CFDictionaryRef* out, char* why, size_t whySize)
{
	char path[256];
	struct stat st;
	enum prefsd_file_verdict verdict;
	UInt8* bytes = NULL;
	size_t length = 0;
	enum domain_state state = DOMAIN_UNUSABLE;

	*out = NULL;
	domain_path(domain, path, sizeof(path));

	// O_NOFOLLOW: a symbolic link would point the checks at another file
	int fd = open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
	if (fd == -1)
	{
		if (errno == ENOENT)
			return DOMAIN_ABSENT;
		snprintf(why, whySize, "%s cannot be opened: %s", path, strerror(errno));
		return DOMAIN_UNUSABLE;
	}

	if (fstat(fd, &st) != 0)
	{
		snprintf(why, whySize, "%s cannot be examined: %s", path, strerror(errno));
		goto out;
	}

	verdict = prefsd_check_file(&st);
	if (verdict != PREFSD_FILE_OK)
	{
		snprintf(why, whySize, "%s is %s (mode %03o)", path, prefsd_file_verdict_string(verdict), (unsigned) (st.st_mode & 0777));
		goto out;
	}

	// one byte more than allowed, to notice a file that grew after fstat()
	bytes = malloc(PREFSD_FILE_SIZE_MAX + 1);
	if (bytes == NULL)
	{
		snprintf(why, whySize, "out of memory reading %s", path);
		goto out;
	}

	while (length <= PREFSD_FILE_SIZE_MAX)
	{
		ssize_t count = read(fd, bytes + length, PREFSD_FILE_SIZE_MAX + 1 - length);
		if (count == 0)
			break;
		if (count < 0)
		{
			if (errno == EINTR)
				continue;
			snprintf(why, whySize, "%s cannot be read: %s", path, strerror(errno));
			goto out;
		}
		length += count;
	}

	if (length > PREFSD_FILE_SIZE_MAX)
	{
		snprintf(why, whySize, "%s is %s", path, prefsd_file_verdict_string(PREFSD_FILE_TOO_LARGE));
		goto out;
	}

	CFDataRef data = CFDataCreate(kCFAllocatorDefault, bytes, length);
	CFPropertyListRef plist = NULL;
	if (data != NULL)
	{
		plist = CFPropertyListCreateWithData(kCFAllocatorDefault, data, kCFPropertyListImmutable, NULL, NULL);
		CFRelease(data);
	}

	if (plist == NULL || CFGetTypeID(plist) != CFDictionaryGetTypeID())
	{
		snprintf(why, whySize, "%s is not a property list dictionary", path);
		if (plist != NULL)
			CFRelease(plist);
		goto out;
	}

	*out = (CFDictionaryRef) plist;
	state = DOMAIN_LOADED;

out:
	free(bytes);
	close(fd);
	return state;
}

static CFTypeRef lookup(CFDictionaryRef domain, const char* key)
{
	CFStringRef cfKey = CFStringCreateWithCString(kCFAllocatorDefault, key, kCFStringEncodingASCII);
	CFTypeRef value = NULL;

	if (cfKey != NULL)
	{
		value = CFDictionaryGetValue(domain, cfKey);
		CFRelease(cfKey);
	}
	return value;
}

// Copies a string value without its terminating NUL into `buffer` and stores a
// NUL after it. Fails for anything but a string, for a string that does not fit
// (with the NUL) and for characters the encoding cannot represent.
static bool copy_string(CFTypeRef value, CFStringEncoding encoding, char* buffer, size_t size, size_t* length)
{
	CFIndex used = 0;

	if (value == NULL || CFGetTypeID(value) != CFStringGetTypeID() || size == 0)
		return false;

	CFStringRef string = (CFStringRef) value;
	CFIndex characters = CFStringGetLength(string);
	CFIndex converted = CFStringGetBytes(string, CFRangeMake(0, characters), encoding, 0, false, (UInt8*) buffer, size - 1, &used);
	if (converted != characters)
		return false;

	buffer[used] = '\0';
	*length = used;
	return true;
}

static bool get_integer(CFTypeRef value, int64_t* out)
{
	if (value == NULL || CFGetTypeID(value) != CFNumberGetTypeID() || CFNumberIsFloatType((CFNumberRef) value))
		return false;
	return CFNumberGetValue((CFNumberRef) value, kCFNumberSInt64Type, out);
}

//
// org.darlinghq.platform
//

static const struct {
	const char* key;
	uint32_t bit;
	size_t offset;
} kIdentityStrings[] = {
	{ "Model", DSERVER_PLATFORM_IDENTITY_HAS_MODEL, offsetof(dserver_platform_identity_t, model) },
	{ "ProductName", DSERVER_PLATFORM_IDENTITY_HAS_PRODUCT_NAME, offsetof(dserver_platform_identity_t, product_name) },
	{ "BoardID", DSERVER_PLATFORM_IDENTITY_HAS_BOARD_ID, offsetof(dserver_platform_identity_t, board_id) },
	{ "SerialNumber", DSERVER_PLATFORM_IDENTITY_HAS_SERIAL_NUMBER, offsetof(dserver_platform_identity_t, serial_number) },
	{ "MLBSerialNumber", DSERVER_PLATFORM_IDENTITY_HAS_MLB_SERIAL_NUMBER, offsetof(dserver_platform_identity_t, mlb_serial_number) },
	{ "CPUBrandString", DSERVER_PLATFORM_IDENTITY_HAS_CPU_BRAND_STRING, offsetof(dserver_platform_identity_t, cpu_brand_string) },
};

static const struct {
	const char* key;
	uint32_t bit;
	size_t offset;
} kIdentityUUIDs[] = {
	{ "PlatformUUID", DSERVER_PLATFORM_IDENTITY_HAS_PLATFORM_UUID, offsetof(dserver_platform_identity_t, platform_uuid) },
	{ "ProvisioningUDID", DSERVER_PLATFORM_IDENTITY_HAS_PROVISIONING_UDID, offsetof(dserver_platform_identity_t, provisioning_udid) },
};

static const struct {
	const char* key;
	uint32_t bit;
	size_t offset;
	size_t length;
} kIdentityData[] = {
	{ "ROM", DSERVER_PLATFORM_IDENTITY_HAS_ROM, offsetof(dserver_platform_identity_t, rom), sizeof(((dserver_platform_identity_t*) NULL)->rom) },
	{ "SystemID", DSERVER_PLATFORM_IDENTITY_HAS_SYSTEM_ID, offsetof(dserver_platform_identity_t, system_id), sizeof(((dserver_platform_identity_t*) NULL)->system_id) },
};

static void identity_init(dserver_platform_identity_t* identity)
{
	memset(identity, 0, sizeof(*identity));
	identity->version = DSERVER_PLATFORM_IDENTITY_VERSION;
	identity->size = sizeof(*identity);
}

// Fills in the keys the domain sets. Keys of other components that share the
// domain (the GPU and display keys) are left alone.
static bool identity_from_domain(CFDictionaryRef domain, dserver_platform_identity_t* identity, char* why, size_t whySize)
{
	for (size_t i = 0; i < ARRAY_SIZE(kIdentityStrings); i++)
	{
		CFTypeRef value = lookup(domain, kIdentityStrings[i].key);
		char buffer[DSERVER_PLATFORM_IDENTITY_STRING_MAX];
		size_t length;

		if (value == NULL)
			continue;
		if (!copy_string(value, kCFStringEncodingASCII, buffer, sizeof(buffer), &length)
			|| !prefsd_is_printable_ascii(buffer, length, sizeof(buffer)))
		{
			snprintf(why, whySize, "%s must be a string of 1 to %zu printable ASCII characters",
				kIdentityStrings[i].key, sizeof(buffer) - 1);
			return false;
		}

		memcpy((char*) identity + kIdentityStrings[i].offset, buffer, length);
		identity->present |= kIdentityStrings[i].bit;
	}

	for (size_t i = 0; i < ARRAY_SIZE(kIdentityUUIDs); i++)
	{
		CFTypeRef value = lookup(domain, kIdentityUUIDs[i].key);
		char buffer[64];
		size_t length;
		uint8_t uuid[16];

		if (value == NULL)
			continue;
		if (!copy_string(value, kCFStringEncodingASCII, buffer, sizeof(buffer), &length)
			|| !prefsd_parse_uuid(buffer, length, uuid) || prefsd_is_all_zero(uuid, sizeof(uuid)))
		{
			snprintf(why, whySize, "%s must be a UUID string (XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX), not the nil UUID",
				kIdentityUUIDs[i].key);
			return false;
		}

		memcpy((uint8_t*) identity + kIdentityUUIDs[i].offset, uuid, sizeof(uuid));
		identity->present |= kIdentityUUIDs[i].bit;
	}

	for (size_t i = 0; i < ARRAY_SIZE(kIdentityData); i++)
	{
		CFTypeRef value = lookup(domain, kIdentityData[i].key);

		if (value == NULL)
			continue;
		if (CFGetTypeID(value) != CFDataGetTypeID()
			|| (size_t) CFDataGetLength((CFDataRef) value) != kIdentityData[i].length
			|| prefsd_is_all_zero(CFDataGetBytePtr((CFDataRef) value), kIdentityData[i].length))
		{
			snprintf(why, whySize, "%s must be data of %zu bytes, not all zero",
				kIdentityData[i].key, kIdentityData[i].length);
			return false;
		}

		memcpy((uint8_t*) identity + kIdentityData[i].offset, CFDataGetBytePtr((CFDataRef) value), kIdentityData[i].length);
		identity->present |= kIdentityData[i].bit;
	}

	CFTypeRef memorySize = lookup(domain, "MemorySize");
	if (memorySize != NULL)
	{
		int64_t bytes;
		if (!get_integer(memorySize, &bytes) || bytes <= 0)
		{
			snprintf(why, whySize, "MemorySize must be a positive integer (bytes)");
			return false;
		}
		identity->memory_size = (uint64_t) bytes;
		identity->present |= DSERVER_PLATFORM_IDENTITY_HAS_MEMORY_SIZE;
	}

	// darlingserver checks a ProvisioningUDID against the platform UUID in
	// effect; when both come from this domain, they can be checked here
	if ((identity->present & DSERVER_PLATFORM_IDENTITY_HAS_PROVISIONING_UDID)
		&& (identity->present & DSERVER_PLATFORM_IDENTITY_HAS_PLATFORM_UUID)
		&& memcmp(identity->provisioning_udid, identity->platform_uuid, sizeof(identity->platform_uuid)) != 0)
	{
		snprintf(why, whySize, "ProvisioningUDID must equal PlatformUUID");
		return false;
	}

	return true;
}

//
// org.darlinghq.security
//

static void config_init(dserver_system_config_t* config)
{
	memset(config, 0, sizeof(*config));
	config->version = DSERVER_SYSTEM_CONFIG_VERSION;
	config->size = sizeof(*config);
}

static bool config_from_domain(CFDictionaryRef domain, dserver_system_config_t* config, char* why, size_t whySize)
{
	CFTypeRef value;

	value = lookup(domain, "CodeSigningPolicy");
	if (value != NULL)
	{
		char buffer[16];
		size_t length;
		enum prefsd_code_signing_policy policy;

		if (!copy_string(value, kCFStringEncodingASCII, buffer, sizeof(buffer), &length)
			|| !prefsd_parse_code_signing_policy(buffer, length, &policy))
		{
			snprintf(why, whySize, "CodeSigningPolicy must be \"off\", \"warn\" or \"enforce\"");
			return false;
		}

		switch (policy)
		{
			case PREFSD_CODE_SIGNING_OFF:
				config->code_signing_policy = dserver_code_signing_policy_off;
				break;
			case PREFSD_CODE_SIGNING_WARN:
				config->code_signing_policy = dserver_code_signing_policy_warn;
				break;
			case PREFSD_CODE_SIGNING_ENFORCE:
				config->code_signing_policy = dserver_code_signing_policy_enforce;
				break;
		}
		config->present |= DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY;
	}

	value = lookup(domain, "csr-active-config");
	if (value != NULL)
	{
		int64_t flags;

		if (!get_integer(value, &flags) || flags < 0 || flags > UINT32_MAX
			|| ((uint64_t) flags & ~(uint64_t) CSR_VALID_FLAGS) != 0)
		{
			snprintf(why, whySize, "csr-active-config must be an integer made of CSR_ALLOW_* flags (at most 0x%x)", (unsigned) CSR_VALID_FLAGS);
			return false;
		}
		config->csr_active_config = (uint32_t) flags;
		config->present |= DSERVER_SYSTEM_CONFIG_HAS_CSR_ACTIVE_CONFIG;
	}

	value = lookup(domain, "SealImages");
	if (value != NULL)
	{
		if (CFGetTypeID(value) != CFBooleanGetTypeID())
		{
			snprintf(why, whySize, "SealImages must be a boolean");
			return false;
		}
		config->seal_images = CFBooleanGetValue((CFBooleanRef) value) ? 1 : 0;
		config->present |= DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES;
	}

	value = lookup(domain, "DisablePointerAuthentication");
	if (value != NULL)
	{
		if (CFGetTypeID(value) != CFBooleanGetTypeID())
		{
			snprintf(why, whySize, "DisablePointerAuthentication must be a boolean");
			return false;
		}
		config->disable_pointer_authentication = CFBooleanGetValue((CFBooleanRef) value) ? 1 : 0;
		config->present |= DSERVER_SYSTEM_CONFIG_HAS_DISABLE_POINTER_AUTHENTICATION;
	}

	value = lookup(domain, "TrustStorePath");
	if (value != NULL)
	{
		size_t length;

		if (!copy_string(value, kCFStringEncodingUTF8, config->trust_store_path, sizeof(config->trust_store_path), &length)
			|| !prefsd_is_absolute_path(config->trust_store_path, length, sizeof(config->trust_store_path)))
		{
			memset(config->trust_store_path, 0, sizeof(config->trust_store_path));
			snprintf(why, whySize, "TrustStorePath must be an absolute path shorter than %zu bytes", sizeof(config->trust_store_path));
			return false;
		}
		config->present |= DSERVER_SYSTEM_CONFIG_HAS_TRUST_STORE_PATH;
	}

	return true;
}

//
// pushing
//

// Builds what a domain asks for. Any problem with the file or a value is
// described in `why`, and the result is then the defaults for the whole
// domain, so a half-valid file never takes effect partially.
static void read_identity(dserver_platform_identity_t* identity, char* why, size_t whySize)
{
	CFDictionaryRef domain;

	why[0] = '\0';
	identity_init(identity);

	enum domain_state state = load_domain(PLATFORM_DOMAIN, &domain, why, whySize);
	if (state != DOMAIN_LOADED)
		return;

	char problem[256] = "";
	if (!identity_from_domain(domain, identity, problem, sizeof(problem)))
	{
		snprintf(why, whySize, PREFERENCES_DIRECTORY "/" PLATFORM_DOMAIN ".plist: %s", problem);
		identity_init(identity);
	}
	CFRelease(domain);
}

static void read_config(dserver_system_config_t* config, char* why, size_t whySize)
{
	CFDictionaryRef domain;

	why[0] = '\0';
	config_init(config);

	enum domain_state state = load_domain(SECURITY_DOMAIN, &domain, why, whySize);
	if (state != DOMAIN_LOADED)
		return;

	char problem[256] = "";
	if (!config_from_domain(domain, config, problem, sizeof(problem)))
	{
		snprintf(why, whySize, PREFERENCES_DIRECTORY "/" SECURITY_DOMAIN ".plist: %s", problem);
		config_init(config);
	}
	CFRelease(domain);
}

static bool push_identity(const dserver_platform_identity_t* identity)
{
	int error = __darling_set_platform_identity(identity);
	if (error == 0)
	{
		prefsd_log(OS_LOG_TYPE_DEFAULT, "pushed the platform identity (%d keys from " PLATFORM_DOMAIN ")",
			__builtin_popcount(identity->present));
		return true;
	}

	prefsd_log(OS_LOG_TYPE_ERROR, "darlingserver did not accept the platform identity from " PLATFORM_DOMAIN ": %s", strerror(error));
	if (error != EINVAL || identity->present == 0)
		return false;

	dserver_platform_identity_t defaults;
	identity_init(&defaults);
	error = __darling_set_platform_identity(&defaults);
	if (error != 0)
	{
		prefsd_log(OS_LOG_TYPE_ERROR, "darlingserver did not accept the default platform identity: %s", strerror(error));
		return false;
	}
	prefsd_log(OS_LOG_TYPE_DEFAULT, "using the default platform identity");
	return true;
}

static bool push_config(const dserver_system_config_t* config)
{
	int error = __darling_set_system_config(config);
	if (error == 0)
	{
		prefsd_log(OS_LOG_TYPE_DEFAULT, "pushed the system configuration (%d keys from " SECURITY_DOMAIN ")",
			__builtin_popcount(config->present));
		return true;
	}

	if (error == EPERM)
	{
		// The caller is root (checked in main()), so darlingserver refused to
		// relax csr-active-config while the prefix runs.
		prefsd_log(OS_LOG_TYPE_ERROR, "darlingserver kept the system configuration in effect: relaxing csr-active-config takes a restart of the prefix");
		return false;
	}

	prefsd_log(OS_LOG_TYPE_ERROR, "darlingserver did not accept the system configuration from " SECURITY_DOMAIN ": %s", strerror(error));
	if (error != EINVAL || config->present == 0)
		return false;

	dserver_system_config_t defaults;
	config_init(&defaults);
	error = __darling_set_system_config(&defaults);
	if (error != 0)
	{
		prefsd_log(OS_LOG_TYPE_ERROR, "darlingserver did not accept the default system configuration: %s", strerror(error));
		return false;
	}
	prefsd_log(OS_LOG_TYPE_DEFAULT, "using the default system configuration");
	return true;
}

// What darlingserver holds, as far as this daemon can change it: the fields
// whose bits are in `present`, everything else zero, as in a push. Read back,
// darlingserver also fills in the defaults and the generation, which a push
// must leave zero.
static void identity_normalize(const dserver_platform_identity_t* in, dserver_platform_identity_t* out)
{
	identity_init(out);
	out->present = in->present;

	for (size_t i = 0; i < ARRAY_SIZE(kIdentityStrings); i++)
	{
		// only the string, so that the comparison ignores bytes after its NUL
		const char* string = (const char*) in + kIdentityStrings[i].offset;
		if (in->present & kIdentityStrings[i].bit)
			memcpy((char*) out + kIdentityStrings[i].offset, string, strnlen(string, DSERVER_PLATFORM_IDENTITY_STRING_MAX - 1));
	}
	for (size_t i = 0; i < ARRAY_SIZE(kIdentityUUIDs); i++)
	{
		if (in->present & kIdentityUUIDs[i].bit)
			memcpy((uint8_t*) out + kIdentityUUIDs[i].offset, (const uint8_t*) in + kIdentityUUIDs[i].offset, 16);
	}
	for (size_t i = 0; i < ARRAY_SIZE(kIdentityData); i++)
	{
		if (in->present & kIdentityData[i].bit)
			memcpy((uint8_t*) out + kIdentityData[i].offset, (const uint8_t*) in + kIdentityData[i].offset, kIdentityData[i].length);
	}
	if (in->present & DSERVER_PLATFORM_IDENTITY_HAS_MEMORY_SIZE)
		out->memory_size = in->memory_size;
}

static void config_normalize(const dserver_system_config_t* in, dserver_system_config_t* out)
{
	config_init(out);
	out->present = in->present;

	if (in->present & DSERVER_SYSTEM_CONFIG_HAS_CODE_SIGNING_POLICY)
		out->code_signing_policy = in->code_signing_policy;
	if (in->present & DSERVER_SYSTEM_CONFIG_HAS_CSR_ACTIVE_CONFIG)
		out->csr_active_config = in->csr_active_config;
	if (in->present & DSERVER_SYSTEM_CONFIG_HAS_SEAL_IMAGES)
		out->seal_images = in->seal_images;
	if (in->present & DSERVER_SYSTEM_CONFIG_HAS_DISABLE_POINTER_AUTHENTICATION)
		out->disable_pointer_authentication = in->disable_pointer_authentication;
	if (in->present & DSERVER_SYSTEM_CONFIG_HAS_TRUST_STORE_PATH)
		memcpy(out->trust_store_path, in->trust_store_path, strnlen(in->trust_store_path, sizeof(in->trust_store_path) - 1));
}

// The last values pushed (or found in darlingserver) for each domain; a domain
// whose `have` flag is clear is pushed whatever it contains.
struct prefsd_state {
	bool haveIdentity;
	bool haveConfig;
	dserver_platform_identity_t identity;
	dserver_system_config_t config;
};

// Starts from what darlingserver holds, so that a run that finds the files as
// they were pushed pushes nothing. A domain that was never pushed (generation
// 0) is always pushed: shellspawn waits for the first configuration.
static void state_from_server(struct prefsd_state* state)
{
	dserver_platform_identity_t identity;
	dserver_system_config_t config;

	memset(state, 0, sizeof(*state));

	memset(&identity, 0, sizeof(identity));
	if (__darling_get_platform_identity(&identity) == 0 && identity.generation != 0)
	{
		identity_normalize(&identity, &state->identity);
		state->haveIdentity = true;
	}

	memset(&config, 0, sizeof(config));
	if (__darling_get_system_config(&config) == 0 && config.generation != 0)
	{
		config_normalize(&config, &state->config);
		state->haveConfig = true;
	}
}

// Reads both domains and pushes each one whose content differs from `state`.
// Sets *changed when something differed. Returns false if a push failed.
static bool push_round(struct prefsd_state* state, bool* changed)
{
	dserver_platform_identity_t identity;
	dserver_system_config_t config;
	char identityProblem[512];
	char configProblem[512];
	bool ok = true;

	read_identity(&identity, identityProblem, sizeof(identityProblem));
	read_config(&config, configProblem, sizeof(configProblem));

	bool identityChanged = !state->haveIdentity || memcmp(&identity, &state->identity, sizeof(identity)) != 0;
	bool configChanged = !state->haveConfig || memcmp(&config, &state->config, sizeof(config)) != 0;

	// The identity goes first: shellspawn waits only for the first system
	// configuration, and by then the identity must be in place as well.
	if (identityChanged)
	{
		if (identityProblem[0] != '\0')
			prefsd_log(OS_LOG_TYPE_ERROR, "%s; using the default platform identity", identityProblem);
		ok = push_identity(&identity) && ok;
		state->identity = identity;
		state->haveIdentity = true;
	}

	if (configChanged)
	{
		if (configProblem[0] != '\0')
			prefsd_log(OS_LOG_TYPE_ERROR, "%s; using the default system configuration", configProblem);
		ok = push_config(&config) && ok;
		state->config = config;
		state->haveConfig = true;
	}

	*changed = identityChanged || configChanged;
	return ok;
}

int main(int argc, const char** argv)
{
	struct prefsd_state state;
	bool once = false;
	bool changed = false;
	bool ok;

	if (argc == 2 && strcmp(argv[1], "--push-once") == 0)
		once = true;
	else if (argc != 1)
	{
		fprintf(stderr, "usage: %s [--push-once]\n", getprogname());
		return EX_USAGE;
	}

	if (getuid() != 0)
	{
		fprintf(stderr, "%s: only root can push the system preferences\n", getprogname());
		return EX_NOPERM;
	}

	umask(022);

	// Neither mode may hang: the bootstrap waits for --push-once, and launchd
	// does not start the job for a change while it still runs. The disposition
	// may have been inherited as ignored.
	signal(SIGALRM, SIG_DFL);
	alarm(RUN_TIMEOUT_SECONDS);

	if (once)
	{
		memset(&state, 0, sizeof(state));
		return push_round(&state, &changed) ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	// The launchd job: started at load and whenever one of the two files
	// changes (WatchPaths). launchd does not start it again for a change made
	// while it runs, so the files are read again after a push until a round
	// finds nothing new.
	state_from_server(&state);
	ok = push_round(&state, &changed);
	if (!changed)
	{
		prefsd_log(OS_LOG_TYPE_INFO, "the system preferences are unchanged");
		return ok ? EXIT_SUCCESS : EXIT_FAILURE;
	}

	for (int round = 1; changed && round < MAX_ROUNDS; round++)
		ok = push_round(&state, &changed) && ok;

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
