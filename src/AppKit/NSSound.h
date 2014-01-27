#import <AppKit/AppKitDefines.h>
#import <AppKit/NSPasteboard.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDate.h>

@class NSData, NSURL;
@protocol NSSoundDelegate;

EXTERN_C NSString * const NSSoundPboardType;

@interface NSSound : NSObject <NSCopying, NSCoding, NSPasteboardReading, NSPasteboardWriting>
{
@private
    id _delegate;
    id _info;
    id _reserved[6];
    uint32_t _sFlags;
}



+ (id)soundNamed:(NSString *)name;



- (id)initWithContentsOfURL:(NSURL *)url byReference:(BOOL)byRef;


- (id)initWithContentsOfFile:(NSString *)path byReference:(BOOL)byRef;



- (id)initWithData:(NSData *)data;

- (BOOL)setName:(NSString *)string;
- (NSString *)name;


+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;
+ (NSArray*)soundUnfilteredTypes;
- (id)initWithPasteboard:(NSPasteboard *)pasteboard;
- (void)writeToPasteboard:(NSPasteboard *)pasteboard;


- (BOOL)play;
- (BOOL)pause;
- (BOOL)resume;
- (BOOL)stop;
- (BOOL)isPlaying;

- (id <NSSoundDelegate>)delegate;
- (void)setDelegate:(id <NSSoundDelegate>)aDelegate;




- (NSTimeInterval)duration;



- (void)setVolume:(float)volume;
- (float)volume;



- (NSTimeInterval)currentTime;



- (void)setCurrentTime:(NSTimeInterval)seconds;



- (void)setLoops:(BOOL)val;


- (BOOL)loops;



- (void)setPlaybackDeviceIdentifier:(NSString *)deviceUID;



- (NSString *)playbackDeviceIdentifier;



- (void)setChannelMapping:(NSArray *)channelMapping;



- (NSArray *)channelMapping;


@end

@interface NSSound (NSDeprecated)



+ (NSArray *)soundUnfilteredFileTypes;
+ (NSArray *)soundUnfilteredPasteboardTypes;

@end

@protocol NSSoundDelegate <NSObject>
@optional

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool;

@end

@interface NSBundle (NSBundleSoundExtensions)



- (NSString *)pathForSoundResource:(NSString *)name;

@end
