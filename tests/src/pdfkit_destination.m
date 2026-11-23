// CFLAGS: -framework Foundation -framework PDFKit
// Copyright (C) 2026 Darling developers
//
// PDFDestination describes a point on a PDF page, in page space
// (https://developer.apple.com/documentation/pdfkit/pdfdestination): init(page:at:) takes the page and the
// point, page and point return them, and kPDFDestinationUnspecifiedValue is the x or y value to give when
// the position is unimportant (the constant itself is pinned in pdfkit_constants.m).
// The page property is weak (the pages declare it so), so a destination does not retain its page. The last two cases
// pin observations of macOS 26.7.3 and 10.14.4 that the pages leave open (see PDFDestination.m): a destination cannot be
// made without a page, and a plain -init has an unspecified point.
// Prints PASS, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <PDFKit/PDFKit.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// Whether the destination's page is the given one. Reading a weak reference retains and autoreleases the object,
// so the read is drained here and does not keep the page alive for the rest of the test.
static BOOL pageIs(PDFDestination* destination, PDFPage* expected)
{
	@autoreleasepool
	{
		return destination.page == expected;
	}
}

int main(void)
{
	@autoreleasepool
	{
		PDFPage* page = [[PDFPage alloc] init];
		const NSUInteger before = [page retainCount];
		PDFDestination* destination = [[PDFDestination alloc] initWithPage:page atPoint:CGPointMake(12.5, 700)];
		CHECK(destination != nil);
		CHECK(pageIs(destination, page));
		CHECK(destination.point.x == 12.5 && destination.point.y == 700);

		// A negative or unspecified coordinate is stored as given.
		PDFDestination* top = [[PDFDestination alloc] initWithPage:page
			atPoint:CGPointMake(-3, kPDFDestinationUnspecifiedValue)];
		CHECK(top.point.x == -3);
		CHECK(top.point.y == kPDFDestinationUnspecifiedValue);

		// The page is a weak reference: the destination does not own it, so its reference count is unchanged.
		CHECK([page retainCount] == before);
		[top release];
		[destination release];
		CHECK([page retainCount] == before);

		// The reference is cleared when the page is freed. The page is made with -init, so no PDFDocument is needed.
		PDFDestination* kept = [[PDFDestination alloc] initWithPage:page atPoint:CGPointMake(5, 6)];
		CHECK(pageIs(kept, page));
		[page release];
		CHECK(pageIs(kept, nil));
		CHECK(kept.point.x == 5 && kept.point.y == 6);
		[kept release];

		// Initialising without a page returns nil.
		PDFPage* none = nil;
		PDFDestination* orphan = [[PDFDestination alloc] initWithPage:none atPoint:CGPointMake(1, 2)];
		CHECK(orphan == nil);

		// A plain -init gives an unspecified point.
		PDFDestination* plain = [[PDFDestination alloc] init];
		CHECK(plain != nil);
		CHECK(plain.point.x == kPDFDestinationUnspecifiedValue && plain.point.y == kPDFDestinationUnspecifiedValue);
		CHECK(pageIs(plain, nil));
		[plain release];
	}

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
