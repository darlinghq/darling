// CFLAGS: -framework CoreServices -framework CoreFoundation
// Copyright (C) 2026 Darling developers
//
// Search Kit summarization objects on a text with sentences and paragraphs. The sentence and paragraph
// boundaries are CFStringTokenizer's sentence and paragraph units, so these expectations need the
// darling-corefoundation CFStringTokenizer fixes (darlinghq/darling-corefoundation#80: paragraph unit, text
// kept alive for the break iterator). On the current submodule pin all sentence and paragraph boundary
// results are unreliable, not only the paragraph ones, and this test needs #80. The cases that do not need
// the boundaries are in searchkit_summary_basic.c.
//
// The text is "One. Two.\nThree.": a line feed ends a paragraph and a sentence, so it has three sentences
// and two paragraphs. The ranking that chooses the summary is not public and is not first-N (macOS 26.7.3
// and 10.14.4 return the third sentence for a one sentence summary of a three sentence text), so the summary
// strings and the summary info are not implemented and are checked to report nothing. The sentence units
// are pinned to what macOS reports for the same texts (black-box observation): they keep their trailing
// separators, and a blank line between paragraphs is not a unit.
// Prints OK, or the failures with a non-zero exit status.
#include <CoreServices/CoreServices.h>
#include <stdio.h>

static int failures;

#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } } while (0)

static Boolean hasPrefix(CFStringRef string, CFStringRef prefix)
{
	return string != NULL && CFStringHasPrefix(string, prefix);
}

static Boolean contains(CFStringRef string, CFStringRef needle)
{
	return string != NULL && CFStringFind(string, needle, 0).location != kCFNotFound;
}

static Boolean equals(CFStringRef string, CFStringRef expected)
{
	return string != NULL && CFStringCompare(string, expected, 0) == kCFCompareEqualTo;
}

