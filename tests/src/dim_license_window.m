// Copyright (C) 2026 Lubos Dolezel
// DiskImageMounter's licence agreement window (src/DiskImageMounter/DIMLicenseWindowController.m), in process: the
// language it starts with, what each language shows, the defaults for strings an image lacks, RTF agreements, and
// how Agree, Disagree and closing the window answer.
// Usage: dim_license_window <dir>, where <dir> holds sla-test.dmg, sla-one.dmg, sla-rtf.dmg and sla-nostrings.dmg
// (tests/fixtures, see tests/dmg-license.md). Needs a display. Prints PASS/FAIL lines; exits 1 on a failure.
// Built by tests/disk-image-mounter.sh with the controller's sources, -fobjc-arc -framework AppKit -ldarling-dmg.
#import <AppKit/AppKit.h>
#import "DIMLicenseWindowController.h"

static int failures;

static void expect(BOOL ok, NSString *what)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", [what UTF8String]);
    if (!ok)
        failures++;
}

static NSString *dir;

static darling_dmg_image *openImage(NSString *name)
{
    darling_dmg_image *image = NULL;
    NSString *path = [dir stringByAppendingPathComponent:name];
    if (darling_dmg_open([path fileSystemRepresentation], &image) != 0) {
        expect(NO, [NSString stringWithFormat:@"%@ opens", name]);
        return NULL;
    }
    return image;
}

static DIMLicenseWindowController *controller(darling_dmg_image *image, NSString *title, NSArray *languages)
{
    return [[DIMLicenseWindowController alloc] initWithImage:image title:title preferredLanguages:languages];
}

static void spin(void)
{
    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.2]];
}

static NSArray *titles(NSPopUpButton *popUp)
{
    return [popUp itemTitles];
}

static void choose(DIMLicenseWindowController *c, NSInteger index)
{
    [c.languagePopUp selectItemAtIndex:index];
    [c.languagePopUp sendAction:[c.languagePopUp action] to:[c.languagePopUp target]];
}

static NSString *const defaultPrompt = @"To use the contents of this disk image, you must agree to its license.";

