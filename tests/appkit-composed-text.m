/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Licensed under the MIT license, the same as the rest of this project.
 */
// A key sequence that an input method (or Xlib's own compose handling) turns into text is delivered by
// AppKit as one key down event whose characters are that text, and the first responder's
// interpretKeyEvents: hands it to the first responder as inserted text (insertText: or, for a text input
// client, insertText:replacementRange:).
// The sequences are composed by the window system, so this needs a session and keys fed to it;
// see native-x11-locale.sh, which runs it as the program of its cases. Build without ARC.
//
//   appkit-composed-text EXPECTED [SECONDS]
//
// prints READY once the window is key, then PASS when a key down with the characters EXPECTED (UTF-8)
// arrived and the text EXPECTED was inserted into the first responder, FAIL otherwise.
#import <AppKit/AppKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

@interface ComposeView : NSView <NSTextInputClient> {
@public
    NSString *expected;
    BOOL sawCharacters;
    BOOL sawInsert;
}
@end

@implementation ComposeView
- (BOOL) acceptsFirstResponder { return YES; }
- (void) keyDown: (NSEvent *) event {
    NSString *characters = [event characters];
    printf("key down: characters \"%s\"\n", [characters UTF8String]);
    if ([characters isEqualToString: expected])
        sawCharacters = YES;
    [self interpretKeyEvents: [NSArray arrayWithObject: event]];
}
- (void) insertText: (id) string {
    [self insertText: string replacementRange: NSMakeRange(NSNotFound, 0)];
}
- (void) insertText: (id) string replacementRange: (NSRange) range {
    NSString *plain = [string isKindOfClass: [NSAttributedString class]] ? [string string] : string;
    printf("insertText: \"%s\"\n", [plain UTF8String]);
    if ([plain isEqualToString: expected])
        sawInsert = YES;
}
- (void) setMarkedText: (id) string selectedRange: (NSRange) selectedRange replacementRange: (NSRange) replacementRange {}
- (void) unmarkText {}
- (NSRange) selectedRange { return NSMakeRange(NSNotFound, 0); }
- (NSRange) markedRange { return NSMakeRange(NSNotFound, 0); }
- (BOOL) hasMarkedText { return NO; }
- (NSAttributedString *) attributedSubstringForProposedRange: (NSRange) range actualRange: (NSRangePointer) actualRange { return nil; }
- (NSArray *) validAttributesForMarkedText { return [NSArray array]; }
- (NSRect) firstRectForCharacterRange: (NSRange) range actualRange: (NSRangePointer) actualRange { return NSMakeRect(0, 0, 1, 16); }
- (NSUInteger) characterIndexForPoint: (NSPoint) point { return NSNotFound; }
@end

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s EXPECTED [SECONDS]\n", argv[0]);
        return 2;
    }
    @autoreleasepool {
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];
        double seconds = argc > 2 ? atof(argv[2]) : 15;

        NSWindow *window = [[NSWindow alloc] initWithContentRect: NSMakeRect(100, 100, 300, 200)
                                                       styleMask: NSWindowStyleMaskTitled
                                                         backing: NSBackingStoreBuffered
                                                           defer: NO];
        ComposeView *view = [[ComposeView alloc] initWithFrame: NSMakeRect(0, 0, 300, 200)];
        view->expected = [[NSString alloc] initWithUTF8String: argv[1]];
        [window setContentView: view];
        [NSApp activateIgnoringOtherApps: YES];
        [window makeKeyAndOrderFront: nil];
        [window makeFirstResponder: view];
        [[NSRunLoop currentRunLoop] runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 1.0]];
        printf("READY\n");
        fflush(stdout);

        NSDate *end = [NSDate dateWithTimeIntervalSinceNow: seconds];
        while (!(view->sawCharacters && view->sawInsert) && [end timeIntervalSinceNow] > 0) {
            NSEvent *event = [NSApp nextEventMatchingMask: NSEventMaskAny
                                                untilDate: [NSDate dateWithTimeIntervalSinceNow: 0.1]
                                                   inMode: NSDefaultRunLoopMode
                                                  dequeue: YES];
            if (event != nil)
                [NSApp sendEvent: event];
            fflush(stdout);
        }
        BOOL passed = view->sawCharacters && view->sawInsert;
        printf(passed ? "PASS\n" : "FAIL: the key down characters %s and the inserted text %s\n",
               view->sawCharacters ? "matched" : "did not match", view->sawInsert ? "matched" : "did not match");
        return passed ? 0 : 1;
    }
}
