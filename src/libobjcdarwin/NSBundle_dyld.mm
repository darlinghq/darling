#import "NSBundle_dyld.h"
#import <limits.h>
#import <Foundation/NSString.h>
#include <string>
#include <cstdio>
#include <algorithm>
#include "../util/log.h"

extern char g_darwin_executable_path[PATH_MAX];
static NSBundle* _mainBundle = 0;

void MethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel);

__attribute__((constructor)) static void myinit()
{
	LOG << "Swizzling methods in NSBundle\n";
	
	MethodSwizzle(objc_getMetaClass("NSBundle"), @selector(mainBundle), @selector(x_mainBundle));
}


@implementation NSBundle (NSBundle_dyld)
/*
+(void) load
{
	LOG << "Swizzling methods in NSBundle\n";
	RenameSelector([self class], @selector(mainBundle), @selector(gnu_mainBundle));
	RenameSelector([self class], @selector(x_mainBundle), @selector(mainBundle));
}
*/

+(NSBundle*) x_mainBundle
{
	LOG << "x_mainBundle() called\n";
	if (!_mainBundle)
	{
		std::string path = g_darwin_executable_path;
		size_t pos = path.find_last_of('/');
		if (pos == std::string::npos)
			return nil;

		path.resize(pos+1);
		//path += "Resources";

		LOG << "NSBundle::x_mainBundle(): deduced " << path << " as resource path\n";

		_mainBundle = [self alloc];
		_mainBundle = [_mainBundle initWithPath:[NSString stringWithUTF8String:path.c_str()]];
	}

	return _mainBundle;
}

+(NSBundle*) x_bundleForClass: (Class) aClass
{
	return [super gnu_bundleForClass:aClass];
}

@end

struct objc_method
{
	SEL selector;
	const char* types;
	IMP imp;
};

void MethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel)
{
	Method orig_method, alt_method;
   
	orig_method = (Method) class_getInstanceMethod(aClass, orig_sel);
	alt_method = (Method) class_getInstanceMethod(aClass, alt_sel);
   
	if (orig_method && alt_method)
	{
		std::swap(orig_method->types, alt_method->types);
		std::swap(orig_method->imp, alt_method->imp);
	}
}

