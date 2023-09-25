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

#ifndef _ICONSERVICES_H_
#define _ICONSERVICES_H_

#import <Foundation/Foundation.h>

#import <IconServices/CALayerDelegate.h>
#import <IconServices/CIContext+IconServicesAdditions.h>
#import <IconServices/CUICatalog+IconServicesAdditions.h>
//#import <IconServices/IFImage+ISScalableCompositorResource.h>
//#import <IconServices/IFImageBag+ISScalableCompositorResource.h>
//#import <IconServices/IFSymbol+ISScalableCompositorResource.h>
#import <IconServices/ISAliasIcon.h>
#import <IconServices/ISAppNotificationBadgeRecipe.h>
#import <IconServices/ISAssetCatalogResource.h>
#import <IconServices/ISBindableIcon.h>
#import <IconServices/ISBlurEffect.h>
#import <IconServices/ISBorderEffect.h>
#import <IconServices/ISBundleIcon.h>
#import <IconServices/ISBundleIdentifierIcon.h>
#import <IconServices/ISBundleResourceProvider.h>
#import <IconServices/ISCenterBadgeRecipe.h>
#import <IconServices/ISCenterEmbossRecipe.h>
#import <IconServices/ISCircle.h>
#import <IconServices/ISClippingRecipe.h>
#import <IconServices/ISCompositVariantResource.h>
#import <IconServices/ISCompositVariantResourceLayer.h>
#import <IconServices/ISCompositVariantResourceLayerProtocol.h>
#import <IconServices/ISCompositor.h>
#import <IconServices/ISCompositorElement.h>
#import <IconServices/ISCompositorElement.h>
#import <IconServices/ISCompositorRecipe.h>
#import <IconServices/ISCompositorResource.h>
#import <IconServices/ISCompositorResourceAdapter.h>
#import <IconServices/ISCompositorResourceProvider.h>
#import <IconServices/ISCompositorResourceProviderKey.h>
#import <IconServices/ISConcreteIcon.h>
#import <IconServices/ISContentLayer.h>
#import <IconServices/ISContinuousRoundedRect.h>
#import <IconServices/ISCurrentDeviceIcon.h>
#import <IconServices/ISCustomIcon.h>
#import <IconServices/ISCustomIconManager.h>
#import <IconServices/ISCustomRecipe.h>
#import <IconServices/ISDecoratedIcon.h>
#import <IconServices/ISDefaults.h>
#import <IconServices/ISDeviceInfo.h>
#import <IconServices/ISDimmedDarkEffect.h>
#import <IconServices/ISDimmedEffect.h>
#import <IconServices/ISDropShaddowEffect.h>
#import <IconServices/ISEffect.h>
#import <IconServices/ISEmbossedEffect.h>
#import <IconServices/ISEmbossedFolder.h>
#import <IconServices/ISEmbossedSmartFolder.h>
#import <IconServices/ISGenerationRequest.h>
#import <IconServices/ISGenerationResponse.h>
#import <IconServices/ISGenericApplicationIcon.h>
#import <IconServices/ISGenericDocumentIcon.h>
#import <IconServices/ISGenericFolderIcon.h>
#import <IconServices/ISGenericRecipe.h>
#import <IconServices/ISGraphicIconConfiguration.h>
#import <IconServices/ISGraphicSymbolDescriptor.h>
#import <IconServices/ISGraphicSymbolResource.h>
#import <IconServices/ISHintedFloat.h>
#import <IconServices/ISHintedRect.h>
#import <IconServices/ISHintedSize.h>
#import <IconServices/ISHintedValue.h>
#import <IconServices/ISIcns.h>
#import <IconServices/ISIcnsIcon.h>
#import <IconServices/ISIcon.h>
#import <IconServices/ISIconCache.h>
#import <IconServices/ISIconCacheConfiguration.h>
#import <IconServices/ISIconCacheServiceProtocol.h>
#import <IconServices/ISIconDecoration.h>
#import <IconServices/ISIconDecoration.h>
#import <IconServices/ISIconDecorationInternal.h>
#import <IconServices/ISIconDecorationResource.h>
#import <IconServices/ISIconFactory.h>
#import <IconServices/ISIconLayer.h>
#import <IconServices/ISIconManager.h>
#import <IconServices/ISIconManagerObserver.h>
#import <IconServices/ISIconObserver.h>
#import <IconServices/ISIconResourceLocator.h>
#import <IconServices/ISIconSpecification.h>
#import <IconServices/ISIconTypeResourceLocator.h>
#import <IconServices/ISIconset.h>
#import <IconServices/ISIconsetVariant.h>
#import <IconServices/ISImage.h>
#import <IconServices/ISImageBag.h>
#import <IconServices/ISImageBagIcon.h>
#import <IconServices/ISImageCache.h>
#import <IconServices/ISImageDescriptor.h>
#import <IconServices/ISImageDescriptor.h>
#import <IconServices/ISImageSetVariantResource.h>
#import <IconServices/ISImageSetVariantResourceImage.h>
#import <IconServices/ISImageSpecification.h>
#import <IconServices/ISImageStyleDescriptor.h>
#import <IconServices/ISLayer.h>
#import <IconServices/ISLeadingStatusBadgeRecipe.h>
#import <IconServices/ISLegibilityEffect.h>
#import <IconServices/ISMessagesAppRecipe.h>
#import <IconServices/ISMultipleFilesRecipe.h>
#import <IconServices/ISMutableIcns.h>
#import <IconServices/ISMutableStoreIndex.h>
#import <IconServices/ISOverlayEmbossRecipe.h>
#import <IconServices/ISOverlayEmbossedFolder.h>
#import <IconServices/ISPrefsCache.h>
#import <IconServices/ISRecordResourceProvider.h>
#import <IconServices/ISResourceFile.h>
#import <IconServices/ISResourceMetaData.h>
//#import <IconServices/ISResourceMetadata.h>
#import <IconServices/ISResourceProvider.h>
#import <IconServices/ISRuntimeCompositorResourceLayer.h>
#import <IconServices/ISScalableCompositorResource.h>
#import <IconServices/ISShapeCompositorResource.h>
#import <IconServices/ISStackedCompositorResource.h>
#import <IconServices/ISStore.h>
#import <IconServices/ISStoreIndex.h>
#import <IconServices/ISStoreMapTable.h>
#import <IconServices/ISStoreServiceProtocol.h>
#import <IconServices/ISStoreUnit.h>
#import <IconServices/ISSymbol.h>
#import <IconServices/ISSymbolIcon.h>
#import <IconServices/ISSymbolImageDescriptor.h>
#import <IconServices/ISSymbolLayer.h>
#import <IconServices/ISTagIcon.h>
#import <IconServices/ISTemplateEffect.h>
#import <IconServices/ISTextLayer.h>
#import <IconServices/ISTintEffect.h>
#import <IconServices/ISTrailingStatusBadgeRecipe.h>
#import <IconServices/ISTransparentIcon.h>
#import <IconServices/ISTypeIcon.h>
#import <IconServices/ISURLResourcePropertySpecification.h>
#import <IconServices/ISUnknownIcon.h>
#import <IconServices/ISVariant.h>
#import <IconServices/ISVariantResourceProtocol.h>
#import <IconServices/ISVariantResourceProviderProtocol.h>
#import <IconServices/ISVectorVariantResource.h>
#import <IconServices/ISVolumeIcon.h>
#import <IconServices/ISiOSAppClipRecipe.h>
#import <IconServices/ISiOSAppRecipe.h>
#import <IconServices/ISiOSMacAppRecipe.h>
#import <IconServices/ISiosDocumentRecipe.h>
#import <IconServices/ISiosmacDocumentRecipe.h>
#import <IconServices/ISmacosDocumentRecipe.h>
#import <IconServices/ISmacosDocumentRecipe1016.h>
#import <IconServices/ISwatchOSAppRecipe.h>
//#import <IconServices/LSApplicationExtensionRecord+IconServicesAdditions.h>
//#import <IconServices/LSBundleRecord+IconServicesAdditions.h>
//#import <IconServices/LSClaimBindingConfiguration+IconServicesAdditions.h>
//#import <IconServices/LSClaimRecord+IconServicesAdditions.h>
//#import <IconServices/LSExtensionPointRecord+IconServicesAdditions.h>
//#import <IconServices/LSRecord+IconServices.h>
#import <IconServices/NSArray+ISStackedCompositorResource.h>
#import <IconServices/NSBundle+IconServicesAdditions.h>
#import <IconServices/NSData+ISStoreIndex.h>
#import <IconServices/NSDictionary+ISCompositorResourceProvider.h>
#import <IconServices/NSError+Iconset.h>
#import <IconServices/NSLocale+IconServicesAdditions.h>
#import <IconServices/NSString+ISCompositorResource.h>
#import <IconServices/NSURL+UTIAdditions.h>
#import <IconServices/NSUUID+IconServices.h>
#import <IconServices/NSXPCConnection+ISStore.h>
#import <IconServices/NSXPCInterface+ISIconCacheServiceProtocol.h>
//#import <IconServices/UTTypeRecord+IconServicesAdditions.h>
#import <IconServices/_IFColor.h>
#import <IconServices/_ISCompositorElement.h>
#import <IconServices/_ISImage.h>


