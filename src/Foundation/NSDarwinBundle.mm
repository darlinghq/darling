#include "NSDarwinBundle.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProcessInfo.h>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <util/debug.h>
#include <libdyld/MachOMgr.h>
#include <libdyld/MachOObject.h>
#include <libdyld/dl_public.h>

extern int g_argc asm("NXArgc");
extern char** g_argv asm("NXArgv");
static NSBundle* _mainBundle = 0;
static NSAutoreleasePool* g_pool = 0;

static void MethodSwizzle(Class aClass, SEL orig_sel, SEL alt_sel);

__attribute__((constructor)) static void myinit()
{
	if (g_argv != nullptr)
		GSInitializeProcess(g_argc, g_argv, environ);
	LOG << "Swizzling methods in NSBundle\n";
	
	MethodSwizzle(objc_getMetaClass("NSBundle"), @selector(mainBundle), @selector(x_mainBundle));
	//[NSBundle mainBundle];

	// Many OS X apps assume that there is a "default" autorelease pool provided
	g_pool = [[NSAutoreleasePool alloc] init];
}

__attribute__((destructor)) static void myexit()
{
	[g_pool drain];
	g_pool = 0;
}

@implementation NSDarwinBundle
+ (long) _loadModuleWithFilename: (NSString*)filename
                     errorStream: (FILE*)errorStream
                    loadCallback: (void (*)(Class, struct objc_category *))loadCallback
                          header: (void**)header
                   debugFilename: (NSString*)debugFilename
{
	const char* path = [filename UTF8String];
	void* lib = __darwin_dlopen(path, DARWIN_RTLD_LAZY);

	// TODO: report loaded classes via loadCallback

	if (!lib)
		fprintf(errorStream, "Failed to __darwin_dlopen: %s\n", __darwin_dlerror());

	return lib ? 0 : 1;
}

@end

@implementation NSBundle (NSBundle_Darling)

+(NSBundle*) x_mainBundle
{
	LOG << "x_mainBundle() called\n";
	if (!_mainBundle)
	{
		std::string path = Darling::MachOMgr::instance()->mainModule()->path();
		size_t pos;

		//size_t pos = path.find_last_of('/');
		//if (pos == std::string::npos)
		//	return nil;

		//path.resize(pos+1);
		//path += "Resources";
		
		if ((pos = path.rfind("Contents/")) != std::string::npos)
		{
			path.resize(pos+8);
		}
		else if ((pos = path.rfind(".app/")) != std::string::npos)
		{
			// "path.endsWith()"
			// if (path.compare(path.size()-7, 7, "/MacOS/") == 0)
			// 	path.resize(path.size() - 7);
			path.resize(pos+4);
		}

		LOG << "NSBundle::x_mainBundle(): deduced " << path << " as resource path\n";

		_mainBundle = [self alloc];
		_mainBundle = [_mainBundle initWithPath:[NSString stringWithUTF8String:path.c_str()]];
	}

	return _mainBundle;
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

