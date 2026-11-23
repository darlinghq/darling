// Copyright (C) 2026 Lubos Dolezel
// DiskImageMounter's image opener (src/DiskImageMounter/DIMImageOpener.m), in process, with a recording stand-in for
// its user interface and for opening the volume: what it asks, what it attaches and opens, and the errors it shows.
// Usage: dim_image_opener <dir>, where <dir> holds unmount-test.dmg, sla-test.dmg and bad-blkx.dmg (tests/fixtures;
// bad-blkx.dmg opens, but cannot be attached) and not-an-image.dmg (a text file).
// Nothing else may have their volume ("Unmount Test") attached. Prints PASS/FAIL lines; exits 1 on a failure.
// Built by tests/disk-image-mounter.sh with the opener's sources, -fobjc-arc -framework AppKit -ldarling-dmg.
#import <AppKit/AppKit.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#import "DIMImageOpener.h"

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

@interface Recorder : NSObject <DIMImageOpenerUI>
@property BOOL answer;
@property NSInteger asked;
@property (copy) NSString *message, *informative;
@end

@implementation Recorder
- (void)imageOpener:(DIMImageOpener *)opener askAgreementForImage:(darling_dmg_image *)image
              reply:(void (^)(BOOL agreed))reply
{
    self.asked++;
    BOOL answer = self.answer;
    dispatch_async(dispatch_get_main_queue(), ^{ reply(answer); });
}
- (void)imageOpener:(DIMImageOpener *)opener showError:(NSString *)message
        informative:(NSString *)informative completion:(void (^)(void))done
{
    self.message = message;
    self.informative = informative;
    dispatch_async(dispatch_get_main_queue(), done);
}
@end

static int failingAttach(darling_dmg_image *image, const struct darling_dmg_attach_options *options, char *mountPoint,
                         size_t size)
{
    return EIO;
}

// Runs an opener on one image until it is done; returns what it gave done, and what it opened in *opened.
static NSURL *run(NSString *path, Recorder *ui, DIMAttachFunction attach, NSURL **opened)
{
    DIMImageOpener *opener = [[DIMImageOpener alloc] initWithURL:[NSURL fileURLWithPath:path] ui:ui];
    if (attach != NULL)
        opener.attachFunction = attach;
    __block NSURL *openedVolume = nil, *result = nil;
    __block BOOL finished = NO;
    opener.openVolume = ^(NSURL *volume) { openedVolume = volume; };
    [opener startWithCompletion:^(NSURL *volume) {
        result = volume;
        finished = YES;
    }];
    NSDate *limit = [NSDate dateWithTimeIntervalSinceNow:60];
    while (!finished && [limit timeIntervalSinceNow] > 0)
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    expect(finished, [NSString stringWithFormat:@"%@: the opener finishes", [path lastPathComponent]]);
    *opened = openedVolume;
    return result;
}

// The image's volume when it is attached, else nil; detaches it when asked.
static NSString *attached(NSString *path, BOOL detach)
{
    darling_dmg_image *image = NULL;
    if (darling_dmg_open([path fileSystemRepresentation], &image) != 0)
        return nil;
    char mountPoint[PATH_MAX];
    NSString *result = nil;
    if (darling_dmg_attached_mount_point(image, mountPoint, sizeof(mountPoint)) == 0) {
        result = [NSString stringWithUTF8String:mountPoint];
        if (detach)
            darling_dmg_detach(mountPoint, 0);
    }
    darling_dmg_close(image);
    return result;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <fixtures dir>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        NSString *dir = [NSString stringWithUTF8String:argv[1]];
        NSString *plain = [dir stringByAppendingPathComponent:@"unmount-test.dmg"];
        NSString *sla = [dir stringByAppendingPathComponent:@"sla-test.dmg"];
        NSString *bad = [dir stringByAppendingPathComponent:@"bad-blkx.dmg"];
        NSString *text = [dir stringByAppendingPathComponent:@"not-an-image.dmg"];
        NSURL *volume = [NSURL fileURLWithPath:@"/Volumes/Unmount Test" isDirectory:YES];
        NSURL *opened = nil;

        // An image without a licence agreement: attached and opened, nothing asked.
        Recorder *ui = [Recorder new];
        NSURL *result = run(plain, ui, NULL, &opened);
        expect(ui.asked == 0, @"no licence: nothing is asked");
        expect([opened isEqual:volume], [NSString stringWithFormat:@"no licence: the volume is opened (%@)", opened]);
        expect([result isEqual:volume], @"no licence: done gets the volume");
        expect(attached(plain, YES) != nil, @"no licence: the image is attached");

        // The licence agreement declined: nothing attached or opened.
        ui = [Recorder new];
        ui.answer = NO;
        result = run(sla, ui, NULL, &opened);
        expect(ui.asked == 1, @"declined: the agreement is asked for once");
        expect(result == nil && opened == nil, @"declined: nothing is opened");
        expect(attached(sla, YES) == nil, @"declined: nothing is attached");

        // The licence agreement accepted: attached and opened.
        ui = [Recorder new];
        ui.answer = YES;
        result = run(sla, ui, NULL, &opened);
        expect(ui.asked == 1, @"agreed: the agreement is asked for once");
        expect([opened isEqual:volume] && [result isEqual:volume], @"agreed: the volume is opened");
        // Attached already: opened again without asking.
        ui = [Recorder new];
        result = run(sla, ui, NULL, &opened);
        expect(ui.asked == 0 && [opened isEqual:volume], @"attached already: opened without asking");
        expect(attached(sla, YES) != nil, @"agreed: the image is attached");

        // Not a disk image: an error, nothing opened.
        ui = [Recorder new];
        result = run(text, ui, NULL, &opened);
        expect([ui.message isEqual:@"“not-an-image.dmg” could not be opened."],
              [NSString stringWithFormat:@"not an image: the message (%@)", ui.message]);
        expect([ui.informative isEqual:@"The file is not a disk image this system can read."],
              [NSString stringWithFormat:@"not an image: the explanation (%@)", ui.informative]);
        expect(result == nil && opened == nil, @"not an image: nothing is opened");

        // A damaged image: it opens, but attaching it fails; an error, nothing opened.
        ui = [Recorder new];
        result = run(bad, ui, NULL, &opened);
        expect([ui.message isEqual:@"“bad-blkx.dmg” could not be opened."] && [ui.informative length] > 0,
              [NSString stringWithFormat:@"damaged image: the error (%@ / %@)", ui.message, ui.informative]);
        expect(result == nil && opened == nil, @"damaged image: nothing is opened");

        // Attaching fails: its error, nothing opened or attached.
        ui = [Recorder new];
        result = run(plain, ui, failingAttach, &opened);
        expect([ui.informative hasPrefix:[NSString stringWithUTF8String:strerror(EIO)]],
              [NSString stringWithFormat:@"attach fails: the explanation (%@)", ui.informative]);
        expect(result == nil && opened == nil, @"attach fails: nothing is opened");
        expect(attached(plain, YES) == nil, @"attach fails: nothing is attached");
    }
    return failures != 0;
}
