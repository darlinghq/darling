#include <stdio.h>
#import <Foundation/NSObject.h>

@protocol MyProtocol <NSObject>
@required
+ (void)doHelloStatic;
- (void)doHello;
@optional
- (void)optFunc;
@end

@protocol EmptyProtocol
+ (void)dummy;
@end

@interface helloclass : NSObject<MyProtocol> {
}

+ (void)doHelloStatic;
- (void)doHello;
@end

@interface otherclass : NSObject {
}

@end

@implementation otherclass
@end

@implementation helloclass
+ (void)doHelloStatic
{
	puts("Hello world static");
}
- (void)doHello
{
	puts("Hello world instance");
}
@end

int main()
{
	if ([helloclass conformsToProtocol:@protocol(MyProtocol)])
		puts("helloclass conforms");
	if (![otherclass conformsToProtocol:@protocol(MyProtocol)])
		puts("otherclass doesn't conform");
	id<MyProtocol> proto = [helloclass new];
	[proto doHello];
	return 0;
}


