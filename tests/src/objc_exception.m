// CFLAGS: -framework foundation
#include <stdio.h>
#import <Foundation/NSException.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	@try
	{
		@throw([NSException exceptionWithName: @"MyException" reason:@"Test" userInfo:nil]);
	}
	@catch(NSException* e)
	{
		puts([[e name] UTF8String]);
	}
	@finally
	{
		puts("Finally");
	}
	[pool release];
	return 0;
}

