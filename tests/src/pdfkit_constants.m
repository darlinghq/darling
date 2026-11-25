// CFLAGS: -framework Foundation -framework PDFKit
// Copyright (C) 2026 Darling developers
//
// PDFKit exports notification names, document attributes, document write options, annotation keys
// and annotation subtypes as NSString constants, and kPDFDestinationUnspecifiedValue as a CGFloat.
// The documentation gives no string values, so the test pins the values macOS 26.7.3 and 10.14.4
// report (black-box observation; the four write options exist only on 26.7.3, whose values are used),
// and checks that every constant is a non-empty NSString and that the constants of one group are
// pairwise distinct (they are used as dictionary keys and notification names).
// Prints OK, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <PDFKit/PDFKit.h>
#include <float.h>
#include <math.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

// Adds one constant to its group's set; an invalid value or a repeat of an earlier one is a failure.
#define CHECK_STRING(group, constant) do { \
	NSString *value = (constant); \
	if (value == nil || ![value isKindOfClass:[NSString class]] || [value length] == 0) { \
		printf("FAIL: %s is nil, not an NSString or empty\n", #constant); \
		failures++; \
	} else if ([(group) containsObject:value]) { \
		printf("FAIL: %s repeats the value of another %s constant\n", #constant, #group); \
		failures++; \
	} else { \
		[(group) addObject:value]; \
	} \
} while (0)

#define CHECK_VALUE(constant, expected) do { \
	NSString *actual_ = (constant); \
	if (![actual_ isEqualToString:(expected)]) { \
		printf("FAIL: %s is \"%s\", expected \"%s\"\n", #constant, [actual_ UTF8String] ?: "(nil)", [(expected) UTF8String]); \
		failures++; \
	} \
} while (0)

int main(void)
{
	@autoreleasepool {
		CHECK_VALUE(PDFDocumentDidBeginFindNotification, @"PDFDidBeginDocumentFind");
		CHECK_VALUE(PDFDocumentDidBeginPageFindNotification, @"PDFDidBeginPageFind");
		CHECK_VALUE(PDFDocumentDidEndFindNotification, @"PDFDidEndDocumentFind");
		CHECK_VALUE(PDFDocumentDidEndPageFindNotification, @"PDFDidEndPageFind");
		CHECK_VALUE(PDFDocumentDidUnlockNotification, @"PDFDocumentDidUnlock");
		CHECK_VALUE(PDFThumbnailViewDocumentEditedNotification, @"PDFThumbnailViewDocumentEditedNotification");
		CHECK_VALUE(PDFViewDocumentChangedNotification, @"PDFViewChangedDocument");
		CHECK_VALUE(PDFViewPageChangedNotification, @"PDFViewChangedPage");
		CHECK_VALUE(PDFViewSelectionChangedNotification, @"PDFViewSelectionChanged");
		CHECK_VALUE(PDFDocumentCreatorAttribute, @"Creator");
		CHECK_VALUE(PDFDocumentSubjectAttribute, @"Subject");
		CHECK_VALUE(PDFDocumentModificationDateAttribute, @"ModDate");
		CHECK_VALUE(PDFDocumentBurnInAnnotationsOption, @"PDFDocumentBurnInAnnotationsOption");
		CHECK_VALUE(PDFDocumentOptimizeImagesForScreenOption, @"PDFDocumentOptimizeImagesForScreenOption");
		CHECK_VALUE(PDFDocumentSaveImagesAsJPEGOption, @"PDFDocumentSaveImagesAsJPEGOption");
		CHECK_VALUE(PDFDocumentSaveTextFromOCROption, @"PDFDocumentSaveTextFromOCROption");
		CHECK_VALUE(PDFAnnotationKeyBorder, @"/Border");
		CHECK_VALUE(PDFAnnotationKeyColor, @"/C");
		CHECK_VALUE(PDFAnnotationKeyDefaultAppearance, @"/DA");
		CHECK_VALUE(PDFAnnotationKeyPage, @"/P");
		CHECK_VALUE(PDFAnnotationKeyParent, @"/Parent");
		CHECK_VALUE(PDFAnnotationKeyPopup, @"/Popup");
		CHECK_VALUE(PDFAnnotationKeySubtype, @"/Subtype");
		CHECK_VALUE(PDFAnnotationSubtypeHighlight, @"/Highlight");
		CHECK_VALUE(PDFAnnotationSubtypeStrikeOut, @"/StrikeOut");
		CHECK_VALUE(PDFAnnotationSubtypeUnderline, @"/Underline");

		// Notification names share one namespace, so they are checked as one group.
		NSMutableSet *notifications = [NSMutableSet set];
		CHECK_STRING(notifications, PDFDocumentDidBeginFindNotification);
		CHECK_STRING(notifications, PDFDocumentDidBeginPageFindNotification);
		CHECK_STRING(notifications, PDFDocumentDidEndFindNotification);
		CHECK_STRING(notifications, PDFDocumentDidEndPageFindNotification);
		CHECK_STRING(notifications, PDFDocumentDidUnlockNotification);
		CHECK_STRING(notifications, PDFThumbnailViewDocumentEditedNotification);
		CHECK_STRING(notifications, PDFViewDocumentChangedNotification);
		CHECK_STRING(notifications, PDFViewPageChangedNotification);
		CHECK_STRING(notifications, PDFViewSelectionChangedNotification);

		NSMutableSet *attributes = [NSMutableSet set];
		CHECK_STRING(attributes, PDFDocumentAuthorAttribute);
		CHECK_STRING(attributes, PDFDocumentCreationDateAttribute);
		CHECK_STRING(attributes, PDFDocumentCreatorAttribute);
		CHECK_STRING(attributes, PDFDocumentKeywordsAttribute);
		CHECK_STRING(attributes, PDFDocumentModificationDateAttribute);
		CHECK_STRING(attributes, PDFDocumentSubjectAttribute);
		CHECK_STRING(attributes, PDFDocumentTitleAttribute);

		NSMutableSet *writeOptions = [NSMutableSet set];
		CHECK_STRING(writeOptions, PDFDocumentBurnInAnnotationsOption);
		CHECK_STRING(writeOptions, PDFDocumentOptimizeImagesForScreenOption);
		CHECK_STRING(writeOptions, PDFDocumentSaveImagesAsJPEGOption);
		CHECK_STRING(writeOptions, PDFDocumentSaveTextFromOCROption);

		NSMutableSet *annotationKeys = [NSMutableSet set];
		CHECK_STRING(annotationKeys, PDFAnnotationKeyBorder);
		CHECK_STRING(annotationKeys, PDFAnnotationKeyColor);
		CHECK_STRING(annotationKeys, PDFAnnotationKeyDefaultAppearance);
		CHECK_STRING(annotationKeys, PDFAnnotationKeyPage);
		CHECK_STRING(annotationKeys, PDFAnnotationKeyParent);
		CHECK_STRING(annotationKeys, PDFAnnotationKeyPopup);
		CHECK_STRING(annotationKeys, PDFAnnotationKeySubtype);

		NSMutableSet *annotationSubtypes = [NSMutableSet set];
		CHECK_STRING(annotationSubtypes, PDFAnnotationSubtypeHighlight);
		CHECK_STRING(annotationSubtypes, PDFAnnotationSubtypeStrikeOut);
		CHECK_STRING(annotationSubtypes, PDFAnnotationSubtypeUnderline);

		// The value is not documented; macOS reports FLT_MAX stored as a CGFloat, not CGFLOAT_MAX.
		double unspecified = (double)kPDFDestinationUnspecifiedValue;
		CHECK(isfinite(unspecified) && unspecified == (double)FLT_MAX, "kPDFDestinationUnspecifiedValue is not FLT_MAX");
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
