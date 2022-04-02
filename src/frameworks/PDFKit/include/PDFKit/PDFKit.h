/*
 This file is part of Darling.

 Copyright (C) 2019 Lubos Dolezel

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


#ifndef _PDFKit_H_
#define _PDFKit_H_

#import <Foundation/Foundation.h>

#import <PDFKit/AKChildAnnotationProtocol.h>
#import <PDFKit/AKControllerDelegateProtocol.h>
#import <PDFKit/AKParentAnnotationProtocol.h>
#import <PDFKit/AKTextAnnotationProtocol.h>
#import <PDFKit/NSAccessibility.h>
#import <PDFKit/NSAccessibilityCustomRotorItemSearchDelegate.h>
#import <PDFKit/NSAccessibilityElement.h>
#import <PDFKit/NSAccessibilityElementLoading.h>
#import <PDFKit/NSAccessibilityGroup.h>
#import <PDFKit/NSAccessibilityRow.h>
#import <PDFKit/NSAnimationDelegate.h>
#import <PDFKit/NSCollectionViewDataSource.h>
#import <PDFKit/NSCollectionViewDelegate.h>
#import <PDFKit/NSMenuDelegate.h>
#import <PDFKit/NSTextDelegate.h>
#import <PDFKit/NSTextViewDelegate.h>
#import <PDFKit/PDFAccessibilityNodeParent.h>
#import <PDFKit/PDFDocumentPageChangeDelegate.h>
#import <PDFKit/PDFHUDLayerControllerDelegate.h>
#import <PDFKit/PDFPageBackgroundManagerDelegate.h>
#import <PDFKit/PDFPageDrawProgressCallback.h>
#import <PDFKit/PDFPageLayerInterface.h>
#import <PDFKit/PDFTilePoolDelegate.h>
#import <PDFKit/PDFAccessibilityNodeAnnotation.h>
#import <PDFKit/PDFAccessibilityAssociationInfo.h>
#import <PDFKit/PDFAnnotationFreeTextPrivateVars.h>
#import <PDFKit/PDFAnnotationFreeText.h>
#import <PDFKit/PDFDocumentPrivate.h>
#import <PDFKit/PDFDocument.h>
#import <PDFKit/SelectionRectInfo.h>
#import <PDFKit/PDFPageLayerEffectPrivate.h>
#import <PDFKit/PDFSelectionLayer.h>
#import <PDFKit/PDFPageLayerAnnotationEffect.h>
#import <PDFKit/PDFPageLayerMarkupAnnotationEffect.h>
#import <PDFKit/PDFPageLayerNoteEffect.h>
#import <PDFKit/PDFPageLayerSelectionEffect.h>
#import <PDFKit/PDFPageLayerScannerResultEffect.h>
#import <PDFKit/PDFPageLayerEffect.h>
#import <PDFKit/PDFViewLayoutPrivate.h>
#import <PDFKit/PDFViewLayout.h>
#import <PDFKit/PDFPasswordViewController.h>
#import <PDFKit/PDFActionNamedPrivateVars.h>
#import <PDFKit/PDFActionNamed.h>
#import <PDFKit/PDFIconView.h>
#import <PDFKit/PDFIconCollectionView.h>
#import <PDFKit/PDFAccessibilityNodePagePrivate.h>
#import <PDFKit/PDFAccessibilityNodePage.h>
#import <PDFKit/PDFAnnotationLinePrivateVars.h>
#import <PDFKit/PDFAnnotationLine.h>
#import <PDFKit/PDFFormFieldPrivateVars.h>
#import <PDFKit/PDFFormField.h>
#import <PDFKit/PDFFormPrivateVars.h>
#import <PDFKit/PDFForm.h>
#import <PDFKit/PDFAccessibilityEffectLayerPrivate.h>
#import <PDFKit/PDFAccessibilityEffectLayer.h>
#import <PDFKit/PDFAccessibilityDocumentControllerPrivate.h>
#import <PDFKit/PDFAccessibilityDocumentController.h>
#import <PDFKit/PDFAccessibilityCustomRotorLoadingToken.h>
#import <PDFKit/PDFMarkupStylePickerCellPrivate.h>
#import <PDFKit/PDFMarkupStylePickerCell.h>
#import <PDFKit/PDFMarkupStylePickerPrivate.h>
#import <PDFKit/PDFMarkupStylePicker.h>
#import <PDFKit/PDFClipView.h>
#import <PDFKit/PDFScrollViewPrivate.h>
#import <PDFKit/PDFScrollView.h>
#import <PDFKit/PDFAnnotationCirclePrivateVars.h>
#import <PDFKit/PDFAnnotationCircle.h>
#import <PDFKit/PDFTileSurface.h>
#import <PDFKit/TileRenderRequest.h>
#import <PDFKit/PDFTilePoolPrivate.h>
#import <PDFKit/PDFTilePool.h>
#import <PDFKit/PDFAnnotationStampPrivateVars.h>
#import <PDFKit/PDFAnnotationStamp.h>
#import <PDFKit/PDFAccessibilityNodeTableCell.h>
#import <PDFKit/PDFAKDocumentAdaptorPrivate.h>
#import <PDFKit/PDFAKDocumentAdaptor.h>
#import <PDFKit/PDFAKOverlayAdaptor_osx.h>
#import <PDFKit/PDFAccessibilityNodeTablePrivate.h>
#import <PDFKit/PDFAccessibilityNodeTable.h>
#import <PDFKit/PDFThumbnailViewPrivate.h>
#import <PDFKit/PDFThumbnailView.h>
#import <PDFKit/PDFAccessibilityNodeText.h>
#import <PDFKit/PDFCoachMark.h>
#import <PDFKit/PDFCoachMarkManagerPrivate.h>
#import <PDFKit/PDFCoachMarkManager.h>
#import <PDFKit/PDFKitTextViewPrivate.h>
#import <PDFKit/PDFKitTextView.h>
#import <PDFKit/PDFAKOverlayAdaptorPrivate.h>
#import <PDFKit/PDFAKOverlayAdaptor.h>
#import <PDFKit/PDFAKAnnotationSerializationHelper.h>
#import <PDFKit/PDFAnnotationChoiceWidgetPrivateVars.h>
#import <PDFKit/PDFAnnotationChoiceWidget.h>
#import <PDFKit/PDFAKPageAdaptorPrivate.h>
#import <PDFKit/PDFAKPageAdaptor.h>
#import <PDFKit/PDFAnnotationChangePrivate.h>
#import <PDFKit/PDFAnnotationChange.h>
#import <PDFKit/PDFSelectionTrackingData.h>
#import <PDFKit/PDFLayerControllerPrivate.h>
#import <PDFKit/PDFLayerController.h>
#import <PDFKit/PDFHUDLayerControllerPrivate.h>
#import <PDFKit/PDFHUDLayerController.h>
#import <PDFKit/PDFAnnotationMarkupPrivateVars.h>
#import <PDFKit/PDFAnnotationMarkup.h>
#import <PDFKit/PDFAccessibilityNodeHeading.h>
#import <PDFKit/PDFAppearanceCharacteristicsPrivate.h>
#import <PDFKit/PDFAppearanceCharacteristics.h>
#import <PDFKit/PDFAnnotationLinkPrivateVars.h>
#import <PDFKit/PDFAnnotationLink.h>
#import <PDFKit/PDFAnnotationButtonWidgetPrivateVars.h>
#import <PDFKit/PDFAnnotationButtonWidget.h>
#import <PDFKit/PDFAnnotationSquarePrivateVars.h>
#import <PDFKit/PDFAnnotationSquare.h>
#import <PDFKit/PDFAccessibilityNodePrivate.h>
#import <PDFKit/PDFAccessibilityNode.h>
#import <PDFKit/PDFViewControllerPrivate.h>
#import <PDFKit/PDFViewController.h>
#import <PDFKit/PDFSelectionPrivate.h>
#import <PDFKit/PDFSelection.h>
#import <PDFKit/PDFAnnotationTextPrivateVars.h>
#import <PDFKit/PDFAnnotationText.h>
#import <PDFKit/PDFAccessibilityNodeImage.h>
#import <PDFKit/PageSignature.h>
#import <PDFKit/PDFPageViewPrivate.h>
#import <PDFKit/PDFPageView.h>
#import <PDFKit/PDFAnnotationTextWidgetPrivateVars.h>
#import <PDFKit/PDFAnnotationTextWidget.h>
#import <PDFKit/PDFDocumentLayerPrivate.h>
#import <PDFKit/PDFDocumentLayer.h>
#import <PDFKit/PDFScannerResultPrivate.h>
#import <PDFKit/PDFScannerResult.h>
#import <PDFKit/PDFRevealManagerPrivate.h>
#import <PDFKit/PDFRevealManager.h>
#import <PDFKit/PDFActionURLPrivateVars.h>
#import <PDFKit/PDFActionURL.h>
#import <PDFKit/PDFAnnotationDrawing.h>
#import <PDFKit/PDFPrintViewPrivate.h>
#import <PDFKit/PDFPrintView.h>
#import <PDFKit/PDFPagePrivate.h>
#import <PDFKit/PDFPage.h>
#import <PDFKit/PDFPasswordLayerControllerPrivate.h>
#import <PDFKit/PDFPasswordLayerController.h>
#import <PDFKit/PDFPageRangePrivate.h>
#import <PDFKit/PDFPageRange.h>
#import <PDFKit/PDFAnnotationCGPDFObject.h>
#import <PDFKit/PDFAnnotationPrivateVars.h>
#import <PDFKit/PDFAnnotation.h>
#import <PDFKit/PDFActionResetFormPrivateVars.h>
#import <PDFKit/PDFActionResetForm.h>
#import <PDFKit/PDFRenderingPropertiesPrivate.h>
#import <PDFKit/PDFRenderingProperties.h>
#import <PDFKit/PDFDestinationPrivate.h>
#import <PDFKit/PDFDestination.h>
#import <PDFKit/PDFAnnotationInk.h>
#import <PDFKit/PDFBorderPrivateVars.h>
#import <PDFKit/PDFBorder.h>
#import <PDFKit/PDFTimerPrivate.h>
#import <PDFKit/PDFTimer.h>
#import <PDFKit/PDFActionGoToPrivateVars.h>
#import <PDFKit/PDFActionGoTo.h>
#import <PDFKit/PDFKitPopupViewPrivate.h>
#import <PDFKit/PDFKitPopupView.h>
#import <PDFKit/PDFAccessibilityNodeTableRowPrivate.h>
#import <PDFKit/PDFAccessibilityNodeTableRow.h>
#import <PDFKit/PDFPageBackgroundManagerPrivate.h>
#import <PDFKit/PDFPageBackgroundManager.h>
#import <PDFKit/PDFAnnotationPopupPrivateVars.h>
#import <PDFKit/PDFAnnotationPopup.h>
#import <PDFKit/PDFViewPrivate.h>
#import <PDFKit/PDFView.h>
#import <PDFKit/PDFDocumentContentView.h>
#import <PDFKit/PDFDocumentViewPrivate.h>
#import <PDFKit/PDFDocumentView.h>
#import <PDFKit/PDFOutlinePrivate.h>
#import <PDFKit/PDFOutline.h>
#import <PDFKit/PDFPageLayerTile.h>
#import <PDFKit/PDFPageLayerPrivate.h>
#import <PDFKit/PDFPageLayer.h>
#import <PDFKit/PDFAKAnnotationAdaptorPrivate.h>
#import <PDFKit/PDFAKAnnotationAdaptor.h>
#import <PDFKit/PDFPageViewAnnotationControllerPrivate.h>
#import <PDFKit/PDFPageViewAnnotationController.h>
#import <PDFKit/PDFActionRemoteGoToPrivateVars.h>
#import <PDFKit/PDFActionRemoteGoTo.h>
#import <PDFKit/PDFActionPrivate.h>
#import <PDFKit/PDFAction.h>

void* PDFPointEqualToPoint(void);
void* PDFPointFromCGPoint(void);
void* PDFPointMake(void);
void* PDFPointToCGPoint(void);
void* PDFRectContainsPoint(void);
void* PDFRectContainsRect(void);
void* PDFRectEqualToRect(void);
void* PDFRectFromCGRect(void);
void* PDFRectGetMaxX(void);
void* PDFRectGetMaxY(void);
void* PDFRectGetMidX(void);
void* PDFRectGetMidY(void);
void* PDFRectGetMinX(void);
void* PDFRectGetMinY(void);
void* PDFRectInset(void);
void* PDFRectIntegral(void);
void* PDFRectIntersection(void);
void* PDFRectIntersectsRect(void);
void* PDFRectIsEmpty(void);
void* PDFRectMake(void);
void* PDFRectOffset(void);
void* PDFRectToCGRect(void);
void* PDFRectUnion(void);
void* PDFSizeFromCGSize(void);
void* PDFSizeMake(void);
void* PDFSizeToCGSize(void);

#endif
