// CFLAGS: -framework foundation

// Bugs found:
// objc_superrefs not properly fixed at load time

#import <Foundation/NSObject.h>
#include <stdio.h>
#include <objc/runtime.h>

@interface baseclass : NSObject
+(long double)test;
@end

@interface subclass : baseclass
+(long double)test;
@end

@implementation baseclass
+(long double)test
{
	puts("Test called");
	return 5.0;
}
@end

@implementation subclass
+(long double)test
{
	long double v = [super test];
	printf("After supercall: %Lf\n", v);
	v += 1.0;
	return v;
}
@end

int main()
{
	long double v = [subclass test];
	printf("Return value: %Lf\n", v);
	return 0;
}

