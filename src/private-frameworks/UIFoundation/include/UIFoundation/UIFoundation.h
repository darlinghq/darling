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


#ifndef _UIFoundation_H_
#define _UIFoundation_H_

#import <Foundation/Foundation.h>

#import <UIFoundation/NSLayoutManagerDelegate.h>
#import <UIFoundation/NSProgressReporting.h>
#import <UIFoundation/NSTextAttachmentContainer.h>
#import <UIFoundation/NSTextLayoutOrientationProvider.h>
#import <UIFoundation/NSXMLParserDelegate.h>
#import <UIFoundation/NSFontAssetRequest.h>
#import <UIFoundation/NSCollectionViewData.h>
#import <UIFoundation/NSCoreTypesetter.h>
#import <UIFoundation/NSExtraLMData.h>
#import <UIFoundation/NSLayoutManagerTextBlockHelper.h>
#import <UIFoundation/NSLayoutManagerTextBlockRowArrayCache.h>
#import <UIFoundation/NSLayoutManager.h>
#import <UIFoundation/UINibCoderValue.h>
#import <UIFoundation/NSStorage.h>
#import <UIFoundation/NSStringDrawingContext.h>
#import <UIFoundation/_NSCachedAttributedString.h>
#import <UIFoundation/NSStringDrawingTextStorage.h>
#import <UIFoundation/NSStringDrawingTextStorageSettings.h>
#import <UIFoundation/NSZipTextReader.h>
#import <UIFoundation/_NSCollectionViewItemKey.h>
#import <UIFoundation/_UICollectionViewTrackedValueItem.h>
#import <UIFoundation/_UICollectionViewTrackedValue.h>
#import <UIFoundation/UICollectionViewAnimation.h>
#import <UIFoundation/NSTextAlternatives.h>
#import <UIFoundation/UICollectionViewLayoutAttributesAuxiliary.h>
#import <UIFoundation/NSCollectionViewLayoutAttributes.h>
#import <UIFoundation/NSCollectionViewLayoutInvalidationContext.h>
#import <UIFoundation/NSCollectionViewLayout.h>
#import <UIFoundation/NSCollectionViewUpdateItem.h>
#import <UIFoundation/_NSCollectionViewCore.h>
#import <UIFoundation/UIViewAnimationContext.h>
#import <UIFoundation/NSShadow.h>
#import <UIFoundation/NSTextAttachment.h>
#import <UIFoundation/NSTextAttachmentViewProvider.h>
#import <UIFoundation/_UIPointVector.h>
#import <UIFoundation/UIPointFIFO.h>
#import <UIFoundation/UIBoxcarFilterPointFIFO.h>
#import <UIFoundation/UIQuadCurvePointFIFO.h>
#import <UIFoundation/NSFontDescriptor.h>
#import <UIFoundation/NSCTFontDescriptor.h>
#import <UIFoundation/NSMutableFontDescriptor.h>
#import <UIFoundation/NSTextContainer.h>
#import <UIFoundation/NSTextList.h>
#import <UIFoundation/NSTextLineFragment.h>
#import <UIFoundation/UICollectionViewIndexMapper.h>
#import <UIFoundation/NSCollectionViewUpdate.h>
#import <UIFoundation/NSCollectionViewTransitionLayout.h>
#import <UIFoundation/NSConcreteNotifyingMutableAttributedString.h>
#import <UIFoundation/_NSTextStorageSideData.h>
#import <UIFoundation/NSTextStorage.h>
#import <UIFoundation/NSConcreteTextStorage.h>
#import <UIFoundation/_NSFlowLayoutItem.h>
#import <UIFoundation/_NSFlowLayoutSection.h>
#import <UIFoundation/_NSFlowLayoutRow.h>
#import <UIFoundation/_NSFlowLayoutInfo.h>
#import <UIFoundation/NSTextBlockLayoutHelper.h>
#import <UIFoundation/NSTextBlock.h>
#import <UIFoundation/NSTextTableBlock.h>
#import <UIFoundation/NSTextTable.h>
#import <UIFoundation/NSTypesetter.h>
#import <UIFoundation/NSRTFReaderTableState.h>
#import <UIFoundation/NSRTFReader.h>
#import <UIFoundation/NSRTFWriter.h>
#import <UIFoundation/NSSubstituteWebResource.h>
#import <UIFoundation/NSHTMLWebDelegate.h>
#import <UIFoundation/NSHTMLReader.h>
#import <UIFoundation/NSHTMLWriter.h>
#import <UIFoundation/NSMutableIndexPath.h>
#import <UIFoundation/NSTempAttributeDictionary.h>
#import <UIFoundation/NSAttributeDictionary.h>
#import <UIFoundation/NSAttributeDictionaryEnumerator.h>
#import <UIFoundation/UINibStringIDTable.h>
#import <UIFoundation/__NSFontTypefaceInfo.h>
#import <UIFoundation/__NSSharedFontInstanceInfo.h>
#import <UIFoundation/NSFont.h>
#import <UIFoundation/NSCTFont.h>
#import <UIFoundation/_NSAttributes.h>
#import <UIFoundation/_NSAttributeRun.h>
#import <UIFoundation/NSParagraphArbitrator.h>
#import <UIFoundation/UINibDecoder.h>
#import <UIFoundation/NSGlyphGenerator.h>
#import <UIFoundation/NSGlyphInfo.h>
#import <UIFoundation/NSIdentityGlyphInfo.h>
#import <UIFoundation/NSGlyphNameGlyphInfo.h>
#import <UIFoundation/NSCIDGlyphInfo.h>
#import <UIFoundation/NSCTGlyphInfo.h>
#import <UIFoundation/NSTextTab.h>
#import <UIFoundation/NSParagraphStyleExtraData.h>
#import <UIFoundation/NSParagraphStyle.h>
#import <UIFoundation/NSMutableParagraphStyle.h>
#import <UIFoundation/NSOpenDocumentReader.h>
#import <UIFoundation/NSOpenDocumentWriter.h>
#import <UIFoundation/_NSUIAnimator.h>
#import <UIFoundation/NSRunStorage.h>
#import <UIFoundation/NSIdRunStorage.h>
#import <UIFoundation/NSInsertionPointHelper.h>
#import <UIFoundation/NSDocFormatReader.h>
#import <UIFoundation/NSDocFormatWriter.h>
#import <UIFoundation/NSTextLayoutFragment.h>
#import <UIFoundation/_NSCollectionViewPrefetchingContext.h>
#import <UIFoundation/_UICollectionViewPrefetchItem.h>
#import <UIFoundation/NSCollectionViewFlowLayoutInvalidationContext.h>
#import <UIFoundation/UICollectionViewFlowLayoutAuxiliary.h>
#import <UIFoundation/NSCollectionViewFlowLayout.h>
#import <UIFoundation/_NSATSTypesetterGuts.h>
#import <UIFoundation/NSATSTypesetter.h>
#import <UIFoundation/__NSATSStringSegment.h>
#import <UIFoundation/NSATSGlyphStorage.h>
#import <UIFoundation/NSATSLineFragment.h>
#import <UIFoundation/NSLineFragmentRenderingContext.h>
#import <UIFoundation/NSSingleLineTypesetter.h>
#import <UIFoundation/UINibEncoder.h>
#import <UIFoundation/NSConcreteGlyphGenerator.h>
#import <UIFoundation/NSWordMLReader.h>
#import <UIFoundation/NSWordMLWriter.h>

