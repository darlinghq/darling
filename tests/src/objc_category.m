#import <Foundation/NSString.h>
#include <stdio.h>

@interface NSString (MyCategory)
+(void) dummy;
@end

@implementation NSString (MyCategory)
+(void) dummy
{
	puts("dummy called");
}
@end

int main()
{
	[NSString dummy];
	return 0;
}

