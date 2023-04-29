/*
 * This file is part of Darling.
 *
 * Copyright (C) 2023 Darling Developers
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

#import <objc/NSObject.h>

@interface NSString : NSObject
@end

#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void __simple_kprintf(const char* format, ...) __attribute__((format(printf, 1, 2)));

#define LOG_FUNC __simple_kprintf

NSString *const kCTFontAttributeName = @"kCTFontAttributeName";
NSString *const kCTFontFamilyNameAttribute = @"kCTFontFamilyNameAttribute";
NSString *const kCTFontNameAttribute = @"kCTFontNameAttribute";
NSString *const kCTFontStyleNameAttribute = @"kCTFontStyleNameAttribute";
NSString *const kCTForegroundColorAttributeName = @"kCTForegroundColorAttributeName";
NSString *const kCTFrameIntegerLineMetricsAttributeName = @"kCTFrameIntegerLineMetricsAttributeName";
NSString *const kCTKernAttributeName = @"kCTKernAttributeName";
NSString *const kCTParagraphStyleAttributeName = @"kCTParagraphStyleAttributeName";
NSString *const kCTRunDelegateAttributeName = @"kCTRunDelegateAttributeName";
NSString *const kCTUnderlineColorAttributeName = @"kCTUnderlineColorAttributeName";
NSString *const kCTUnderlineStyleAttributeName = @"kCTUnderlineStyleAttributeName";

void* CTFontCopyFamilyName(void) {
	if (verbose) LOG_FUNC("STUB: CTFontCopyFamilyName called\n");
	return NULL;
};

void* CTFontCopyFullName(void) {
	if (verbose) LOG_FUNC("STUB: CTFontCopyFullName called\n");
	return NULL;
};

void* CTFontCreatePathForGlyph(void) {
	if (verbose) LOG_FUNC("STUB: CTFontCreatePathForGlyph called\n");
	return NULL;
};

void* CTFontCreateWithName(void) {
	if (verbose) LOG_FUNC("STUB: CTFontCreateWithName called\n");
	return NULL;
};

void* CTFontDescriptorCopyAttribute(void) {
	if (verbose) LOG_FUNC("STUB: CTFontDescriptorCopyAttribute called\n");
	return NULL;
};

void* CTFontDescriptorCopyLocalizedAttribute(void) {
	if (verbose) LOG_FUNC("STUB: CTFontDescriptorCopyLocalizedAttribute called\n");
	return NULL;
};

void* CTFontDescriptorCreateMatchingFontDescriptors(void) {
	if (verbose) LOG_FUNC("STUB: CTFontDescriptorCreateMatchingFontDescriptors called\n");
	return NULL;
};

void* CTFontDescriptorCreateWithAttributes(void) {
	if (verbose) LOG_FUNC("STUB: CTFontDescriptorCreateWithAttributes called\n");
	return NULL;
};

void* CTFontDrawGlyphs(void) {
	if (verbose) LOG_FUNC("STUB: CTFontDrawGlyphs called\n");
	return NULL;
};

void* CTFontGetAdvancesForGlyphs(void) {
	if (verbose) LOG_FUNC("STUB: CTFontGetAdvancesForGlyphs called\n");
	return NULL;
};

void* CTFontGetGlyphsForCharacters(void) {
	if (verbose) LOG_FUNC("STUB: CTFontGetGlyphsForCharacters called\n");
	return NULL;
};

void* CTFontGetSymbolicTraits(void) {
	if (verbose) LOG_FUNC("STUB: CTFontGetSymbolicTraits called\n");
	return NULL;
};

void* CTFontManagerRegisterFontsForURL(void) {
	if (verbose) LOG_FUNC("STUB: CTFontManagerRegisterFontsForURL called\n");
	return NULL;
};

void* CTFrameDraw(void) {
	if (verbose) LOG_FUNC("STUB: CTFrameDraw called\n");
	return NULL;
};

void* CTFrameGetLineOrigins(void) {
	if (verbose) LOG_FUNC("STUB: CTFrameGetLineOrigins called\n");
	return NULL;
};

void* CTFrameGetLines(void) {
	if (verbose) LOG_FUNC("STUB: CTFrameGetLines called\n");
	return NULL;
};

void* CTFramesetterCreateFrame(void) {
	if (verbose) LOG_FUNC("STUB: CTFramesetterCreateFrame called\n");
	return NULL;
};

void* CTFramesetterCreateWithAttributedString(void) {
	if (verbose) LOG_FUNC("STUB: CTFramesetterCreateWithAttributedString called\n");
	return NULL;
};

void* CTFramesetterSuggestFrameSizeWithConstraints(void) {
	if (verbose) LOG_FUNC("STUB: CTFramesetterSuggestFrameSizeWithConstraints called\n");
	return NULL;
};

void* CTLineCreateTruncatedLine(void) {
	if (verbose) LOG_FUNC("STUB: CTLineCreateTruncatedLine called\n");
	return NULL;
};

void* CTLineCreateWithAttributedString(void) {
	if (verbose) LOG_FUNC("STUB: CTLineCreateWithAttributedString called\n");
	return NULL;
};

void* CTLineDraw(void) {
	if (verbose) LOG_FUNC("STUB: CTLineDraw called\n");
	return NULL;
};

void* CTLineGetBoundsWithOptions(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetBoundsWithOptions called\n");
	return NULL;
};

void* CTLineGetGlyphRuns(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetGlyphRuns called\n");
	return NULL;
};

void* CTLineGetImageBounds(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetImageBounds called\n");
	return NULL;
};

void* CTLineGetOffsetForStringIndex(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetOffsetForStringIndex called\n");
	return NULL;
};

void* CTLineGetStringIndexForPosition(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetStringIndexForPosition called\n");
	return NULL;
};

void* CTLineGetStringRange(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetStringRange called\n");
	return NULL;
};

void* CTLineGetTrailingWhitespaceWidth(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetTrailingWhitespaceWidth called\n");
	return NULL;
};

void* CTLineGetTypographicBounds(void) {
	if (verbose) LOG_FUNC("STUB: CTLineGetTypographicBounds called\n");
	return NULL;
};

void* CTParagraphStyleCreate(void) {
	if (verbose) LOG_FUNC("STUB: CTParagraphStyleCreate called\n");
	return NULL;
};

void* CTRunDelegateCreate(void) {
	if (verbose) LOG_FUNC("STUB: CTRunDelegateCreate called\n");
	return NULL;
};

void* CTRunDraw(void) {
	if (verbose) LOG_FUNC("STUB: CTRunDraw called\n");
	return NULL;
};

void* CTRunGetAttributes(void) {
	if (verbose) LOG_FUNC("STUB: CTRunGetAttributes called\n");
	return NULL;
};

void* CTRunGetGlyphCount(void) {
	if (verbose) LOG_FUNC("STUB: CTRunGetGlyphCount called\n");
	return NULL;
};

void* CTRunGetGlyphs(void) {
	if (verbose) LOG_FUNC("STUB: CTRunGetGlyphs called\n");
	return NULL;
};

void* CTRunGetPositions(void) {
	if (verbose) LOG_FUNC("STUB: CTRunGetPositions called\n");
	return NULL;
};

void* CTTypesetterCreateLineWithOffset(void) {
	if (verbose) LOG_FUNC("STUB: CTTypesetterCreateLineWithOffset called\n");
	return NULL;
};

void* CTTypesetterCreateWithAttributedString(void) {
	if (verbose) LOG_FUNC("STUB: CTTypesetterCreateWithAttributedString called\n");
	return NULL;
};

void* CTTypesetterSuggestClusterBreakWithOffset(void) {
	if (verbose) LOG_FUNC("STUB: CTTypesetterSuggestClusterBreakWithOffset called\n");
	return NULL;
};


