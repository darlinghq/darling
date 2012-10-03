#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
}

- (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g;
@end

@implementation helloclass
- (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g
{
        printf("Hello world, %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g);
}
@end

void run()
{
	helloclass* c = [helloclass alloc];
	puts("After alloc");
	[c init];
	puts("After init");
	[c doHello:1:2:3:4:5:6:7];
	puts("After hello");
	[c release];
	puts("After release");
}

int main()
{
	run();
	run();
	return 0;
}


