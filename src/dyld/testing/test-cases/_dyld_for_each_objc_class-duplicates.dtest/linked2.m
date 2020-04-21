
#import <Foundation/NSObject.h>

@interface NSString : NSObject
@end

@implementation NSString
@end

extern Class OBJC_CLASS_$_NSString;

Class getLinked2NSString() {
	return (Class)&OBJC_CLASS_$_NSString;
}