void *ISCreateCGImageFromBindingWithSizeScaleAndOptions(void);
void *ISIsResourceKey(void);
void *_ISCopyCacheContentsDescription(void);
void *_ISCopyCachePath(void);
void *_ISCopyICNSDataFromBinding(void);
void *_ISCreateCGImageForTypeAtScaleInternal(void);
void *_ISCreateCGImageFromBindingWithSizeScaleAndOptions(void);
void *_ISCreateCGImageFromBindingWithSizeScaleAndOptionsAsync(void);
void *_ISDefaultLog(void);
void *_ISGDBIconsCGCacheList(void);
void *_ISGenerateCacheImagesFromBindingWithSizeRangeScaleAndOptions(void);
void *_ISGetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void);
void *_ISGetIcnsIndex(void);
void *_ISGetIconFamilyData(void);
void *_ISGetIconRefVariant(void);
void *_ISIcnsIndexGetInfo(void);
void *_ISIcnsIndexNext(void);
void *_ISIcnsIndexPrev(void);
void *_ISIconDataForResourceProxy(void);
void *_ISIconRefContainsCGPoint(void);
void *_ISIconRefIntersectsCGRect(void);
void *_ISIconRefToHIShape(void);
void *_ISIconRefToIconFamily(void);
void *_ISIconServicesGetCGImageRefFromIconRef(void);
void *_ISIconServicesGetCGImageRefFromURL(void);
void *_ISIndexIsValid(void);
void *_ISInvalidateCacheEntriesForBundleIdentifier(void);
void *_ISIsIconRefMaskEmpty(void);
void *_ISPlotIconRefInContext(void);
void *_ISPlotInContext(void);
void *_ISPrepareISIconSignpostLog(void);
void *_ISPrimaryIconDataForApplicationProxy(void);
void *_ISProcessIsRoot(void);
void *_ISSetGlobalIconImagesCacheMaxEntriesAndMaxDataSize(void);
void *_ISSetIconFamilyData(void);
void *_ISTraceLog(void);
void *_ISURLCacheLog(void);
void *getFocusRingTint(void);


