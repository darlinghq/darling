
#import <Foundation/Foundation.h>

@interface DyldClass : NSObject
@end

@implementation DyldClass
@end

@interface DyldLinkedClass : NSObject
@end

@implementation DyldLinkedClass
@end

extern Class OBJC_CLASS_$_DyldClass;
extern Class OBJC_CLASS_$_DyldLinkedClass;

Class getLinked2DyldClass() {
	return (Class)&OBJC_CLASS_$_DyldClass; 
}

Class getLinked2DyldLinkedClass() {
	return (Class)&OBJC_CLASS_$_DyldLinkedClass; 
}