#import <Foundation/Foundation.h>

@class NSArray, NSPasteboard, NSURL;

#if __LP64__
@class QTMovie;
#endif

#if !__LP64__
@interface NSMovie : NSObject <NSCopying, NSCoding> {
#else
@interface NSMovie : NSObject <NSCoding> {
#endif

  @private
    void* _movie;
    NSURL* _url;
    struct {
 int dispose:1;
 int reserved:31;
    } _movieFlags;
    long _reserved1;
    long _reserved2;
}

#if !__LP64__

- (id) initWithMovie:(void* )movie;
- (id) initWithURL:(NSURL*)url byReference:(BOOL)byRef;
- (id) initWithPasteboard:(NSPasteboard*)pasteboard;

- (void* )QTMovie ;
- (NSURL*)URL;

+ (NSArray*) movieUnfilteredFileTypes;
+ (NSArray*) movieUnfilteredPasteboardTypes;
+ (BOOL) canInitWithPasteboard:(NSPasteboard*)pasteboard;

#else

- (id) initWithMovie:(QTMovie*)movie;
- (QTMovie *)QTMovie;

#endif

@end
