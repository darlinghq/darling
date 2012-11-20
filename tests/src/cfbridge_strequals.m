// CFLAGS: -framework foundation -framework corefoundation

#import <Foundation/NSString.h>
#import <CoreFoundation/CFString.h>
#include <stdio.h>
#include <assert.h>

#define pass(cond) { int ok = (cond); printf("Pass %s: %s\n", #cond, (cond) ? "OK" : "FAIL"); if(!cond) exitcode = 1;  }

int main()
{
	NSString* cfstr = (NSString*) CFSTR("TestString");
	NSString* nsstr = [NSString stringWithUTF8String:"TestString"];
	int exitcode = 0;

	pass([nsstr isEqual:cfstr]);
	pass([nsstr isEqualToString:cfstr]);
	pass([cfstr isEqual:nsstr]);
	pass([cfstr isEqualToString:nsstr]);
	pass([nsstr hash] == [cfstr hash]);

	return exitcode;
}


