// CFLAGS: -framework CoreServices -framework CoreFoundation
// Copyright (C) 2026 Darling developers
//
// Search Kit summarization objects: the type, the Create and NULL handling, and an empty text. None of
// this depends on CFStringTokenizer's sentence and paragraph boundaries (an empty text has none, and is
// not handed to the tokenizer), so it does not need the darling-corefoundation CFStringTokenizer fixes
// (darlinghq/darling-corefoundation#80).
// The sentence and paragraph cases are in searchkit_summary_boundaries.c. Prints OK, or the failures with
// a non-zero exit status.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

int main(void)
{
	// The type id is stable, is not the id of another CF type and identifies the objects the Create function returns.
	CFTypeID type = SKSummaryGetTypeID();
	CHECK(type != 0, "type id is 0");
	CHECK(type == SKSummaryGetTypeID(), "type id changes between calls");
	CHECK(type != CFStringGetTypeID() && type != CFDictionaryGetTypeID() && type != CFArrayGetTypeID(),
		"type id equals the id of another CF type");

	// Create returns NULL for a missing text.
	CHECK(SKSummaryCreateWithString(NULL) == NULL, "create with NULL text is not NULL");

	// An empty text is not a failure and has nothing in it (macOS 26.7.3 and 10.14.4: a summary with 0 sentences
	// and 0 paragraphs, index -1 and 0 give NULL).
	SKSummaryRef empty = SKSummaryCreateWithString(CFSTR(""));
	CHECK(empty != NULL, "create with an empty text returned NULL");
	if (empty != NULL) {
		CHECK(CFGetTypeID(empty) == type, "summary has another type id than SKSummaryGetTypeID");
		CHECK(SKSummaryGetSentenceCount(empty) == 0, "empty text: sentence count is not 0");
		CHECK(SKSummaryGetParagraphCount(empty) == 0, "empty text: paragraph count is not 0");
		CHECK(SKSummaryCopySentenceAtIndex(empty, 0) == NULL, "empty text: sentence 0 exists");
		CHECK(SKSummaryCopyParagraphAtIndex(empty, 0) == NULL, "empty text: paragraph 0 exists");
		CHECK(SKSummaryCopySentenceAtIndex(empty, -1) == NULL, "empty text: sentence -1 exists");
		CHECK(SKSummaryCopyParagraphAtIndex(empty, -1) == NULL, "empty text: paragraph -1 exists");

		// The summary has no units, so the sentence and paragraph summaries cover none.
		CFIndex ranks[4], indexes[4], paragraphs[4];
		CHECK(SKSummaryGetSentenceSummaryInfo(empty, 3, ranks, indexes, paragraphs) == 0,
			"empty text: sentence summary info is not 0");
		CHECK(SKSummaryGetParagraphSummaryInfo(empty, 3, ranks, indexes) == 0,
			"empty text: paragraph summary info is not 0");
		CHECK(SKSummaryGetSentenceSummaryInfo(empty, 3, NULL, NULL, NULL) == 0,
			"empty text: sentence summary info with NULL arrays is not 0");
		CHECK(SKSummaryGetParagraphSummaryInfo(empty, 3, NULL, NULL) == 0,
			"empty text: paragraph summary info with NULL arrays is not 0");
		CFStringRef sentences = SKSummaryCopySentenceSummaryString(empty, 3);
		CHECK(sentences == NULL || CFStringGetLength(sentences) == 0, "empty text: sentence summary is not empty");
		if (sentences != NULL)
			CFRelease(sentences);
		CFStringRef paragraphsString = SKSummaryCopyParagraphSummaryString(empty, 3);
		CHECK(paragraphsString == NULL || CFStringGetLength(paragraphsString) == 0, "empty text: paragraph summary is not empty");
		if (paragraphsString != NULL)
			CFRelease(paragraphsString);

		// Summary strings and summary info are not implemented (the ranking is not known), also for a
		// negative number of units; no unit is in the summary info.
		CFStringRef negative = SKSummaryCopySentenceSummaryString(empty, -1);
		CHECK(negative == NULL || CFStringGetLength(negative) == 0, "negative N: sentence summary is not empty");
		if (negative != NULL)
			CFRelease(negative);
		negative = SKSummaryCopyParagraphSummaryString(empty, -1);
		CHECK(negative == NULL || CFStringGetLength(negative) == 0, "negative N: paragraph summary is not empty");
		if (negative != NULL)
			CFRelease(negative);
		CHECK(SKSummaryGetSentenceSummaryInfo(empty, -1, ranks, indexes, paragraphs) == 0,
			"negative N: sentence summary info is not 0");
		CHECK(SKSummaryGetParagraphSummaryInfo(empty, -1, ranks, indexes) == 0,
			"negative N: paragraph summary info is not 0");

		// The object follows the CF retain and release rules.
		CFIndex retains = CFGetRetainCount(empty);
		CHECK(CFRetain(empty) == empty, "CFRetain returned another object");
		CHECK(CFGetRetainCount(empty) == retains + 1, "CFRetain did not raise the retain count");
		CFRelease(empty);
		CHECK(CFGetRetainCount(empty) == retains, "CFRelease did not lower the retain count");
		CFStringRef description = CFCopyDescription(empty);
		CHECK(description != NULL, "CFCopyDescription returned NULL");
		if (description != NULL)
			CFRelease(description);
		CFRelease(empty);
	}

	// A NULL summary has no sentences or paragraphs and must not crash. The original crashes for a NULL
	// summary; the failure values here are chosen, not observed.
	CHECK(SKSummaryGetSentenceCount(NULL) == 0, "NULL summary: sentence count is not 0");
	CHECK(SKSummaryGetParagraphCount(NULL) == 0, "NULL summary: paragraph count is not 0");
	CHECK(SKSummaryCopySentenceAtIndex(NULL, 0) == NULL, "NULL summary: sentence 0 exists");
	CHECK(SKSummaryCopyParagraphAtIndex(NULL, 0) == NULL, "NULL summary: paragraph 0 exists");
	CHECK(SKSummaryCopySentenceSummaryString(NULL, 1) == NULL, "NULL summary: sentence summary string is not NULL");
	CHECK(SKSummaryCopyParagraphSummaryString(NULL, 1) == NULL, "NULL summary: paragraph summary string is not NULL");
	CHECK(SKSummaryGetSentenceSummaryInfo(NULL, 1, NULL, NULL, NULL) == 0, "NULL summary: sentence summary info is not 0");
	CHECK(SKSummaryGetParagraphSummaryInfo(NULL, 1, NULL, NULL) == 0, "NULL summary: paragraph summary info is not 0");

	if (failures == 0)
		printf("OK\n");
	return failures == 0 ? 0 : 1;
}
