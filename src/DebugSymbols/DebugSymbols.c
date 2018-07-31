#include <DebugSymbols/DebugSymbols.h>

const char* SYSTEM_DSYMS_DIR = "/System/Library/Caches/dsym/uuid";

CFURLRef DBGCopyFullDSYMURLForUUID(CFUUIDRef uuid, CFURLRef exec_url)
{
	// Look up by UUID
	if (uuid != NULL)
	{
		CFStringRef uuidString = CFUUIDCreateString(NULL, uuid);
		CFStringRef dsymPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%s/%@.dSYM"), SYSTEM_DSYMS_DIR, uuidString);

		CFRelease(uuidString);

		if (access(CFStringGetCStringPtr(dsymPath, kCFStringEncodingUTF8), F_OK) == 0)
		{
			// printf("Found debug symbols: %s\n", CFStringGetCStringPtr(dsymPath, kCFStringEncodingUTF8));
			CFURLRef url = CFURLCreateWithFileSystemPath(NULL, dsymPath, kCFURLPOSIXPathStyle, false);
			CFRelease(dsymPath);
			return url;
		}

		fprintf(stderr, "Didn't find debug symbols at %s based on UUID\n",CFStringGetCStringPtr(dsymPath, kCFStringEncodingUTF8));
		CFRelease(dsymPath);
	}


	// Try looking next to the executable
	if (exec_url != NULL)
	{
		CFStringRef exec_path = CFURLCopyFileSystemPath(exec_url, kCFURLPOSIXPathStyle);
		CFStringRef dsymPath = CFStringCreateWithFormat(NULL, NULL, CFSTR("%@.dSYM"), exec_path);
		CFRelease(exec_path);

		if (access(CFStringGetCStringPtr(dsymPath, kCFStringEncodingUTF8), F_OK) == 0)
		{
			CFURLRef url = CFURLCreateWithFileSystemPath(NULL, dsymPath, kCFURLPOSIXPathStyle, false);
			CFRelease(dsymPath);
			return url;
		}
		fprintf(stderr, "Debug symbols not found at %s\n", CFStringGetCStringPtr(exec_path, kCFStringEncodingUTF8));

		CFRelease(dsymPath);
	}

	return NULL;
}

CFDictionaryRef DBGCopyDSYMPropertyLists(CFURLRef dsym_url)
{
    return NULL;
}
