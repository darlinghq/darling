// Copyright (C) 2026 Lubos Dolezel
//
// QLPreviewPanel keeps the items its data source supplies. An index with no
// item (the data source returned nil) reads back as a nil current item, never
// as a placeholder object; an index past the end reads back as nil; and the
// current index follows a reload that shrinks the list. The panel is a
// per-process singleton that exists only after +sharedPreviewPanel.
#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>
#include <stdio.h>

static int failures;

static void expect(const char *what, int cond)
{
	printf("%s: %s\n", cond ? "PASS" : "FAIL", what);
	if (!cond)
		failures++;
}

// Explicit instance variable and accessors: the 32-bit runtime has no property synthesis.
@interface Source : NSObject <QLPreviewPanelDataSource> {
	NSArray *_items;
}
- (NSArray *) items;
- (void) setItems: (NSArray *) items;
@end

@implementation Source
- (NSArray *) items
{
	return _items;
}

- (void) setItems: (NSArray *) items
{
	[_items autorelease];
	_items = [items retain];
}

- (NSInteger) numberOfPreviewItemsInPreviewPanel: (QLPreviewPanel *) panel
{
	return (NSInteger) [_items count];
}

- (id<QLPreviewItem>) previewPanel: (QLPreviewPanel *) panel previewItemAtIndex: (NSInteger) index
{
	id item = [self.items objectAtIndex: (NSUInteger) index];
	return item == [NSNull null] ? nil : item;
}
@end

/* The number of items the panel accepted, seen through the public index property:
   an index is only taken when an item exists there. The selection is restored. */
static NSInteger item_count(QLPreviewPanel *panel)
{
	NSInteger saved = panel.currentPreviewItemIndex, n = 0;
	for (;; n++) {
		panel.currentPreviewItemIndex = n;
		if (panel.currentPreviewItemIndex != n)
			break;
	}
	panel.currentPreviewItemIndex = saved;
	return n;
}

int main(void)
{
	@autoreleasepool {
		expect("panel does not exist before it is asked for", ![QLPreviewPanel sharedPreviewPanelExists]);
		QLPreviewPanel *panel = [QLPreviewPanel sharedPreviewPanel];
		expect("shared panel", panel != nil && [QLPreviewPanel sharedPreviewPanelExists]);
		expect("same panel every time", [QLPreviewPanel sharedPreviewPanel] == panel);
		expect("no data source: no items", [panel currentPreviewItem] == nil);

		NSURL *first = [NSURL fileURLWithPath: @"/tmp/quicklook-first.txt"];
		NSURL *third = [NSURL fileURLWithPath: @"/tmp/quicklook-third.txt"];
		Source *source = [[Source alloc] init];
		[source setItems: @[ first, [NSNull null], third ]];
		panel.dataSource = source;
		[panel reloadData];
		expect("three items", item_count(panel) == 3);

		panel.currentPreviewItemIndex = 1;
		expect("index with a nil item selected", panel.currentPreviewItemIndex == 1);
		expect("nil item reads back as nil", [panel currentPreviewItem] == nil);

		panel.currentPreviewItemIndex = 3;
		expect("index past the end is ignored", panel.currentPreviewItemIndex == 1);
		panel.currentPreviewItemIndex = 2;
		expect("last item", [[panel currentPreviewItem] isEqual: third]);

		[source setItems: @[ first ]];
		[panel reloadData];
		expect("reload shrinks the list", item_count(panel) == 1);
		expect("index follows the shrunk list", panel.currentPreviewItemIndex == 0);
		expect("item after shrink", [[panel currentPreviewItem] isEqual: first]);

		[source setItems: @[]];
		[panel reloadData];
		expect("empty list: no current item", [panel currentPreviewItem] == nil && panel.currentPreviewItemIndex == 0);
		panel.dataSource = nil;
	}
	printf("%s\n", failures ? "FAILED" : "ALL PASSED");
	return failures ? 1 : 0;
}
