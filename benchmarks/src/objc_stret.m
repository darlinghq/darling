#include <stdio.h>
#import <Foundation/NSObject.h>

struct st
{
	float f1;
	int buf[50];
};

@interface helloclass : NSObject {
}

+ (struct st)mult:(float)a :(float)b;
@end

@implementation helloclass
+ (struct st)mult:(float)a :(float)b
{
	struct st s;
	s.f1 = a+b;
	return s;
}
@end

int main()
{
	int i;
	for (i = 0; i < 10000000; i++)
	{
		struct st v = [helloclass mult:1.0f:2.0f];
		v.f1;
	}
	
	return 0;
}


