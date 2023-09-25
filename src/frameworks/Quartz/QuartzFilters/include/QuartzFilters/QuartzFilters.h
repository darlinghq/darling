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

#ifndef _QUARTZFILTERS_H_
#define _QUARTZFILTERS_H_

#import <Foundation/Foundation.h>

#import <QuartzFilters/AssignComponentInfo.h>
#import <QuartzFilters/DefaultComponentInfo.h>
#import <QuartzFilters/IntermediateComponentInfo.h>
#import <QuartzFilters/NSControlTextEditingDelegate.h>
#import <QuartzFilters/NSTableViewDelegate.h>
#import <QuartzFilters/NSTextFieldDelegate.h>
#import <QuartzFilters/QComponentAssignProfileCellView.h>
#import <QuartzFilters/QComponentConvertToProfileCellView.h>
#import <QuartzFilters/QComponentDefaultProfileCellView.h>
#import <QuartzFilters/QComponentImageMonoSamplingCellView.h>
#import <QuartzFilters/QComponentImageSamplingCellView.h>
#import <QuartzFilters/QComponentInfoTuple.h>
#import <QuartzFilters/QComponentIntermediateCellView.h>
#import <QuartzFilters/QComponentPDFX3OptionsCellView.h>
#import <QuartzFilters/QComponentRenderingIntentCellView.h>
#import <QuartzFilters/QComponentTupleSetCellView.h>
#import <QuartzFilters/QCompressionIsJPEGTransformer.h>
#import <QuartzFilters/QConvolutionIsNotNone.h>
#import <QuartzFilters/QEditableTextField.h>
#import <QuartzFilters/QFilter.h>
#import <QuartzFilters/QFilterActionMenu.h>
#import <QuartzFilters/QFilterButton.h>
#import <QuartzFilters/QFilterCategory.h>
#import <QuartzFilters/QFilterComponent.h>
#import <QuartzFilters/QFilterComponentAssignProfile.h>
#import <QuartzFilters/QFilterComponentChild.h>
#import <QuartzFilters/QFilterComponentComments.h>
#import <QuartzFilters/QFilterComponentConvertToProfile.h>
#import <QuartzFilters/QFilterComponentDefaultProfile.h>
#import <QuartzFilters/QFilterComponentDomains.h>
#import <QuartzFilters/QFilterComponentImageCompression.h>
#import <QuartzFilters/QFilterComponentImageConvolution.h>
#import <QuartzFilters/QFilterComponentImageMonoSampling.h>
#import <QuartzFilters/QFilterComponentImageSampling.h>
#import <QuartzFilters/QFilterComponentIntermediate.h>
#import <QuartzFilters/QFilterComponentPDFPolicies.h>
#import <QuartzFilters/QFilterComponentPDFX3Options.h>
#import <QuartzFilters/QFilterComponentRenderingIntent.h>
#import <QuartzFilters/QFilterImageInfo.h>
#import <QuartzFilters/QFilterInfo.h>
#import <QuartzFilters/QFilterOutlineView.h>
#import <QuartzFilters/QFilterPDFX3Info.h>
#import <QuartzFilters/QFilterPanel.h>
#import <QuartzFilters/QFilterPanelPrivate.h>
#import <QuartzFilters/QFilterTableCellView.h>
#import <QuartzFilters/QFilterUndoController.h>
#import <QuartzFilters/QFilterUndoControllerDelegate.h>
#import <QuartzFilters/QFilterView.h>
#import <QuartzFilters/QFilterViewController.h>
#import <QuartzFilters/QFilterViewControllerDelegate.h>
#import <QuartzFilters/QImageQualityTransformer.h>
#import <QuartzFilters/QIntentTuple.h>
#import <QuartzFilters/QIntermediateTuple.h>
#import <QuartzFilters/QProfileTuple.h>
#import <QuartzFilters/QProfileTupleCellView.h>
#import <QuartzFilters/QRectObject.h>
#import <QuartzFilters/QSizeObject.h>
#import <QuartzFilters/QValueIsNonzeroTransformer.h>
#import <QuartzFilters/QuartzFilter.h>
#import <QuartzFilters/QuartzFilterInternal.h>
#import <QuartzFilters/QuartzFilterManager.h>
#import <QuartzFilters/QuartzFilterManagerInternal.h>
#import <QuartzFilters/QuartzFilterView.h>
#import <QuartzFilters/RenderingIntentInfo.h>
#import <QuartzFilters/TargetComponentInfo.h>


