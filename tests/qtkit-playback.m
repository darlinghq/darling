// Copyright (C) 2026 Darling Developers
//
// QTMovie and QTMovieView playback. The notifications, the order they come in, their timing (posted
// while the call runs), the error domain and code of a movie that cannot be opened, and the defaults of
// the view are what the real QTKit does on macOS 10.14.4 (i386), observed through its public interface.
//
// The fixture is tests/avplayer-fixtures/redblue-h264.mp4 (64x48, 1.2 s: red for the first 0.6 s, blue
// for the rest). Pass its path. With "movie-only" as the second argument the view is skipped; with "window" the view is also put in a window
// and shown, which needs a display (X11 or Wayland). Works for i386 and x86_64:
//   guest-cc.sh -a x86_64 -o qtkit-playback tests/qtkit-playback.m -framework Foundation -framework AppKit -framework QTKit -framework QuartzCore -framework AVFoundation
// Exits 0 if everything passed, 77 if the movie cannot be opened at all (no host FFmpeg for that
// architecture), 1 on failure.
#import <AppKit/AppKit.h>
#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import <QTKit/QTKit.h>
#import <QuartzCore/QuartzCore.h>
#include <objc/runtime.h>
#include <stdio.h>
#include <time.h>

static int failures;

#define CHECK(cond, ...) do { if (!(cond)) { printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); failures++; } } while (0)

static double nowSeconds(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static BOOL spin(double timeout, BOOL (^condition)(void))
{
	double end = nowSeconds() + timeout;
	while (nowSeconds() < end) {
		if (condition && condition())
			return YES;
		[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.01]];
	}
	return condition ? condition() : YES;
}

// Records the notifications of a movie in the order they arrive.
@interface Recorder : NSObject {
@public
	NSMutableArray *names;
	NSMutableArray *rates; // the rate parameter of each rate notification
}
@end

@implementation Recorder
- (instancetype)initWithMovie:(QTMovie *)movie
{
	if ((self = [super init])) {
		names = [[NSMutableArray alloc] init];
		rates = [[NSMutableArray alloc] init];
		for (NSString *name in @[QTMovieDidEndNotification, QTMovieRateDidChangeNotification, QTMovieTimeDidChangeNotification,
		                         QTMovieVolumeDidChangeNotification, QTMovieMutedDidChangeNotification])
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(note:) name:name object:movie];
	}
	return self;
}
- (void)note:(NSNotification *)n
{
	[names addObject:[n name]];
	if ([[n name] isEqualToString:QTMovieRateDidChangeNotification])
		[rates addObject:[[n userInfo] objectForKey:QTMovieRateDidChangeNotificationParameter] ?: [NSNull null]];
}
- (void)clear { [names removeAllObjects]; [rates removeAllObjects]; }
- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[names release];
	[rates release];
	[super dealloc];
}
@end

// The center pixel of an image as 0xRRGGBB, -1 if there is none.
static long centerPixel(NSImage *image)
{
	if (!image)
		return -1;
	NSBitmapImageRep *rep = (NSBitmapImageRep *)[[image representations] firstObject];
	if (![rep isKindOfClass:[NSBitmapImageRep class]])
		rep = [[[NSBitmapImageRep alloc] initWithData:[image TIFFRepresentation]] autorelease];
	if (!rep)
		return -1;
	NSColor *color = [[rep colorAtX:[rep pixelsWide] / 2 y:[rep pixelsHigh] / 2] colorUsingColorSpaceName:NSCalibratedRGBColorSpace];
	return ((long)lround([color redComponent] * 255) << 16) | ((long)lround([color greenComponent] * 255) << 8) | (long)lround([color blueComponent] * 255);
}

static BOOL isRed(long p) { return p >= 0 && ((p >> 16) & 0xff) > 180 && ((p >> 8) & 0xff) < 80 && (p & 0xff) < 80; }
static BOOL isBlue(long p) { return p >= 0 && ((p >> 16) & 0xff) < 80 && ((p >> 8) & 0xff) < 80 && (p & 0xff) > 180; }

static AVPlayerLayer *playerLayerOf(NSView *view)
{
	for (CALayer *layer in [[view layer] sublayers]) {
		if ([layer isKindOfClass:[AVPlayerLayer class]])
			return (AVPlayerLayer *)layer;
	}
	return nil;
}

