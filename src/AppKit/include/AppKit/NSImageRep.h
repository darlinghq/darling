#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSGraphics.h>

@class NSArray, NSPasteboard, NSGraphicsContext, NSURL;



enum {
    NSImageRepMatchesDevice = 0
};

@interface NSImageRep : NSObject <NSCopying, NSCoding> {

    struct __repFlags {
 unsigned int hasAlpha:1;
 unsigned int isOpaque:1;
        unsigned int cacheParamsComputed:1 __attribute__((deprecated));
        unsigned int cacheAlphaComputed:1 __attribute__((deprecated));
        unsigned int loadState:2;
        unsigned int keepCacheWindow:1 __attribute__((deprecated));
        unsigned int reserved:1;
        unsigned int bitsPerSample:8;
 unsigned int gsaved:16;
    } _repFlags;
    NSString *_colorSpaceName;
    NSSize _size;
    int _pixelsWide;
    int _pixelsHigh;
}



- (BOOL)draw;
- (BOOL)drawAtPoint:(NSPoint)point;
- (BOOL)drawInRect:(NSRect)rect;
- (BOOL)drawInRect:(NSRect)dstSpacePortionRect fromRect:(NSRect)srcSpacePortionRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints;



- (void)setSize:(NSSize)aSize;
- (NSSize)size;
- (void)setAlpha:(BOOL)flag;
- (BOOL)hasAlpha;
- (void)setOpaque:(BOOL)flag;
- (BOOL)isOpaque;
- (void)setColorSpaceName:(NSString *)string;
- (NSString *)colorSpaceName;
- (void)setBitsPerSample:(NSInteger)anInt;
- (NSInteger)bitsPerSample;
- (void)setPixelsWide:(NSInteger)anInt;
- (NSInteger)pixelsWide;
- (void)setPixelsHigh:(NSInteger)anInt;
- (NSInteger)pixelsHigh;






+ (void)registerImageRepClass:(Class)imageRepClass;
+ (void)unregisterImageRepClass:(Class)imageRepClass;
+ (NSArray *)registeredImageRepClasses;
+ (Class)imageRepClassForFileType:(NSString *)type;
+ (Class)imageRepClassForPasteboardType:(NSString *)type;
+ (Class)imageRepClassForType:(NSString *)type;
+ (Class)imageRepClassForData:(NSData *)data;



+ (BOOL)canInitWithData:(NSData *)data;



+ (NSArray *)imageUnfilteredFileTypes;
+ (NSArray *)imageUnfilteredPasteboardTypes;



+ (NSArray *)imageFileTypes;
+ (NSArray *)imagePasteboardTypes;




+ (NSArray *)imageUnfilteredTypes;



+ (NSArray *)imageTypes;




+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;



+ (NSArray *)imageRepsWithContentsOfFile:(NSString *)filename;
+ (id)imageRepWithContentsOfFile:(NSString *)filename;
+ (NSArray *)imageRepsWithContentsOfURL:(NSURL *)url;
+ (id)imageRepWithContentsOfURL:(NSURL *)url;
+ (NSArray *)imageRepsWithPasteboard:(NSPasteboard *)pasteboard;
+ (id)imageRepWithPasteboard:(NSPasteboard *)pasteboard;
- (CGImageRef)CGImageForProposedRect:(NSRect *)proposedDestRect context:(NSGraphicsContext *)context hints:(NSDictionary *)hints;

@end


#define NSImageRepRegistryChangedNotification NSImageRepRegistryDidChangeNotification
EXTERN_C NSString *NSImageRepRegistryDidChangeNotification;
