// CFLAGS: -framework Foundation -framework WebKit -F/System/Library/PrivateFrameworks -framework MobileAsset -framework RTTUtilities
// Copyright (C) 2026 Lubos Dolezel
//
// Legacy WebKit element and action keys, WebHistoryItem, MobileAsset's download size attribute and
// the RTTUtilities settings. Expected strings are the values macOS reports.
#import <Foundation/Foundation.h>
#import <WebKit/WebKit.h>
#import <WebKit/WebHistoryItem.h>
#import <MobileAsset/ASAsset.h>
#import <RTTUtilities/RTTUtilities.h>
#include <stdio.h>

static int failures;

static void expectString(const char *name, NSString *value, NSString *expected)
{
	if (![value isEqualToString:expected]) {
		printf("FAIL: %s is '%s', expected '%s'\n", name, [value UTF8String] ?: "(null)", [expected UTF8String]);
		failures++;
	}
}

#define EXPECT(sym, expected) expectString(#sym, sym, expected)

int main(void)
{
	@autoreleasepool {
		EXPECT(WebElementDOMNodeKey, @"WebElementDOMNode");
		EXPECT(WebElementFrameKey, @"WebElementFrame");
		EXPECT(WebElementImageKey, @"WebElementImage");
		EXPECT(WebElementIsSelectedKey, @"WebElementIsSelected");
		EXPECT(WebElementLinkLabelKey, @"WebElementLinkLabel");
		EXPECT(WebElementLinkTargetFrameKey, @"WebElementTargetFrame");
		EXPECT(WebElementLinkTitleKey, @"WebElementLinkTitle");
		EXPECT(WebElementLinkURLKey, @"WebElementLinkURL");
		EXPECT(WebActionElementKey, @"WebActionElementKey");
		EXPECT(WebActionModifierFlagsKey, @"WebActionModifierFlagsKey");
		EXPECT(WebActionOriginalURLKey, @"WebActionOriginalURLKey");
		EXPECT(ASAttributeDownloadSize, @"_DownloadSize");

		if (!NSClassFromString(@"WebHTMLView")) {
			printf("FAIL: WebHTMLView class is missing\n");
			failures++;
		}

		WebHistoryItem *item = [[WebHistoryItem alloc] initWithURLString:@"https://example.com/" title:@"Example" lastVisitedTimeInterval:42];
		expectString("-[WebHistoryItem URLString]", [item URLString], @"https://example.com/");
		expectString("-[WebHistoryItem title]", [item title], @"Example");
		if ([item lastVisitedTimeInterval] != 42) {
			printf("FAIL: lastVisitedTimeInterval is %f\n", [item lastVisitedTimeInterval]);
			failures++;
		}

		if ([[RTTSettings sharedInstance] TTYSoftwareEnabled]) {
			printf("FAIL: TTY software reported enabled\n");
			failures++;
		}
	}
	if (failures == 0)
		printf("OK\n");
	return failures != 0;
}
