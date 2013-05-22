#ifndef NSDARWINFRAMEWORK_H
#define NSDARWINFRAMEWORK_H
#include <cstddef>

#ifdef __OBJC__
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>

// This is a class gnustep-base expect to find in every framework

@interface NSDarwinFramework : NSObject {
	NSString* _frameworkVersion;
	NSString** _frameworkClasses;
}

+ (void) initialize;

// Darling's API
- (id) initWithVersion: (NSString*)version
               classes: (NSString**)classes;
- (void) dealloc;

- (NSString*) thisFrameworkVersion;
- (NSString**) thisFrameworkClasses;

// GNUstep's API
+ (NSString*) frameworkVersion;
+ (NSString**) frameworkClasses;
@end


// NSBundle private implementation details
@interface NSBundle (Private)
+ (NSBundle*) _addFrameworkFromClass: (Class)frameworkClass
                            withPath:(NSString*)path;
@end

#endif // __OBJC__

void RegisterFramework(const char** classNames, size_t count, const char* path);

#endif

