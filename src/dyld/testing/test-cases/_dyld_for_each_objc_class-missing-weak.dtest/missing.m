
#import <Foundation/Foundation.h>

@interface MissingWeakSuperclass : NSObject
@end

@implementation MissingWeakSuperclass
@end

@interface DyldClass : MissingWeakSuperclass
@end

@implementation DyldClass
@end

id getMissingDyldClass() {
	return [DyldClass class]; 
}