/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import "DIMImageOpener.h"
#import <AppKit/AppKit.h>
#include <errno.h>
#include <limits.h>
#include <string.h>

@implementation DIMImageOpener {
    __weak id<DIMImageOpenerUI> _ui;
    darling_dmg_image *_image;
    void (^_done)(NSURL *volume);
}

- (instancetype)initWithURL:(NSURL *)imageURL ui:(id<DIMImageOpenerUI>)ui
{
    self = [super init];
    if (self != nil) {
        _imageURL = imageURL;
        _ui = ui;
        _attachFunction = darling_dmg_attach;
        _openVolume = ^(NSURL *volume) {
            [[NSWorkspace sharedWorkspace] openURL:volume];
        };
    }
    return self;
}

- (void)dealloc
{
    if (_image != NULL)
        darling_dmg_close(_image);
}

- (void)startWithCompletion:(void (^)(NSURL *volume))done
{
    _done = [done copy];
    int err = darling_dmg_open([_imageURL fileSystemRepresentation], &_image);
    if (err != 0) {
        [self failWithError:err];
        return;
    }

    // An image that is attached already is not asked about or attached again: its volume is opened.
    char mountPoint[PATH_MAX];
    if (darling_dmg_attached_mount_point(_image, mountPoint, sizeof(mountPoint)) == 0) {
        [self openMountPoint:[NSString stringWithUTF8String:mountPoint]];
        return;
    }

    if (darling_dmg_license_count(_image) > 0) {
        [_ui imageOpener:self askAgreementForImage:_image reply:^(BOOL agreed) {
            if (agreed)
                [self attach];
            else
                [self finishWithVolume:nil];
        }];
        return;
    }
    [self attach];
}

// Attaching waits for the volume to be mounted, so it runs off the main thread.
- (void)attach
{
    DIMAttachFunction attach = _attachFunction;
    darling_dmg_image *image = _image;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        char mountPoint[PATH_MAX];
        int err = attach(image, NULL, mountPoint, sizeof(mountPoint));
        NSString *point = err == 0 ? [NSString stringWithUTF8String:mountPoint] : nil;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (err != 0)
                [self failWithError:err];
            else
                [self openMountPoint:point];
        });
    });
}

- (void)openMountPoint:(NSString *)mountPoint
{
    NSURL *volume = [NSURL fileURLWithPath:mountPoint isDirectory:YES];
    if (_openVolume != nil)
        _openVolume(volume);
    [self finishWithVolume:volume];
}

- (void)failWithError:(int)err
{
    NSString *message = [NSString stringWithFormat:@"“%@” could not be opened.", [_imageURL lastPathComponent]];
    NSString *informative;
    if (err == EFTYPE) {
        informative = @"The file is not a disk image this system can read.";
    } else {
        informative = [NSString stringWithUTF8String:strerror(err)];
        const char *messages = _image != NULL ? darling_dmg_attach_messages(_image) : NULL;
        if (messages != NULL && messages[0] != '\0')
            informative = [NSString stringWithFormat:@"%@\n%@", informative, [NSString stringWithUTF8String:messages]];
    }
    id<DIMImageOpenerUI> ui = _ui;
    if (ui == nil) {
        [self finishWithVolume:nil];
        return;
    }
    [ui imageOpener:self showError:message informative:informative completion:^{
        [self finishWithVolume:nil];
    }];
}

- (void)finishWithVolume:(NSURL *)volume
{
    if (_image != NULL) {
        darling_dmg_close(_image);
        _image = NULL;
    }
    void (^done)(NSURL *) = _done;
    _done = nil;
    if (done != nil)
        done(volume);
}

@end
