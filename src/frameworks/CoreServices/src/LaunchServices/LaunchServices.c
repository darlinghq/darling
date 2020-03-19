/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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
#include <stdio.h>
#include <stdlib.h>

static CFStringRef get_main_executable(CFURLRef appURL);

OSStatus LSRegisterURL(CFURLRef inURL, Boolean inUpdate)
{
	printf("STUB: LSRegisterURL\n");
	return 0;
}

OSStatus LSOpenCFURLRef(CFURLRef inURL, CFURLRef *outLaunchedURL)
{
	OSStatus ret;
	LSLaunchURLSpec spec;
	spec.itemURLs = CFArrayCreate(kCFAllocatorDefault, (const void**)&inURL, 1, NULL);
	ret = LSOpenFromURLSpec(&spec, outLaunchedURL);
	CFRelease(spec.itemURLs);
	return ret;
}

OSStatus LSOpenFromURLSpec(const LSLaunchURLSpec *inLaunchSpec, CFURLRef *outLaunchedURL)
{
	CFStringRef scheme, extension, executable;
	CFIndex count, i;
	CFURLRef url, url_plus_exec, pre;
	CFRange range;
	OSStatus ret = 0;

	/* Inspect scheme of URL */
	count = CFArrayGetCount(inLaunchSpec->itemURLs);
	for (i = 0; i < count; i++)
	{
		url = CFArrayGetValueAtIndex(inLaunchSpec->itemURLs, i);
		scheme = CFURLCopyScheme(url);
		range = CFRangeMake(0, CFStringGetLength(scheme));

		if (CFStringCompareWithOptions(scheme, CFSTR("file"), range, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
		{
			/* First check if it's a "mac thing", if not forward to xdg-open */

			/* Check for .app suffix */
			extension = CFURLCopyPathExtension(url);
			range = CFRangeMake(0, CFStringGetLength(extension));
			if (CFStringCompareWithOptions(extension, CFSTR("app"), range, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
			{
				/* Read info.plist, find path to main executable */
				if ((executable = get_main_executable(url)) == NULL)
				{
					ret = 1;
				}
				else
				{
					url_plus_exec = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, url, CFSTR("Contents"), TRUE);
					pre = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, url_plus_exec, CFSTR("MacOS"), TRUE);
					CFRelease(url_plus_exec);
					url_plus_exec = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, pre, executable, FALSE);
					CFRelease(pre);

					/* Launch the executable */
					executable = CFURLGetString(url_plus_exec);
					CFShow(executable);

					CFRelease(url_plus_exec);
					CFRelease(executable);
				}
			}
			else
			{
				printf("TODO: forward to xdg-open\n");
				ret = 1;
			}

			CFRelease(extension);
		}
		else
		{
			printf("We can't handle this scheme yet\n");
			CFShow(scheme);
			ret = kLSApplicationNotFoundErr;
		}
		CFRelease(scheme);
	}
	return ret;
}

static CFStringRef get_main_executable(CFURLRef appURL)
{
	CFURLRef pre = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, appURL, CFSTR("Contents"), TRUE);
	CFURLRef info_plist_url = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, pre, CFSTR("Info.plist"), FALSE);
	CFReadStreamRef rs = CFReadStreamCreateWithFile(kCFAllocatorDefault, info_plist_url);
	CFDictionaryRef info_plist;
	CFStringRef ret = NULL;

	CFRelease(pre);

	if (!CFReadStreamOpen(rs))
	{
		printf("Failed to read Info.plist\n");
		goto err_pre_info;
	}

	info_plist = CFPropertyListCreateWithStream(kCFAllocatorDefault, rs, 0, 0, NULL, NULL);

	if (CFGetTypeID(info_plist) != CFDictionaryGetTypeID())
	{
		printf("Root object of Info.plist not CFDictionaryRef!\n");
		goto err;
	}

	ret = CFDictionaryGetValue(info_plist, CFSTR("CFBundleExecutable"));
	if (ret != NULL)
		ret = CFStringCreateCopy(kCFAllocatorDefault, ret);

err:
	CFRelease(info_plist);
err_pre_info:
	CFRelease(rs);
	CFRelease(info_plist_url);
	return ret;
}
