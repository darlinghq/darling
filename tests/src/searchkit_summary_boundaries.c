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
// and two paragraphs. Whether a unit includes its trailing white space is not documented, so units are
// compared by prefix and by what they do not contain, except for the last unit, which ends at the end of the
// text. The ranking that chooses the summary is not public, so only the documented properties of the summary
// info are checked (a permutation of the ranks 1..N, valid and distinct unit indexes, the paragraph that
// contains each sentence), and the summary strings are checked for size, not for which units they hold.
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

// The summary of the first N units: for the sentence summaries the check is on size and on the text staying
// inside the original, so a mutation that returns the whole text or nothing fails.
static void checkSummaryString(CFStringRef string, CFIndex units, CFStringRef text, const char *name)
{
	CFIndex total = CFStringGetLength(text);
	if (units == 0) {
		CHECK(string != NULL && CFStringGetLength(string) == 0, name);
		return;
	}
	CHECK(string != NULL && CFStringGetLength(string) > 0 && CFStringGetLength(string) <= total
		&& CFStringFind(text, string, 0).location != kCFNotFound, name);
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
		CFStringRef all = SKSummaryCopySentenceSummaryString(copied, 3);
		CHECK(equals(all, text), "the summary text follows a later change of the original (summary string)");
		if (all != NULL)
			CFRelease(all);
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

	// Summary strings hold at most the requested number of units and stay inside the text.
	for (CFIndex n = 0; n <= 4; n++) {
		CFIndex units = n < 3 ? n : 3;
		CFStringRef summaryString = SKSummaryCopySentenceSummaryString(summary, n);
		checkSummaryString(summaryString, units, text, "sentence summary string");
		if (n == 1)
			CHECK(!contains(summaryString, CFSTR("Two")) && !contains(summaryString, CFSTR("Three")),
				"a one sentence summary holds more than one sentence");
		if (n >= 3)
			CHECK(equals(summaryString, text), "a summary of all or more sentences is not the whole text");
		if (summaryString != NULL)
			CFRelease(summaryString);

		units = n < 2 ? n : 2;
		CFStringRef paragraphString = SKSummaryCopyParagraphSummaryString(summary, n);
		checkSummaryString(paragraphString, units, text, "paragraph summary string");
		if (n == 1)
			CHECK(!contains(paragraphString, CFSTR("Three")), "a one paragraph summary holds the second paragraph");
		if (n >= 2)
			CHECK(equals(paragraphString, text), "a summary of all or more paragraphs is not the whole text");
		if (paragraphString != NULL)
			CFRelease(paragraphString);
	}

	// Sentence summary info: the return value is the number of sentences in the summary (at most the
	// request), the ranks are 1..N, the sentence indexes are distinct and valid, and the paragraph
	// index is the paragraph that contains the sentence (sentences 0 and 1 are in paragraph 0, 2 in 1).
	for (CFIndex n = 1; n <= 4; n++) {
		CFIndex expected = n < 3 ? n : 3;
		CFIndex ranks[4] = {0}, sentences[4] = {-1, -1, -1, -1}, paragraphs[4] = {-1, -1, -1, -1};
		CFIndex returned = SKSummaryGetSentenceSummaryInfo(summary, n, ranks, sentences, paragraphs);
		CHECK(returned == expected, "sentence summary info: wrong number of sentences");
		Boolean seenRank[5] = {false}, seenSentence[3] = {false};
		for (CFIndex i = 0; i < returned && i < 3; i++) {
			CHECK(ranks[i] >= 1 && ranks[i] <= returned && !seenRank[ranks[i]], "sentence summary info: rank is not part of 1..N");
			if (ranks[i] >= 1 && ranks[i] <= 4)
				seenRank[ranks[i]] = true;
			CHECK(sentences[i] >= 0 && sentences[i] < 3 && !seenSentence[sentences[i]], "sentence summary info: sentence index is invalid or repeated");
			if (sentences[i] >= 0 && sentences[i] < 3) {
				seenSentence[sentences[i]] = true;
				CHECK(paragraphs[i] == (sentences[i] < 2 ? 0 : 1), "sentence summary info: wrong paragraph for the sentence");
			}
		}
	}
	// Each output array may be NULL.
	CHECK(SKSummaryGetSentenceSummaryInfo(summary, 2, NULL, NULL, NULL) == 2, "sentence summary info with NULL arrays: wrong number");
	{
		CFIndex sentences[2] = {-1, -1};
		CHECK(SKSummaryGetSentenceSummaryInfo(summary, 2, NULL, sentences, NULL) == 2 && sentences[0] != -1 && sentences[1] != -1,
			"sentence summary info with only the index array: index array not filled");
	}

	// Paragraph summary info: the same for paragraphs.
	for (CFIndex n = 1; n <= 3; n++) {
		CFIndex expected = n < 2 ? n : 2;
		CFIndex ranks[3] = {0}, paragraphs[3] = {-1, -1, -1};
		CFIndex returned = SKSummaryGetParagraphSummaryInfo(summary, n, ranks, paragraphs);
		CHECK(returned == expected, "paragraph summary info: wrong number of paragraphs");
		Boolean seenRank[4] = {false}, seenParagraph[2] = {false};
		for (CFIndex i = 0; i < returned && i < 2; i++) {
			CHECK(ranks[i] >= 1 && ranks[i] <= returned && !seenRank[ranks[i]], "paragraph summary info: rank is not part of 1..N");
			if (ranks[i] >= 1 && ranks[i] <= 3)
				seenRank[ranks[i]] = true;
			CHECK(paragraphs[i] >= 0 && paragraphs[i] < 2 && !seenParagraph[paragraphs[i]], "paragraph summary info: paragraph index is invalid or repeated");
			if (paragraphs[i] >= 0 && paragraphs[i] < 2)
				seenParagraph[paragraphs[i]] = true;
		}
	}
	CHECK(SKSummaryGetParagraphSummaryInfo(summary, 2, NULL, NULL) == 2, "paragraph summary info with NULL arrays: wrong number");

	// A negative number of units behaves as 0 (GUESS, the pages do not say).
	CFStringRef negative = SKSummaryCopySentenceSummaryString(summary, -1);
	CHECK(negative != NULL && CFStringGetLength(negative) == 0, "negative N: sentence summary is not empty");
	if (negative != NULL)
		CFRelease(negative);
	negative = SKSummaryCopyParagraphSummaryString(summary, -1);
	CHECK(negative != NULL && CFStringGetLength(negative) == 0, "negative N: paragraph summary is not empty");
	if (negative != NULL)
		CFRelease(negative);
	CHECK(SKSummaryGetSentenceSummaryInfo(summary, -1, NULL, NULL, NULL) == 0, "negative N: sentence summary info is not 0");
	CHECK(SKSummaryGetParagraphSummaryInfo(summary, -1, NULL, NULL) == 0, "negative N: paragraph summary info is not 0");

	CFRelease(summary);

	// A unit with only white space, such as the blank line between two paragraphs, is not a sentence or a
	// paragraph (GUESS, the rule for blank units is not public): "A.\n\nB." has 2 sentences and 2 paragraphs.
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

	if (failures == 0)
		printf("OK\n");
	return failures == 0 ? 0 : 1;
}
