#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
}

- (void)doHello;
@end

@implementation helloclass
- (void)doHello
{
	puts("Hello world from helloclass");
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
	[super doHello];
}
@end

int main()
{
	subclass* c = [subclass new];
	[c doHello];
	[c release];
	return 0;
}


