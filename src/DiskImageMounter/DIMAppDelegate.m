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

#import "DIMAppDelegate.h"
#import "DIMLicenseWindowController.h"

@implementation DIMAppDelegate {
    NSMutableArray<NSURL *> *_pending;
    DIMImageOpener *_current;
    DIMLicenseWindowController *_licenseWindow;
    BOOL _launched;
}

- (instancetype)init
{
    self = [super init];
    if (self != nil)
        _pending = [NSMutableArray array];
    return self;
}

- (void)application:(NSApplication *)application openURLs:(NSArray<NSURL *> *)urls
{
    for (NSURL *url in urls) {
        if (![url isFileURL])
            continue;
        NSURL *image = [url URLByStandardizingPath];
        // An image that is being opened, or waits to be, is not opened twice.
        if ([_current.imageURL isEqual:image] || [_pending containsObject:image])
            continue;
        [_pending addObject:image];
    }
    [self openNext];
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    _launched = YES;
    // Documents of the launch arrive around now; without any, there is nothing to do.
    dispatch_async(dispatch_get_main_queue(), ^{
        [self openNext];
    });
}

- (void)openNext
{
    if (_current != nil)
        return;
    if ([_pending count] == 0) {
        if (_launched)
            [NSApp terminate:nil];
        return;
    }
    NSURL *image = [_pending objectAtIndex:0];
    [_pending removeObjectAtIndex:0];
    _current = [[DIMImageOpener alloc] initWithURL:image ui:self];
    [_current startWithCompletion:^(NSURL *volume) {
        self->_current = nil;
        [self openNext];
    }];
}

#pragma mark DIMImageOpenerUI

- (void)imageOpener:(DIMImageOpener *)opener askAgreementForImage:(darling_dmg_image *)image
              reply:(void (^)(BOOL agreed))reply
{
    DIMLicenseWindowController *controller =
        [[DIMLicenseWindowController alloc] initWithImage:image
                                                    title:[opener.imageURL lastPathComponent]
                                       preferredLanguages:[NSLocale preferredLanguages]];
    // The controller lives until it has answered.
    _licenseWindow = controller;
    [controller runWithReply:^(BOOL agreed) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if (self->_licenseWindow == controller)
                self->_licenseWindow = nil;
            reply(agreed);
        });
    }];
}

- (void)imageOpener:(DIMImageOpener *)opener showError:(NSString *)message
        informative:(NSString *)informative completion:(void (^)(void))done
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.alertStyle = NSAlertStyleWarning;
    alert.messageText = message;
    alert.informativeText = informative;
    [NSApp activateIgnoringOtherApps:YES];
    [alert runModal];
    dispatch_async(dispatch_get_main_queue(), done);
}

@end
