// BUILD_ONLY: MacOSX

// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_get_objc_selector-chained.exe -lobjc -Wl,-fixup_chains

// RUN:  ./_dyld_get_objc_selector-chained.exe

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
	printf("[BEGIN] _dyld_get_objc_selector-chained\n");

	// dyldClassFoo
	const char* sel = _dyld_get_objc_selector("dyldClassFoo");
	if (sel) {
		if ((SEL)sel != @selector(dyldClassFoo)) {
			printf("[FAIL] _dyld_get_objc_selector-chained: dyldClassFoo is wrong\n");
			return 0;
		}
	}

	// dyldMainClassFoo
	sel = _dyld_get_objc_selector("dyldMainClassFoo");
	if (sel) {
		if ((SEL)sel != @selector(dyldMainClassFoo)) {
			printf("[FAIL] _dyld_get_objc_selector-chained: dyldMainClassFoo is wrong\n");
			return 0;
		}
	}

	// dyldMainClassFoo2
	sel = _dyld_get_objc_selector("dyldMainClassFoo2");
	if (sel) {
		if ((SEL)sel != @selector(dyldMainClassFoo2)) {
			printf("[FAIL] _dyld_get_objc_selector-chained: dyldMainClassFoo2 is wrong\n");
			return 0;
		}
	}

	printf("[PASS] _dyld_get_objc_selector-chained\n");

	return 0;
}
