// Copyright (C) 2026 Lubos Dolezel
//
// What NSPrintOperation makes of a view when the job is saved as a PDF file (the job disposition NSPrintSaveJob
// and a path): the number of pages, the size of each page and what is on each, for
//   - a view that is one page,
//   - a view that gives its own page range (knowsPageRange:, rectForPage:),
//   - a view taller than a page, which the operation paginates by itself,
//   - the job disposition set before the operation is made (the paper is the one of the print info, as ever),
//   - the job disposition set on the operation's own print info,
//   - where the printed rectangle is on the paper: the margins, the centering, the scaling factor, a flipped view, the
//     view's own -locationOfPrintRect: and the pagination of a view taller than the page area (observed on macOS
//     26.7.3 with the margins out of the printer's non-imageable border).
// A page of a printed document has the paper of the page format, in its orientation; no page is added or left
// empty by the pagination. The pages are drawn into bitmaps to see what is on them.
// No window system is needed.
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

static int failures;
static int n; // the number of the job being checked
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, msg); failures++; } } while (0)

// A view of the given number of pages; page n (from 1) has a black square at (10 * n, 10) of 60 by 60 units
@interface PagedView : NSView {
	@public
	NSInteger pageCount;
	NSSize pageSize;
	BOOL ownRange;
}
@end
@implementation PagedView
- (BOOL) isFlipped { return NO; }
- (BOOL) knowsPageRange: (NSRangePointer) range {
	if (!ownRange)
		return NO;
	*range = NSMakeRange(1, pageCount);
	return YES;
}
- (NSRect) rectForPage: (NSInteger) page {
	return NSMakeRect(0, 0, pageSize.width, pageSize.height);
}
- (void) drawRect: (NSRect) rect {
	NSInteger page = ownRange ? [[NSPrintOperation currentOperation] currentPage] : 1;
	[[NSColor blackColor] set];
	NSRectFill(NSMakeRect(10 * page, 10, 60, 60));
}
@end

// pages of the file: count, and the media box of each (up to 8)
static size_t pagesOf(NSString *path, CGRect boxes[8], CGPDFDocumentRef *documentOut) {
	CGPDFDocumentRef document = CGPDFDocumentCreateWithURL((CFURLRef) [NSURL fileURLWithPath: path]);
	size_t count = document ? CGPDFDocumentGetNumberOfPages(document) : 0;

	for (size_t i = 0; i < count && i < 8; i++)
		boxes[i] = CGPDFPageGetBoxRect(CGPDFDocumentGetPage(document, i + 1), kCGPDFMediaBox);
	if (documentOut)
		*documentOut = document;
	else if (document)
		CGPDFDocumentRelease(document);
	return count;
}

static NSString *pdfPath(void) {
	return [NSString stringWithFormat: @"%@/job%d.pdf", [NSString stringWithUTF8String: getenv("PAGES_DIR")], n++];
}

// the file is removed, unless PAGES_DUMP is set: the files then stay there for a look at them
static void forget(NSString *path) {
	if (getenv("PAGES_DUMP"))
		[[NSFileManager defaultManager] copyItemAtPath: path toPath: [NSString stringWithFormat: @"%s/job%d.pdf", getenv("PAGES_DUMP"), n - 1] error: NULL];
	unlink([path fileSystemRepresentation]);
}

static NSString *titleOf(CGPDFDocumentRef document) {
	CGPDFDictionaryRef info = CGPDFDocumentGetInfo(document);
	CGPDFStringRef title = NULL;

	if (info == NULL || !CGPDFDictionaryGetString(info, "Title", &title))
		return nil;
	return [(NSString *) CGPDFStringCopyTextString(title) autorelease];
}


