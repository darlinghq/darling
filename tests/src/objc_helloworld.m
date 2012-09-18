// CFLAGS: -lobjc
#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
}

- (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g;
@end

@implementation helloclass
- (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g
{
        puts("Hello world");
}
@end

int main()
{
        helloclass* c = [[helloclass alloc] init];
		[c doHellox:1:2:3:4:5:6:7];
        [c release];
        return 0;
}