void *QuartzFilterApplyToContext(void);
void *QuartzFilterCopyContextDict(void);
void *QuartzFilterCreateHostMatchingFilter(void);
void *QuartzFilterCreateImageCoverter(void);
void *QuartzFilterCreateWithOutputIntents(void);
void *QuartzFilterCreateWithURL(void);
void *QuartzFilterGetName(void);
void *QuartzFilterOptionsAddPDFX3Info(void);
void *QuartzFilterOptionsCreateCalibrationOutputIntents(void);
void *QuartzFilterOptionsCreateContextFilterInfo(void);
void *QuartzFilterOptionsCreateFilterData(void);
void *QuartzFilterOptionsCreateOutputIntents(void);
void *QuartzFilterOptionsGetAssignCMYKProfileNames(void);
void *QuartzFilterOptionsGetAssignGrayProfileNames(void);
void *QuartzFilterOptionsGetAssignRGBProfileNames(void);
void *QuartzFilterOptionsGetDefaultCMYKProfileNames(void);
void *QuartzFilterOptionsGetDefaultGrayProfileNames(void);
void *QuartzFilterOptionsGetDefaultRGBProfileNames(void);
void *QuartzFilterOptionsGetMidProfileNames(void);
void *QuartzFilterOptionsGetPDFXProfileNames(void);
void *QuartzFilterOptionsGetPrinterDeviceIDFromOutputIntents(void);
void *QuartzFilterOptionsGetPrinterInfo(void);
void *QuartzFilterOptionsGetPrinterProfilesFromOutputIntents(void);
void *QuartzFilterOptionsGetSummary(void);
void *QuartzFilterOptionsGetTargetProfileNames(void);
void *QuartzFilterOptionsInitializeWithFilterData(void);
void *QuartzFilterOptionsInitializeWithOptionsInfo(void);
void *QuartzFilterOptionsInitializeWithOutputIntents(void);
void *QuartzFilterOptionsInitializeWithPDFDocument(void);
void *QuartzFilterOptionsNewInfo(void);
void *QuartzFilterOptionsNewInfoWithClassProfiles(void);
void *QuartzFilterOptionsRelease(void);
void *QuartzFilterOptionsReleaseInfo(void);
void *QuartzFilterOptionsUpdate(void);
void *QuartzFilterRelease(void);
void *QuartzFilterRemoveFromContext(void);
void *UpdateQFilterInfo(void);
void *filterFileIsInPDFServices(void);
void *filterFileIsReadWrite(void);
void *localized(void);


