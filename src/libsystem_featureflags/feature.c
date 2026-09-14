/*
 * This file is part of Darling.
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include <stdbool.h>

// Backs the os_feature_enabled(domain, feature) macro used by binaries built against newer SDKs.
// On macOS this consults the FeatureFlags configuration; Darling has no feature flag database,
// so every feature reports its default of being disabled.
__attribute__((visibility("default")))
bool _os_feature_enabled_impl(const char *domain, const char *feature)
{
	return false;
}
