/*
 This file is part of Darling.

 Copyright (C) 2023 Darling Team

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

#ifndef _IMAGEKIT_H_
#define _IMAGEKIT_H_

#import <Foundation/Foundation.h>

#import <ImageKit/AVCaptureVideoDataOutputSampleBufferDelegate.h>
#import <ImageKit/CALayer+LayerExtra.h>
#import <ImageKit/CAShapeLayer+IKNImageView.h>
#import <ImageKit/CIColor+CIFilterUIViewCreationAddition.h>
#import <ImageKit/CIFilter+IKFilterUIAddition.h>
#import <ImageKit/CIFilterListProvider.h>
#import <ImageKit/CIImage+CIFilterUIViewCreationAddition.h>
#import <ImageKit/CIMutableVector.h>
#import <ImageKit/CIVector+CIFilterUIViewCreationAddition.h>
#import <ImageKit/CIVectorView.h>
#import <ImageKit/EncapsulatedTextValueTransformer.h>
#import <ImageKit/GPSTextFieldCell.h>
#import <ImageKit/HorizontalSlider.h>
#import <ImageKit/HorizontalSliderCell.h>
#import <ImageKit/ICCameraDeviceDelegate.h>
#import <ImageKit/ICCameraDeviceDownloadDelegate.h>
#import <ImageKit/ICCameraFile+IK.h>
#import <ImageKit/ICDevice+IKDeviceBrowserViewExtension.h>
#import <ImageKit/ICDeviceDelegate.h>
#import <ImageKit/IK2PartVertical.h>
#import <ImageKit/IK3PartHorizontal.h>
#import <ImageKit/IKAccessoryView.h>
#import <ImageKit/IKAccessoryViewController.h>
#import <ImageKit/IKAdjustSlider.h>
#import <ImageKit/IKAnimationData.h>
#import <ImageKit/IKAnimationGroup.h>
#import <ImageKit/IKAnimationManager.h>
#import <ImageKit/IKAnnotation.h>
#import <ImageKit/IKAnnotationLayer.h>
#import <ImageKit/IKAnnotationPanel.h>
#import <ImageKit/IKAppKitTextDriver.h>
#import <ImageKit/IKArrowAnnotation.h>
#import <ImageKit/IKAutoDetection.h>
#import <ImageKit/IKAutodetectedItem.h>
#import <ImageKit/IKAutolayoutImageView.h>
#import <ImageKit/IKBookletCell.h>
#import <ImageKit/IKBookletPlasticCover.h>
#import <ImageKit/IKBorderedView.h>
#import <ImageKit/IKBox.h>
#import <ImageKit/IKCGRenderer.h>
#import <ImageKit/IKCIFilterCorrection.h>
#import <ImageKit/IKCacheContext.h>
#import <ImageKit/IKCacheData.h>
#import <ImageKit/IKCacheDatabase.h>
#import <ImageKit/IKCacheDatabaseEntry.h>
#import <ImageKit/IKCacheDatabaseUIDInfo.h>
#import <ImageKit/IKCacheFragHandler.h>
#import <ImageKit/IKCacheFragHandlerView.h>
#import <ImageKit/IKCacheFragHandler_Hole.h>
#import <ImageKit/IKCacheManager.h>
#import <ImageKit/IKCacheManagerRegistry.h>
#import <ImageKit/IKCacheNode.h>
#import <ImageKit/IKCacheRamNode.h>
#import <ImageKit/IKCacheTimeOutLauncher.h>
#import <ImageKit/IKCacheVRamNode.h>
#import <ImageKit/IKCameraBackgroundView.h>
#import <ImageKit/IKCameraCollectionViewItem.h>
#import <ImageKit/IKCameraDeviceView.h>
#import <ImageKit/IKCameraDeviceViewHandler.h>
#import <ImageKit/IKCameraDeviceViewHandlerIB.h>
#import <ImageKit/IKCameraDownloader.h>
#import <ImageKit/IKCameraDownloaderDelegate.h>
#import <ImageKit/IKCameraFolderWatcher.h>
#import <ImageKit/IKCameraIconCellView.h>
#import <ImageKit/IKCameraItem.h>
#import <ImageKit/IKCameraServices.h>
#import <ImageKit/IKCameraTableView.h>
#import <ImageKit/IKCenteredLayer.h>
#import <ImageKit/IKCenteringClipView.h>
#import <ImageKit/IKCircleAnnotation.h>
#import <ImageKit/IKCircleSelection.h>
#import <ImageKit/IKCollageCell.h>
#import <ImageKit/IKColor.h>
#import <ImageKit/IKColorValueTransformer.h>
#import <ImageKit/IKColorView.h>
#import <ImageKit/IKComposer.h>
#import <ImageKit/IKCropLayer.h>
#import <ImageKit/IKCropRectSelection.h>
#import <ImageKit/IKCustomPathPopupButton.h>
#import <ImageKit/IKDVGrabber.h>
#import <ImageKit/IKDatasourceDiff.h>
#import <ImageKit/IKDatasourceDiffResult.h>
#import <ImageKit/IKDatasourceProxy.h>
#import <ImageKit/IKDeviceBrowserDataView.h>
#import <ImageKit/IKDeviceBrowserHandler.h>
#import <ImageKit/IKDeviceBrowserHandlerIB.h>
#import <ImageKit/IKDeviceBrowserHeaderView.h>
#import <ImageKit/IKDeviceBrowserOutlineView.h>
#import <ImageKit/IKDeviceBrowserView.h>
#import <ImageKit/IKDeviceItem.h>
#import <ImageKit/IKDrawing.h>
#import <ImageKit/IKEmbeddedImageEditToolbar.h>
#import <ImageKit/IKEmbeddedImageView.h>
#import <ImageKit/IKFSEvent.h>
#import <ImageKit/IKFilterBrowserPanel.h>
#import <ImageKit/IKFilterBrowserView.h>
#import <ImageKit/IKFilterChain.h>
#import <ImageKit/IKFilterPreviewView.h>
#import <ImageKit/IKFilterUIView.h>
#import <ImageKit/IKFinderCell.h>
#import <ImageKit/IKFinderReflectiveIconCell.h>
#import <ImageKit/IKFinderStackIconCell.h>
#import <ImageKit/IKFlippedView.h>
#import <ImageKit/IKFlockingDatasourceItem.h>
#import <ImageKit/IKGLLayer.h>
#import <ImageKit/IKGLScroller.h>
#import <ImageKit/IKGLSharedContextRegistry.h>
#import <ImageKit/IKGLTextCache.h>
#import <ImageKit/IKGLTextCacheFragHandler.h>
#import <ImageKit/IKGLTextGenerator.h>
#import <ImageKit/IKGLTextInfo.h>
#import <ImageKit/IKGLTextRenderer.h>
#import <ImageKit/IKGLTextSizeCache.h>
#import <ImageKit/IKGLTextSubpixelShader.h>
#import <ImageKit/IKGradientHorizontalSeparatorGrooved.h>
#import <ImageKit/IKGradientHorizontalSeparatorTapered.h>
#import <ImageKit/IKGradientImageButton.h>
#import <ImageKit/IKGradientImageButtonCell.h>
#import <ImageKit/IKGraySlider.h>
#import <ImageKit/IKGraySliderCell.h>
#import <ImageKit/IKGuidesLayer.h>
#import <ImageKit/IKHardwareCapsChangeNotifier.h>
#import <ImageKit/IKHierarchicalDatasourceAdaptor.h>
#import <ImageKit/IKICInfoHandler.h>
#import <ImageKit/IKIMGPreviewCell.h>
#import <ImageKit/IKIconCell.h>
#import <ImageKit/IKIconDatabase.h>
#import <ImageKit/IKImageAdjust.h>
#import <ImageKit/IKImageAdjustSlider.h>
#import <ImageKit/IKImageAdjustSliderCell.h>
#import <ImageKit/IKImageAdjustView.h>
#import <ImageKit/IKImageAnalysisManager.h>
#import <ImageKit/IKImageBackgroundLayer.h>
#import <ImageKit/IKImageBackgroundThumbnailMaskLayer.h>
#import <ImageKit/IKImageBannerView.h>
#import <ImageKit/IKImageBlockLayer.h>
#import <ImageKit/IKImageBrowserAccessibilityCell.h>
#import <ImageKit/IKImageBrowserAppearAnimation.h>
#import <ImageKit/IKImageBrowserBookletGrid.h>
#import <ImageKit/IKImageBrowserCell.h>
#import <ImageKit/IKImageBrowserCellOffscreenRenderer.h>
#import <ImageKit/IKImageBrowserCoverFlowGrid.h>
#import <ImageKit/IKImageBrowserCoverFlowIntertiaAnimation.h>
#import <ImageKit/IKImageBrowserCoverFlowScrollingAnimation.h>
#import <ImageKit/IKImageBrowserDisappearAnimation.h>
#import <ImageKit/IKImageBrowserExpandCollapseGroupAnimation.h>
#import <ImageKit/IKImageBrowserExpandCollapseItemsAnimation.h>
#import <ImageKit/IKImageBrowserExpandCollapseiOSGroupAnimation.h>
#import <ImageKit/IKImageBrowserFloatingGroupGrid.h>
#import <ImageKit/IKImageBrowserFreeFormLayout.h>
#import <ImageKit/IKImageBrowserFreezeAnimation.h>
#import <ImageKit/IKImageBrowserGenieEffectManager.h>
#import <ImageKit/IKImageBrowserGenieView.h>
#import <ImageKit/IKImageBrowserGenieWindow.h>
#import <ImageKit/IKImageBrowserGrid.h>
#import <ImageKit/IKImageBrowserGridGroup.h>
#import <ImageKit/IKImageBrowserImportAnimation.h>
#import <ImageKit/IKImageBrowserLayoutManager.h>
#import <ImageKit/IKImageBrowserMagnifying.h>
#import <ImageKit/IKImageBrowserReorderAnimation.h>
#import <ImageKit/IKImageBrowserScrollAnimation.h>
#import <ImageKit/IKImageBrowserSubsetLayoutManager.h>
#import <ImageKit/IKImageBrowserVMScheduler.h>
#import <ImageKit/IKImageBrowserView.h>
#import <ImageKit/IKImageBrowseriOSGroupDimCellsAnimation.h>
#import <ImageKit/IKImageBrowseriOSGroupGrid.h>
#import <ImageKit/IKImageBrowseriOSGroupHighlightCellAnimation.h>
#import <ImageKit/IKImageCell.h>
#import <ImageKit/IKImageCellDatasourceProxy.h>
#import <ImageKit/IKImageCellReservedIvars.h>
#import <ImageKit/IKImageCellTrackingViewDatasourceProxy.h>
#import <ImageKit/IKImageClipView.h>
#import <ImageKit/IKImageContentView.h>
#import <ImageKit/IKImageCorrection.h>
#import <ImageKit/IKImageCorrectionHandler.h>
#import <ImageKit/IKImageCropPRS.h>
#import <ImageKit/IKImageCropView.h>
#import <ImageKit/IKImageCropViewEffect.h>
#import <ImageKit/IKImageCropViewSlider.h>
#import <ImageKit/IKImageEditDSHandler.h>
#import <ImageKit/IKImageEditFrame.h>
#import <ImageKit/IKImageEditFrameToolbar.h>
#import <ImageKit/IKImageEditPanel.h>
#import <ImageKit/IKImageEditPanelButton.h>
#import <ImageKit/IKImageEditPanelController.h>
#import <ImageKit/IKImageEditPanelPrivateData.h>
#import <ImageKit/IKImageEditView.h>
#import <ImageKit/IKImageEditWorldMap.h>
#import <ImageKit/IKImageEditWorldMapOld.h>
#import <ImageKit/IKImageEffects.h>
#import <ImageKit/IKImageEffectsView.h>
#import <ImageKit/IKImageFlowAccessibilityCell.h>
#import <ImageKit/IKImageFlowAccessibilityList.h>
#import <ImageKit/IKImageFlowAppearAnimation.h>
#import <ImageKit/IKImageFlowCell.h>
#import <ImageKit/IKImageFlowDisappearAnimation.h>
#import <ImageKit/IKImageFlowFlipAnimation.h>
#import <ImageKit/IKImageFlowImportAnimation.h>
#import <ImageKit/IKImageFlowScrollingAnimation.h>
#import <ImageKit/IKImageFlowView.h>
#import <ImageKit/IKImageGizmo.h>
#import <ImageKit/IKImageGridItem.h>
#import <ImageKit/IKImageHistogram.h>
#import <ImageKit/IKImageInfo.h>
#import <ImageKit/IKImageInfoView.h>
#import <ImageKit/IKImageLayer.h>
#import <ImageKit/IKImagePasteboardLayer.h>
#import <ImageKit/IKImagePicker.h>
#import <ImageKit/IKImageProxy.h>
#import <ImageKit/IKImageRenderInfo.h>
#import <ImageKit/IKImageState.h>
#import <ImageKit/IKImageTextureRange.h>
#import <ImageKit/IKImageView.h>
#import <ImageKit/IKImageView2.h>
#import <ImageKit/IKImageView2ScrollView.h>
#import <ImageKit/IKImageViewLayerQueue.h>
#import <ImageKit/IKImageViewPrivateData.h>
#import <ImageKit/IKImageViewUtils.h>
#import <ImageKit/IKImageWrapper.h>
#import <ImageKit/IKImageWrapperAnimatedGifCache.h>
#import <ImageKit/IKInfoTabView.h>
#import <ImageKit/IKInterfaceBuilderImage.h>
#import <ImageKit/IKInterfaceBuilderSharedDatasource.h>
#import <ImageKit/IKInterfaceBuilderSharedDelegate.h>
#import <ImageKit/IKIrisListener.h>
#import <ImageKit/IKKnob.h>
#import <ImageKit/IKKnobLayer.h>
#import <ImageKit/IKLassoSelection.h>
#import <ImageKit/IKLayerRenderer.h>
#import <ImageKit/IKLinkedList.h>
#import <ImageKit/IKLinkedListLink.h>
#import <ImageKit/IKLinkedListLinkProtocol.h>
#import <ImageKit/IKLinkedListNode.h>
#import <ImageKit/IKLinkedListNodePool.h>
#import <ImageKit/IKMediaPlugin.h>
#import <ImageKit/IKMetadataHandler.h>
#import <ImageKit/IKMipmapImage.h>
#import <ImageKit/IKMipmapItem.h>
#import <ImageKit/IKMonitorBrightnessController.h>
#import <ImageKit/IKMultipleSegmentedRawDataBuffer.h>
#import <ImageKit/IKNAnnotation.h>
#import <ImageKit/IKNCustomLayer.h>
#import <ImageKit/IKNImageLayer.h>
#import <ImageKit/IKNImageView.h>
#import <ImageKit/IKNImageViewHandler.h>
#import <ImageKit/IKNKnobsLayer.h>
#import <ImageKit/IKNProgressLayer.h>
#import <ImageKit/IKNRootLayer.h>
#import <ImageKit/IKNSelection.h>
#import <ImageKit/IKNStatusRoot.h>
#import <ImageKit/IKNStatusView.h>
#import <ImageKit/IKNStatusView2.h>
#import <ImageKit/IKNavigationImageLayer.h>
#import <ImageKit/IKNavigationLayer.h>
#import <ImageKit/IKNavigationRectLayer.h>
#import <ImageKit/IKNoActionShapeLayer.h>
#import <ImageKit/IKOpenGLRenderer.h>
#import <ImageKit/IKOpenGLRoundedRectRenderer.h>
#import <ImageKit/IKOpenGLRoundedRectRendererCache.h>
#import <ImageKit/IKPBNotePlayer.h>
#import <ImageKit/IKPPFloatingWindow.h>
#import <ImageKit/IKPPFloatingWindowAnimation.h>
#import <ImageKit/IKPTArrowButton.h>
#import <ImageKit/IKPTBackgroundWindow.h>
#import <ImageKit/IKPTBorderView.h>
#import <ImageKit/IKPTCenteredTextLayer.h>
#import <ImageKit/IKPTContentViewController.h>
#import <ImageKit/IKPTCountdownView.h>
#import <ImageKit/IKPTCropView.h>
#import <ImageKit/IKPTEditorController.h>
#import <ImageKit/IKPTEffectsController.h>
#import <ImageKit/IKPTFxButton.h>
#import <ImageKit/IKPTGridMaskLayer.h>
#import <ImageKit/IKPTImageGridCell.h>
#import <ImageKit/IKPTImageGridView.h>
#import <ImageKit/IKPTImageViewForAnimation.h>
#import <ImageKit/IKPTImporterController.h>
#import <ImageKit/IKPTRecentsService.h>
#import <ImageKit/IKPTRoundedButton.h>
#import <ImageKit/IKPTRoundedButtonCell.h>
#import <ImageKit/IKPTSidebar.h>
#import <ImageKit/IKPTSidebarRowView.h>
#import <ImageKit/IKPTSlider.h>
#import <ImageKit/IKPTSliderCell.h>
#import <ImageKit/IKPTSquareButton.h>
#import <ImageKit/IKPTSquareButtonCell.h>
#import <ImageKit/IKPTTitleCell.h>
#import <ImageKit/IKPastedImage.h>
#import <ImageKit/IKPathPopupButton.h>
#import <ImageKit/IKPathToCIImageValueTransformer.h>
#import <ImageKit/IKPictureTaker.h>
#import <ImageKit/IKPictureTakerController.h>
#import <ImageKit/IKPictureTakerInterface.h>
#import <ImageKit/IKPictureTakerNibInterface.h>
#import <ImageKit/IKPictureTakerRecentPicture.h>
#import <ImageKit/IKPictureTakerRecentPictureRepository.h>
#import <ImageKit/IKPictureTakerRecentsManager.h>
#import <ImageKit/IKPictureTakerView.h>
#import <ImageKit/IKPlaceholderItem.h>
#import <ImageKit/IKPlaceholderLayer.h>
#import <ImageKit/IKProKitCell.h>
#import <ImageKit/IKProfilePictureAttributedImage.h>
#import <ImageKit/IKProfilePictureRolloverLayer.h>
#import <ImageKit/IKProfilePictureView.h>
#import <ImageKit/IKRadianToDegreeValueTransformer.h>
#import <ImageKit/IKRamManager.h>
#import <ImageKit/IKRamManagerDelegateProtocol.h>
#import <ImageKit/IKRangeFormatter.h>
#import <ImageKit/IKRecentPicturesButtonDelegate.h>
#import <ImageKit/IKRectAnnotation.h>
#import <ImageKit/IKRectSelection.h>
#import <ImageKit/IKRectSelectionImageCapture.h>
#import <ImageKit/IKRectanglePacker.h>
#import <ImageKit/IKReflectionCell.h>
#import <ImageKit/IKReflectiveIconCell.h>
#import <ImageKit/IKRenderer.h>
#import <ImageKit/IKRootLayer.h>
#import <ImageKit/IKRootLayout.h>
#import <ImageKit/IKRotationLayer.h>
#import <ImageKit/IKSFCropElement.h>
#import <ImageKit/IKSFEffectDescription.h>
#import <ImageKit/IKSFElement.h>
#import <ImageKit/IKSSBackgroundImageView.h>
#import <ImageKit/IKSSBackgroundWindow.h>
#import <ImageKit/IKSSButton.h>
#import <ImageKit/IKSSContentLayer.h>
#import <ImageKit/IKSSEventLessLayer.h>
#import <ImageKit/IKSSGradientLayer.h>
#import <ImageKit/IKSSImageView.h>
#import <ImageKit/IKSSIndexHandler.h>
#import <ImageKit/IKSSIndexSheetSelectionLayer.h>
#import <ImageKit/IKSSIndexSheetTextLayer.h>
#import <ImageKit/IKSSIndexView.h>
#import <ImageKit/IKSSPDFView.h>
#import <ImageKit/IKSSPanel.h>
#import <ImageKit/IKSSThumbnailLayer.h>
#import <ImageKit/IKSSToolTip.h>
#import <ImageKit/IKSSToolTipView.h>
#import <ImageKit/IKSaveOptions.h>
#import <ImageKit/IKSaveOptionsHandler.h>
#import <ImageKit/IKScan.h>
#import <ImageKit/IKScanArea.h>
#import <ImageKit/IKScanInfo.h>
#import <ImageKit/IKScanResult.h>
#import <ImageKit/IKScanResultsHandler.h>
#import <ImageKit/IKScanResultsTextCellView.h>
#import <ImageKit/IKScanUIController.h>
#import <ImageKit/IKScanUIControllerAdvanced.h>
#import <ImageKit/IKScanUIControllerSimple.h>
#import <ImageKit/IKScanUIViewAdvanced.h>
#import <ImageKit/IKScanUIViewSimple.h>
#import <ImageKit/IKScannerDeviceView.h>
#import <ImageKit/IKScannerDeviceViewHandler.h>
#import <ImageKit/IKScannerDeviceViewHandlerIB.h>
#import <ImageKit/IKScannerNoDeviceView.h>
#import <ImageKit/IKScannerParameterView.h>
#import <ImageKit/IKScannerPreviewAdvanced.h>
#import <ImageKit/IKScannerPreviewSimple.h>
#import <ImageKit/IKScannerSelfTest.h>
#import <ImageKit/IKSegmentedRawDataBuffer.h>
#import <ImageKit/IKSelection.h>
#import <ImageKit/IKSelectionLayer.h>
#import <ImageKit/IKSelfTestHandler.h>
#import <ImageKit/IKShadowTool.h>
#import <ImageKit/IKSingleImageEffect.h>
#import <ImageKit/IKSlider.h>
#import <ImageKit/IKSliderCell.h>
#import <ImageKit/IKSlideshow.h>
#import <ImageKit/IKSlideshowAudioPlayer.h>
#import <ImageKit/IKSlideshowDataSourceHandler.h>
#import <ImageKit/IKSlideshowDebugItem.h>
#import <ImageKit/IKSlideshowDebugger.h>
#import <ImageKit/IKSlideshowElement.h>
#import <ImageKit/IKSlideshowExporter.h>
#import <ImageKit/IKSlideshowHandler.h>
#import <ImageKit/IKSlideshowImage.h>
#import <ImageKit/IKSlideshowItemLoadOperation.h>
#import <ImageKit/IKSlideshowItemLoadThumbnailOperation.h>
#import <ImageKit/IKSlideshowPDF.h>
#import <ImageKit/IKSlideshowQL.h>
#import <ImageKit/IKSpacerButton.h>
#import <ImageKit/IKSpinnerLayer.h>
#import <ImageKit/IKStatusLayer.h>
#import <ImageKit/IKSyncTaskBag.h>
#import <ImageKit/IKSyncTaskBagItem.h>
#import <ImageKit/IKTabView.h>
#import <ImageKit/IKTaskAsyncOperation.h>
#import <ImageKit/IKTaskManager.h>
#import <ImageKit/IKTempDirectoryHandler.h>
#import <ImageKit/IKTemperatureCorrection.h>
#import <ImageKit/IKTemplateLayer.h>
#import <ImageKit/IKTextAnnotation.h>
#import <ImageKit/IKTextDriver.h>
#import <ImageKit/IKTextField.h>
#import <ImageKit/IKTextFieldContentView.h>
#import <ImageKit/IKTextFieldWindow.h>
#import <ImageKit/IKTexturePacker.h>
#import <ImageKit/IKTexturePackerItem.h>
#import <ImageKit/IKTexturePackerView.h>
#import <ImageKit/IKTheme.h>
#import <ImageKit/IKThresholdCorrection.h>
#import <ImageKit/IKThumbnailBuilder.h>
#import <ImageKit/IKThumbnailBuilderQueue.h>
#import <ImageKit/IKThumbnailOperation.h>
#import <ImageKit/IKTintCorrection.h>
#import <ImageKit/IKTransitionAnimation.h>
#import <ImageKit/IKUIHelper.h>
#import <ImageKit/IKUIHelper.h>
#import <ImageKit/IKVImageCorrection.h>
#import <ImageKit/IKVRamManager.h>
#import <ImageKit/IKVRamManagerDelegateProtocol.h>
#import <ImageKit/IKViewController.h>
#import <ImageKit/IKWindowEffect.h>
#import <ImageKit/IKWindowTransitionAnimation.h>
#import <ImageKit/IKiOSStyleGroupCell.h>
#import <ImageKit/MKAnnotation.h>
#import <ImageKit/MTKViewDelegate.h>
#import <ImageKit/NSAffineTransform+CIFilterUIViewCreationAddition.h>
#import <ImageKit/NSAffineTransformView.h>
#import <ImageKit/NSAnimationDelegate.h>
#import <ImageKit/NSArray+IKPictureTakerAdditions.h>
#import <ImageKit/NSAttributedString+IKEditingExtensions.h>
#import <ImageKit/NSBezierPath+IKAddition.h>
#import <ImageKit/NSBitmapImageRep+ik_cgexamples.h>
#import <ImageKit/NSButton+IKScanButton.h>
#import <ImageKit/NSCollectionView+IK.h>
#import <ImageKit/NSCollectionViewDataSource.h>
#import <ImageKit/NSCollectionViewDelegate.h>
#import <ImageKit/NSCollectionViewDelegateFlowLayout.h>
#import <ImageKit/NSColor+IKAdditions.h>
#import <ImageKit/NSComboBoxDelegate.h>
#import <ImageKit/NSControlTextEditingDelegate.h>
#import <ImageKit/NSDraggingSource.h>
#import <ImageKit/NSFileManager+IKPictureTakerAdditions.h>
#import <ImageKit/NSFont+IKCommonFonts.h>
#import <ImageKit/NSGraphicsContext+IKScan.h>
#import <ImageKit/NSImage+IKAdditions.h>
#import <ImageKit/NSImageRep+IKAdditions.h>
#import <ImageKit/NSImageView+IC.h>
#import <ImageKit/NSMenu+FileFormat.h>
#import <ImageKit/NSMenuDelegate.h>
#import <ImageKit/NSMenuItemValidation.h>
#import <ImageKit/NSMutableAffineTransform.h>
#import <ImageKit/NSNumber+CIFilterUIViewCreationAddition.h>
#import <ImageKit/NSObject+IKInterThread.h>
#import <ImageKit/NSPopUpButton+IKSO.h>
#import <ImageKit/NSPopoverDelegate.h>
#import <ImageKit/NSScroller+IKImageBrowserAdditions.h>
#import <ImageKit/NSString+IKCocoaFixes.h>
#import <ImageKit/NSTableViewDataSource.h>
#import <ImageKit/NSTableViewDelegate.h>
#import <ImageKit/NSTextFieldDelegate.h>
#import <ImageKit/NSTreeNode+IK.h>
#import <ImageKit/NSUserInterfaceItemIdentification.h>
#import <ImageKit/NSView+DelayedHide.h>
#import <ImageKit/NSWindow+IKPictureTakerAdditions.h>
#import <ImageKit/NSWindowDelegate.h>
#import <ImageKit/QLPreviewItem.h>
#import <ImageKit/VKCImageAnalysisOverlayViewDelegate.h>
#import <ImageKit/_IKIRLMainProxy.h>
#import <ImageKit/_IKTypeSelectData.h>
#import <ImageKit/_IKiOSStyleGroupCell.h>


void *IKBestRepresentationFromNSImageForPhysicalSize(void);
void *IKCGBitmapContextRefFromCGImageRef(void);
void *IKCGBitmapContextRefFromData(void);
void *IKCGBitmapContextRefFromNSBitmapImageRep(void);
void *IKCGBitmapContextRefFromNSImage(void);
void *IKCGImageFromCGBitmapContextRef(void);
void *IKCGImageFromCGImageSourceRef(void);
void *IKCGImageFromData(void);
void *IKCGImageFromIconRef(void);
void *IKCGImageFromNSBitmapImageRep(void);
void *IKCGImageFromNSImage(void);
void *IKCreateCGBitmapContextWithSize(void);
void *IKCreateCGContextRefFromCGImageRef(void);
void *IKCreateCGContextRefFromData(void);
void *IKCreateCGContextRefFromNSBitmapImageRep(void);
void *IKCreateCGContextRefFromNSImage(void);
void *IKCreateIconRefFromPath(void);
void *IKCreateIconRefFromURL(void);
void *IKFastThumbnailImageFromPath(void);
void *IKFastThumbnailImageFromSourceRef(void);
void *IKIconImageFromPath(void);
void *IKIconImageFromURL(void);
void *IKNSBitmapImageRepFromNSImage(void);
void *IKNSImageFromCGBitmapContextRef(void);
void *IKNSImageFromCGImage(void);
void *IKNSImageFromCGImageSourceRef(void);
void *IKNSImageFromCIImage(void);
void *IKNSImageFromCIImageWithSize(void);
void *IKNSImageFromNSBitmapImageRep(void);
void *IKSizeOfIcon(void);
void *IKThumbnailImage(void);
void *IKThumbnailImageFromCGImage(void);
void *IKThumbnailImageFromSourceRef(void);


extern void* const IKAnimationDelayKey;
extern void* const IKAnimationDurationKey;
extern void* const IKAnimationFrameRateKey;
extern void* const IKAnimationVelocityCurveKey;
extern void* IKCameraIrisStatusChangeNotification;
extern void* IKCameraStatusChangeNotification;
extern void* const IKDisableFontSmothing;
extern void* const IKFilterBrowserDefaultInputImage;
extern void* const IKFilterBrowserExcludeCategories;
extern void* const IKFilterBrowserExcludeFilters;
extern void* const IKFilterBrowserFilterDoubleClickNotification;
extern void* const IKFilterBrowserFilterSelectedNotification;
extern void* const IKFilterBrowserShowCategories;
extern void* const IKFilterBrowserShowPreview;
extern void* const IKFilterBrowserWillPreviewFilterNotification;
extern void* const IKFilterPboardType;
extern void* const IKIconCellDisableTwoLineTitlesKey;
extern void* const IKIconCellStatusProgress;
extern void* const IKIconCellStatusString;
extern void* const IKIconCellStatusTextAttributes;
extern void* const IKIconCellTextSizeKey;
extern void* const IKIconCellTitlesOnRightKey;
extern void* const IKImageBrowserBackgroundColorKey;
extern void* const IKImageBrowserCGImageRepresentationType;
extern void* const IKImageBrowserCGImageSourceRepresentationType;
extern void* const IKImageBrowserCIImageRepresentationType;
extern void* const IKImageBrowserCellBackgroundLayer;
extern void* const IKImageBrowserCellForegroundLayer;
extern void* const IKImageBrowserCellPlaceHolderLayer;
extern void* const IKImageBrowserCellSelectionLayer;
extern void* const IKImageBrowserCellsHighlightedTitleAttributesKey;
extern void* const IKImageBrowserCellsHighlightedUnfocusedTitleAttributesKey;
extern void* const IKImageBrowserCellsOutlineColorKey;
extern void* const IKImageBrowserCellsSubtitleAttributesKey;
extern void* const IKImageBrowserCellsTitleAttributesKey;
extern void* const IKImageBrowserDidFinishNicestRendering;
extern void* const IKImageBrowserDidStabilize;
extern void* const IKImageBrowserFlavorCellPartKey;
extern void* const IKImageBrowserFlavorGridSpacingKey;
extern void* const IKImageBrowserFlavorIconSizeKey;
extern void* const IKImageBrowserFlavorMaxIconSizeKey;
extern void* const IKImageBrowserFlavorShowItemInfoKey;
extern void* const IKImageBrowserFlavorTextAttributesKey;
extern void* const IKImageBrowserFlavorTextSizeKey;
extern void* const IKImageBrowserFlavorTitlesOnRightKey;
extern void* const IKImageBrowserGroupBackgroundColorKey;
extern void* const IKImageBrowserGroupFooterLayer;
extern void* const IKImageBrowserGroupHeaderLayer;
extern void* const IKImageBrowserGroupIdentifierKey;
extern void* const IKImageBrowserGroupRangeKey;
extern void* const IKImageBrowserGroupStyleKey;
extern void* const IKImageBrowserGroupTagImageKey;
extern void* const IKImageBrowserGroupTitleKey;
extern void* const IKImageBrowserIconRefPathRepresentationType;
extern void* const IKImageBrowserIconRefRepresentationType;
extern void* const IKImageBrowserNSBitmapImageRepresentationType;
extern void* const IKImageBrowserNSDataRepresentationType;
extern void* const IKImageBrowserNSImageRepresentationType;
extern void* const IKImageBrowserNSURLRepresentationType;
extern void* const IKImageBrowserOutlinesCellsBinding;
extern void* const IKImageBrowserPDFPageRepresentationType;
extern void* const IKImageBrowserPathRepresentationType;
extern void* const IKImageBrowserPromisedRepresentationType;
extern void* const IKImageBrowserQCCompositionPathRepresentationType;
extern void* const IKImageBrowserQCCompositionRepresentationType;
extern void* const IKImageBrowserQTMoviePathRepresentationType;
extern void* const IKImageBrowserQTMovieRepresentationType;
extern void* const IKImageBrowserQuickLookPathRepresentationType;
extern void* const IKImageBrowserSelectionColorKey;
extern void* const IKImageBrowserZoomValueBinding;
extern void* const IKImageCompletedInitialLoadingNotification;
extern void* const IKImageCompletedTileLoadNotification;
extern void* const IKImageFlowSubtitleAttributesKey;
extern void* const IKImageFlowTitleAttributesKey;
extern void* const IKImageViewLoadingDidFailNotification;
extern void* const IKOverlayTypeBackground;
extern void* const IKOverlayTypeImage;
extern void* const IKPictureTakerAllowsEditingKey;
extern void* const IKPictureTakerAllowsFileChoosingKey;
extern void* const IKPictureTakerAllowsVideoCaptureKey;
extern void* const IKPictureTakerCropAreaSizeKey;
extern void* const IKPictureTakerCustomSourcesKey;
extern void* const IKPictureTakerEnableFaceDetectionKey;
extern void* const IKPictureTakerImageTransformsKey;
extern void* const IKPictureTakerInformationalTextKey;
extern void* const IKPictureTakerOutputImageMaxSizeKey;
extern void* const IKPictureTakerRemainOpenAfterValidateKey;
extern void* const IKPictureTakerShowAddressBookPicture;
extern void* const IKPictureTakerShowAddressBookPictureKey;
extern void* const IKPictureTakerShowChatIconsKey;
extern void* const IKPictureTakerShowEffectsKey;
extern void* const IKPictureTakerShowEmptyPicture;
extern void* const IKPictureTakerShowEmptyPictureKey;
extern void* const IKPictureTakerShowRecentPictureKey;
extern void* const IKPictureTakerShowUserPicturesKey;
extern void* const IKPictureTakerSourceDefaults;
extern void* const IKPictureTakerSourceKey;
extern void* const IKPictureTakerSourceRecentPictures;
extern void* const IKPictureTakerUpdateRecentPictureKey;
extern void* const IKProfilePictureEditorCropSizeKey;
extern void* const IKProfilePictureEditorCustomSourcesKey;
extern void* const IKProfilePictureEditorHidePhotoStreamPictureKey;
extern void* const IKProfilePictureEditorHideRecentPicturesKey;
extern void* const IKProfilePictureEditorInputRectKey;
extern void* const IKProfilePictureEditorShowAddressBookPictureKey;
extern void* const IKProfilePictureEditorShowChatIconsKey;
extern void* const IKProfilePictureEditorShowEmptyPictureKey;
extern void* const IKProfilePictureEditorUpdateRecentPictureKey;
extern void* const IKProfilePictureEditorViewSizeKey;
extern void* const IKQuickLookContentRect;
extern void* const IKQuickLookProperties;
extern void* const IKSaveOptionsDefaultCompressionQuality;
extern void* const IKSizeHint;
extern void* const IKSlideshowAudioFile;
extern void* const IKSlideshowModeImages;
extern void* const IKSlideshowModeOther;
extern void* const IKSlideshowModePDF;
extern void* const IKSlideshowPDFDisplayBox;
extern void* const IKSlideshowPDFDisplayMode;
extern void* const IKSlideshowPDFDisplaysAsBook;
extern void* const IKSlideshowScreen;
extern void* const IKSlideshowStartIndex;
extern void* const IKSlideshowStartPaused;
extern void* const IKSlideshowWrapAround;
extern void* IKTaskManagerTaskDoneNotification;
extern void* const IKToolModeAnnotate;
extern void* const IKToolModeCrop;
extern void* const IKToolModeMove;
extern void* const IKToolModeNone;
extern void* const IKToolModeRotate;
extern void* const IKToolModeSelect;
extern void* const IKToolModeSelectEllipse;
extern void* const IKToolModeSelectLasso;
extern void* const IKToolModeSelectRect;
extern void* const IKToolModeSelectRectImageCapture;
extern void* IKUIFlavorAllowFallback;
extern void* IKUISizeFlavor;
extern void* IKUISizeMini;
extern void* IKUISizeRegular;
extern void* IKUISizeSmall;
extern void* IKUImaxSize;
extern void* const IK_ApertureBundleIdentifier;
extern void* const IK_MailBundleIdentifier;
extern void* const IK_PhotosBundleIdentifier;
extern void* const IK_iPhotoBundleIdentifier;
extern void* const kIKAllowLowPowerRendering;
extern void* const kIKColorSpace;
extern void* const kIKEnableImageSourceHardwareAcceleration;
extern void* const kIKGeometryChanged;
extern void* const kIKInitialBackingScale;
extern void* const kIKInitialCenter;
extern void* const kIKInitialZoomFactor;
extern void* const kIKKeepZoomToFitSticky;
extern void* const kIKOutputImageChangedNotification;
extern void* const kIKPictureTakenNotification;
extern void* const kIKTakePictureAbortedNotification;
extern void* const kIKThumbnailLowResFactor;
extern void* const kIKWebImageURL;
extern void* const kIKWebPageURL;
extern void* const kIKZoomToFitOnStart;
extern void* const kIKZoomToFitOnStartIgnoreWindowSize;

#endif