// Shows a controller, does something to it, and returns the replies it gave ("YES"/"NO", one per call).
static NSArray *replies(DIMLicenseWindowController *c, void (^action)(DIMLicenseWindowController *c))
{
    NSMutableArray *got = [NSMutableArray array];
    [c runWithReply:^(BOOL agreed) { [got addObject:agreed ? @"YES" : @"NO"]; }];
    spin();
    action(c);
    for (int i = 0; i < 5; i++)
        spin();
    return got;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc != 2) {
            fprintf(stderr, "usage: %s <fixtures dir>\n", argv[0]);
            return 2;
        }
        [NSApplication sharedApplication];
        dir = [NSString stringWithUTF8String:argv[1]];

        // Three languages, German preferred.
        darling_dmg_image *sla = openImage(@"sla-test.dmg");
        DIMLicenseWindowController *c = controller(sla, @"sla-test.dmg", @[ @"de-DE" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        expect([titles(c.languagePopUp) isEqual:(@[ @"English", @"Deutsch", @"日本語" ])],
               [NSString stringWithFormat:@"the languages, in the image's order (%@)", titles(c.languagePopUp)]);
        expect([[c.languagePopUp titleOfSelectedItem] isEqual:@"Deutsch"] && ![c.languagePopUp isHidden],
               @"de-DE preferred: Deutsch is selected, and the popup shows");
        expect([[c.textView string] hasPrefix:@"SLA TEST AGREEMENT (Deutsch)"],
               [NSString stringWithFormat:@"the German text (%@)", [c.textView string]]);
        expect([[c.agreeButton title] isEqual:@"Akzeptieren"] && [[c.disagreeButton title] isEqual:@"Ablehnen"],
               @"the German button titles");
        expect([[c.promptField stringValue] isEqual:@"Klicken Sie auf „Akzeptieren“, wenn Sie einverstanden sind."],
               [NSString stringWithFormat:@"the German prompt (%@)", [c.promptField stringValue]]);
        expect([[c.window title] isEqual:@"sla-test.dmg"], @"the window's title is the image's name");
        expect(![c.textView isEditable], @"the agreement cannot be edited");
        expect([[c.agreeButton keyEquivalent] isEqual:@"\r"] && [[c.disagreeButton keyEquivalent] isEqual:@"\033"],
               @"Return is Agree and Escape is Disagree");

        // Choosing English shows the English agreement.
        choose(c, 0);
        expect([[c.textView string] hasPrefix:@"SLA TEST AGREEMENT (English)"], @"English chosen: the English text");
        expect([[c.agreeButton title] isEqual:@"Agree"] && [[c.disagreeButton title] isEqual:@"Disagree"],
               @"English chosen: the English button titles");
        expect([[c.promptField stringValue] hasPrefix:@"If you agree with the terms of this license"],
               @"English chosen: the English prompt");
        [c close];

        c = controller(sla, @"sla-test.dmg", @[ @"fr-FR" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        expect([[c.languagePopUp titleOfSelectedItem] isEqual:@"English"], @"fr-FR preferred: the default, English");
        [c close];

        c = controller(sla, @"sla-test.dmg", @[ @"ja" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        expect([[c.languagePopUp titleOfSelectedItem] isEqual:@"日本語"]
                   && [[c.textView string] hasPrefix:@"SLA TEST AGREEMENT (日本語)"],
               @"ja preferred: the Japanese agreement");
        [c close];

        // One language: no choice to offer.
        darling_dmg_image *one = openImage(@"sla-one.dmg");
        c = controller(one, @"sla-one.dmg", @[ @"en" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        expect([c.languagePopUp isHidden], @"one language: the popup is hidden");
        [c close];

        // RTF: attributed text.
        darling_dmg_image *rtf = openImage(@"sla-rtf.dmg");
        c = controller(rtf, @"sla-rtf.dmg", @[ @"en" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        NSTextStorage *storage = [c.textView textStorage];
        expect([[storage string] isEqual:@"SLA TEST AGREEMENT bold\n"],
               [NSString stringWithFormat:@"RTF: the text (%@)", [storage string]]);
        NSRange bold = [[storage string] rangeOfString:@"bold"];
        NSFont *font = bold.location != NSNotFound ? [storage attribute:NSFontAttributeName atIndex:bold.location
                                                          effectiveRange:NULL] : nil;
        expect(([[NSFontManager sharedFontManager] traitsOfFont:font] & NSBoldFontMask) != 0, @"RTF: bold is bold");
        [c close];

        // No strings in the image: our defaults.
        darling_dmg_image *nostrings = openImage(@"sla-nostrings.dmg");
        c = controller(nostrings, @"sla-nostrings.dmg", @[ @"en" ]);
        [c runWithReply:^(BOOL agreed) {}];
        spin();
        expect([[c.agreeButton title] isEqual:@"Agree"] && [[c.disagreeButton title] isEqual:@"Disagree"]
                   && [[c.promptField stringValue] isEqual:defaultPrompt],
               [NSString stringWithFormat:@"no strings: the defaults (%@ / %@ / %@)", [c.agreeButton title],
                   [c.disagreeButton title], [c.promptField stringValue]]);
        [c close];

        // The answers: each controller replies once, then the window goes away.
        __block DIMLicenseWindowController *shown;
        NSArray *got = replies(controller(sla, @"sla-test.dmg", @[ @"en" ]), ^(DIMLicenseWindowController *c) {
            shown = c;
            [c.agreeButton performClick:nil];
            [c.agreeButton performClick:nil];
        });
        expect([got isEqual:@[ @"YES" ]], [NSString stringWithFormat:@"Agree replies YES once (%@)", got]);
        expect(![shown.window isVisible], @"after Agree the window is gone");
        got = replies(controller(sla, @"sla-test.dmg", @[ @"en" ]), ^(DIMLicenseWindowController *c) {
            [c.disagreeButton performClick:nil];
        });
        expect([got isEqual:@[ @"NO" ]], [NSString stringWithFormat:@"Disagree replies NO (%@)", got]);
        got = replies(controller(sla, @"sla-test.dmg", @[ @"en" ]), ^(DIMLicenseWindowController *c) {
            shown = c;
            [c.window performClose:nil];
        });
        expect([got isEqual:@[ @"NO" ]], [NSString stringWithFormat:@"closing the window replies NO (%@)", got]);
        expect(![shown.window isVisible], @"after closing, the window is gone");

        darling_dmg_close(sla);
        darling_dmg_close(one);
        darling_dmg_close(rtf);
        darling_dmg_close(nostrings);
    }
    return failures != 0;
}
