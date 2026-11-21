// CFLAGS: -framework Foundation -framework AppKit
// Copyright (C) 2026 Lubos Dolezel
//
// NSTextStorage's change-tracking API is declared with NSInteger and
// NSUInteger: -edited:range:changeInLength:, -changeInLength and -editedMask.
// A subclass overriding them, or a caller passing a delta wider than 32 bits,
// has to see the full register width, and NSLayoutManager's edit notification
// carries the same delta. The calls below go through objc_msgSend with
// explicit NSInteger signatures so the result does not depend on which
// version of the headers the test was compiled against.

#import <AppKit/AppKit.h>
#import <objc/message.h>
#import <objc/runtime.h>
#include <stdio.h>
#include <string.h>

// The overrides spell the mask as NSUInteger, which every SDK version accepts;
// newer headers name it NSTextStorageEditActions.
#pragma clang diagnostic ignored "-Wmismatched-parameter-types"
#pragma clang diagnostic ignored "-Wmismatched-return-types"
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

static int failures = 0;
#define expect(cond) do { int ok_ = (cond) ? 1 : 0; printf("%s: %s\n", ok_ ? "PASS" : "FAIL", #cond); if (!ok_) failures++; } while (0)

// A delta that does not fit in 32 bits where NSInteger is 64-bit, else the
// widest one that does.
static const NSInteger kBig = sizeof(NSInteger) == 8 ? (NSInteger) 0x1234567890LL : (NSInteger) 0x7FFFFFF0L;
#define MASK_BOTH ((NSUInteger) 3)

typedef void (*EditedFn)(id, SEL, NSUInteger, NSRange, NSInteger);
typedef NSInteger (*DeltaFn)(id, SEL);
typedef NSUInteger (*MaskFn)(id, SEL);

static void sendEdited(id storage, NSUInteger mask, NSRange range, NSInteger delta) {
	((EditedFn) objc_msgSend)(storage, @selector(edited:range:changeInLength:), mask, range, delta);
}
static NSInteger sendChangeInLength(id storage) {
	return ((DeltaFn) objc_msgSend)(storage, @selector(changeInLength));
}
static NSUInteger sendEditedMask(id storage) {
	return ((MaskFn) objc_msgSend)(storage, @selector(editedMask));
}

// Records what the storage tells it about an edit, without invalidating
// anything (the deltas used here are far larger than the text).
@interface RecordingLayoutManager : NSLayoutManager {
@public
	int notifications;
	NSUInteger mask;
	NSRange range;
	NSInteger delta;
	NSRange invalidated;
}
- (void) reset;
@end

@implementation RecordingLayoutManager
- (void) reset {
	notifications = 0;
	mask = 0;
	range = NSMakeRange(0, 0);
	delta = 0;
	invalidated = NSMakeRange(0, 0);
}
- (void) textStorage: (NSTextStorage *) storage
	      edited: (NSUInteger) editedMask
	       range: (NSRange) editedRange
	changeInLength: (NSInteger) changeInLength
    invalidatedRange: (NSRange) invalidatedRange
{
	notifications++;
	mask = editedMask;
	range = editedRange;
	delta = changeInLength;
	invalidated = invalidatedRange;
}
// the name of the notification from macOS 12 on
- (void) processEditingForTextStorage: (NSTextStorage *) storage
				edited: (NSUInteger) editedMask
				 range: (NSRange) editedRange
			changeInLength: (NSInteger) changeInLength
		      invalidatedRange: (NSRange) invalidatedRange
{
	[self textStorage: storage edited: editedMask range: editedRange
	   changeInLength: changeInLength invalidatedRange: invalidatedRange];
}
@end

// A custom storage: the four primitives over an attributed string, with
// NSInteger overrides of the change-tracking methods.
@interface CustomStorage : NSTextStorage {
	NSMutableAttributedString *backing;
@public
	int editedCalls;
	NSUInteger seenMask;
	NSRange seenRange;
	NSInteger seenDelta;
	int processCalls;
	NSUInteger procMask;
	NSRange procRange;
	NSInteger procDelta;
}
@end