static BOOL run(PagedView *view, BOOL dispositionFirst, NSString *path) {
	NSPrintInfo *info = [[[NSPrintInfo alloc] init] autorelease];
	[info setPaperSize: NSMakeSize(595, 842)];
	[info setOrientation: NSLandscapeOrientation];
	if (dispositionFirst) {
		[info setJobDisposition: NSPrintSaveJob];
		[[info dictionary] setObject: path forKey: NSPrintSavePath];
	}
	NSPrintOperation *operation = [NSPrintOperation printOperationWithView: view printInfo: info];
	if (!dispositionFirst) {
		[[operation printInfo] setJobDisposition: NSPrintSaveJob];
		[[[operation printInfo] dictionary] setObject: path forKey: NSPrintSavePath];
	}
	[operation setJobTitle: @"Pages test job"];
	[operation setShowsPrintPanel: NO];
	[operation setShowsProgressPanel: NO];
	return [operation runOperation];
}

// the decoded content stream of a page of the file
static NSString *contentOfPage(CGPDFDocumentRef document, size_t index) {
	CGPDFDictionaryRef dictionary = CGPDFPageGetDictionary(CGPDFDocumentGetPage(document, index + 1));
	CGPDFStreamRef stream = NULL;
	CGPDFArrayRef array = NULL;
	NSMutableData *data = [NSMutableData data];

	if (CGPDFDictionaryGetStream(dictionary, "Contents", &stream)) {
		CGPDFDataFormat format;
		CFDataRef part = CGPDFStreamCopyData(stream, &format);
		[data appendData: (NSData *) part];
		if (part)
			CFRelease(part);
	} else if (CGPDFDictionaryGetArray(dictionary, "Contents", &array)) {
		for (size_t i = 0; i < CGPDFArrayGetCount(array); i++) {
			CGPDFDataFormat format;
			if (CGPDFArrayGetStream(array, i, &stream)) {
				CFDataRef part = CGPDFStreamCopyData(stream, &format);
				[data appendData: (NSData *) part];
				if (part)
					CFRelease(part);
			}
		}
	}
	return [[[NSString alloc] initWithData: data encoding: NSISOLatin1StringEncoding] autorelease];
}

// The black square of a page is there when the content has a filled path
static BOOL squareOnPage(CGPDFDocumentRef document, size_t index, int square, CGRect box) {
	NSString *content = contentOfPage(document, index);
	NSString *dump = getenv("PAGES_DUMP") ? [NSString stringWithFormat: @"%s/job%d-page%zu.txt", getenv("PAGES_DUMP"), n - 1, index + 1] : nil;

	if (dump)
		[content writeToFile: dump atomically: NO encoding: NSISOLatin1StringEncoding error: NULL];
	return [content rangeOfString: @" h f"].location != NSNotFound;
}

// A view that draws a black square at (10, 10) of 60 by 60, that can be flipped and give its own place on the paper
@interface PlacedView : NSView {
	@public
	BOOL flipped;
	BOOL ownLocation;
	NSPoint location;
}
@end
@implementation PlacedView
- (BOOL) isFlipped { return flipped; }
- (NSPoint) locationOfPrintRect: (NSRect) rect {
	return ownLocation ? location : [super locationOfPrintRect: rect];
}
- (void) drawRect: (NSRect) rect {
	[[NSColor blackColor] set];
	NSRectFill(NSMakeRect(10, 10, 60, 60));
}
@end

