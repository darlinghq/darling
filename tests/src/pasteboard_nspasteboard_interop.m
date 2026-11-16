// Copyright (C) 2026 Lubos Dolezel
//
// NSPasteboard and the Pasteboard Manager are two views of the same pasteboards: a string written with NSPasteboard
// is read with PasteboardCopyItemFlavorData and the other way round, change counts agree, the general and find boards
// are the same boards under both names, and promises made through either are kept when read through the other.
// Observed on macOS 26.7.3 and 10.14.4.
//
// Depends on NSPasteboard being implemented over CoreFoundation's pasteboard (not yet the case in Darling when this
// was written), so it does not pass before that. Build without ARC.
#import <Cocoa/Cocoa.h>
#include <stdio.h>
#include <string.h>

static int failures;
static void expect(const char *name, int ok)
{
	printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
	failures += !ok;
}

@interface Owner : NSObject { @public int provided; }
@end
@implementation Owner
- (void)pasteboard:(NSPasteboard *)pb provideDataForType:(NSString *)type
{
	provided++;
	[pb setString:@"promised-text" forType:type];
}
@end

int main(void)
{
	@autoreleasepool {
		NSPasteboard *general = [NSPasteboard generalPasteboard];
		PasteboardRef clip = NULL;
		PasteboardCreate(kPasteboardClipboard, &clip);

		// NSPasteboard -> Pasteboard Manager
		NSInteger before = [general changeCount];
		[general clearContents];
		[general setString:@"héllo" forType:NSPasteboardTypeString];
		expect("change count moved by a clear", [general changeCount] == before + 1);
		expect("the reference is told", (PasteboardSynchronize(clip) & kPasteboardModified) != 0);
		ItemCount count = 0;
		PasteboardItemID item = 0;
		CFDataRef data = NULL;
		expect("one item", PasteboardGetItemCount(clip, &count) == noErr && count == 1);
		expect("its identifier", PasteboardGetItemIdentifier(clip, 1, &item) == noErr);
		expect("UTF-8 data", PasteboardCopyItemFlavorData(clip, item, CFSTR("public.utf8-plain-text"), &data) == noErr
			&& CFDataGetLength(data) == 6 && memcmp(CFDataGetBytePtr(data), "h\xc3\xa9llo", 6) == 0);
		CFStringRef name = NULL;
		PasteboardCopyName(clip, &name);
		expect("same name", [(NSString *)name isEqualToString:[general name]]);

		// Pasteboard Manager -> NSPasteboard
		before = [general changeCount];
		expect("clear", PasteboardClear(clip) == noErr);
		expect("the change count moved", [general changeCount] == before + 1);
		CFDataRef text = (CFDataRef)[@"carbon-text" dataUsingEncoding:NSUTF8StringEncoding];
		expect("put", PasteboardPutItemFlavor(clip, (PasteboardItemID)1, CFSTR("public.utf8-plain-text"), text, 0) == noErr);
		expect("string", [[general stringForType:NSPasteboardTypeString] isEqualToString:@"carbon-text"]);
		expect("types", [[general types] containsObject:@"public.utf8-plain-text"]);
		expect("legacy type", [[general dataForType:@"NSStringPboardType"] length] == 11);

		// find
		NSPasteboard *find = [NSPasteboard pasteboardWithName:NSPasteboardNameFind];
		PasteboardRef carbonFind = NULL;
		PasteboardCreate(kPasteboardFind, &carbonFind);
		[find clearContents];
		[find setString:@"needle" forType:NSPasteboardTypeString];
		PasteboardSynchronize(carbonFind);
		expect("find board shared", PasteboardGetItemIdentifier(carbonFind, 1, &item) == noErr
			&& PasteboardCopyItemFlavorData(carbonFind, item, CFSTR("public.utf8-plain-text"), &data) == noErr && CFDataGetLength(data) == 6);
		expect("find is not the clipboard", ![[general stringForType:NSPasteboardTypeString] isEqualToString:@"needle"]);

		// a promise made through NSPasteboard is kept for a Pasteboard Manager read
		Owner *owner = [[Owner alloc] init];
		[general declareTypes:@[NSPasteboardTypeString] owner:owner];
		PasteboardSynchronize(clip);
		PasteboardGetItemIdentifier(clip, 1, &item);
		expect("not provided yet", owner->provided == 0);
		PasteboardFlavorFlags promiseFlags = 99;
		expect("an NSPasteboard promise is not flagged Promised through the Pasteboard Manager",
			PasteboardGetItemFlavorFlags(clip, item, CFSTR("public.utf8-plain-text"), &promiseFlags) == noErr && !(promiseFlags & kPasteboardFlavorPromised));
		expect("read provides", PasteboardCopyItemFlavorData(clip, item, CFSTR("public.utf8-plain-text"), &data) == noErr
			&& CFDataGetLength(data) == 13 && owner->provided == 1);
		PasteboardCopyItemFlavorData(clip, item, CFSTR("public.utf8-plain-text"), &data);
		expect("provided once", owner->provided == 1);
	}
	printf(failures ? "FAILED\n" : "ALL PASSED\n");
	return failures ? 1 : 0;
}