@implementation CustomStorage
- (instancetype) init {
	if ((self = [super init]))
		backing = [[NSMutableAttributedString alloc] init];
	return self;
}
- (void) dealloc {
	[backing release];
	[super dealloc];
}
- (NSString *) string { return [backing string]; }
- (NSDictionary *) attributesAtIndex: (NSUInteger) index effectiveRange: (NSRangePointer) range {
	return [backing attributesAtIndex: index effectiveRange: range];
}
- (void) replaceCharactersInRange: (NSRange) range withString: (NSString *) string {
	NSInteger delta = (NSInteger) [string length] - (NSInteger) range.length;
	[backing replaceCharactersInRange: range withString: string];
	[self edited: NSTextStorageEditedCharacters range: range changeInLength: delta];
}
- (void) setAttributes: (NSDictionary *) attrs range: (NSRange) range {
	[backing setAttributes: attrs range: range];
	[self edited: NSTextStorageEditedAttributes range: range changeInLength: 0];
}
// The ranges the tests pass are not inside the text.
- (void) fixAttributesInRange: (NSRange) range {
}
- (void) edited: (NSUInteger) editedMask range: (NSRange) range changeInLength: (NSInteger) delta {
	editedCalls++;
	seenMask = editedMask;
	seenRange = range;
	seenDelta = delta;
	[super edited: editedMask range: range changeInLength: delta];
}
// The pending-edit accessors are specified to be valid while the edit is
// being processed.
- (void) processEditing {
	processCalls++;
	procMask = sendEditedMask(self);
	procRange = [self editedRange];
	procDelta = sendChangeInLength(self);
	[super processEditing];
}
@end

static RecordingLayoutManager *attachLayoutManager(NSTextStorage *storage) {
	RecordingLayoutManager *lm = [[[RecordingLayoutManager alloc] init] autorelease];
	[storage addLayoutManager: lm];
	return lm;
}

