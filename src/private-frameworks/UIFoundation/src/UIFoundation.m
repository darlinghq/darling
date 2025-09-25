/*
 This file is part of Darling.

 Copyright (C) 2025 Darling Developers

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


#include <UIFoundation/UIFoundation.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}

void* CFArrayCreateWithNonRetainedObjectsFromNSArray(void)
{
    if (verbose) puts("STUB: CFArrayCreateWithNonRetainedObjectsFromNSArray called");
    return NULL;
}

void* CFDictionaryCreateWithNonRetainedValuesFromNSDictionary(void)
{
    if (verbose) puts("STUB: CFDictionaryCreateWithNonRetainedValuesFromNSDictionary called");
    return NULL;
}

void* NSConvertGlyphsToPackedGlyphs(void)
{
    if (verbose) puts("STUB: NSConvertGlyphsToPackedGlyphs called");
    return NULL;
}

void* NSDefaultFont(void)
{
    if (verbose) puts("STUB: NSDefaultFont called");
    return NULL;
}

void* NSStringFromCGAffineTransform(void)
{
    if (verbose) puts("STUB: NSStringFromCGAffineTransform called");
    return NULL;
}

void* NSTextAlignmentFromCTTextAlignment(void)
{
    if (verbose) puts("STUB: NSTextAlignmentFromCTTextAlignment called");
    return NULL;
}

void* NSTextAlignmentToCTTextAlignment(void)
{
    if (verbose) puts("STUB: NSTextAlignmentToCTTextAlignment called");
    return NULL;
}

void* UIAppendBytesForValueToData(void)
{
    if (verbose) puts("STUB: UIAppendBytesForValueToData called");
    return NULL;
}

void* UIAppendVInt32ToData(void)
{
    if (verbose) puts("STUB: UIAppendVInt32ToData called");
    return NULL;
}

void* UIArrayByKeepingObjectsInSet(void)
{
    if (verbose) puts("STUB: UIArrayByKeepingObjectsInSet called");
    return NULL;
}

void* UICreateOrderedAndStrippedCoderValues(void)
{
    if (verbose) puts("STUB: UICreateOrderedAndStrippedCoderValues called");
    return NULL;
}

void* UIDataLooksLikeNibArchive(void)
{
    if (verbose) puts("STUB: UIDataLooksLikeNibArchive called");
    return NULL;
}

void* UIDistanceBetweenPointAndRect(void)
{
    if (verbose) puts("STUB: UIDistanceBetweenPointAndRect called");
    return NULL;
}

void* UIFixedByteLengthForType(void)
{
    if (verbose) puts("STUB: UIFixedByteLengthForType called");
    return NULL;
}

void* UINibArchiveIndexFromNumber(void)
{
    if (verbose) puts("STUB: UINibArchiveIndexFromNumber called");
    return NULL;
}

void* UINibCoderValueTypeToString(void)
{
    if (verbose) puts("STUB: UINibCoderValueTypeToString called");
    return NULL;
}

void* UINumberWithNibArchiveIndex(void)
{
    if (verbose) puts("STUB: UINumberWithNibArchiveIndex called");
    return NULL;
}

void* UIRetainedIdentityKeyDictionaryCallbacks(void)
{
    if (verbose) puts("STUB: UIRetainedIdentityKeyDictionaryCallbacks called");
    return NULL;
}

void* UIRetainedIdentitySetCallbacks(void)
{
    if (verbose) puts("STUB: UIRetainedIdentitySetCallbacks called");
    return NULL;
}

void* UIRetainedIdentityValueDictionaryCallbacks(void)
{
    if (verbose) puts("STUB: UIRetainedIdentityValueDictionaryCallbacks called");
    return NULL;
}

void* UIWriteArchiveToData(void)
{
    if (verbose) puts("STUB: UIWriteArchiveToData called");
    return NULL;
}

void* _NSAddDirtyLayoutManager(void)
{
    if (verbose) puts("STUB: _NSAddDirtyLayoutManager called");
    return NULL;
}

void* _NSAttachmentCharacterSet(void)
{
    if (verbose) puts("STUB: _NSAttachmentCharacterSet called");
    return NULL;
}

void* _NSBidiControlCharacterSet(void)
{
    if (verbose) puts("STUB: _NSBidiControlCharacterSet called");
    return NULL;
}

void* _NSBidiEmbeddingAndOverrideCharSet(void)
{
    if (verbose) puts("STUB: _NSBidiEmbeddingAndOverrideCharSet called");
    return NULL;
}

void* _NSBlockNumberForIndex(void)
{
    if (verbose) puts("STUB: _NSBlockNumberForIndex called");
    return NULL;
}

void* _NSCalculateContainerOrigin(void)
{
    if (verbose) puts("STUB: _NSCalculateContainerOrigin called");
    return NULL;
}

void* _NSClearGlyphIndexForPointCache(void)
{
    if (verbose) puts("STUB: _NSClearGlyphIndexForPointCache called");
    return NULL;
}

void* _NSFastFillAllGlyphHolesForCharacterRange(void)
{
    if (verbose) puts("STUB: _NSFastFillAllGlyphHolesForCharacterRange called");
    return NULL;
}

void* _NSFastFillAllGlyphHolesForGlyphRange(void)
{
    if (verbose) puts("STUB: _NSFastFillAllGlyphHolesForGlyphRange called");
    return NULL;
}

void* _NSFastFillAllGlyphHolesUpToGlyphIndex(void)
{
    if (verbose) puts("STUB: _NSFastFillAllGlyphHolesUpToGlyphIndex called");
    return NULL;
}

void* _NSFastFillAllLayoutHolesForGlyphRange(void)
{
    if (verbose) puts("STUB: _NSFastFillAllLayoutHolesForGlyphRange called");
    return NULL;
}

void* _NSFastFillAllLayoutHolesUpToEndOfContainerForGlyphIndex(void)
{
    if (verbose) puts("STUB: _NSFastFillAllLayoutHolesUpToEndOfContainerForGlyphIndex called");
    return NULL;
}

void* _NSFastFillAllLayoutHolesUpToGlyphIndex(void)
{
    if (verbose) puts("STUB: _NSFastFillAllLayoutHolesUpToGlyphIndex called");
    return NULL;
}

void* _NSFontAttributeNames(void)
{
    if (verbose) puts("STUB: _NSFontAttributeNames called");
    return NULL;
}

void* _NSGetAppKitVersionNumber(void)
{
    if (verbose) puts("STUB: _NSGetAppKitVersionNumber called");
    return NULL;
}

void* _NSHTMLEncoding(void)
{
    if (verbose) puts("STUB: _NSHTMLEncoding called");
    return NULL;
}

void* _NSLayoutManagerLogDebug(void)
{
    if (verbose) puts("STUB: _NSLayoutManagerLogDebug called");
    return NULL;
}

void* _NSNonAttachmentCharacterSet(void)
{
    if (verbose) puts("STUB: _NSNonAttachmentCharacterSet called");
    return NULL;
}

void* _NSReadAttributedStringFromData(void)
{
    if (verbose) puts("STUB: _NSReadAttributedStringFromData called");
    return NULL;
}

void* _NSReadAttributedStringFromURL(void)
{
    if (verbose) puts("STUB: _NSReadAttributedStringFromURL called");
    return NULL;
}

void* _NSReadAttributedStringFromURLOrData(void)
{
    if (verbose) puts("STUB: _NSReadAttributedStringFromURLOrData called");
    return NULL;
}

void* _NSRemoveDirtyLayoutManager(void)
{
    if (verbose) puts("STUB: _NSRemoveDirtyLayoutManager called");
    return NULL;
}

void* _NSStringDrawingCore(void)
{
    if (verbose) puts("STUB: _NSStringDrawingCore called");
    return NULL;
}

void* _NSStringHasRightToLeftBaseWritingDirectionAtIndex(void)
{
    if (verbose) puts("STUB: _NSStringHasRightToLeftBaseWritingDirectionAtIndex called");
    return NULL;
}

void* _NSStringHasRightToLeftCharactersInRange(void)
{
    if (verbose) puts("STUB: _NSStringHasRightToLeftCharactersInRange called");
    return NULL;
}

void* _NSStringImputedBaseWritingDirectionAtIndex(void)
{
    if (verbose) puts("STUB: _NSStringImputedBaseWritingDirectionAtIndex called");
    return NULL;
}

void* _NSUsedRectFromStoredLineFragment(void)
{
    if (verbose) puts("STUB: _NSUsedRectFromStoredLineFragment called");
    return NULL;
}

void* _NSXMLEncoding(void)
{
    if (verbose) puts("STUB: _NSXMLEncoding called");
    return NULL;
}

void* __NSGetNSAppearanceClass(void)
{
    if (verbose) puts("STUB: __NSGetNSAppearanceClass called");
    return NULL;
}

void* __NSTokenizerLanguageSet(void)
{
    if (verbose) puts("STUB: __NSTokenizerLanguageSet called");
    return NULL;
}

void* mutableDictionaryByTransformingLeafDictionariesToWeakValued(void)
{
    if (verbose) puts("STUB: mutableDictionaryByTransformingLeafDictionariesToWeakValued called");
    return NULL;
}
