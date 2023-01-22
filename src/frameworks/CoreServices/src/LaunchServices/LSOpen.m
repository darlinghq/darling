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
	puts("LSOpenURLsWithRole STUB");
	return unimpErr;
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
			for (CFIndex j; j < i; j++)
				CFRelease(urls[j]);
			free(urls);
			return status;
		}

		CFStringRef urlStr = CFStringCreateWithCString(NULL, path, kCFStringEncodingUTF8);
		urls[i] = CFURLCreateWithFileSystemPath(NULL, urlStr, kCFURLPOSIXPathStyle, FALSE);
		CFRelease(urlStr);
	}

	CFArrayRef urlArray = CFArrayCreate(NULL, (const void**) urls, inItemCount, &kCFTypeArrayCallBacks);
	for (CFIndex i; i < inItemCount; i++)
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
