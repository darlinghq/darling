// Copyright (C) 2026 Lubos Dolezel
//
// What a newly started process sees of the platform identity that
// darling-prefsd pushes from /Library/Preferences/org.darlinghq.platform.plist:
// sysctl hw.model, gethostuuid(), and IOPlatformExpertDevice's "model" and
// IOPlatformUUID (through IOKit), which must all agree.
//
// usage: prefsd_identity [--no-iokit] pushed|unpushed default|MODEL [UUID]
//   pushed    a system configuration has been pushed since the prefix started
//             (darling-prefsd ran); unpushed: none has (darling-prefsd is
//             missing, so darlingserver's defaults are in effect)
//   default   no identity key is configured: hw.model is the default model
//   MODEL     hw.model and IOKit's model are MODEL
//   UUID      gethostuuid() and IOPlatformUUID are UUID
//   --no-iokit  skip IOKit: iokitd reads the identity once, when it starts,
//             so a change pushed while the prefix runs reaches only new
//             processes, not iokitd
//
// The Darling-private calls are looked up with dlsym, so that the test reports
// a missing call instead of failing to load.
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <uuid/uuid.h>

#include <darlingserver/rpc-supplement.h>

typedef int (*get_identity_fn)(dserver_platform_identity_t*);
typedef int (*get_config_fn)(dserver_system_config_t*);

static int g_failures = 0;

static void check(int ok, const char* what)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		g_failures++;
}

static void finish(void)
{
	printf("%s\n", g_failures ? "FAILED" : "ALL PASSED");
	fflush(stdout);
	_exit(g_failures ? 1 : 0);
}

static void on_alarm(int sig)
{
	(void) sig;
	printf("FAIL: timed out\n");
	g_failures++;
	finish();
}

static void usage(void)
{
	printf("usage: prefsd_identity [--no-iokit] pushed|unpushed default|MODEL [UUID]\n");
	fflush(stdout);
	_exit(2);
}

// IOPlatformExpertDevice's "model" (NUL-terminated data) and IOPlatformUUID
static int read_iokit(char* model, size_t modelSize, char* uuid, size_t uuidSize)
{
	io_service_t service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("IOPlatformExpertDevice"));
	int ok = 0;

	model[0] = uuid[0] = '\0';
	if (service == IO_OBJECT_NULL)
		return 0;

	CFTypeRef modelData = IORegistryEntryCreateCFProperty(service, CFSTR("model"), kCFAllocatorDefault, 0);
	CFTypeRef uuidString = IORegistryEntryCreateCFProperty(service, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);

	if (modelData != NULL && CFGetTypeID(modelData) == CFDataGetTypeID())
	{
		CFIndex length = CFDataGetLength(modelData);
		if (length > 0 && (size_t) length <= modelSize)
		{
			memcpy(model, CFDataGetBytePtr(modelData), length);
			model[length - 1] = '\0';
			ok = 1;
		}
	}
	if (uuidString != NULL && CFGetTypeID(uuidString) == CFStringGetTypeID())
		CFStringGetCString(uuidString, uuid, uuidSize, kCFStringEncodingASCII);

	if (modelData != NULL)
		CFRelease(modelData);
	if (uuidString != NULL)
		CFRelease(uuidString);
	IOObjectRelease(service);
	return ok;
}

int main(int argc, char** argv)
{
	int iokit = 1;
	int arg = 1;

	signal(SIGALRM, on_alarm);
	alarm(30);

	if (arg < argc && strcmp(argv[arg], "--no-iokit") == 0)
	{
		iokit = 0;
		arg++;
	}
	if (argc - arg < 2 || argc - arg > 3)
		usage();

	const char* pushed = argv[arg];
	const char* expectedModel = argv[arg + 1];
	const char* expectedUUID = (argc - arg == 3) ? argv[arg + 2] : NULL;
	int expectDefault = strcmp(expectedModel, "default") == 0;

	if (strcmp(pushed, "pushed") != 0 && strcmp(pushed, "unpushed") != 0)
		usage();

	get_identity_fn get_identity = (get_identity_fn) dlsym(RTLD_DEFAULT, "__darling_get_platform_identity");
	get_config_fn get_config = (get_config_fn) dlsym(RTLD_DEFAULT, "__darling_get_system_config");
	check(get_identity != NULL && get_config != NULL, "libsystem_kernel has the platform identity and system configuration calls");
	if (get_identity == NULL || get_config == NULL)
		finish();

	// the system configuration is pushed last, so it tells whether darling-prefsd ran
	dserver_system_config_t config;
	memset(&config, 0, sizeof(config));
	int error = get_config(&config);
	check(error == 0, "the system configuration can be read");
	printf("system configuration generation: %llu\n", (unsigned long long) config.generation);
	if (strcmp(pushed, "pushed") == 0)
		check(config.generation >= 1, "a system configuration was pushed before this process started");
	else
		check(config.generation == 0, "no system configuration was pushed (darlingserver's defaults are in effect)");

	dserver_platform_identity_t identity;
	memset(&identity, 0, sizeof(identity));
	error = get_identity(&identity);
	check(error == 0, "the platform identity can be read");
	printf("identity: present 0x%x, model \"%.*s\"\n", identity.present,
		(int) sizeof(identity.model), identity.model);

	char model[256] = "";
	size_t length = sizeof(model);
	check(sysctlbyname("hw.model", model, &length, NULL, 0) == 0, "sysctl hw.model is reported");
	printf("hw.model: %s\n", model);

	if (expectDefault)
	{
		check(identity.present == 0, "no platform identity key is configured");
		check(model[0] != '\0' && strncmp(model, identity.model, sizeof(identity.model)) == 0,
			"hw.model is the default model");
	}
	else
	{
		check((identity.present & DSERVER_PLATFORM_IDENTITY_HAS_MODEL) != 0, "the model is configured");
		check(strcmp(model, expectedModel) == 0, "hw.model is the configured model");
	}

	uuid_t hostUUID;
	struct timespec wait = { 0, 0 };
	uuid_string_t hostUUIDString = "";
	int haveHostUUID = gethostuuid(hostUUID, &wait) == 0;
	check(haveHostUUID, "gethostuuid() succeeds");
	if (haveHostUUID)
		uuid_unparse_upper(hostUUID, hostUUIDString);
	printf("gethostuuid(): %s\n", hostUUIDString);

	if (expectedUUID != NULL)
	{
		uuid_t expected;
		check(uuid_parse(expectedUUID, expected) == 0, "the expected UUID is well-formed");
		check(haveHostUUID && uuid_compare(hostUUID, expected) == 0, "gethostuuid() is the configured UUID");
	}
	else if (expectDefault)
	{
		check((identity.present & DSERVER_PLATFORM_IDENTITY_HAS_PLATFORM_UUID) == 0, "the platform UUID is not configured");
	}

	if (iokit)
	{
		char iokitModel[256];
		char iokitUUID[64];
		int found = read_iokit(iokitModel, sizeof(iokitModel), iokitUUID, sizeof(iokitUUID));

		check(found, "IOPlatformExpertDevice has a model");
		printf("IOKit model: %s, IOPlatformUUID: %s\n", iokitModel, iokitUUID);
		check(strcmp(iokitModel, model) == 0, "IOKit's model equals hw.model");
		check(strcasecmp(iokitUUID, hostUUIDString) == 0, "IOPlatformUUID equals gethostuuid()");
	}

	finish();
}
