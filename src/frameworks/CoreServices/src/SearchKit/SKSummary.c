/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SearchKit/SKSummary.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>

// A summarization object analyzes its text once, when it is created. The sentence and
// paragraph boundaries come from CFStringTokenizer's sentence and paragraph units, so the
// results are only as consistent as the tokenizer. This depends on the darling-corefoundation
// CFStringTokenizer fixes of darlinghq/darling-corefoundation#80 (paragraph unit, text kept alive
// for the break iterator): without them the boundaries are unreliable.

struct __SKSummary {
	CFRuntimeBase base;
	CFStringRef string;
	CFRange *sentences;
	CFIndex sentenceCount;
	CFRange *paragraphs;
	CFIndex paragraphCount;
};

static CFTypeID summaryTypeID;

static void summaryFinalize(CFTypeRef cf)
{
	struct __SKSummary *summary = (struct __SKSummary *) cf;
	free(summary->sentences);
	free(summary->paragraphs);
	CFRelease(summary->string);
}

static CFStringRef summaryCopyDescription(CFTypeRef cf)
{
	struct __SKSummary *summary = (struct __SKSummary *) cf;
	return CFStringCreateWithFormat(NULL, NULL, CFSTR("<SKSummary %p sentences %ld paragraphs %ld>"),
		cf, (long) summary->sentenceCount, (long) summary->paragraphCount);
}

static const CFRuntimeClass summaryClass = {
	0, "SKSummary", NULL, NULL, summaryFinalize, NULL, NULL,
	NULL, summaryCopyDescription,
};

CFTypeID SKSummaryGetTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		summaryTypeID = _CFRuntimeRegisterClass(&summaryClass);
	});
	return summaryTypeID;
}

static Boolean hasNonWhitespace(CFStringRef string, CFRange range)
{
	CFCharacterSetRef whitespace = CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline);
	for (CFIndex i = range.location; i < range.location + range.length; i++) {
		if (!CFCharacterSetIsCharacterMember(whitespace, CFStringGetCharacterAtIndex(string, i)))
			return true;
	}
	return false;
}

// Returns the ranges of all tokens of the given unit in document order, in a malloc'ed
// array, or NULL with a zero count when the string has none. Returns false on failure.
// A unit with no non-white-space character (a blank line between paragraphs) does not count as a
// sentence or paragraph: "A.\n\nB." has 2 sentences and 2 paragraphs on macOS 26.7.3 and 10.14.4
// (black-box observation). Test: tests/src/searchkit_summary_boundaries.c
static Boolean copyTokenRanges(CFStringRef string, CFOptionFlags unit, CFRange **outRanges, CFIndex *outCount)
{
	*outRanges = NULL;
	*outCount = 0;

	CFIndex length = CFStringGetLength(string);
	if (length == 0)
		return true;

	CFStringTokenizerRef tokenizer = CFStringTokenizerCreate(NULL, string, CFRangeMake(0, length), unit, NULL);
	if (tokenizer == NULL)
		return false;

	CFRange *ranges = NULL;
	CFIndex count = 0, capacity = 0;
	Boolean ok = true;
	while (CFStringTokenizerAdvanceToNextToken(tokenizer) != kCFStringTokenizerTokenNone) {
		if (count == capacity) {
			capacity = capacity == 0 ? 16 : capacity * 2;
			CFRange *grown = realloc(ranges, capacity * sizeof(CFRange));
			if (grown == NULL) {
				ok = false;
				break;
			}
			ranges = grown;
		}
		CFRange token = CFStringTokenizerGetCurrentTokenRange(tokenizer);
		if (hasNonWhitespace(string, token))
			ranges[count++] = token;
	}
	CFRelease(tokenizer);

	if (!ok) {
		free(ranges);
		return false;
	}
	*outRanges = ranges;
	*outCount = count;
	return true;
}

static Boolean isSummary(SKSummaryRef summary)
{
	return summary != NULL && CFGetTypeID(summary) == SKSummaryGetTypeID();
}

