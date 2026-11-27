// Copyright (C) 2026 Lubos Dolezel
//
// What NSPrintOperation makes of a view when the job is saved as a PDF file (the job disposition NSPrintSaveJob
// and a path): the number of pages, the size of each page and what is on each, for
//   - a view that is one page,
//   - a view that gives its own page range (knowsPageRange:, rectForPage:),
//   - a view taller than a page, which the operation paginates by itself,
//   - the job disposition set before the operation is made (the operation then has the view's size as its paper),
//   - the job disposition set on the operation's own print info.
// A page of a printed document has the paper of the page format, in its orientation; no page is added or left
// empty by the pagination. The pages are drawn into bitmaps to see what is on them.
// No window system is needed.
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

		// the job disposition is set first: the operation has the view's size as its paper
		PagedView *first = [[[PagedView alloc] initWithFrame: NSMakeRect(0, 0, 400, 300)] autorelease];
		path = pdfPath();
		CHECK(run(first, YES, path), "disposition first: the operation ran");
		count = pagesOf(path, boxes, &document);
		CHECK(count == 1, "disposition first: the document has one page");
		if (count >= 1) {
			CHECK(boxes[0].size.width == 400 && boxes[0].size.height == 300, "disposition first: the page is the size of the view");
			CHECK(squareOnPage(document, 0, 1, boxes[0]), "disposition first: the square is on the page");
		}
		if (document)
			CGPDFDocumentRelease(document);
		forget(path);

		rmdir(dir);
	}
	printf(failures ? "FAIL appkit_print_operation_pages\n" : "PASS appkit_print_operation_pages\n");
	return failures != 0;
}
