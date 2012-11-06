#include <stdio.h>
#include <assert.h>
#import <Foundation/NSObject.h>

@class subclass;
subclass* c;

@interface helloclass : NSObject {
}

- (void)doHello;
@end

@implementation helloclass
- (void)doHello
{
	puts("Hello world from helloclass");
	//printf("self at %p\n", self);
	assert(self == c);
}
@end

@interface subclass : helloclass {
}

- (void)doHello;
@end

@implementation subclass
- (void)doHello
{
	puts("Calling superclass");
	//printf("self at %p\n", self);
	[super doHello];
}
@end

int main()
{
	c = [subclass new];
	//printf("Obj at %p\n", c);
	[c doHello];
	[c release];
	return 0;
}