static void testOpening(NSString *path)
{
	CHECK([QTMovie canInitWithFile:path], "canInitWithFile: of the movie");
	CHECK(![QTMovie canInitWithFile:@"/nonexistent.mov"], "canInitWithFile: of a missing file");
	CHECK([QTMovie canInitWithURL:[NSURL fileURLWithPath:path]], "canInitWithURL:");
	CHECK([[QTMovie movieFileTypes:QTIncludeCommonTypes] containsObject:@"mp4"], "movieFileTypes: has mp4");

	// A movie that cannot be opened: nil and an error of QuickTime's.
	NSError *error = nil;
	QTMovie *missing = [QTMovie movieWithFile:@"/nonexistent.mov" error:&error];
	CHECK(missing == nil, "no movie for a missing file");
	CHECK([[error domain] isEqualToString:NSOSStatusErrorDomain] && [error code] == -2000,
	      "the error is NSOSStatusErrorDomain -2000 (got %s %ld)", [[error domain] UTF8String], (long)[error code]);
	error = nil;
	CHECK([QTMovie movieWithFile:nil error:&error] == nil && error != nil, "no movie for a nil file name");
	CHECK([QTMovie movieWithURL:nil error:NULL] == nil, "no movie for a nil URL, and a NULL error is fine");
	CHECK([QTMovie movieWithData:nil error:NULL] == nil, "no movie for nil data");

	QTMovie *empty = [QTMovie movie];
	CHECK(empty != nil && [empty rate] == 0, "an empty movie");
	CHECK(QTTimeCompare([empty duration], QTZeroTime) == NSOrderedSame, "an empty movie has no duration");
	[empty play]; // must not crash
	CHECK([empty currentFrameImage] == nil, "an empty movie has no frame");
}

