/*
 Copyright (C) 2026 Lubos Dolezel

 Licensed under the MIT license, the same as the rest of this project.
*/
// Contract (NSTextStorageDelegate): the delegate of a text storage is registered as an observer of
// NSTextStorageWillProcessEditingNotification and NSTextStorageDidProcessEditingNotification for the methods
// it implements, and is unregistered when the delegate is replaced or set to nil. Editing the text posts them
// (the delegate gets each once, with the notification as the argument), and so does posting by hand.
// Prints PASS or FAIL.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(c, ...) do { if (!(c)) { failures++; printf("FAIL line %d: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

@interface Delegate : NSObject <NSTextStorageDelegate> {
@public
    int willCount, didCount;
    id lastWillObject;
    id lastDidObject;
    NSString *lastDidName;
}
@end
@implementation Delegate
- (void) textStorageWillProcessEditing: (NSNotification *) n { willCount++; lastWillObject = [n object]; }
- (void) textStorageDidProcessEditing: (NSNotification *) n { didCount++; lastDidObject = [n object]; [lastDidName release]; lastDidName = [[n name] copy]; }
@end

@interface WillOnly : NSObject {
@public
    int count;
}
@end
@implementation WillOnly
- (void) textStorageWillProcessEditing: (NSNotification *) n { count++; }
@end

@interface Other : NSObject {
@public
    NSMutableArray *log;
}
@end
@implementation Other
- (void) note: (NSNotification *) n { [log addObject: [n name]]; }
@end

int main(void) {
    @autoreleasepool {
        NSTextStorage *storage = [[NSTextStorage alloc] initWithString: @"abc"];
        Delegate *d = [Delegate new];
        Other *other = [Other new];
        other->log = [NSMutableArray new];
        [[NSNotificationCenter defaultCenter] addObserver: other selector: @selector(note:)
                                                     name: NSTextStorageDidProcessEditingNotification object: storage];

        [storage setDelegate: d];
        [storage replaceCharactersInRange: NSMakeRange(0, 1) withString: @"xy"];
        CHECK(d->willCount == 1 && d->didCount == 1, "editing: will %d did %d", d->willCount, d->didCount);
        CHECK(d->lastDidObject == storage && [d->lastDidName isEqualToString: NSTextStorageDidProcessEditingNotification],
              "did notification argument");
        CHECK(d->lastWillObject == storage, "will notification object");
        CHECK([other->log count] == 1, "other observer got %lu", (unsigned long) [other->log count]);

        // by hand
        [[NSNotificationCenter defaultCenter] postNotificationName: NSTextStorageWillProcessEditingNotification object: storage];
        CHECK(d->willCount == 2, "posted by hand: will %d", d->willCount);

        // the same delegate again is not registered twice
        [storage setDelegate: d];
        [storage replaceCharactersInRange: NSMakeRange(0, 1) withString: @"q"];
        CHECK(d->willCount == 3 && d->didCount == 2, "set twice: will %d did %d", d->willCount, d->didCount);

        // a delegate implementing one of the methods gets only that one; the old one gets nothing
        WillOnly *w = [WillOnly new];
        [storage setDelegate: w];
        [storage replaceCharactersInRange: NSMakeRange(0, 1) withString: @"r"];
        CHECK(w->count == 1, "replacement delegate called %d", w->count);
        CHECK(d->willCount == 3 && d->didCount == 2, "replaced delegate called: will %d did %d", d->willCount, d->didCount);

        // nil unregisters
        [storage setDelegate: nil];
        [storage replaceCharactersInRange: NSMakeRange(0, 1) withString: @"s"];
        CHECK(w->count == 1, "delegate called after setDelegate:nil: %d", w->count);
        CHECK([other->log count] == 4, "other observer count %lu", (unsigned long) [other->log count]);

        // another storage's notification does not reach the delegate
        [storage setDelegate: d];
        NSTextStorage *s2 = [[NSTextStorage alloc] initWithString: @"z"];
        [s2 replaceCharactersInRange: NSMakeRange(0, 1) withString: @"y"];
        CHECK(d->willCount == 3, "delegate of another text storage called: %d", d->willCount);

        printf(failures ? "FAIL\n" : "PASS\n");
    }
    return failures != 0;
}
