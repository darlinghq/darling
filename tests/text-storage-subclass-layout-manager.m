// Copyright (C) 2026 Lubos Dolezel
//
// A subclass of NSTextStorage that implements the primitive methods and is
// initialized with -init (through [super init]) takes layout managers like a
// plain text storage: -addLayoutManager: lists the layout manager and makes the
// storage its text storage, and a text view whose container is then added to
// that layout manager reaches the storage. Same results as macOS 26.7.3.
// No display needed. MRC.
#import <AppKit/AppKit.h>
#include <stdio.h>

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); failures++; } } while (0)

@interface WrappingStorage : NSTextStorage {
    NSMutableAttributedString *_contents;
}
@end

@implementation WrappingStorage
- (instancetype)init
{
    if ((self = [super init]))
        _contents = [[NSMutableAttributedString alloc] initWithString:@"abc\ndef"];
    return self;
}
- (void)dealloc
{
    [_contents release];
    [super dealloc];
}
- (NSString *)string { return [_contents string]; }
- (NSDictionary *)attributesAtIndex:(NSUInteger)i effectiveRange:(NSRangePointer)r { return [_contents attributesAtIndex:i effectiveRange:r]; }
- (void)replaceCharactersInRange:(NSRange)r withString:(NSString *)s
{
    [_contents replaceCharactersInRange:r withString:s];
    [self edited:NSTextStorageEditedCharacters range:r changeInLength:(NSInteger)[s length] - (NSInteger)r.length];
}
- (void)setAttributes:(NSDictionary *)a range:(NSRange)r
{
    [_contents setAttributes:a range:r];
    [self edited:NSTextStorageEditedAttributes range:r changeInLength:0];
}
@end

static void checkStorage(NSTextStorage *storage)
{
    NSLayoutManager *lm = [[[NSLayoutManager alloc] init] autorelease];
    [storage addLayoutManager:lm];
    CHECK([lm textStorage] == storage);
    CHECK([[storage layoutManagers] count] == 1 && [[storage layoutManagers] objectAtIndex:0] == lm);

    NSTextContainer *tc = [[[NSTextContainer alloc] initWithContainerSize:NSMakeSize(200, 1e7)] autorelease];
    NSTextView *tv = [[[NSTextView alloc] initWithFrame:NSMakeRect(0, 0, 200, 100) textContainer:tc] autorelease];
    [lm addTextContainer:tc];
    CHECK([tv layoutManager] == lm);
    CHECK([tv textStorage] == storage);
    CHECK([[tv string] isEqual:@"abc\ndef"]);
}

int main(void)
{
    @autoreleasepool {
        [NSApplication sharedApplication];
        checkStorage([[[NSTextStorage alloc] initWithString:@"abc\ndef"] autorelease]);
        checkStorage([[[WrappingStorage alloc] init] autorelease]);
    }
    if (failures == 0)
        printf("PASS text-storage-subclass-layout-manager\n");
    return failures != 0;
}
