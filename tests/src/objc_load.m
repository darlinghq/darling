#import <Foundation/NSObject.h>
#import <stdio.h>

@interface MyClass : NSObject
+(void) load;
@end

@implementation MyClass
+(void) load
{
	puts("MyClass::load called");
}
@end

int main()
{
	MyClass* c = [MyClass new];
	[c release];
	return 0;
}