int main(void)
{
	CFStringRef text = CFSTR("One. Two.\nThree.");
	SKSummaryRef summary = SKSummaryCreateWithString(text);
	if (summary == NULL) {
		printf("FAIL: create returned NULL\n");
		return 1;
	}

	CHECK(SKSummaryGetSentenceCount(summary) == 3, "sentence count is not 3");
	CHECK(SKSummaryGetParagraphCount(summary) == 2, "paragraph count is not 2");

	// The text of the object is a copy: changing the start of the original after Create does not change the summary.
	CFMutableStringRef mutableText = CFStringCreateMutableCopy(NULL, 0, text);
	SKSummaryRef copied = SKSummaryCreateWithString(mutableText);
	CFStringInsert(mutableText, 0, CFSTR("Zero. "));
	CHECK(copied != NULL && SKSummaryGetSentenceCount(copied) == 3, "the summary sentence count follows a later change of the original");
	if (copied != NULL) {
		CFStringRef first = SKSummaryCopySentenceAtIndex(copied, 0);
		CHECK(hasPrefix(first, CFSTR("One.")), "the summary text follows a later change of the original (sentence 0)");
		if (first != NULL)
			CFRelease(first);
		CFRelease(copied);
	}
	CFRelease(mutableText);

	// Sentences, zero-based.
	CFStringRef sentence0 = SKSummaryCopySentenceAtIndex(summary, 0);
	CFStringRef sentence1 = SKSummaryCopySentenceAtIndex(summary, 1);
	CFStringRef sentence2 = SKSummaryCopySentenceAtIndex(summary, 2);
	CHECK(hasPrefix(sentence0, CFSTR("One.")) && !contains(sentence0, CFSTR("Two")), "sentence 0 is not One.");
	CHECK(hasPrefix(sentence1, CFSTR("Two.")) && !contains(sentence1, CFSTR("Three")), "sentence 1 is not Two.");
	CHECK(equals(sentence2, CFSTR("Three.")), "sentence 2 is not Three.");
	CHECK(SKSummaryCopySentenceAtIndex(summary, 3) == NULL, "sentence 3 exists");
	CHECK(SKSummaryCopySentenceAtIndex(summary, -1) == NULL, "sentence -1 exists");
	if (sentence0 != NULL) CFRelease(sentence0);
	if (sentence1 != NULL) CFRelease(sentence1);
	if (sentence2 != NULL) CFRelease(sentence2);

	// Paragraphs, zero-based.
	CFStringRef paragraph0 = SKSummaryCopyParagraphAtIndex(summary, 0);
	CFStringRef paragraph1 = SKSummaryCopyParagraphAtIndex(summary, 1);
	CHECK(hasPrefix(paragraph0, CFSTR("One. Two.")) && !contains(paragraph0, CFSTR("Three")), "paragraph 0 is not One. Two.");
	CHECK(equals(paragraph1, CFSTR("Three.")), "paragraph 1 is not Three.");
	CHECK(SKSummaryCopyParagraphAtIndex(summary, 2) == NULL, "paragraph 2 exists");
	CHECK(SKSummaryCopyParagraphAtIndex(summary, -1) == NULL, "paragraph -1 exists");
	if (paragraph0 != NULL) CFRelease(paragraph0);
	if (paragraph1 != NULL) CFRelease(paragraph1);

	// Summary strings and summary info are not implemented (the ranking is unknown): NULL and no unit, for
	// every number of units, negative and larger than the text included.
	for (CFIndex n = -1; n <= 4; n++) {
		CFIndex ranks[4] = {0}, sentences[4] = {0}, paragraphs[4] = {0};
		CHECK(SKSummaryCopySentenceSummaryString(summary, n) == NULL, "sentence summary string is not NULL");
		CHECK(SKSummaryCopyParagraphSummaryString(summary, n) == NULL, "paragraph summary string is not NULL");
		CHECK(SKSummaryGetSentenceSummaryInfo(summary, n, ranks, sentences, paragraphs) == 0, "sentence summary info is not 0");
		CHECK(SKSummaryGetParagraphSummaryInfo(summary, n, ranks, paragraphs) == 0, "paragraph summary info is not 0");
		CHECK(SKSummaryGetSentenceSummaryInfo(summary, n, NULL, NULL, NULL) == 0, "sentence summary info with NULL arrays is not 0");
		CHECK(SKSummaryGetParagraphSummaryInfo(summary, n, NULL, NULL) == 0, "paragraph summary info with NULL arrays is not 0");
	}

	CFRelease(summary);

	// A unit with only white space, such as the blank line between two paragraphs, is not a sentence or a
	// paragraph: "A.\n\nB." has 2 sentences and 2 paragraphs on macOS 26.7.3 and 10.14.4.
	CFStringRef blankText = CFSTR("A.\n\nB.");
	SKSummaryRef blank = SKSummaryCreateWithString(blankText);
	if (blank == NULL) {
		printf("FAIL: create with a blank line returned NULL\n");
		return 1;
	}
	CHECK(SKSummaryGetSentenceCount(blank) == 2, "blank line: sentence count is not 2");
	CHECK(SKSummaryGetParagraphCount(blank) == 2, "blank line: paragraph count is not 2");
	CFStringRef blankSentence0 = SKSummaryCopySentenceAtIndex(blank, 0);
	CFStringRef blankSentence1 = SKSummaryCopySentenceAtIndex(blank, 1);
	CHECK(hasPrefix(blankSentence0, CFSTR("A.")), "blank line: sentence 0 is not A.");
	CHECK(equals(blankSentence1, CFSTR("B.")), "blank line: sentence 1 is not B.");
	CFStringRef blankParagraph0 = SKSummaryCopyParagraphAtIndex(blank, 0);
	CFStringRef blankParagraph1 = SKSummaryCopyParagraphAtIndex(blank, 1);
	CHECK(hasPrefix(blankParagraph0, CFSTR("A.")), "blank line: paragraph 0 is not A.");
	CHECK(equals(blankParagraph1, CFSTR("B.")), "blank line: paragraph 1 is not B.");
	if (blankSentence0 != NULL) CFRelease(blankSentence0);
	if (blankSentence1 != NULL) CFRelease(blankSentence1);
	if (blankParagraph0 != NULL) CFRelease(blankParagraph0);
	if (blankParagraph1 != NULL) CFRelease(blankParagraph1);
	CFRelease(blank);

	// Texts and results observed on macOS 26.7.3 and 10.14.4: sentences keep their trailing separators.
	SKSummaryRef three = SKSummaryCreateWithString(CFSTR("The cat sat on the mat. Dogs bark loudly at night. The cat then slept all day long in the warm sun."));
	CHECK(three != NULL && SKSummaryGetSentenceCount(three) == 3 && SKSummaryGetParagraphCount(three) == 1, "three sentences: counts are not 3 and 1");
	if (three != NULL) {
		CFStringRef sentence = SKSummaryCopySentenceAtIndex(three, 0);
		CHECK(equals(sentence, CFSTR("The cat sat on the mat. ")), "three sentences: sentence 0 lost its trailing space");
		if (sentence != NULL) CFRelease(sentence);
		sentence = SKSummaryCopySentenceAtIndex(three, 1);
		CHECK(equals(sentence, CFSTR("Dogs bark loudly at night. ")), "three sentences: sentence 1 lost its trailing space");
		if (sentence != NULL) CFRelease(sentence);
		sentence = SKSummaryCopySentenceAtIndex(three, 2);
		CHECK(equals(sentence, CFSTR("The cat then slept all day long in the warm sun.")), "three sentences: sentence 2");
		if (sentence != NULL) CFRelease(sentence);
		CFRelease(three);
	}
	SKSummaryRef paras = SKSummaryCreateWithString(CFSTR("First paragraph is here. It has two sentences.\n\nSecond paragraph is short.\n\nThird paragraph, the longest of them all, talks about cats and dogs and mats at length."));
	CHECK(paras != NULL && SKSummaryGetSentenceCount(paras) == 4 && SKSummaryGetParagraphCount(paras) == 3, "three paragraphs: counts are not 4 and 3");
	if (paras != NULL) {
		CFStringRef sentence = SKSummaryCopySentenceAtIndex(paras, 1);
		CHECK(equals(sentence, CFSTR("It has two sentences.\n")), "three paragraphs: sentence 1 lost its trailing line feed");
		if (sentence != NULL) CFRelease(sentence);
		sentence = SKSummaryCopySentenceAtIndex(paras, 2);
		CHECK(equals(sentence, CFSTR("Second paragraph is short.\n")), "three paragraphs: sentence 2 lost its trailing line feed");
		if (sentence != NULL) CFRelease(sentence);
		CFStringRef paragraph = SKSummaryCopyParagraphAtIndex(paras, 1);
		CHECK(equals(paragraph, CFSTR("Second paragraph is short.\n")), "three paragraphs: paragraph 1");
		if (paragraph != NULL) CFRelease(paragraph);
		CFRelease(paras);
	}
	SKSummaryRef one = SKSummaryCreateWithString(CFSTR("Cats purr."));
	CHECK(one != NULL && SKSummaryGetSentenceCount(one) == 1 && SKSummaryGetParagraphCount(one) == 1, "one sentence: counts are not 1 and 1");
	if (one != NULL) {
		CFStringRef sentence = SKSummaryCopySentenceAtIndex(one, 0);
		CHECK(equals(sentence, CFSTR("Cats purr.")), "one sentence: sentence 0");
		if (sentence != NULL) CFRelease(sentence);
		CFRelease(one);
	}

	if (failures == 0)
		printf("OK\n");
	return failures == 0 ? 0 : 1;
}
