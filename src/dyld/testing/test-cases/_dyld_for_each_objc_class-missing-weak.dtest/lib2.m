
#import <Foundation/Foundation.h>

__attribute__((weak_import))
@interface MissingWeakSuperclass : NSObject
@end

// Note this class will be ignored at runtime as its superclass is weak
@interface DyldClass : MissingWeakSuperclass
@end

@implementation DyldClass
@end

// Note that this class does not have a weak superclass, but we can't optimize anything in this
// dylib as DyldClass does have a missing weak superclass.  So we shouldn't find DyldLinkedClass in our
// closure class map
@interface DyldLinkedClass : NSObject
@end

@implementation DyldLinkedClass
@end

extern Class OBJC_CLASS_$_DyldLinkedClass;

Class getLinked2DyldLinkedClass() {
	return (Class)&OBJC_CLASS_$_DyldLinkedClass; 
}