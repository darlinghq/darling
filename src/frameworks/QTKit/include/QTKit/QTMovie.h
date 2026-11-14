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

#include <Foundation/Foundation.h>
#include <QTKit/QTKitDefines.h>

@interface QTMovie : NSObject

@end

QTKIT_EXTERN NSString * const QTAddImageCodecQuality;
QTKIT_EXTERN NSString * const QTAddImageCodecType;
QTKIT_EXTERN NSString * const QTAlternates_AlternateTrack;
QTKIT_EXTERN NSString * const QTAlternates_FallbackTrack;
QTKIT_EXTERN NSString * const QTAlternates_LanguageCodeEncoding_ISO_639_2T;
QTKIT_EXTERN NSString * const QTAlternates_LanguageCodeEncoding_MacType_LangCode;
QTKIT_EXTERN NSString * const QTAlternates_LanguageCodeEncoding_RFC_4646;
QTKIT_EXTERN NSString * const QTAlternates_LanguageCodeNativeEncoding;
QTKIT_EXTERN NSString * const QTAlternates_QTTrack;
QTKIT_EXTERN NSString * const QTAlternates_TrackIsAutoExcluded;
QTKIT_EXTERN NSString * const QTCarbonMoviePasteboardType;
QTKIT_EXTERN NSString * const QTMovieActiveSegmentAttribute;
QTKIT_EXTERN NSString * const QTMovieAlbumArtworkData;
QTKIT_EXTERN NSString * const QTMovieAlbumArtworkType;
QTKIT_EXTERN NSString * const QTMovieAllowDRMContentAttribute;
QTKIT_EXTERN NSString * const QTMovieAllowPersistentCacheAttribute;
QTKIT_EXTERN NSString * const QTMovieApertureModeAttribute;
QTKIT_EXTERN NSString * const QTMovieApertureModeClassic;
QTKIT_EXTERN NSString * const QTMovieApertureModeClean;
QTKIT_EXTERN NSString * const QTMovieApertureModeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieApertureModeEncodedPixels;
QTKIT_EXTERN NSString * const QTMovieApertureModeProduction;
QTKIT_EXTERN NSString * const QTMovieAskUnresolvedDataRefsAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioBalanceAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioBassAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioChannelsAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioGainAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioIsMutedAttribute;
QTKIT_EXTERN NSString * const QTMovieAudioTrebleAttribute;
QTKIT_EXTERN NSString * const QTMovieAutoAlternatesAttribute;
QTKIT_EXTERN NSString * const QTMovieAvailableRangesAttribute;
QTKIT_EXTERN NSString * const QTMovieAvailableRangesDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieBackgroundColorDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieBalanceDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieBassDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieBoxIsOpaqueDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieCanDRMInteractWithUserAttribute;
QTKIT_EXTERN NSString * const QTMovieChapterDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieChapterEndTime;
QTKIT_EXTERN NSString * const QTMovieChapterListDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieChapterName;
QTKIT_EXTERN NSString * const QTMovieChapterStartTime;
QTKIT_EXTERN NSString * const QTMovieChapterTargetTrackAttribute;
QTKIT_EXTERN NSString * const QTMovieCloseWindowRequestNotification;
QTKIT_EXTERN NSString * const QTMovieClosedCaptionDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieClosedCaptionNotificationCommandDataParameter;
QTKIT_EXTERN NSString * const QTMovieClosedCaptionNotificationCommandParameter;
QTKIT_EXTERN NSString * const QTMovieCompositingModeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieControllerDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieCopyrightAttribute;
QTKIT_EXTERN NSString * const QTMovieCreationTimeAttribute;
QTKIT_EXTERN NSString * const QTMovieCurrentSizeAttribute;
QTKIT_EXTERN NSString * const QTMovieCurrentTimeAttribute;
QTKIT_EXTERN NSString * const QTMovieDataAttribute;
QTKIT_EXTERN NSString * const QTMovieDataHandlerSyncReadTimeoutAttribute;
QTKIT_EXTERN NSString * const QTMovieDataReferenceAttribute;
QTKIT_EXTERN NSString * const QTMovieDataSizeAttribute;
QTKIT_EXTERN NSString * const QTMovieDelegateAttribute;
QTKIT_EXTERN NSString * const QTMovieDidEndNotification;
QTKIT_EXTERN NSString * const QTMovieDidSuspendClosedCaptionChangeNotification;
QTKIT_EXTERN NSString * const QTMovieDidSuspendSubtitleChangeNotification;
QTKIT_EXTERN NSString * const QTMovieDisplayNameAttribute;
QTKIT_EXTERN NSString * const QTMovieDontInteractWithUserAttribute;
QTKIT_EXTERN NSString * const QTMovieDrawsMovieBoxBackgroundColorDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieDurationAttribute;
QTKIT_EXTERN NSString * const QTMovieEditabilityDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieEditableAttribute;
QTKIT_EXTERN NSString * const QTMovieEditedNotification;
QTKIT_EXTERN NSString * const QTMovieEnableForkedH264DecodingAttribute;
QTKIT_EXTERN NSString * const QTMovieEnterFullScreenRequestNotification;
QTKIT_EXTERN NSString * const QTMovieErrorNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieExitFullScreenRequestNotification;
QTKIT_EXTERN NSString * const QTMovieExport;
QTKIT_EXTERN NSString * const QTMovieExportFlags;
QTKIT_EXTERN NSString * const QTMovieExportHighResolutionAudioFeatures;
QTKIT_EXTERN NSString * const QTMovieExportManufacturer;
QTKIT_EXTERN NSString * const QTMovieExportName;
QTKIT_EXTERN NSString * const QTMovieExportPresetName;
QTKIT_EXTERN NSString * const QTMovieExportSettings;
QTKIT_EXTERN NSString * const QTMovieExportSettingsPresetName;
QTKIT_EXTERN NSString * const QTMovieExportSize;
QTKIT_EXTERN NSString * const QTMovieExportSubType;
QTKIT_EXTERN NSString * const QTMovieExportType;
QTKIT_EXTERN NSString * const QTMovieFadeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieFieldOfViewAttribute;
QTKIT_EXTERN NSString * const QTMovieFileNameAttribute;
QTKIT_EXTERN NSString * const QTMovieFileOffsetAttribute;
QTKIT_EXTERN NSString * const QTMovieFlatten;
QTKIT_EXTERN NSString * const QTMovieFrameImageCloneMovie;
QTKIT_EXTERN NSString * const QTMovieFrameImageDeinterlaceFields;
QTKIT_EXTERN NSString * const QTMovieFrameImageForceNoVisualContexts;
QTKIT_EXTERN NSString * const QTMovieFrameImageForceVisualContexts;
QTKIT_EXTERN NSString * const QTMovieFrameImageHighQuality;
QTKIT_EXTERN NSString * const QTMovieFrameImageOpenGLContext;
QTKIT_EXTERN NSString * const QTMovieFrameImagePegLoopingTimeToDuration;
QTKIT_EXTERN NSString * const QTMovieFrameImagePixelFormat;
QTKIT_EXTERN NSString * const QTMovieFrameImagePreflightVisualData;
QTKIT_EXTERN NSString * const QTMovieFrameImagePreviousKeyframeOK;
QTKIT_EXTERN NSString * const QTMovieFrameImageRepresentationsType;
QTKIT_EXTERN NSString * const QTMovieFrameImageSessionMode;
QTKIT_EXTERN NSString * const QTMovieFrameImageSingleField;
QTKIT_EXTERN NSString * const QTMovieFrameImageSize;
QTKIT_EXTERN NSString * const QTMovieFrameImageStopMovie;
QTKIT_EXTERN NSString * const QTMovieFrameImageType;
QTKIT_EXTERN NSString * const QTMovieFrameImageTypeCGImageRef;
QTKIT_EXTERN NSString * const QTMovieFrameImageTypeCIImage;
QTKIT_EXTERN NSString * const QTMovieFrameImageTypeCVOpenGLTextureRef;
QTKIT_EXTERN NSString * const QTMovieFrameImageTypeCVPixelBufferRef;
QTKIT_EXTERN NSString * const QTMovieFrameImageTypeNSImage;
QTKIT_EXTERN NSString * const QTMovieHasApertureModeDimensionsAttribute;
QTKIT_EXTERN NSString * const QTMovieHasAudioAttribute;
QTKIT_EXTERN NSString * const QTMovieHasDurationAttribute;
QTKIT_EXTERN NSString * const QTMovieHasVideoAttribute;
QTKIT_EXTERN NSString * const QTMovieHotspotCountAttribute;
QTKIT_EXTERN NSString * const QTMovieHotspotsAreVisibleAttribute;
QTKIT_EXTERN NSString * const QTMovieIdleImportOKAttribute;
QTKIT_EXTERN NSString * const QTMovieImportSampleDurationAttribute;
QTKIT_EXTERN NSString * const QTMovieImportTimeScaleAttribute;
QTKIT_EXTERN NSString * const QTMovieInteractivityFeatureGeneral;
QTKIT_EXTERN NSString * const QTMovieInteractivityFeatureHyperlinks;
QTKIT_EXTERN NSString * const QTMovieInteractivityFeatureQTWiredActions;
QTKIT_EXTERN NSString * const QTMovieInteractivityFeaturesAttribute;
QTKIT_EXTERN NSString * const QTMovieIsActiveAttribute;
QTKIT_EXTERN NSString * const QTMovieIsDRMAuthorizedAttribute;
QTKIT_EXTERN NSString * const QTMovieIsDRMProtectedAttribute;
QTKIT_EXTERN NSString * const QTMovieIsInteractiveAttribute;
QTKIT_EXTERN NSString * const QTMovieIsLinearAttribute;
QTKIT_EXTERN NSString * const QTMovieIsSteppableAttribute;
QTKIT_EXTERN NSString * const QTMovieIsVRMovieAttribute;
QTKIT_EXTERN NSString * const QTMovieLoadStateAttribute;
QTKIT_EXTERN NSString * const QTMovieLoadStateDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieLoadStateErrorAttribute;
QTKIT_EXTERN NSString * const QTMovieLoadedRangesAttribute;
QTKIT_EXTERN NSString * const QTMovieLoadedRangesDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieLoopModeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieLoopsAttribute;
QTKIT_EXTERN NSString * const QTMovieLoopsBackAndForthAttribute;
QTKIT_EXTERN NSString * const QTMovieMIMETypeAttribute;
QTKIT_EXTERN NSString * const QTMovieMessageNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieMessageStringPostedNotification;
QTKIT_EXTERN NSString * const QTMovieMissingComponentsOptionsComponentsToIgnoreData;
QTKIT_EXTERN NSString * const QTMovieMissingComponentsOptionsConsiderAllTracks;
QTKIT_EXTERN NSString * const QTMovieMissingComponentsResultComponentsData;
QTKIT_EXTERN NSString * const QTMovieMissingComponentsResultMediaType;
QTKIT_EXTERN NSString * const QTMovieMissingComponentsResultTrackID;
QTKIT_EXTERN NSString * const QTMovieModificationTimeAttribute;
QTKIT_EXTERN NSString * const QTMovieMovieAttributesKey;
QTKIT_EXTERN NSString * const QTMovieMovieBoxBackgroundColorDidChangeNotification;
#if defined(__i386__)
QTKIT_EXTERN NSString * const QTMovieMovieStructureAvailableCallbackAttribute;
#endif
QTKIT_EXTERN NSString * const QTMovieMutedAttribute;
QTKIT_EXTERN NSString * const QTMovieMutedDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieNaturalSizeAttribute;
QTKIT_EXTERN NSString * const QTMovieNaturalSizeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieNodeCountAttribute;
QTKIT_EXTERN NSString * const QTMovieOpenAsyncOKAttribute;
QTKIT_EXTERN NSString * const QTMovieOpenAsyncRequiredAttribute;
QTKIT_EXTERN NSString * const QTMovieOpenForPlaybackAttribute;
QTKIT_EXTERN NSString * const QTMoviePanAngleAttribute;
QTKIT_EXTERN NSString * const QTMoviePanTiltSpeedAttribute;
QTKIT_EXTERN NSString * const QTMoviePasteboardAttribute;
QTKIT_EXTERN NSString * const QTMoviePasteboardType;
QTKIT_EXTERN NSString * const QTMoviePlaysAllFramesAttribute;
QTKIT_EXTERN NSString * const QTMoviePlaysSelectionOnlyAttribute;
QTKIT_EXTERN NSString * const QTMoviePosterTimeAttribute;
QTKIT_EXTERN NSString * const QTMoviePreferredMutedAttribute;
QTKIT_EXTERN NSString * const QTMoviePreferredRateAttribute;
QTKIT_EXTERN NSString * const QTMoviePreferredTransformAttribute;
QTKIT_EXTERN NSString * const QTMoviePreferredVolumeAttribute;
QTKIT_EXTERN NSString * const QTMoviePrerollCompleteNotification;
QTKIT_EXTERN NSString * const QTMoviePreventExternalURLLinksAttribute;
QTKIT_EXTERN NSString * const QTMoviePreventRefMoviesAttribute;
QTKIT_EXTERN NSString * const QTMoviePreviewModeAttribute;
QTKIT_EXTERN NSString * const QTMoviePreviewRangeAttribute;
QTKIT_EXTERN NSString * const QTMovieQTXControllerType;
QTKIT_EXTERN NSString * const QTMovieRateAttribute;
QTKIT_EXTERN NSString * const QTMovieRateChangesPreservePitchAttribute;
QTKIT_EXTERN NSString * const QTMovieRateDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieRateDidChangeNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieResolveDataRefsAttribute;
QTKIT_EXTERN NSString * const QTMovieSelectionAttribute;
QTKIT_EXTERN NSString * const QTMovieSelectionDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieSizeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieStatusCodeNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieStatusFlagsNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieStatusStringNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieStatusStringPostedNotification;
QTKIT_EXTERN NSString * const QTMovieStdControllerType;
QTKIT_EXTERN NSString * const QTMovieSubtitleDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieSubtitleNotificationAttributedStringParameter;
QTKIT_EXTERN NSString * const QTMovieTargetIDNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieTargetNameNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieTiltAngleAttribute;
QTKIT_EXTERN NSString * const QTMovieTimeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieTimeScaleAttribute;
QTKIT_EXTERN NSString * const QTMovieTimedMetadataNotification;
QTKIT_EXTERN NSString * const QTMovieTimedMetadataNotificationDataParameter;
QTKIT_EXTERN NSString * const QTMovieTimedMetadataNotificationKeyParameter;
QTKIT_EXTERN NSString * const QTMovieTimedMetadataNotificationNamespaceParameter;
QTKIT_EXTERN NSString * const QTMovieTimedMetadataNotificationParameter;
QTKIT_EXTERN NSString * const QTMovieTrebleDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieURLAttribute;
QTKIT_EXTERN NSString * const QTMovieUneditableException;
QTKIT_EXTERN NSString * const QTMovieUseHardwareVideoDecoderIfAvailableAttribute;
QTKIT_EXTERN NSString * const QTMovieUsesHardwareVideoDecoderAttribute;
QTKIT_EXTERN NSString * const QTMovieUsesPreferredLanguageAttribute;
QTKIT_EXTERN NSString * const QTMovieVideoProviderQuartzComposerOnlyCurrentPixelBufferDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieVisualBackgroundColorAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualBrightnessAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualContrastAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualDrawsMovieBoxBackgroundColorAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualHueAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualMovieBoxBackgroundColorAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualMovieBoxIsOpaqueAttribute;
QTKIT_EXTERN NSString * const QTMovieVisualSaturationAttribute;
QTKIT_EXTERN NSString * const QTMovieVolumeAttribute;
QTKIT_EXTERN NSString * const QTMovieVolumeDidChangeNotification;
QTKIT_EXTERN NSString * const QTMovieVolumeMeteringIsEnabledAttribute;
QTKIT_EXTERN NSString * const QTMovieVolumePeakLevelsAttribute;
QTKIT_EXTERN NSString * const QTMovieVolumePowerLevelsAttribute;
QTKIT_EXTERN NSString * const QTMovieZoomRateAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyBlockAllWiredActionsAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyNoCrossSiteAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyNoLocalToLocalSiteAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyNoLocalToRemoteSiteAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyNoRemoteToLocalSiteAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicySelfContainedAttribute;
QTKIT_EXTERN NSString * const QTSecurityPolicyZonePolicyAttribute;
