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


#ifndef _AssetCacheServices_H_
#define _AssetCacheServices_H_

#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>

#import <Foundation/Foundation.h>

#import <AssetCacheServices/ACSURLSession.h>
#import <AssetCacheServices/ACSURLSessionUploadTask.h>
#import <AssetCacheServices/ACSURLSessionDataTask.h>
#import <AssetCacheServices/ACSURLSessionTask.h>
#import <AssetCacheServices/ACSURLSessionDownloadTask.h>

void ACSImportFileIntoCachingServer(CFURLRef asset_url, CFStringRef source_path, CFDictionaryRef headers, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(bool));
void ACSLocateCachingServer(CFURLRef asset_url, double timeout, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFURLRef, CFMutableDictionaryRef));
void ACSMightCurrentNetworkHaveCachingServer(CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(bool));
void ACSUpdateCachingServerHealth(CFURLRef url, CFDictionaryRef options, BOOL healthy);
void _ACSIntrospect(CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFTypeRef)); // block argument created with `_CFXPCCreateCFObjectFromXPCObject` from an `introspection` key in the XPC response dictionary
void _ACSLocateAllCachingServers(double timeout, CFDictionaryRef options, dispatch_queue_t callback_queue, void(^callback)(CFMutableArrayRef));
void _ACSSetTestFlags(uint64_t flags, CFDictionaryRef dictionary);
void _A_CALLBACK_BLOCK_RELEASED_AN_ASSETCACHESERVICES_OBJECT_IT_DID_NOT_OWN(int integer, const char* function_name, const char* string, void* object);

#endif
