#include "NSDarwinFramework.h"
#import <Foundation/NSDictionary.h>
#include <map>
#include <string>
#include <objc/runtime.h>
#include "../util/log.h"

static NSMutableDictionary* _frameworks;

@implementation NSDarwinFramework
- (id) initWithVersion: (NSString*)version
               classes: (NSString**)classes
{
	_frameworkVersion = version;
	_frameworkClasses = classes;

	[_frameworks setObject: self
				    forKey: NSStringFromClass([self class])];

	return [super init];
}

+ (void) initialize
{
	if (self == [NSDarwinFramework class])
	{
		_frameworks = [[NSMutableDictionary alloc] init];
	}
}

- (void) dealloc
{
	if (_frameworkVersion != nil)
		[_frameworkVersion release];
	[super dealloc];
}

- (NSString*) thisFrameworkVersion
{
	return _frameworkVersion;
}

- (NSString**) thisFrameworkClasses
{
	return _frameworkClasses;
}

+ (NSString*) frameworkVersion
{
	NSDarwinFramework* instance;

	instance = [_frameworks objectForKey: NSStringFromClass(self)];
	if (instance == nil)
		return nil;
	else
		return [instance thisFrameworkVersion];
}

+ (NSString**) frameworkClasses
{
	NSDarwinFramework* instance;
	static NSString* emptyList[] = { nil };

	instance = [_frameworks objectForKey: NSStringFromClass(self)];
	if (instance == nil)
		return emptyList;
	else
		return [instance thisFrameworkClasses];
}

@end

void RegisterFramework(const char** classNames, size_t count, const char* path)
{
	NSString* bundlePath = [NSString stringWithUTF8String: path];
	NSString *bundleName, *bundleVersion = nil;
	NSString* className;
	Class synthetizedClass;
	NSDarwinFramework* fwInstance;
	NSString** nsClassNames;

	while (![bundlePath isEqual: @"/"])
	{
		NSString* cur = [bundlePath lastPathComponent];
		NSString* ext = [bundlePath pathExtension];

		if ([ext isEqual: @"framework"] || [ext isEqual: @"bundle"] || [ext isEqual: @"app"])
		{
			break;
		}

		bundlePath = [bundlePath stringByDeletingLastPathComponent];
		if ([[bundlePath lastPathComponent] isEqual: @"Versions"])
			bundleVersion = cur;
	}

	if ([bundlePath isEqual: @"/"])
	{
		LOG << "Invalid bundle lib path: " << path << std::endl;
		return;
	}

	bundleName = [[bundlePath lastPathComponent] stringByDeletingPathExtension];

	// Needed to have a valid class name
	bundleName = [bundleName stringByReplacingString: @"_"  withString: @"__"];
	bundleName = [bundleName stringByReplacingString: @"-"  withString: @"_0"];
	bundleName = [bundleName stringByReplacingString: @"+"  withString: @"_1"];

	className = [@"NSFramework_" stringByAppendingString: bundleName];

	synthetizedClass = objc_allocateClassPair([NSDarwinFramework class], [className UTF8String], 0);
	objc_registerClassPair(synthetizedClass);

	nsClassNames = new NSString*[count+1];
	for (size_t i = 0; i < count; i++)
		nsClassNames[i] = [[NSString alloc] initWithUTF8String: classNames[i]];
	
	nsClassNames[count] = nil;

	fwInstance = [[synthetizedClass alloc] initWithVersion: bundleVersion
                                                   classes: nsClassNames];
	
	LOG << "Created NSDarwinFramework instance named " << ([className UTF8String]) <<
		"; version: " << ([bundleVersion UTF8String]) << std::endl;
	
	if ([NSBundle respondsToSelector: @selector(_addFrameworkFromClass:withPath:)])
	{
		[NSBundle _addFrameworkFromClass: synthetizedClass
		                        withPath: [NSString stringWithUTF8String: path]];
	}
	else
		LOG << "NSFramework_* not registered, _addFrameworkFromClass:withPath: not supported by runtime\n";

}

