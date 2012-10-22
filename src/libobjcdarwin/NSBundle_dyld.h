#ifndef NSBUNDLE_DYLD_H
#define NSBUNDLE_DYLD_H
#include <Foundation/NSBundle.h>

@interface NSBundle (NSBundle_dyld)

+(NSBundle*) gnu_mainBundle;
+(NSBundle*) x_mainBundle;

+(NSBundle*) gnu_bundleForClass: (Class) aClass;
+(NSBundle*) x_bundleForClass: (Class) aClass;

@end


#endif

