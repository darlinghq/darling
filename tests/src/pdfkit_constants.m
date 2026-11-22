// CFLAGS: -framework Foundation -framework PDFKit
// Copyright (C) 2026 Darling developers
//
// PDFKit exports notification names, document attributes, document write options, annotation keys
// and annotation subtypes as NSString constants, and kPDFDestinationUnspecifiedValue as a CGFloat.
// The documentation gives no string values, so this test pins none: it checks that every constant
// exists, is a non-empty NSString, and that the constants of one group are pairwise distinct
// (they are used as dictionary keys and notification names, so a collision would be a bug).
// Prints OK, or the failures with a non-zero exit status.
#import <Foundation/Foundation.h>
#import <PDFKit/PDFKit.h>
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

int main(void)
{
	@autoreleasepool {
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

		// The value is not documented, so none is pinned. It stands for "no x or y", so it must be a
		// number and must not be zero, which is a real coordinate.
		double unspecified = (double)kPDFDestinationUnspecifiedValue;
		CHECK(isfinite(unspecified) && unspecified != 0, "kPDFDestinationUnspecifiedValue is not finite and non-zero");
	}
	if (failures) {
		printf("%d failure(s)\n", failures);
		return 1;
	}
	printf("OK\n");
	return 0;
}