// The first clip and the first fill of the content stream of a page, as rectangles on the page: the operators of the
// stream are followed with the transformations (cm) and the saved states (q, Q) in effect.
typedef struct { double x, y, w, h; } PageRect;
static void multiply(const double n[6], double m[6]) {
	double r[6] = { n[0] * m[0] + n[1] * m[2], n[0] * m[1] + n[1] * m[3], n[2] * m[0] + n[3] * m[2],
		n[2] * m[1] + n[3] * m[3], n[4] * m[0] + n[5] * m[2] + m[4], n[4] * m[1] + n[5] * m[3] + m[5] };
	for (int i = 0; i < 6; i++)
		m[i] = r[i];
}
static void growBox(double box[4], int *has, const double m[6], double x, double y) {
	double px = m[0] * x + m[2] * y + m[4], py = m[1] * x + m[3] * y + m[5];
	if (!*has) {
		box[0] = box[2] = px;
		box[1] = box[3] = py;
		*has = 1;
	} else {
		if (px < box[0]) box[0] = px;
		if (px > box[2]) box[2] = px;
		if (py < box[1]) box[1] = py;
		if (py > box[3]) box[3] = py;
	}
}
static void placement(NSString *content, PageRect *clip, PageRect *fill) {
	const char *p = [content UTF8String];
	double m[6] = { 1, 0, 0, 1, 0, 0 }, saved[16][6], operands[8], box[4];
	int depth = 0, count = 0, has = 0, gotClip = 0, gotFill = 0;

	memset(clip, 0, sizeof *clip);
	memset(fill, 0, sizeof *fill);
	while (*p) {
		while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '[' || *p == ']')
			p++;
		if (!*p)
			break;
		char *end;
		double v = strtod(p, &end);
		if (end != p && (*end == ' ' || *end == '\n' || *end == 0)) {
			if (count == 8) { memmove(operands, operands + 1, 7 * sizeof(double)); count = 7; }
			operands[count++] = v;
			p = end;
			continue;
		}
		char op[8] = { 0 };
		size_t n = 0;
		while (*p && *p != ' ' && *p != '\n' && *p != '\r' && n < 7)
			op[n++] = *p++;
		while (*p && *p != ' ' && *p != '\n' && *p != '\r')
			p++;
		if (!strcmp(op, "q") && depth < 16) {
			memcpy(saved[depth++], m, sizeof m);
		} else if (!strcmp(op, "Q") && depth > 0) {
			memcpy(m, saved[--depth], sizeof m);
		} else if (!strcmp(op, "cm") && count >= 6) {
			multiply(operands + count - 6, m);
		} else if (!strcmp(op, "re") && count >= 4) {
			double *r = operands + count - 4;
			growBox(box, &has, m, r[0], r[1]);
			growBox(box, &has, m, r[0] + r[2], r[1] + r[3]);
		} else if ((!strcmp(op, "m") || !strcmp(op, "l")) && count >= 2) {
			growBox(box, &has, m, operands[count - 2], operands[count - 1]);
		} else if (!strcmp(op, "n") && has) {
			if (!gotClip) { clip->x = box[0]; clip->y = box[1]; clip->w = box[2] - box[0]; clip->h = box[3] - box[1]; gotClip = 1; }
			has = 0;
		} else if (!strcmp(op, "f") && has) {
			if (!gotFill) { fill->x = box[0]; fill->y = box[1]; fill->w = box[2] - box[0]; fill->h = box[3] - box[1]; gotFill = 1; }
			has = 0;
		}
		count = 0;
	}
}
static BOOL nearTo(double a, double b) { return fabs(a - b) < 0.01; }
static BOOL sameRect(PageRect r, double x, double y, double w, double h) {
	return nearTo(r.x, x) && nearTo(r.y, y) && nearTo(r.w, w) && nearTo(r.h, h);
}

// A page of 595 by 842 points (or 842 by 595 in landscape), printed to a PDF file with the settings
static BOOL runPlaced(NSView *view, NSPrintingOrientation orientation, CGFloat left, CGFloat right, CGFloat top,
		CGFloat bottom, BOOL horizontal, BOOL vertical, double scale, NSString *path) {
	NSPrintInfo *info = [[[NSPrintInfo alloc] init] autorelease];
	[info setPaperSize: NSMakeSize(595, 842)];
	[info setOrientation: orientation];
	[info setLeftMargin: left];
	[info setRightMargin: right];
	[info setTopMargin: top];
	[info setBottomMargin: bottom];
	[info setHorizontallyCentered: horizontal];
	[info setVerticallyCentered: vertical];
	[[info dictionary] setObject: [NSNumber numberWithDouble: scale] forKey: NSPrintScalingFactor];
	NSPrintOperation *operation = [NSPrintOperation printOperationWithView: view printInfo: info];
	[[operation printInfo] setJobDisposition: NSPrintSaveJob];
	[[[operation printInfo] dictionary] setObject: path forKey: NSPrintSavePath];
	[operation setShowsPrintPanel: NO];
	[operation setShowsProgressPanel: NO];
	return [operation runOperation];
}

