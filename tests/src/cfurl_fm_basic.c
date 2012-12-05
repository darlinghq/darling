// CFLAGS: -framework corefoundation -framework coreservices
#include <CoreFoundation/CFURL.h>
#include <Carbon/Carbon.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define TESTPATH "/home"

int main()
{
	CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFSTR(TESTPATH), kCFURLPOSIXPathStyle, 1);
	FSRef fsref;
	Boolean b;
	char* buffer = (char*) malloc(100);
	OSStatus status;

	b = CFURLGetFSRef(url, &fsref);
	assert(b != 0);

	status = FSRefMakePath(&fsref, buffer, 100);
	assert(status == 0);
	
	puts(buffer);
	assert(strcmp(TESTPATH, buffer) == 0);

	free(buffer);
	CFRelease(url);
	return 0;
}