static void testMovie(NSString *path)
{
	NSError *error = nil;
	QTMovie *movie = [QTMovie movieWithFile:path error:&error];
	CHECK(movie != nil && error == nil, "the movie opens");
	if (!movie)
		return;

	// The first frame is there when the movie is open.
	long pixel = centerPixel([movie currentFrameImage]);
	CHECK(isRed(pixel), "the first frame is red (got %06lx)", pixel);
	NSImage *image = [movie currentFrameImage];
	CHECK(image != nil && [image size].width == 64 && [image size].height == 48, "the frame image is 64x48");
	CHECK([[image representations] count] == 1, "one representation");

	NSTimeInterval duration = 0;
	CHECK(QTGetTimeInterval([movie duration], &duration) && duration > 1.1 && duration < 1.35, "duration (got %g)", duration);
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedSame, "at the start");
	CHECK([movie rate] == 0, "paused");
	CHECK(NSEqualSizes([movie currentSize], NSMakeSize(64, 48)), "current size");
	CHECK([movie loadState] == QTMovieLoadStateComplete, "load state is complete");
	CHECK([[movie attributeForKey:QTMovieLoadStateAttribute] longValue] == QTMovieLoadStateComplete, "load state attribute");

	NSDictionary *attributes = [movie movieAttributes];
	CHECK([[attributes objectForKey:QTMovieFileNameAttribute] isEqualToString:path], "file name attribute");
	CHECK([[(NSURL *)[attributes objectForKey:QTMovieURLAttribute] lastPathComponent] isEqualToString:[path lastPathComponent]], "URL attribute");
	CHECK([[attributes objectForKey:QTMovieHasVideoAttribute] boolValue] && ![[attributes objectForKey:QTMovieHasAudioAttribute] boolValue],
	      "has video, no audio");
	CHECK([[attributes objectForKey:QTMovieHasDurationAttribute] boolValue], "has duration");
	CHECK(QTEqualTimeRanges(QTMakeTimeRange(QTZeroTime, [movie duration]), QTMakeTimeRange(QTZeroTime, [[attributes objectForKey:QTMovieDurationAttribute] QTTimeValue])) ||
	      QTTimeCompare([movie duration], [[attributes objectForKey:QTMovieDurationAttribute] QTTimeValue]) == NSOrderedSame, "duration attribute");
	CHECK(strcmp([[attributes objectForKey:QTMovieCurrentTimeAttribute] objCType], [[NSValue valueWithQTTime:QTZeroTime] objCType]) == 0,
	      "time attributes are QTTime values");
	CHECK(NSEqualSizes([[attributes objectForKey:QTMovieNaturalSizeAttribute] sizeValue], NSMakeSize(64, 48)), "natural size attribute");
	CHECK([[attributes objectForKey:QTMoviePreferredRateAttribute] floatValue] == 1.0f, "preferred rate");

	// The same movie from a URL, from attributes and from data.
	QTMovie *fromURL = [QTMovie movieWithURL:[NSURL fileURLWithPath:path] error:&error];
	CHECK(fromURL != nil, "movieWithURL:");
	QTMovie *fromAttributes = [QTMovie movieWithAttributes:@{ QTMovieFileNameAttribute: path } error:&error];
	CHECK(fromAttributes != nil, "movieWithAttributes:");
	NSData *data = [NSData dataWithContentsOfFile:path];
	QTMovie *fromData = [QTMovie movieWithData:data error:&error];
	CHECK(fromData != nil && isRed(centerPixel([fromData currentFrameImage])), "movieWithData:");
	// Two movies from the same data are independent.
	QTMovie *again = [QTMovie movieWithData:data error:&error];
	CHECK(again != nil && isRed(centerPixel([again currentFrameImage])), "a second movie from the same data");
	[again setCurrentTime:QTMakeTime(9, 10)];
	CHECK(QTTimeCompare([fromData currentTime], QTZeroTime) == NSOrderedSame, "and moving one does not move the other");

	Recorder *recorder = [[Recorder alloc] initWithMovie:movie];

	// Notifications are posted while the call runs, and the new state is readable at once.
	[movie setCurrentTime:QTMakeTime(9, 10)];
	CHECK([recorder->names isEqualToArray:@[QTMovieTimeDidChangeNotification]], "setCurrentTime: posts QTMovieTimeDidChangeNotification at once (%s)", [[recorder->names description] UTF8String]);
	NSTimeInterval now = 0;
	QTGetTimeInterval([movie currentTime], &now);
	CHECK(fabs(now - 0.9) < 0.01, "the current time after setCurrentTime: is the target (got %g)", now);
	[recorder clear];

	[movie gotoEnd];
	CHECK([recorder->names isEqualToArray:(@[QTMovieRateDidChangeNotification, QTMovieTimeDidChangeNotification])], "gotoEnd posts rate then time notifications (%s)", [[recorder->names description] UTF8String]);
	CHECK(QTTimeCompare([movie currentTime], [movie duration]) == NSOrderedSame, "gotoEnd goes to the duration");
	[recorder clear];
	[movie gotoBeginning];
	CHECK([recorder->names isEqualToArray:@[QTMovieTimeDidChangeNotification]], "gotoBeginning posts a time notification (%s)", [[recorder->names description] UTF8String]);
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedSame, "gotoBeginning goes to the start");
	[recorder clear];

	// frameImageAtTime: moves to the time and gives its frame.
	CHECK(isBlue(centerPixel([movie frameImageAtTime:QTMakeTime(10, 10)])), "the frame at 1.0 s is blue");
	CHECK(isRed(centerPixel([movie frameImageAtTime:QTMakeTime(3, 10)])), "the frame at 0.3 s is red");

	// Stepping stops playback and moves by one frame (0.1 s here).
	[movie gotoBeginning];
	[recorder clear];
	[movie stepForward];
	QTGetTimeInterval([movie currentTime], &now);
	CHECK(fabs(now - 0.1) < 0.011, "stepForward moves one frame (at %g)", now);
	CHECK([recorder->names containsObject:QTMovieTimeDidChangeNotification] && [recorder->rates isEqualToArray:@[@0]], "stepForward posts a rate 0 and a time notification (%s)", [[recorder->names description] UTF8String]);
	[movie stepBackward];
	QTGetTimeInterval([movie currentTime], &now);
	CHECK(fabs(now) < 0.011, "stepBackward moves back (at %g)", now);
	[movie stepBackward];
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedSame, "stepBackward stops at the start");

	// Volume and muting.
	[recorder clear];
	[movie setVolume:0.5f];
	CHECK([movie volume] == 0.5f && [recorder->names isEqualToArray:@[QTMovieVolumeDidChangeNotification]], "setVolume: posts a notification");
	[movie setMuted:YES];
	CHECK([movie muted], "muted");
	[movie setAttribute:@YES forKey:QTMovieMutedAttribute];
	[movie setAttribute:[NSNumber numberWithFloat:0.25f] forKey:QTMovieVolumeAttribute];
	CHECK([movie volume] == 0.25f, "setAttribute: for the volume");
	[movie setAttribute:@"x" forKey:@"QTTestCustomAttribute"];
	CHECK([movie attributeForKey:@"QTTestCustomAttribute"] == nil, "attributes the movie does not know are not kept");
	[movie setMuted:NO];

	// Playing to the end: rate 1, then the rate drops to 0 and the end follows, in that order.
	[movie gotoBeginning];
	[recorder clear];
	double start = nowSeconds();
	[movie play];
	CHECK([movie rate] == 1.0f && [recorder->names isEqualToArray:@[QTMovieRateDidChangeNotification]] && [recorder->rates isEqualToArray:@[@1]],
	      "play posts rate 1 at once");
	CHECK(spin(4.0, ^BOOL { return [recorder->names containsObject:QTMovieDidEndNotification]; }), "QTMovieDidEndNotification arrives");
	double elapsed = nowSeconds() - start;
	CHECK(elapsed > 1.0 && elapsed < 2.2, "playing takes about the duration (took %g s)", elapsed);
	NSArray *expected = @[QTMovieRateDidChangeNotification, QTMovieRateDidChangeNotification, QTMovieDidEndNotification];
	CHECK([recorder->names isEqualToArray:expected], "rate 1, rate 0, end (got %s)", [[recorder->names description] UTF8String]);
	CHECK([recorder->rates isEqualToArray:(@[@1, @0])], "the rates are 1 then 0");
	CHECK([movie rate] == 0, "the rate is 0 at the end");
	CHECK(QTTimeCompare([movie currentTime], [movie duration]) == NSOrderedSame, "the time is the duration at the end");

	// Playing from the end starts over from the beginning.
	[recorder clear];
	[movie play];
	CHECK([recorder->names isEqualToArray:(@[QTMovieTimeDidChangeNotification, QTMovieRateDidChangeNotification])], "play at the end goes to the start first (%s)", [[recorder->names description] UTF8String]);
	QTGetTimeInterval([movie currentTime], &now);
	CHECK(now < 0.3, "playing again from the start (at %g)", now);
	[movie stop];
	CHECK([movie rate] == 0 && [[recorder->names lastObject] isEqualToString:QTMovieRateDidChangeNotification] && [[recorder->rates lastObject] isEqual:@0], "stop posts rate 0");

	[recorder release];
}