// The place of the first page of the view on the paper, and the number of pages. The margins are 50 points, further in
// than the border that a printer cannot print in.
static void checkPlaced(const char *what, PlacedView *view, NSPrintingOrientation orientation, CGFloat left,
		CGFloat right, CGFloat top, CGFloat bottom, BOOL horizontal, BOOL vertical, double scale, size_t pages,
		double clipX, double clipY, double clipW, double clipH, double squareX, double squareY, double squareW)
{
	CGPDFDocumentRef document = NULL;
	CGRect boxes[8];
	NSString *path = pdfPath();
	char message[160];
	PageRect clip, fill;

	snprintf(message, sizeof message, "%s: the operation ran", what);
	CHECK(runPlaced(view, orientation, left, right, top, bottom, horizontal, vertical, scale, path), message);
	size_t count = pagesOf(path, boxes, &document);
	snprintf(message, sizeof message, "%s: %zu pages (%zu expected)", what, count, pages);
	CHECK(count == pages, message);
	if (count >= 1 && document) {
		placement(contentOfPage(document, 0), &clip, &fill);
		snprintf(message, sizeof message, "%s: the rectangle of the page is at %g %g, %g by %g", what, clip.x, clip.y, clip.w, clip.h);
		CHECK(sameRect(clip, clipX, clipY, clipW, clipH), message);
		snprintf(message, sizeof message, "%s: the square is at %g %g, %g by %g", what, fill.x, fill.y, fill.w, fill.h);
		CHECK(sameRect(fill, squareX, squareY, squareW, squareW), message);
	}
	if (document)
		CGPDFDocumentRelease(document);
	forget(path);
}


