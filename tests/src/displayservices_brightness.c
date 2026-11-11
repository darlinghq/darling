// Copyright (C) 2026 Lubos Dolezel
//
// The DisplayServices private framework exports the brightness entry points
// and reports that no display can change its brightness.
//
// Darling-specific: on macOS the answer depends on the display hardware, so
// this test is only meaningful on Darling.
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	void *h = dlopen("/System/Library/PrivateFrameworks/DisplayServices.framework/DisplayServices", RTLD_NOW);
	if (!h) {
		printf("FAIL: dlopen: %s\n", dlerror());
		return 1;
	}
	int (*can)(uint32_t) = dlsym(h, "DisplayServicesCanChangeBrightness");
	int (*get)(uint32_t, float *) = dlsym(h, "DisplayServicesGetBrightness");
	int (*set)(uint32_t, float) = dlsym(h, "DisplayServicesSetBrightness");
	int (*getl)(uint32_t, float *) = dlsym(h, "DisplayServicesGetLinearBrightness");
	int (*setl)(uint32_t, float) = dlsym(h, "DisplayServicesSetLinearBrightness");
	CHECK(can && get && set && getl && setl);
	if (!(can && get && set && getl && setl)) return 1;

	CHECK(can(1) == 0);
	float b = -1;
	CHECK(get(1, &b) == 0);
	CHECK(b >= 0.0f && b <= 1.0f);
	b = -1;
	CHECK(getl(1, &b) == 0);
	CHECK(b >= 0.0f && b <= 1.0f);
	CHECK(set(1, 0.5f) == 0);
	CHECK(setl(1, 0.5f) == 0);

	// The calls below are not exported by the macOS framework.
	CHECK(dlsym(h, "DisplayServicesBrightnessChanged") == NULL);
	CHECK(dlsym(h, "DisplayServicesRegisterForAmbientLightEvents") == NULL);

	if (g_failures) {
		printf("FAILED: %d\n", g_failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
