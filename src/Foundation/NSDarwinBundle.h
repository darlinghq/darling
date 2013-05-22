#ifndef NSDARWINBUNDLE_H
#define NSDARWINBUNDLE_H
#import <Foundation/NSBundle.h>

@interface NSDarwinBundle : NSBundle

@end

// NSBundle private implementation details
@interface NSBundle (Private)
+ (long) _loadModuleWithFilename: (NSString*)filename
                     errorStream: (FILE*)errorStream
                    loadCallback: (void (*)(Class, struct objc_category *))loadCallback
                          header: (void**)header
                   debugFilename: (NSString*)debugFilename;
@end

@interface NSBundle (NSBundle_Darling)

+(NSBundle*) x_mainBundle;

@end

#endif