static void testView(NSString *path, BOOL withWindow)
{
	[NSApplication sharedApplication];
	QTMovie *movie = [QTMovie movieWithFile:path error:NULL];
	QTMovieView *view = [[[QTMovieView alloc] initWithFrame:NSMakeRect(0, 0, 320, 240)] autorelease];

	// The defaults of a new view.
	CHECK([view movie] == nil, "no movie");
	CHECK(![view preservesAspectRatio], "does not preserve the aspect ratio by default");
	CHECK([view isControllerVisible], "the controller is visible by default");
	CHECK(![view isEditable], "not editable");
	NSColor *fill = [[view fillColor] colorUsingColorSpaceName:NSCalibratedWhiteColorSpace];
	CHECK(fill != nil && [fill whiteComponent] == 0 && [fill alphaComponent] == 1, "the fill color is black by default");
	CHECK(NSEqualRects([view movieBounds], NSMakeRect(0, 16, 320, 224)), "the movie bounds leave the controller strip (got %s)", [NSStringFromRect([view movieBounds]) UTF8String]);
	[view setControllerVisible:NO];
	CHECK(NSEqualRects([view movieBounds], NSMakeRect(0, 0, 320, 240)), "without the controller the movie bounds are the bounds");
	[view setControllerVisible:YES];

	// Hosting the movie.
	[view setMovie:movie];
	CHECK([view movie] == movie, "the view has the movie");
	AVPlayerLayer *layer = playerLayerOf(view);
	CHECK(layer != nil, "the view hosts an AVPlayerLayer");
	CHECK([layer player] == [movie performSelector:@selector(_player)], "the layer shows the movie's player");
	CHECK(CGRectEqualToRect([layer frame], CGRectMake(0, 16, 320, 224)), "the layer fills the movie bounds");
	CHECK([[layer videoGravity] isEqualToString:AVLayerVideoGravityResize], "stretches by default");
	CHECK(spin(3.0, ^BOOL { return [layer contents] != nil; }), "the layer has the first frame");

	[view setPreservesAspectRatio:YES];
	CHECK([[layer videoGravity] isEqualToString:AVLayerVideoGravityResizeAspect], "preserving the aspect ratio sets the gravity");
	[view setPreservesAspectRatio:NO];
	CHECK([[layer videoGravity] isEqualToString:AVLayerVideoGravityResize], "and back");

	[view setFrameSize:NSMakeSize(400, 300)];
	CHECK(CGRectEqualToRect([layer frame], CGRectMake(0, 16, 400, 284)), "the layer follows the view size (got %s)", [NSStringFromRect(NSRectFromCGRect([layer frame])) UTF8String]);
	[view setControllerVisible:NO];
	CHECK(CGRectEqualToRect([layer frame], CGRectMake(0, 0, 400, 300)), "the layer follows the controller");
	[view setControllerVisible:YES];

	// Actions.
	[view play:nil];
	CHECK([movie rate] == 1.0f, "play: plays the movie");
	[view pause:nil];
	CHECK([movie rate] == 0, "pause: pauses it");
	[view gotoEnd:nil];
	CHECK(QTTimeCompare([movie currentTime], [movie duration]) == NSOrderedSame, "gotoEnd:");
	[view gotoBeginning:nil];
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedSame, "gotoBeginning:");
	[view stepForward:nil];
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedDescending, "stepForward:");
	[view gotoPosterTime:nil];
	CHECK(QTTimeCompare([movie currentTime], QTZeroTime) == NSOrderedSame, "gotoPosterTime:");

	[view setFillColor:[NSColor redColor]];
	CHECK([[view fillColor] isEqual:[NSColor redColor]], "fill color");
	[view setEditable:YES];
	CHECK([view isEditable], "editable");

	// The layer shows the frames while playing.
	id before = [layer contents];
	[movie play];
	CHECK(spin(3.0, ^BOOL { return [layer contents] != before; }), "the layer's contents change while playing");
	[movie stop];

	if (withWindow) {
		NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 320, 240) styleMask:NSWindowStyleMaskTitled backing:NSBackingStoreBuffered defer:NO];
		[window setContentView:view];
		[window makeKeyAndOrderFront:nil];
		[movie gotoBeginning];
		[movie play];
		spin(1.0, nil);
		CHECK([window isVisible], "the window is visible");
		CHECK([layer contents] != nil, "the layer has contents in the window");
		[window orderOut:nil];
		[window setContentView:nil];
		[window close];
	}

	// Removing the movie takes the layer away.
	[view setMovie:nil];
	CHECK([view movie] == nil && playerLayerOf(view) == nil, "setMovie:nil removes the layer");
	[view setMovie:movie];
	QTMovie *second = [QTMovie movieWithFile:path error:NULL];
	[view setMovie:second];
	CHECK(playerLayerOf(view) != nil && [playerLayerOf(view) player] == [second performSelector:@selector(_player)], "replacing the movie replaces the player");
	CHECK([[[view layer] sublayers] count] == 1, "and leaves one layer");
	[second play];
}