extern void* globalUpdateOK;
extern void* kBPCNames;
extern void* kCustomNames;
extern void* kIntermediateNames;
extern void* const kNonCategoryRowIndent;
extern void* kQFilterAppLicationDomain;
extern void* kQFilterApplicationDomain;
extern void* kQFilterAssignCMYKColorSpace;
extern void* kQFilterAssignGrayColorSpace;
extern void* kQFilterAssignProfile;
extern void* kQFilterAssignRGBColorSpace;
extern void* kQFilterBlackPointCompensation;
extern void* kQFilterBlackPointCompensationCMYK;
extern void* kQFilterBlackPointCompensationGray;
extern void* kQFilterBlackPointCompensationRGB;
extern void* kQFilterCIFilter;
extern void* kQFilterCalibrationTargetSettings;
extern void* kQFilterCategories;
extern void* kQFilterColorCategory;
extern void* kQFilterColorSettings;
extern void* kQFilterColorSpaces;
extern void* kQFilterComments;
extern void* kQFilterComponentControlKeys;
extern void* kQFilterComponentKeys;
extern void* kQFilterControlClassName;
extern void* kQFilterControlDisplayName;
extern void* kQFilterConvertToProfile;
extern void* kQFilterCoreImageFilter;
extern void* kQFilterCustomConversion;
extern void* kQFilterCustomCorrection;
extern void* kQFilterCustomCorrectionCMYK;
extern void* kQFilterCustomCorrectionGray;
extern void* kQFilterCustomCorrectionRGB;
extern void* kQFilterData;
extern void* kQFilterDefaultCMYKColorSpace;
extern void* kQFilterDefaultGrayColorSpace;
extern void* kQFilterDefaultProfiles;
extern void* kQFilterDefaultRGBColorSpace;
extern void* kQFilterDocumentColorSettings;
extern void* kQFilterDomains;
extern void* kQFilterDontManageDeviceN;
extern void* kQFilterFlattenTransparency;
extern void* kQFilterFlattenTransparencyRes;
extern void* kQFilterGraphicsColorSettings;
extern void* kQFilterGraphicsObject;
extern void* kQFilterHalfoneBitmapSizeX;
extern void* kQFilterHalfoneBitmapSizeY;
extern void* kQFilterHalfoneColorSpaceModel;
extern void* kQFilterHalftoneSettings;
extern void* kQFilterIdentifier;
extern void* kQFilterImageAutoCompress;
extern void* kQFilterImageBlur;
extern void* kQFilterImageCategory;
extern void* kQFilterImageColorSettings;
extern void* kQFilterImageCompressQuality;
extern void* kQFilterImageCompression;
extern void* kQFilterImageConvolution;
extern void* kQFilterImageConvolutionRadius;
extern void* kQFilterImageFlateCompress;
extern void* kQFilterImageInterpolateNo;
extern void* kQFilterImageInterpolateOverride;
extern void* kQFilterImageInterpolateYes;
extern void* kQFilterImageJPEGCompress;
extern void* kQFilterImageObject;
extern void* kQFilterImageResolution;
extern void* kQFilterImageScaleFactor;
extern void* kQFilterImageScaleInterpolate;
extern void* kQFilterImageScaleSettings;
extern void* kQFilterImageSettings;
extern void* kQFilterImageSharpen;
extern void* kQFilterImageSize;
extern void* kQFilterImageSizeMax;
extern void* kQFilterImageSizeMin;
extern void* kQFilterImageUncompressed;
extern void* kQFilterInfo;
extern void* kQFilterIntermediateColorSpace;
extern void* kQFilterIntermediateColorSpaceCMYK;
extern void* kQFilterIntermediateColorSpaceGray;
extern void* kQFilterIntermediateColorSpaceRGB;
extern void* kQFilterIntermediateProfile;
extern void* kQFilterMonoImageScaleSettings;
extern void* kQFilterMonochromeColorSpace;
extern void* kQFilterName;
extern void* kQFilterNumberBoolean;
extern void* kQFilterNumberDefault;
extern void* kQFilterNumberFloat;
extern void* kQFilterNumberInteger;
extern void* kQFilterNumberMax;
extern void* kQFilterNumberMin;
extern void* kQFilterNumberType;
extern void* kQFilterNumberValue;
extern void* kQFilterObjects;
extern void* kQFilterPDFCategory;
extern void* kQFilterPDFCompression;
extern void* kQFilterPDFImageInterpolation;
extern void* kQFilterPDFMonochrome;
extern void* kQFilterPDFWorkflowDomain;
extern void* kQFilterPDFX3;
extern void* kQFilterPDFX3AddBleedBox;
extern void* kQFilterPDFX3AddTrimBox;
extern void* kQFilterPDFX3BleedBox;
extern void* kQFilterPDFX3DestinationProfile;
extern void* kQFilterPDFX3IsTrapped;
extern void* kQFilterPDFX3OutputCondition;
extern void* kQFilterPDFX3OutputConditionID;
extern void* kQFilterPDFX3OutputConditionInfo;
extern void* kQFilterPDFX3RegistryName;
extern void* kQFilterPDFX3Settings;
extern void* kQFilterPDFX3TrimBox;
extern void* kQFilterPreserveEPS;
extern void* kQFilterPrinterProfiles;
extern void* kQFilterPrintingDomain;
extern void* kQFilterPrivateInfo;
extern void* kQFilterProfileData;
extern void* kQFilterReadOnly;
extern void* kQFilterRenderingIntent;
extern void* kQFilterShadeInterpolate;
extern void* kQFilterShadeSettings;
extern void* kQFilterShadeSizeX;
extern void* kQFilterShadeSizeY;
extern void* kQFilterShadingColorSettings;
extern void* kQFilterShadingObject;
extern void* kQFilterSourceRenderingIntent;
extern void* kQFilterSourceRenderingIntentCMYK;
extern void* kQFilterSourceRenderingIntentGray;
extern void* kQFilterSourceRenderingIntentRGB;
extern void* kQFilterTargetColorSpace;
extern void* kQFilterTargetColorSpaceCMYK;
extern void* kQFilterTargetColorSpaceGray;
extern void* kQFilterTargetColorSpaceRGB;
extern void* kQFilterTargetRenderingIntent;
extern void* kQFilterTargetRenderingIntentCMYK;
extern void* kQFilterTargetRenderingIntentGray;
extern void* kQFilterTargetRenderingIntentRGB;
extern void* kQFilterTextColorSettings;
extern void* kQFilterTextObject;
extern void* kQFilterType;
extern void* kQFilterURL;
extern void* kQuartzFilterApplicationDomain;
extern void* kQuartzFilterManagerDidAddFilterNotification;
extern void* kQuartzFilterManagerDidModifyFilterNotification;
extern void* kQuartzFilterManagerDidRemoveFilterNotification;
extern void* kQuartzFilterManagerDidSelectFilterNotification;
extern void* kQuartzFilterManagerOtherNotification;
extern void* kQuartzFilterPDFWorkflowDomain;
extern void* kQuartzFilterPrintingDomain;
extern void* kSrcIntentNames;
extern void* kTargetNames;
extern void* kTrgIntentNames;

#endif

