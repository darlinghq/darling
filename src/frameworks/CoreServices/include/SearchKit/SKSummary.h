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

#ifndef _SKSUMMARY_H_
#define _SKSUMMARY_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

// GUESS: the pages name the type SKSummary (SKSummaryRef in C) but not its struct tag.
typedef struct __SKSummary *SKSummaryRef;

CFTypeID SKSummaryGetTypeID(void) API_AVAILABLE(macos(10.4));

SKSummaryRef SKSummaryCreateWithString(CFStringRef inString) API_AVAILABLE(macos(10.4));

CFIndex SKSummaryGetSentenceCount(SKSummaryRef summary) API_AVAILABLE(macos(10.4));
CFIndex SKSummaryGetParagraphCount(SKSummaryRef summary) API_AVAILABLE(macos(10.4));

CFStringRef SKSummaryCopySentenceAtIndex(SKSummaryRef summary, CFIndex i) API_AVAILABLE(macos(10.4));
CFStringRef SKSummaryCopyParagraphAtIndex(SKSummaryRef summary, CFIndex i) API_AVAILABLE(macos(10.4));

CFStringRef SKSummaryCopySentenceSummaryString(SKSummaryRef summary, CFIndex numSentences) API_AVAILABLE(macos(10.4));
CFStringRef SKSummaryCopyParagraphSummaryString(SKSummaryRef summary, CFIndex numParagraphs) API_AVAILABLE(macos(10.4));

CFIndex SKSummaryGetSentenceSummaryInfo(SKSummaryRef summary, CFIndex numSentencesInSummary,
	CFIndex *outRankOrderOfSentences, CFIndex *outSentenceIndexOfSentences,
	CFIndex *outParagraphIndexOfSentences) API_AVAILABLE(macos(10.4));
CFIndex SKSummaryGetParagraphSummaryInfo(SKSummaryRef summary, CFIndex numParagraphsInSummary,
	CFIndex *outRankOrderOfParagraphs, CFIndex *outParagraphIndexOfParagraphs) API_AVAILABLE(macos(10.4));

#ifdef __cplusplus
}
#endif

#endif
