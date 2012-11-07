#import <Foundation/NSObject.h>
#include <stdio.h>

// for old runtime testing
@interface RandomClass : NSObject
@end

@implementation RandomClass
@end

@interface NSObject (MyCategory)
+(void) dummy;
@end

@implementation NSObject (MyCategory)
+(void) dummy
{
	puts("dummy called");
}
@end

int main()
{
	[NSObject dummy];
	return 0;
}