extern void* const _kISAttachedResourceInfo;
extern void* _kISFolderCustomIconFileName;
extern void* _kISIconsInvalidatedDarwinNotification;
extern void* _kISIconsInvalidatedNotification;
extern void* const _kISImageRequestTimeoutForeverKey;
extern void* const kISBackgroundResourceKey;
extern void* const kISBadgeResourceKey;
extern void* const kISIcnsDarkVariantName;
extern void* const kISIcnsErrorDomain;
extern void* const kISIcnsPrimaryVariantName;
extern void* const kISIcnsSelectedVariantName;
extern void* const kISIcnsTemplateVariantName;
extern void* const kISIconDecorationAliasType;
extern void* const kISIconDecorationBadgeType;
extern void* const kISIconDecorationCautionAlertType;
extern void* const kISIconDecorationDropFolderBadgeType;
extern void* const kISIconDecorationEmbossType;
extern void* const kISIconDecorationFolderType;
extern void* const kISIconDecorationLockedBadgeType;
extern void* const kISIconDecorationNewFolderType;
extern void* const kISIconDecorationPositionType;
extern void* const kISIconDecorationPositionTypeCenter;
extern void* const kISIconDecorationPositionTypeLeadingBottom;
extern void* const kISIconDecorationPositionTypeTrailingBottom;
extern void* const kISIconDecorationPrivateFolderBadgeType;
extern void* const kISIconDecorationReadOnlyFolderBadgeType;
extern void* const kISIconDecorationSystemType;
extern void* const kISIconDecorationType;
extern void* const kISIconDecorationUnsupportedBadgeType;
extern void* const kISImageDescriptorActivity;
extern void* const kISImageDescriptorBadgedDocumentScalableSize;
extern void* const kISImageDescriptorBadgedDocumentSize;
extern void* const kISImageDescriptorCarLauncher;
extern void* const kISImageDescriptorCarNotification;
extern void* const kISImageDescriptorCustomDocumentCompact;
extern void* const kISImageDescriptorCustomDocumentRegular;
extern void* const kISImageDescriptorCustomDocumentScalableSize;
extern void* const kISImageDescriptorCustomDocumentSize;
extern void* const kISImageDescriptorCustomDocumentTable;
extern void* const kISImageDescriptorHomeScreen;
extern void* const kISImageDescriptorLargeHomeScreen;
extern void* const kISImageDescriptorMessagesExtensionBadge;
extern void* const kISImageDescriptorMessagesExtensionLauncher;
extern void* const kISImageDescriptorMessagesExtensionStatus;
extern void* const kISImageDescriptorNotification;
extern void* const kISImageDescriptorSpotlight;
extern void* const kISImageDescriptorTableUIName;
extern void* const kISPrimaryResourceKey;
extern void* const kISTextResourceKey;
extern void* const kISTintColorResourceKey;
extern void* kIconCompilerImageInfoIsSelectedVariantKey;
extern void* kIconCompilerImageInfoNameKey;
extern void* kIconCompilerImageInfoScaleKey;
extern void* kIconCompilerImageInfoSizeKey;

#endif

