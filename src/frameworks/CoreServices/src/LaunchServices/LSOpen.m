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


#include <LaunchServices/LaunchServices.h>
#include <CarbonCore/MacErrors.h>
#include <stdio.h>

OSStatus
LSOpenURLsWithRole(
  CFArrayRef inURLs,
  LSRolesMask inRole,
  const AEKeyDesc * inAEParam,
  const LSApplicationParameters * inAppParams,
  ProcessSerialNumber * outPSNs,
  CFIndex inMaxPSNCount)
{
	if (!inURLs || CFArrayGetCount(inURLs) == 0)
		return paramErr;

	LSLaunchURLSpec spec;
	memset(&spec, 0, sizeof(spec));
	spec.itemURLs = inURLs;
	// Note: inRole is handled by LaunchServices default role resolution

	CFURLRef appURL = NULL;
	if (inAppParams)
	{
		if (inAppParams->application)
		{
			char path[4096];
			if (FSRefMakePath(inAppParams->application, (UInt8*) path, sizeof(path) - 1) == noErr)
			{
				CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
				if (urlStr)
				{
					// Application bundles (.app) are directories
					appURL = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, TRUE);
					CFRelease(urlStr);
				}
			}
		}
		spec.appURL = appURL;
		spec.launchFlags = inAppParams->flags;
		spec.passThruParams = NULL; // passThruParams expects an AEDesc*, not CFArrayRef
	}

	OSStatus status = LSOpenFromURLSpec(&spec, NULL);
	if (appURL)
		CFRelease(appURL);

	return status;
}

OSStatus
LSOpenItemsWithRole(
  const FSRef * inItems,
  CFIndex inItemCount,
  LSRolesMask inRole,
  const AEKeyDesc * inAEParam,
  const LSApplicationParameters * inAppParams,
  ProcessSerialNumber * outPSNs,
  CFIndex inMaxPSNCount)
{

	CFURLRef* urls = (CFURLRef*) malloc(inItemCount * sizeof(CFURLRef));

	for (CFIndex i = 0; i < inItemCount; i++)
	{
		char path[4096];

		OSStatus status = FSRefMakePath(&inItems[i], (UInt8*) path, sizeof(path)-1);
		if (status != noErr)
		{
			for (CFIndex j = 0; j < i; j++)
				CFRelease(urls[j]);
			free(urls);
			return status;
		}

		CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
		urls[i] = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, FALSE);
		CFRelease(urlStr);
	}

	CFArrayRef urlArray = CFArrayCreate(NULL, (const void**) urls, inItemCount, &kCFTypeArrayCallBacks);
	for (CFIndex i = 0; i < inItemCount; i++)
		CFRelease(urls[i]);
	free(urls);

	OSStatus status = LSOpenURLsWithRole(urlArray, inRole, inAEParam, inAppParams, outPSNs, inMaxPSNCount);

	CFRelease(urlArray);

	return status;
}

OSStatus LSOpenFSRef(const FSRef *inRef, FSRef *outLaunchedRef)
{
	char path[4096];

	OSStatus status = FSRefMakePath(inRef, (UInt8*) path, sizeof(path)-1);
	if (status != noErr)
		return status;

	CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, FALSE);
	CFRelease(urlStr);
	
	CFURLRef launchedUrl;
	status = LSOpenCFURLRef(url, &launchedUrl);

	CFRelease(url);

	if (status != noErr)
		return status;

	if (outLaunchedRef)
	{
		CFStringRef path = CFURLCopyFileSystemPath(launchedUrl, kCFURLPOSIXPathStyle);
		status = FSPathMakeRef((const UInt8*) CFStringGetCStringPtr(path, kCFStringEncodingUTF8), outLaunchedRef, NULL);
		CFRelease(path);
	}
	
	CFRelease(launchedUrl);
	return status;
}

OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef *outLaunchedURL)
{
	OSStatus ret;
	LSLaunchURLSpec spec;

	memset(&spec, 0, sizeof(spec));

	spec.itemURLs = CFArrayCreate(kCFAllocatorDefault, (const void**)&inURL, 1, NULL);
	ret = LSOpenFromURLSpec(&spec, outLaunchedURL);
	CFRelease(spec.itemURLs);
	
	return ret;
}

/*
OSStatus LSOpenApplication(const LSApplicationParameters *appParams, ProcessSerialNumber *outPSN)
{

}
*/

/*
OSStatus
LSOpenFromURLSpec(
  const LSLaunchURLSpec * inLaunchSpec,
  _Nullable CFURLRef *_Nullable outLaunchedURL)
{
}
*/

OSStatus LSOpenFromRefSpec(const LSLaunchFSRefSpec *inLaunchSpec, FSRef *outLaunchedRef)
{
	if (!inLaunchSpec)
		return paramErr;

	CFURLRef appURL = NULL;
	if (inLaunchSpec->appRef != NULL)
	{
		char path[4096];
		OSStatus status = FSRefMakePath(inLaunchSpec->appRef, (UInt8*) path, sizeof(path) - 1);
		if (status != noErr)
			return status;

		CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
		if (!urlStr)
			return coreFoundationUnknownErr;
		appURL = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, FALSE);
		CFRelease(urlStr);
	}

	CFMutableArrayRef itemURLs = NULL;
	if (inLaunchSpec->itemRefs != NULL && inLaunchSpec->numDocs > 0)
	{
		itemURLs = CFArrayCreateMutable(NULL, inLaunchSpec->numDocs, &kCFTypeArrayCallBacks);
		for (ItemCount i = 0; i < inLaunchSpec->numDocs; ++i)
		{
			char path[4096];
			OSStatus status = FSRefMakePath(&inLaunchSpec->itemRefs[i], (UInt8*) path, sizeof(path) - 1);
			if (status == noErr)
			{
				CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
				if (urlStr)
				{
					CFURLRef docURL = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, FALSE);
					CFRelease(urlStr);
					if (docURL)
					{
						CFArrayAppendValue(itemURLs, docURL);
						CFRelease(docURL);
					}
				}
			}
		}
	}

	if (!appURL && (!itemURLs || CFArrayGetCount(itemURLs) == 0))
	{
		if (itemURLs)
			CFRelease(itemURLs);
		return paramErr;
	}

	LSLaunchURLSpec urlSpec;
	memset(&urlSpec, 0, sizeof(urlSpec));
	urlSpec.appURL = appURL;
	urlSpec.itemURLs = itemURLs;
	urlSpec.passThruParams = inLaunchSpec->passThruParams;
	urlSpec.launchFlags = inLaunchSpec->launchFlags;
	urlSpec.asyncRefCon = inLaunchSpec->asyncRefCon;

	CFURLRef launchedURL = NULL;
	OSStatus status = LSOpenFromURLSpec(&urlSpec, &launchedURL);

	if (appURL)
		CFRelease(appURL);
	if (itemURLs)
		CFRelease(itemURLs);

	if (status != noErr)
		return status;

	if (outLaunchedRef && launchedURL)
	{
		char path[4096];
		CFStringRef pathStr = CFURLCopyFileSystemPath(launchedURL, kCFURLPOSIXPathStyle);
		if (pathStr)
		{
			if (CFStringGetCString(pathStr, path, sizeof(path), kCFStringEncodingUTF8))
			{
				FSPathMakeRef((const UInt8*) path, outLaunchedRef, NULL);
			}
			CFRelease(pathStr);
		}
	}

	if (launchedURL)
		CFRelease(launchedURL);

	return status;
}
