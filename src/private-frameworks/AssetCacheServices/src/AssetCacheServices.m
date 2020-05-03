/*
 This file is part of Darling.

 Copyright (C) 2020 Lubos Dolezel

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


#include <AssetCacheServices/AssetCacheServices.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void ACSImportFileIntoCachingServer(CFURLRef asset_url, CFStringRef source_path, CFDictionaryRef headers, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(bool)) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void ACSLocateCachingServer(CFURLRef asset_url, double timeout, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFURLRef, CFMutableDictionaryRef)) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void ACSMightCurrentNetworkHaveCachingServer(CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(bool)) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void ACSUpdateCachingServerHealth(CFURLRef url, CFDictionaryRef options, BOOL healthy) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void _ACSIntrospect(CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFTypeRef)) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void _ACSLocateAllCachingServers(double timeout, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFMutableArrayRef)) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void _ACSSetTestFlags(uint64_t flags, CFDictionaryRef dictionary) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};

void _A_CALLBACK_BLOCK_RELEASED_AN_ASSETCACHESERVICES_OBJECT_IT_DID_NOT_OWN(int integer, const char* function_name, const char* string, void* object) {
	if (verbose)
		printf("STUB: %s called\n", __PRETTY_FUNCTION__);
};
