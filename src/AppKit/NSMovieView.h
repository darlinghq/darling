#if !__LP64__

#import <AppKit/NSView.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSMovie;

enum {
    NSQTMovieNormalPlayback,
    NSQTMovieLoopingPlayback,
    NSQTMovieLoopingBackAndForthPlayback
};
typedef NSUInteger NSQTMovieLoopMode;

typedef struct __MVFlags {
    unsigned int editable:1;
    NSQTMovieLoopMode loopMode:3;
    unsigned int playsEveryFrame:1;
    unsigned int playsSelectionOnly:1;
    unsigned int controllerVisible:1;
    unsigned int reserved:25;
} _MVFlags;

@interface NSMovieView : NSView <NSUserInterfaceValidations>
{
  @protected
    NSMovie* _fMovie;
    float _fRate;
    float _fVolume;
    _MVFlags _fFlags;

    void* _fAuxData;
    unsigned long _fReserved1;
    unsigned long _fReserved2;
    unsigned long _fReserved3;
}

- (void) setMovie:(NSMovie*)movie;
- (NSMovie*) movie;

- (void* ) movieController ;
- (NSRect) movieRect;



- (void)start:(id)sender;
- (void)stop:(id)sender;
- (BOOL)isPlaying;

- (void)gotoPosterFrame:(id)sender;
- (void)gotoBeginning:(id)sender;
- (void)gotoEnd:(id)sender;
- (void)stepForward:(id)sender;
- (void)stepBack:(id)sender;

- (void)setRate:(float)rate;
- (float)rate;



- (void)setVolume:(float)volume;
- (float)volume;
- (void)setMuted:(BOOL)mute;
- (BOOL)isMuted;



- (void)setLoopMode:(NSQTMovieLoopMode)mode;
- (NSQTMovieLoopMode)loopMode;
- (void)setPlaysSelectionOnly:(BOOL)flag;
- (BOOL)playsSelectionOnly;
- (void)setPlaysEveryFrame:(BOOL)flag;
- (BOOL)playsEveryFrame;



- (void)showController:(BOOL)show adjustingSize:(BOOL)adjustSize;
- (BOOL)isControllerVisible;



- (void)resizeWithMagnification:(CGFloat)magnification;
- (NSSize)sizeForMagnification:(CGFloat)magnification;



- (void)setEditable:(BOOL)editable;
- (BOOL)isEditable;

- (void)cut:(id)sender;
- (void)copy:(id)sender;
- (void)paste:(id)sender;
- (void)delete:(id)sender;
- (void)selectAll:(id)sender;

- (void)clear:(id)sender;

@end

#endif