void* CFArrayCreateWithNonRetainedObjectsFromNSArray(void);
void* CFDictionaryCreateWithNonRetainedValuesFromNSDictionary(void);
void* NSConvertGlyphsToPackedGlyphs(void);
void* NSDefaultFont(void);
void* NSStringFromCGAffineTransform(void);
void* NSTextAlignmentFromCTTextAlignment(void);
void* NSTextAlignmentToCTTextAlignment(void);
void* UIAppendBytesForValueToData(void);
void* UIAppendVInt32ToData(void);
void* UIArrayByKeepingObjectsInSet(void);
void* UICreateOrderedAndStrippedCoderValues(void);
void* UIDataLooksLikeNibArchive(void);
void* UIDistanceBetweenPointAndRect(void);
void* UIFixedByteLengthForType(void);
void* UINibArchiveIndexFromNumber(void);
void* UINibCoderValueTypeToString(void);
void* UINumberWithNibArchiveIndex(void);
void* UIRetainedIdentityKeyDictionaryCallbacks(void);
void* UIRetainedIdentitySetCallbacks(void);
void* UIRetainedIdentityValueDictionaryCallbacks(void);
void* UIWriteArchiveToData(void);
void* _NSAddDirtyLayoutManager(void);
void* _NSAttachmentCharacterSet(void);
void* _NSBidiControlCharacterSet(void);
void* _NSBidiEmbeddingAndOverrideCharSet(void);
void* _NSBlockNumberForIndex(void);
void* _NSCalculateContainerOrigin(void);
void* _NSClearGlyphIndexForPointCache(void);
void* _NSFastFillAllGlyphHolesForCharacterRange(void);
void* _NSFastFillAllGlyphHolesForGlyphRange(void);
void* _NSFastFillAllGlyphHolesUpToGlyphIndex(void);
void* _NSFastFillAllLayoutHolesForGlyphRange(void);
void* _NSFastFillAllLayoutHolesUpToEndOfContainerForGlyphIndex(void);
void* _NSFastFillAllLayoutHolesUpToGlyphIndex(void);
void* _NSFontAttributeNames(void);
void* _NSGetAppKitVersionNumber(void);
void* _NSHTMLEncoding(void);
void* _NSLayoutManagerLogDebug(void);
void* _NSNonAttachmentCharacterSet(void);
void* _NSReadAttributedStringFromData(void);
void* _NSReadAttributedStringFromURL(void);
void* _NSReadAttributedStringFromURLOrData(void);
void* _NSRemoveDirtyLayoutManager(void);
void* _NSStringDrawingCore(void);
void* _NSStringHasRightToLeftBaseWritingDirectionAtIndex(void);
void* _NSStringHasRightToLeftCharactersInRange(void);
void* _NSStringImputedBaseWritingDirectionAtIndex(void);
void* _NSUsedRectFromStoredLineFragment(void);
void* _NSXMLEncoding(void);
void* __NSGetNSAppearanceClass(void);
void* __NSTokenizerLanguageSet(void);
void* mutableDictionaryByTransformingLeafDictionariesToWeakValued(void);

#endif