int main(void)
{
	@autoreleasepool {
		// The signature, as the runtime reports it for the framework's own
		// implementation.
		Method edited = class_getInstanceMethod([NSTextStorage class], @selector(edited:range:changeInLength:));
		Method delta = class_getInstanceMethod([NSTextStorage class], @selector(changeInLength));
		Method mask = class_getInstanceMethod([NSTextStorage class], @selector(editedMask));
		expect(edited && delta && mask);
		char *ret = method_copyReturnType(delta);
		expect(strcmp(ret, @encode(NSInteger)) == 0);
		free(ret);
		ret = method_copyReturnType(mask);
		expect(strcmp(ret, @encode(NSUInteger)) == 0);
		free(ret);
		char *arg2 = method_copyArgumentType(edited, 2);
		char *arg4 = method_copyArgumentType(edited, 4);
		expect(strcmp(arg2, @encode(NSUInteger)) == 0);
		expect(strcmp(arg4, @encode(NSInteger)) == 0);
		free(arg2);
		free(arg4);

		// The typesetters' hyphenation hooks are float and UTF32Char (the concrete
		// NSATSTypesetter implements them; NSTypesetter only declares them).
		struct { SEL sel; const char *ret; } hooks[] = {
			{ @selector(hyphenationFactor), @encode(float) },
			{ @selector(hyphenationFactorForGlyphAtIndex:), @encode(float) },
			{ @selector(hyphenCharacterForGlyphAtIndex:), @encode(UTF32Char) },
		};
		for (size_t i = 0; i < sizeof(hooks) / sizeof(hooks[0]); i++) {
			Method m = class_getInstanceMethod([NSATSTypesetter class], hooks[i].sel);
			char *r = m ? method_copyReturnType(m) : NULL;
			expect(r && strcmp(r, hooks[i].ret) == 0);
			free(r);
		}
		Method setFactor = class_getInstanceMethod([NSATSTypesetter class], @selector(setHyphenationFactor:));
		char *factorArg = setFactor ? method_copyArgumentType(setFactor, 2) : NULL;
		expect(factorArg && strcmp(factorArg, @encode(float)) == 0);
		free(factorArg);

		// The public values of the edit actions.
		expect(NSTextStorageEditedAttributes == 1);
		expect(NSTextStorageEditedCharacters == 2);

		// A subclass override receives the full delta from a caller.
		CustomStorage *storage = [[[CustomStorage alloc] init] autorelease];
		RecordingLayoutManager *lm = attachLayoutManager(storage);
		sendEdited(storage, MASK_BOTH, NSMakeRange(3, 0), kBig);
		expect(storage->editedCalls == 1);
		expect(storage->seenDelta == kBig);
		expect(storage->seenMask == MASK_BOTH);
		expect(storage->seenRange.location == 3 && storage->seenRange.length == 0);
		// ... the storage reports all of it while processing the edit ...
		expect(storage->processCalls == 1);
		expect(storage->procDelta == kBig);
		expect(storage->procMask == MASK_BOTH);
		expect(storage->procRange.location == 3);
		expect(storage->procRange.length == (NSUInteger) kBig);
		// ... and so does the layout manager's notification.
		expect(lm->notifications == 1);
		expect(lm->delta == kBig);
		expect(lm->mask == MASK_BOTH);
		expect(lm->range.location == 3 && lm->range.length == (NSUInteger) kBig);

		[lm reset];
		sendEdited(storage, NSTextStorageEditedAttributes, NSMakeRange(7, 0), -kBig);
		expect(storage->seenDelta == -kBig);
		expect(storage->seenMask == NSTextStorageEditedAttributes);
		expect(storage->processCalls == 2);
		expect(storage->procDelta == -kBig);
		expect(storage->procMask == NSTextStorageEditedAttributes);
		expect(lm->notifications == 1);
		expect(lm->delta == -kBig);
		expect(lm->mask == NSTextStorageEditedAttributes);
		expect(lm->range.location == 7 && lm->range.length == (NSUInteger) -kBig);

		// Deltas add up while editing is open; the sum is notified once.
		[lm reset];
		[storage beginEditing];
		sendEdited(storage, NSTextStorageEditedCharacters, NSMakeRange(0, 0), kBig);
		expect(sendChangeInLength(storage) == kBig);
		sendEdited(storage, NSTextStorageEditedAttributes, NSMakeRange(0, 0), -kBig - 5);
		expect(sendChangeInLength(storage) == -5);
		expect(sendEditedMask(storage) == MASK_BOTH);
		expect(lm->notifications == 0);
		[storage endEditing];
		expect(storage->procDelta == -5);
		expect(lm->notifications == 1);
		expect(lm->delta == -5);
		expect(lm->mask == MASK_BOTH);

		// Ordinary edits: a negative delta stays negative.
		[lm reset];
		[storage replaceCharactersInRange: NSMakeRange(0, 0) withString: @"hello world"];
		expect(lm->delta == 11);
		expect(storage->procDelta == 11);
		[lm reset];
		[storage replaceCharactersInRange: NSMakeRange(0, 5) withString: @"h"];
		expect(lm->delta == -4);
		expect(storage->procDelta == -4);
		expect(lm->mask == NSTextStorageEditedCharacters);
		expect([[storage string] isEqualToString: @"h world"]);

		// The framework's own concrete storage.
		NSTextStorage *plain = [[[NSTextStorage alloc] initWithString: @"hello world"] autorelease];
		RecordingLayoutManager *plm = attachLayoutManager(plain);
		[plm reset];
		[plain replaceCharactersInRange: NSMakeRange(0, 5) withString: @"h"];
		expect(plm->notifications == 1);
		expect(plm->delta == -4);
		expect(plm->mask & NSTextStorageEditedCharacters);
		[plm reset];
		[plain replaceCharactersInRange: NSMakeRange(0, 1) withString: @"hello"];
		expect(plm->delta == 4);
		[plm reset];
		[plain beginEditing];
		sendEdited(plain, NSTextStorageEditedAttributes, NSMakeRange(0, 0), kBig);
		expect(sendChangeInLength(plain) == kBig);
		sendEdited(plain, NSTextStorageEditedAttributes, NSMakeRange(0, 0), -kBig);
		expect(sendChangeInLength(plain) == 0);
		[plain endEditing];
		expect(plm->notifications == 1);
		expect(plm->delta == 0);
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
