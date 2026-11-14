/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Frameworks added with IOSSUPPORT_ALIAS must be reachable at both their
 * normal path and under /System/iOSSupport, and the image loaded through the
 * iOSSupport path must be the same one as through the normal path.
 */
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fw { const char *dir; const char *name; };

static const struct fw frameworks[] = {
	{ "Frameworks", "AVKit" },
	{ "Frameworks", "AuthenticationServices" },
	{ "Frameworks", "ContactsUI" },
	{ "Frameworks", "MapKit" },
	{ "Frameworks", "PDFKit" },
	{ "Frameworks", "QuickLook" },
	{ "Frameworks", "SceneKit" },
	{ "Frameworks", "StoreKit" },
	{ "Frameworks", "WebKit" },
	{ "PrivateFrameworks", "AuthKitUI" },
	{ "PrivateFrameworks", "RecapPerformanceTesting" },
};

int main(void)
{
	int failures = 0;

	for (size_t i = 0; i < sizeof(frameworks) / sizeof(frameworks[0]); i++) {
		char normal[PATH_MAX], alias[PATH_MAX], real_normal[PATH_MAX], real_alias[PATH_MAX];

		snprintf(normal, sizeof(normal), "/System/Library/%s/%s.framework/%s",
		         frameworks[i].dir, frameworks[i].name, frameworks[i].name);
		snprintf(alias, sizeof(alias), "/System/iOSSupport/System/Library/%s/%s.framework/%s",
		         frameworks[i].dir, frameworks[i].name, frameworks[i].name);

		if (!realpath(normal, real_normal)) {
			printf("FAIL: %s: normal path does not resolve\n", frameworks[i].name);
			failures++;
			continue;
		}
		if (!realpath(alias, real_alias)) {
			printf("FAIL: %s: iOSSupport path does not resolve\n", frameworks[i].name);
			failures++;
			continue;
		}
		if (strcmp(real_normal, real_alias) != 0) {
			printf("FAIL: %s: iOSSupport path leads to a different file (%s vs %s)\n",
			       frameworks[i].name, real_alias, real_normal);
			failures++;
			continue;
		}

		void *via_alias = dlopen(alias, RTLD_NOW);
		void *via_normal = dlopen(normal, RTLD_NOW);
		if (!via_alias || !via_normal) {
			printf("FAIL: %s: dlopen failed: %s\n", frameworks[i].name, dlerror());
			failures++;
			continue;
		}
		if (via_alias != via_normal) {
			printf("FAIL: %s: the two paths loaded two images\n", frameworks[i].name);
			failures++;
		}
	}

	if (failures)
		return 1;
	printf("PASS: all frameworks reachable at the iOSSupport path\n");
	return 0;
}
