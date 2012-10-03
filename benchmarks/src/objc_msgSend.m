#include <stdio.h>
#import <Foundation/NSObject.h>

@interface helloclass : NSObject {
}

+ (int)mult:(int)a :(int)b;
@end

@implementation helloclass
+ (int)mult:(int)a :(int)b
{
	return a*b;
}
@end

int main()
{
	int i;
	volatile int j;
	for (i = 0; i < 10000000; i++)
	{
		j += [helloclass mult:1:2];
	}
	
	return 0;
}


