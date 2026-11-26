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

#import <AppKit/AppKit.h>
#include <darling-dmg/darling_dmg.h>

// The window that asks whether the user agrees to a disk image's licence agreement, in the languages the image
// has. Return is Agree, Escape is Disagree, and closing the window disagrees.
@interface DIMLicenseWindowController : NSWindowController <NSWindowDelegate>
// The image stays open while the controller is in use. languages: BCP 47 tags, most preferred first.
- (instancetype)initWithImage:(darling_dmg_image *)image title:(NSString *)title
           preferredLanguages:(NSArray<NSString *> *)languages;
// Shows the window; calls reply once, with YES for Agree, and orders the window out.
- (void)runWithReply:(void (^)(BOOL agreed))reply;
@property (nonatomic, readonly) NSPopUpButton *languagePopUp; // hidden when the image has one language
@property (nonatomic, readonly) NSTextView *textView;
@property (nonatomic, readonly) NSTextField *promptField;
@property (nonatomic, readonly) NSButton *agreeButton, *disagreeButton;
@end
