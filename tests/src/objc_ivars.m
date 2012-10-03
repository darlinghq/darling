#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
	// We actually need to have two variables here, or the instance sizes won't match
	// It seems that the instance size is not rounded up to 4/8 for types generated at runtime
	@public int variable, var2;
}

- (void)doHello;
@end

@implementation helloclass
// @synthesize variable;
- (void)doHello
{
        printf("Hello world, %d\n", variable);
}
@end

int main()
{
	printf("instance size: %d\n", (int) class_getInstanceSize([helloclass class]));
	helloclass* c = [helloclass new];
	puts("after alloc");
	c->variable = 3;
	puts("after ivarSet");
	[c doHello];
	puts("after doHello");
	[c release];
	return 0;
}