// The pages say only that NULL is returned on failure. macOS 26.7.3 and 10.14.4 return NULL for a NULL
// text and an empty string gives a summary with no sentences or paragraphs (black-box observation);
// a non-string argument aborts there and is a failure (NULL) here.
SKSummaryRef SKSummaryCreateWithString(CFStringRef inString)
{
	if (inString == NULL || CFGetTypeID(inString) != CFStringGetTypeID())
		return NULL;

	CFStringRef string = CFStringCreateCopy(NULL, inString);
	if (string == NULL)
		return NULL;

	CFRange *sentences = NULL, *paragraphs = NULL;
	CFIndex sentenceCount, paragraphCount;
	if (!copyTokenRanges(string, kCFStringTokenizerUnitSentence, &sentences, &sentenceCount)
			|| !copyTokenRanges(string, kCFStringTokenizerUnitParagraph, &paragraphs, &paragraphCount)) {
		free(sentences);
		CFRelease(string);
		return NULL;
	}

	struct __SKSummary *summary = (struct __SKSummary *) _CFRuntimeCreateInstance(NULL,
		SKSummaryGetTypeID(), sizeof(struct __SKSummary) - sizeof(CFRuntimeBase), NULL);
	if (summary == NULL) {
		free(sentences);
		free(paragraphs);
		CFRelease(string);
		return NULL;
	}
	summary->string = string;
	summary->sentences = sentences;
	summary->sentenceCount = sentenceCount;
	summary->paragraphs = paragraphs;
	summary->paragraphCount = paragraphCount;
	return summary;
}

// An index outside the text gives NULL from the Copy...AtIndex functions (the pages say NULL on failure;
// the same on macOS 26.7.3 and 10.14.4, black-box observation). A NULL or non-summary argument crashes
// the original in every one of these functions; the count of 0 and the NULL result given here are the
// failure values the pages describe, not an observed behaviour.
// A sentence string keeps its trailing separator ("The cat sat on the mat. " includes the space), as
// the original does; a paragraph made of several sentences is returned as the original text of the
// paragraph, whereas the original joins its sentences with three spaces (not reproduced).
CFIndex SKSummaryGetSentenceCount(SKSummaryRef summary)
{
	return isSummary(summary) ? summary->sentenceCount : 0;
}

CFIndex SKSummaryGetParagraphCount(SKSummaryRef summary)
{
	return isSummary(summary) ? summary->paragraphCount : 0;
}

CFStringRef SKSummaryCopySentenceAtIndex(SKSummaryRef summary, CFIndex i)
{
	if (!isSummary(summary) || i < 0 || i >= summary->sentenceCount)
		return NULL;
	return CFStringCreateWithSubstring(NULL, summary->string, summary->sentences[i]);
}

CFStringRef SKSummaryCopyParagraphAtIndex(SKSummaryRef summary, CFIndex i)
{
	if (!isSummary(summary) || i < 0 || i >= summary->paragraphCount)
		return NULL;
	return CFStringCreateWithSubstring(NULL, summary->string, summary->paragraphs[i]);
}

// The ranking that decides which units a summary holds is not public and is not first-N: on macOS 26.7.3
// and 10.14.4 (black-box observation) the single-sentence summary of a three sentence text is its third
// sentence, and the ranks of the three sentences are 2, 3, 1. It is not implemented, so the summary strings are
// NULL (the pages say NULL on failure) and the summary info reports no unit; a placeholder order would give
// callers a wrong summary. How macOS joins the chosen units into a string is not reproduced either.
// Test: tests/src/searchkit_summary_boundaries.c
CFStringRef SKSummaryCopySentenceSummaryString(SKSummaryRef summary, CFIndex numSentences)
{
	(void) summary;
	(void) numSentences;
	return NULL;
}

CFStringRef SKSummaryCopyParagraphSummaryString(SKSummaryRef summary, CFIndex numParagraphs)
{
	(void) summary;
	(void) numParagraphs;
	return NULL;
}

CFIndex SKSummaryGetSentenceSummaryInfo(SKSummaryRef summary, CFIndex numSentencesInSummary,
	CFIndex *outRankOrderOfSentences, CFIndex *outSentenceIndexOfSentences, CFIndex *outParagraphIndexOfSentences)
{
	(void) summary;
	(void) numSentencesInSummary;
	(void) outRankOrderOfSentences;
	(void) outSentenceIndexOfSentences;
	(void) outParagraphIndexOfSentences;
	return 0;
}

CFIndex SKSummaryGetParagraphSummaryInfo(SKSummaryRef summary, CFIndex numParagraphsInSummary,
	CFIndex *outRankOrderOfParagraphs, CFIndex *outParagraphIndexOfParagraphs)
{
	(void) summary;
	(void) numParagraphsInSummary;
	(void) outRankOrderOfParagraphs;
	(void) outParagraphIndexOfParagraphs;
	return 0;
}
