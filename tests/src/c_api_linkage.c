// Copyright (C) 2026 Lubos Dolezel
//
// C functions of CarbonCore and CoreAudio are exported under their C names,
// as on macOS, so programs that import them bind. Each name is looked up with
// dlsym() in the framework a program links for it, so that a missing one is
// reported instead of failing the link.
#include <dlfcn.h>
#include <stdio.h>

struct symbol
{
	const char* framework;
	const char* name;
};

#define CORESERVICES "/System/Library/Frameworks/CoreServices.framework/CoreServices"
#define COREAUDIO "/System/Library/Frameworks/CoreAudio.framework/CoreAudio"
#define AUDIOTOOLBOX "/System/Library/Frameworks/AudioToolbox.framework/AudioToolbox"

static const struct symbol symbols[] = {
	{ CORESERVICES, "FSGetCatalogInfo" },
	{ CORESERVICES, "UCConvertUTCDateTimeToCFAbsoluteTime" },
	{ CORESERVICES, "CreateTextToUnicodeInfo" },
	{ CORESERVICES, "CreateTextToUnicodeInfoByEncoding" },
	{ COREAUDIO, "AudioHardwareGetPropertyInfo" },
	{ AUDIOTOOLBOX, "AudioConverterNewSpecific" },
};

int main(void)
{
	int failures = 0;

	for (size_t i = 0; i < sizeof(symbols) / sizeof(symbols[0]); i++)
	{
		void* handle = dlopen(symbols[i].framework, RTLD_NOW);
		void* address = handle ? dlsym(handle, symbols[i].name) : NULL;

		printf("%s: %s in %s\n", address ? "PASS" : "FAIL", symbols[i].name, symbols[i].framework);
		if (!address)
			failures++;
	}

	if (failures)
	{
		printf("FAILED %d\n", failures);
		return 1;
	}
	printf("ALL PASSED\n");
	return 0;
}