static void testLifetime(NSString *path)
{
	// Movies and views go away while playing.
	for (int i = 0; i < 15; i++) {
		@autoreleasepool {
			QTMovie *movie = [[QTMovie alloc] initWithFile:path error:NULL];
			QTMovieView *view = [[QTMovieView alloc] initWithFrame:NSMakeRect(0, 0, 100, 100)];
			[view setMovie:movie];
			[movie play];
			if (i % 3 == 0)
				[movie setCurrentTime:QTMakeTime(5, 10)];
			if (i % 2 == 0)
				[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.03]];
			if (i % 2)
				[movie release];
			[view release];
			if (i % 2 == 0)
				[movie release];
		}
	}
	[[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
	printf("lifetime ok\n");
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) {
		printf("usage: qtkit-playback fixture.mp4 [window]\n");
		return 1;
	}
	@autoreleasepool {
		NSString *path = [NSString stringWithUTF8String:argv[1]];
		if (![QTMovie canInitWithFile:path]) {
			printf("SKIP: %s cannot be opened (no host FFmpeg for this architecture?)\n", argv[1]);
			return 77;
		}
		printf("== opening\n");
		testOpening(path);
		printf("== movie\n");
		testMovie(path);
		// "movie-only" skips the view, whose layer hosting is how Darling shows the movie (the real
		// QTKit renders differently); the rest runs unchanged against the real QTKit.
		BOOL movieOnly = argc > 2 && !strcmp(argv[2], "movie-only");
		if (!movieOnly) {
			printf("== view\n");
			testView(path, argc > 2 && !strcmp(argv[2], "window"));
		}
		printf("== lifetime\n");
		testLifetime(path);
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("PASS: QTMovie and QTMovieView playback\n");
	return 0;
}
