#include <AppKit/AppKit.h>
#include <Foundation/NSProcessInfo.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSString.h>
#include <Foundation/NSFileManager.h>
#include "CrashManager.h"

@implementation CrashManager


- (void) close: (id)sender
{
	[NSApp terminate: nil];
}


- (void) debugInfo: (id)sender
{
  /* insert your code here */
}


- (void) reopen: (id)sender
{
  /* insert your code here */
}

- (id) init
{
	self = [super init];

	NSArray* args = [[NSProcessInfo processInfo] arguments];
	if ([args count] == 2)
	{
		NSString* dumpPath = [args objectAtIndex: 1];
		NSString* text = [NSString stringWithContentsOfFile:dumpPath encoding:NSUTF8StringEncoding error:NULL];
		NSArray* elems = [text componentsSeparatedByString:@"---CUT---"];

		if ([elems count] >= 4)
		{
			executablePath = [elems objectAtIndex:0];
			commandLine = [elems objectAtIndex:1];
			shortDescription = [elems objectAtIndex:2];
			fullLog = [elems objectAtIndex:3];
		}

		[elems release];
		[text release];
		
		[[NSFileManager defaultManager] removeItemAtPath:dumpPath error:NULL];
	}

	return self;
}

@end
