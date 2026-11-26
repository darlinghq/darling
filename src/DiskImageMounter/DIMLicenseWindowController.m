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

#import "DIMLicenseWindowController.h"

// For strings an image does not provide.
static NSString *const kDefaultAgree = @"Agree";
static NSString *const kDefaultDisagree = @"Disagree";
static NSString *const kDefaultPrompt = @"To use the contents of this disk image, you must agree to its license.";

static NSString *stringOr(const char *s, NSString *fallback)
{
    NSString *string = s != NULL ? [NSString stringWithUTF8String:s] : nil;
    return [string length] > 0 ? string : fallback;
}

@implementation DIMLicenseWindowController {
    darling_dmg_image *_image;
    void (^_reply)(BOOL agreed);
}

- (instancetype)initWithImage:(darling_dmg_image *)image title:(NSString *)title
           preferredLanguages:(NSArray<NSString *> *)languages
{
    NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 560, 440)
                                                   styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                                                     backing:NSBackingStoreBuffered
                                                       defer:YES];
    self = [super initWithWindow:window];
    if (self == nil)
        return nil;
    _image = image;
    window.title = title;
    window.releasedWhenClosed = NO;
    window.delegate = self;
    [self buildContent];

    size_t count = darling_dmg_license_count(image);
    for (size_t i = 0; i < count; i++) {
        const struct darling_dmg_license *license = darling_dmg_license_at(image, i);
        NSString *name = stringOr(license->language_name, stringOr(license->language, @"?"));
        // Through the menu, so that two languages with the same name stay two items.
        [[_languagePopUp menu] addItemWithTitle:name action:NULL keyEquivalent:@""];
    }
    _languagePopUp.hidden = count <= 1;

    const char *preferred[[languages count] + 1];
    NSUInteger n = 0;
    for (NSString *language in languages)
        preferred[n++] = [language UTF8String];
    const struct darling_dmg_license *initial = darling_dmg_license_for_languages(image, preferred, n);
    for (size_t i = 0; i < count; i++) {
        if (darling_dmg_license_at(image, i) == initial)
            [_languagePopUp selectItemAtIndex:(NSInteger)i];
    }
    [self showLicense:initial];
    return self;
}

- (void)buildContent
{
    NSView *content = self.window.contentView;
    const CGFloat margin = 20, width = 560 - 2 * margin;

    _languagePopUp = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(margin, 396, 220, 26) pullsDown:NO];
    _languagePopUp.target = self;
    _languagePopUp.action = @selector(languageChosen:);
    _languagePopUp.autoresizingMask = NSViewMinYMargin;
    [content addSubview:_languagePopUp];

    NSScrollView *scroll = [[NSScrollView alloc] initWithFrame:NSMakeRect(margin, 120, width, 266)];
    scroll.hasVerticalScroller = YES;
    scroll.borderType = NSBezelBorder;
    scroll.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    NSSize size = [scroll contentSize];
    _textView = [[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
    _textView.editable = NO;
    _textView.selectable = YES;
    _textView.richText = YES;
    _textView.verticallyResizable = YES;
    _textView.horizontallyResizable = NO;
    _textView.autoresizingMask = NSViewWidthSizable;
    _textView.textContainer.widthTracksTextView = YES;
    _textView.textContainer.containerSize = NSMakeSize(size.width, CGFLOAT_MAX);
    scroll.documentView = _textView;
    [content addSubview:scroll];

    _promptField = [[NSTextField alloc] initWithFrame:NSMakeRect(margin, 60, width, 48)];
    _promptField.editable = NO;
    _promptField.selectable = NO;
    _promptField.bezeled = NO;
    _promptField.drawsBackground = NO;
    _promptField.autoresizingMask = NSViewWidthSizable | NSViewMaxYMargin;
    [[_promptField cell] setWraps:YES];
    [content addSubview:_promptField];

    _agreeButton = [[NSButton alloc] initWithFrame:NSMakeRect(560 - margin - 120, 16, 120, 32)];
    _agreeButton.bezelStyle = NSBezelStyleRounded;
    _agreeButton.keyEquivalent = @"\r";
    _agreeButton.target = self;
    _agreeButton.action = @selector(agree:);
    _agreeButton.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
    [content addSubview:_agreeButton];

    _disagreeButton = [[NSButton alloc] initWithFrame:NSMakeRect(560 - margin - 250, 16, 120, 32)];
    _disagreeButton.bezelStyle = NSBezelStyleRounded;
    _disagreeButton.keyEquivalent = @"\033";
    _disagreeButton.target = self;
    _disagreeButton.action = @selector(disagree:);
    _disagreeButton.autoresizingMask = NSViewMinXMargin | NSViewMaxYMargin;
    [content addSubview:_disagreeButton];
}

- (void)showLicense:(const struct darling_dmg_license *)license
{
    NSAttributedString *text = nil;
    if (license != NULL && license->text != NULL) {
        if (license->is_rtf) {
            NSData *rtf = [NSData dataWithBytes:license->text length:strlen(license->text)];
            text = [[NSAttributedString alloc] initWithRTF:rtf documentAttributes:NULL];
        }
        if (text == nil) {
            NSDictionary *attributes = @{ NSFontAttributeName : [NSFont systemFontOfSize:0] };
            text = [[NSAttributedString alloc] initWithString:stringOr(license->text, @"") attributes:attributes];
        }
    }
    [[_textView textStorage] setAttributedString:text ?: [[NSAttributedString alloc] init]];
    [_textView scrollRangeToVisible:NSMakeRange(0, 0)];
    _promptField.stringValue = stringOr(license != NULL ? license->message : NULL, kDefaultPrompt);
    _agreeButton.title = stringOr(license != NULL ? license->agree : NULL, kDefaultAgree);
    _disagreeButton.title = stringOr(license != NULL ? license->disagree : NULL, kDefaultDisagree);
}

- (void)languageChosen:(id)sender
{
    NSInteger index = [_languagePopUp indexOfSelectedItem];
    if (index >= 0)
        [self showLicense:darling_dmg_license_at(_image, (size_t)index)];
}

- (void)runWithReply:(void (^)(BOOL agreed))reply
{
    _reply = [reply copy];
    [self.window center];
    [NSApp activateIgnoringOtherApps:YES];
    [self showWindow:nil];
    [self.window makeKeyAndOrderFront:nil];
}

// Replies once; later answers are ignored.
- (void)answer:(BOOL)agreed
{
    void (^reply)(BOOL) = _reply;
    _reply = nil;
    if (reply != nil)
        reply(agreed);
}

- (void)agree:(id)sender
{
    [self.window orderOut:nil];
    [self answer:YES];
}

- (void)disagree:(id)sender
{
    [self.window orderOut:nil];
    [self answer:NO];
}

- (BOOL)windowShouldClose:(NSWindow *)sender
{
    [self answer:NO];
    return YES;
}

@end
