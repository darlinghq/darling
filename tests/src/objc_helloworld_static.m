#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
}

+ (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g;
@end

@implementation helloclass
+ (void)doHello:(int)a :(int)b :(int)c :(int)d :(int)e :(int)f :(int)g
{
        printf("Hello world, %d %d %d %d %d %d %d\n", a, b, c, d, e, f, g);
}
@end

int main()
{
		[helloclass doHello:1:2:3:4:5:6:7];
        return 0;
}


