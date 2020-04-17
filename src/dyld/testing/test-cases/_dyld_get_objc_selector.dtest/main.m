
// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_get_objc_selector.exe -lobjc

// RUN:  ./_dyld_get_objc_selector.exe

#include <mach-o/dyld_priv.h>

#import <Foundation/Foundation.h>

@interface DyldClass : NSObject
@end

@implementation DyldClass
-(void) dyldClassFoo {
	
}
+(void) dyldClassFoo {
	
}
@end

@interface DyldMainClass : NSObject
@end

@implementation DyldMainClass
-(void) dyldMainClassFoo {
	
}
-(void) dyldMainClassFoo2 {
	
}
@end

extern int printf(const char*, ...);

extern id objc_getClass(const char *name);

int main() {
	printf("[BEGIN] _dyld_get_objc_selector\n");

	// dyldClassFoo
	const char* sel = _dyld_get_objc_selector("dyldClassFoo");
	if (sel) {
		if ((SEL)sel != @selector(dyldClassFoo)) {
			printf("[FAIL] _dyld_get_objc_selector: dyldClassFoo is wrong\n");
			return 0;
		}
	}

	// dyldMainClassFoo
	sel = _dyld_get_objc_selector("dyldMainClassFoo");
	if (sel) {
		if ((SEL)sel != @selector(dyldMainClassFoo)) {
			printf("[FAIL] _dyld_get_objc_selector: dyldMainClassFoo is wrong\n");
			return 0;
		}
	}

	// dyldMainClassFoo2
	sel = _dyld_get_objc_selector("dyldMainClassFoo2");
	if (sel) {
		if ((SEL)sel != @selector(dyldMainClassFoo2)) {
			printf("[FAIL] _dyld_get_objc_selector: dyldMainClassFoo2 is wrong\n");
			return 0;
		}
	}

	printf("[PASS] _dyld_get_objc_selector\n");

	return 0;
}