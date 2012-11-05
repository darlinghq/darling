// CFLAGS: -framework corefoundation -framework foundation
#import <CoreFoundation/CFString.h>
#import <Foundation/NSString.h>
#include <stdio.h>

int main()
{
	CFStringRef str = CFSTR("Test");
	const char* text = [(NSString*)str UTF8String];
	puts(text);
	NSLog(@"%@\n", str);
	[str retain];
	printf("%u\n", [str retainCount]); // This should output UINT_MAX

	CFStringRef str2 = CFStringCreateWithBytes(NULL, "Test2", 6, kCFStringEncodingASCII, 0);
	CFRetain(str2);
	printf("%u\n", [str2 retainCount]);

	return 0;
}


