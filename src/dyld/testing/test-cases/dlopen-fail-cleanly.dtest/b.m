#include <Foundation/Foundation.h>

extern void c_extra();

// The existence of this class should prevent libb.dylib from being unloaded

@interface TestClass : NSObject
-(void) foo;
@end

@implementation TestClass
-(void) foo { c_extra(); }
@end