int main(void) {
	char dir[] = "/tmp/appkit-print-pages.XXXXXX";

	@autoreleasepool {
		CGRect boxes[8];
		CGPDFDocumentRef document;
		size_t count;

		CHECK(mkdtemp(dir) != NULL, "temporary directory");
		setenv("PAGES_DIR", dir, 1);

		// a view of one page
		PagedView *one = [[[PagedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		NSString *path = pdfPath();
		CHECK(run(one, NO, path), "one page: the operation ran");
		count = pagesOf(path, boxes, &document);
		CHECK(count == 1, "one page: the document has one page");
		if (count >= 1) {
			CHECK(boxes[0].size.width == 842 && boxes[0].size.height == 595, "one page: the page has the paper in landscape");
			CHECK(squareOnPage(document, 0, 1, boxes[0]), "one page: the square is on the page");
			CHECK([titleOf(document) isEqualToString: @"Pages test job"], "one page: the job title is the title of the document");
		}
		if (document)
			CGPDFDocumentRelease(document);
		forget(path);

		// a view that knows its three pages
		PagedView *three = [[[PagedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		three->ownRange = YES;
		three->pageCount = 3;
		three->pageSize = NSMakeSize(400, 300);
		path = pdfPath();
		CHECK(run(three, NO, path), "three pages: the operation ran");
		count = pagesOf(path, boxes, &document);
		CHECK(count == 3, "three pages: the document has three pages");
		for (size_t i = 0; i < count && i < 3; i++) {
			CHECK(boxes[i].size.width == 842 && boxes[i].size.height == 595, "three pages: each page has the paper in landscape");
			CHECK(squareOnPage(document, i, (int) i + 1, boxes[i]), "three pages: the square is on the page");
		}
		if (document)
			CGPDFDocumentRelease(document);
		forget(path);

		// a view taller than the page, paginated by the operation (595 by 842 in landscape, 842 by 595: 1500 / 595 pages)
		PagedView *tall = [[[PagedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 1500)] autorelease];
		path = pdfPath();
		CHECK(run(tall, NO, path), "tall view: the operation ran");
		count = pagesOf(path, boxes, NULL);
		CHECK(count == 3, "tall view: three pages for a view of 1500 units on pages of 595");
		forget(path);

		// the job disposition is set first: the operation has the paper of its print info as well (as on macOS 26.7.3)
		PagedView *first = [[[PagedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		path = pdfPath();
		CHECK(run(first, YES, path), "disposition first: the operation ran");
		count = pagesOf(path, boxes, &document);
		CHECK(count == 1, "disposition first: the document has one page");
		if (count >= 1)
			CHECK(boxes[0].size.width == 842 && boxes[0].size.height == 595, "disposition first: the page has the paper in landscape");
		if (document)
			CGPDFDocumentRelease(document);
		forget(path);

		// Where the rectangle of the view is on the paper (a view of 400 by 300 on 595 by 842 points)
		PlacedView *placed = [[[PlacedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		// at the upper left corner inside the margins
		checkPlaced("margins", placed, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 1, 1, 50, 492, 400, 300, 60, 502, 60);
		checkPlaced("unequal margins", placed, NSPortraitOrientation, 100, 60, 60, 50, NO, NO, 1, 1, 100, 482, 400, 300, 110, 492, 60);
		// in the middle of what is inside the margins
		checkPlaced("centered", placed, NSPortraitOrientation, 50, 50, 50, 50, YES, YES, 1, 1, 97.5, 271, 400, 300, 107.5, 281, 60);
		checkPlaced("centered horizontally", placed, NSPortraitOrientation, 50, 50, 50, 50, YES, NO, 1, 1, 97.5, 492, 400, 300, 107.5, 502, 60);
		checkPlaced("centered vertically", placed, NSPortraitOrientation, 50, 50, 50, 50, NO, YES, 1, 1, 50, 271, 400, 300, 60, 281, 60);
		checkPlaced("centered, unequal margins", placed, NSPortraitOrientation, 100, 60, 60, 50, YES, YES, 1, 1, 117.5, 266, 400, 300, 127.5, 276, 60);
		// the scaling factor makes the rectangle smaller on the paper
		checkPlaced("scaled", placed, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 0.5, 1, 50, 642, 200, 150, 55, 647, 30);
		checkPlaced("scaled and centered", placed, NSPortraitOrientation, 50, 50, 50, 50, YES, YES, 0.5, 1, 197.5, 346, 200, 150, 202.5, 351, 30);
		// the page is turned in landscape
		checkPlaced("landscape", placed, NSLandscapeOrientation, 50, 50, 50, 50, NO, NO, 1, 1, 50, 245, 400, 300, 60, 255, 60);
		checkPlaced("landscape centered", placed, NSLandscapeOrientation, 100, 60, 60, 50, YES, YES, 1, 1, 241, 142.5, 400, 300, 251, 152.5, 60);
		// the top of a flipped view is the top of the rectangle
		PlacedView *flipped = [[[PlacedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		flipped->flipped = YES;
		checkPlaced("flipped", flipped, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 1, 1, 50, 492, 400, 300, 60, 722, 60);
		checkPlaced("flipped and centered", flipped, NSPortraitOrientation, 50, 50, 50, 50, YES, YES, 1, 1, 97.5, 271, 400, 300, 107.5, 501, 60);
		// the view that places the rectangle by itself: the margins and the centering are not applied
		PlacedView *own = [[[PlacedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		own->ownLocation = YES;
		own->location = NSMakePoint(70, 80);
		checkPlaced("own location", own, NSPortraitOrientation, 50, 50, 50, 50, YES, YES, 1, 1, 70, 80, 400, 300, 80, 90, 60);
		checkPlaced("own location, scaled", own, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 0.5, 1, 70, 80, 200, 150, 75, 85, 30);
		// a view taller than the part of the page inside the margins (742 points): the pages start at the top of the view
		PlacedView *tallPlaced = [[[PlacedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 1500)] autorelease];
		checkPlaced("tall view", tallPlaced, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 1, 3, 50, 50, 400, 742, 60, -698, 60);
		// the scaled view is shorter on the paper (1500 * 0.4 = 600 points, which fits)
		checkPlaced("tall, scaled", tallPlaced, NSPortraitOrientation, 50, 50, 50, 50, NO, NO, 0.4, 1, 50, 192, 160, 600, 54, 196, 24);

		rmdir(dir);
	}
	printf(failures ? "FAIL appkit_print_operation_pages\n" : "PASS appkit_print_operation_pages\n");
	return failures != 0;
}
