#include <stdio.h>
#import <Foundation/NSObject.h>
#import <objc/runtime.h>

@interface helloclass : NSObject {
	@private int var;
}
@property (readwrite,assign) int var;
@end

@implementation helloclass
@synthesize var;
@end

int main()
{
	helloclass* c = [helloclass new];
	c.var = 5;

	unsigned int outCount, i;
	objc_property_t *properties = class_copyPropertyList([helloclass class], &outCount);
	printf("%u properties\n", outCount);
	for(i = 0; i < outCount; i++)
	{
		    objc_property_t property = properties[i];
			printf("%s %s\n", property_getName(property), property_getAttributes(property));
	}
	free(properties);
	
	Method* methods = class_copyMethodList([helloclass class], &outCount);
	printf("%u methods\n", outCount);
	free(methods);

	return 0;
}


