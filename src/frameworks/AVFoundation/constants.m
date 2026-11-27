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

#import <Foundation/NSString.h>
#import <AVFoundation/AVFoundation.h>

NSString *const AVAssetExportPresetHighestQuality = @"AVAssetExportPresetHighestQuality";
NSString *const AVAssetExportPresetMediumQuality = @"AVAssetExportPresetMediumQuality";
NSString *const AVAssetExportPresetLowQuality = @"AVAssetExportPresetLowQuality";
// The documentation gives no string value for this preset; macOS 26.7.3 and 10.14.4 report the symbol
// name, as for the three presets above (black-box observation).
// Test: tests/src/avfoundation_constants.m
NSString *const AVAssetExportPresetAppleM4A = @"AVAssetExportPresetAppleM4A";
NSString *const AVCaptureDeviceWasConnectedNotification = @"AVCaptureDeviceWasConnectedNotification";
NSString *const AVCaptureDeviceWasDisconnectedNotification = @"AVCaptureDeviceWasDisconnectedNotification";
NSString *const AVCaptureSessionDidStartRunningNotification = @"AVCaptureSessionDidStartRunningNotification";
NSString *const AVCaptureSessionDidStopRunningNotification = @"AVCaptureSessionDidStopRunningNotification";
NSString *const AVCaptureSessionErrorKey = @"AVCaptureSessionErrorKey";
NSString *const AVCaptureSessionRuntimeErrorNotification = @"AVCaptureSessionRuntimeErrorNotification";
NSString *const AVChannelLayoutKey = @"AVChannelLayoutKey";
NSString *const AVErrorDeviceKey = @"AVErrorDeviceKey";
NSString *const AVErrorFileSizeKey = @"AVErrorFileSizeKey";
NSString *const AVErrorMediaSubTypeKey = @"AVErrorMediaSubTypeKey";
NSString *const AVErrorMediaTypeKey = @"AVErrorMediaTypeKey";
NSString *const AVErrorTimeKey = @"AVErrorTimeKey";
NSString *const AVErrorPIDKey = @"AVErrorPIDKey";
NSString *const AVErrorRecordingSuccessfullyFinishedKey = @"AVErrorRecordingSuccessfullyFinishedKey";
NSString *const AVErrorPresentationTimeStampKey = @"AVErrorPresentationTimeStampKey";
NSString *const AVErrorPersistentTrackIDKey = @"AVErrorPersistentTrackIDKey";
NSString *const AVErrorFileTypeKey = @"AVErrorFileTypeKey";
NSString *const AVFormatIDKey = @"AVFormatIDKey";
NSString *const AVLinearPCMBitDepthKey = @"AVLinearPCMBitDepthKey";
NSString *const AVLinearPCMIsBigEndianKey = @"AVLinearPCMIsBigEndianKey";
NSString *const AVLinearPCMIsFloatKey = @"AVLinearPCMIsFloatKey";
NSString *const AVLinearPCMIsNonInterleaved = @"AVLinearPCMIsNonInterleaved";
NSString *const AVNumberOfChannelsKey = @"AVNumberOfChannelsKey";
NSString *const AVPlayerItemDidPlayToEndTimeNotification = @"AVPlayerItemDidPlayToEndTimeNotification";
NSString *const AVPlayerItemFailedToPlayToEndTimeNotification = @"AVPlayerItemFailedToPlayToEndTimeNotification";
NSString *const AVPlayerItemPlaybackStalledNotification = @"AVPlayerItemPlaybackStalledNotification";
NSString *const AVPlayerWaitingToMinimizeStallsReason = @"AVPlayerWaitingToMinimizeStallsReason";
NSString *const AVPlayerWaitingWhileEvaluatingBufferingRateReason = @"AVPlayerWaitingWhileEvaluatingBufferingRateReason";
NSString *const AVPlayerWaitingWithNoItemToPlayReason = @"AVPlayerWaitingWithNoItemToPlayReason";
NSString *const AVPlayerItemTimeJumpedNotification = @"AVPlayerItemTimeJumpedNotification";
NSNotificationName const AVSampleBufferAudioRendererWasFlushedAutomaticallyNotification = @"AVSampleBufferAudioRendererWasFlushedAutomaticallyNotification";
NSString *const AVSampleRateKey = @"AVSampleRateKey";
NSString *const AVURLAssetPreferPreciseDurationAndTimingKey = @"AVURLAssetPreferPreciseDurationAndTimingKey";
NSString *const AVVideoAverageBitRateKey = @"AverageBitRate";
NSString *const AVVideoCodecAppleProRes422 = @"apcn";
NSString *const AVVideoCodecAppleProRes4444 = @"ap4h";
NSString *const AVVideoCodecH264 = @"avc1";
NSString *const AVVideoCodecHEVC = @"hvc1";
NSString *const AVVideoCodecJPEG = @"jpeg";
NSString *const AVVideoCodecKey = @"AVVideoCodecKey";
NSString *const AVVideoCodecTypeJPEG = @"jpeg";
NSString *const AVVideoCodecTypeAppleProRes422HQ = @"apch";
NSString *const AVVideoCodecTypeAppleProRes422LT = @"apcs";
NSString *const AVVideoCodecTypeAppleProRes422Proxy = @"apco";
NSString *const AVVideoTransferFunction_IEC_sRGB = @"IEC_sRGB";
NSString *const AVVideoTransferFunction_Linear = @"Linear";
NSString *const AVAssetImageGeneratorDynamicRangePolicyMatchSource = @"MatchSource";
NSString *const AVVideoCompressionPropertiesKey = @"AVVideoCompressionPropertiesKey";
NSString *const AVVideoHeightKey = @"AVVideoHeightKey";
NSString *const AVVideoMaxKeyFrameIntervalKey = @"MaxKeyFrameInterval";
NSString *const AVVideoWidthKey = @"AVVideoWidthKey";

NSString *const AVVideoScalingModeKey = @"AVVideoScalingModeKey";
NSString *const AVVideoScalingModeResizeAspectFill = @"AVVideoScalingModeResizeAspectFill";

const AVCaptureSessionPreset AVCaptureSessionPreset3840x2160 = @"AVCaptureSessionPreset3840x2160";
const AVCaptureSessionPreset AVCaptureSessionPreset1920x1080 = @"AVCaptureSessionPreset1920x1080";
const AVCaptureSessionPreset AVCaptureSessionPreset1280x720 = @"AVCaptureSessionPreset1280x720";
const AVCaptureSessionPreset AVCaptureSessionPreset960x540 = @"AVCaptureSessionPreset960x540";
const AVCaptureSessionPreset AVCaptureSessionPreset640x480 = @"AVCaptureSessionPreset640x480";
const AVCaptureSessionPreset AVCaptureSessionPreset352x288 = @"AVCaptureSessionPreset352x288";
const AVCaptureSessionPreset AVCaptureSessionPreset320x240 = @"AVCaptureSessionPreset320x240";
const AVCaptureSessionPreset AVCaptureSessionPresetiFrame1280x720 = @"AVCaptureSessionPresetiFrame1280x720";
const AVCaptureSessionPreset AVCaptureSessionPresetiFrame960x540 = @"AVCaptureSessionPresetiFrame960x540";
const AVCaptureSessionPreset AVCaptureSessionPresetHigh = @"AVCaptureSessionPresetHigh";
const AVCaptureSessionPreset AVCaptureSessionPresetLow = @"AVCaptureSessionPresetLow";
const AVCaptureSessionPreset AVCaptureSessionPresetMedium = @"AVCaptureSessionPresetMedium";

const NSErrorDomain AVFoundationErrorDomain = @"AVFoundationErrorDomain";

const AVFileType AVFileTypeMPEG4 = @"public.mpeg-4";
const AVFileType AVFileTypeQuickTimeMovie = @"com.apple.quicktime-movie";
const AVFileType AVFileTypeAppleM4A = @"com.apple.m4a-audio";

const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmTimeDomain = @"TimeDomain";
const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmSpectral = @"Spectral";
const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmVarispeed = @"Varispeed";
const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmLowQualityZeroLatency = @"LowQualityZeroLatency";

const AVLayerVideoGravity AVLayerVideoGravityResize = @"AVLayerVideoGravityResize";
const AVLayerVideoGravity AVLayerVideoGravityResizeAspect = @"AVLayerVideoGravityResizeAspect";
const AVLayerVideoGravity AVLayerVideoGravityResizeAspectFill = @"AVLayerVideoGravityResizeAspectFill";

const AVMediaType AVMediaTypeAudio = @"soun";
const AVMediaType AVMediaTypeMuxed = @"muxx";
const AVMediaType AVMediaTypeVideo = @"vide";
const AVMediaType AVMediaTypeTimecode = @"tmcd";
const AVMediaType AVMediaTypeMetadata = @"meta";

NSString *const AVMediaCharacteristicVisual = @"AVMediaCharacteristicVisual";
NSString *const AVMediaCharacteristicAudible = @"AVMediaCharacteristicAudible";

// The documentation gives no string value for this key space. macOS 26.7.3 and 10.14.4 report the
// four-character code "comn", like the sibling key spaces (black-box observation; the symbol name is
// not the value). Test: tests/src/avfoundation_constants.m
const AVMetadataKeySpace AVMetadataKeySpaceCommon = @"comn";

const AVMetadataKey AVMetadataCommonKeyAlbumName = @"albumName";
const AVMetadataKey AVMetadataCommonKeyArtist = @"artist";
const AVMetadataKey AVMetadataCommonKeyArtwork = @"artwork";
const AVMetadataKey AVMetadataCommonKeyDescription = @"description";
const AVMetadataKey AVMetadataCommonKeyTitle = @"title";

const AVMetadataKey AVMetadataID3MetadataKeyUserURL = @"WXXX";

const AVCaptureSessionPreset AVCaptureSessionPresetPhoto = @"AVCaptureSessionPresetPhoto";

NSString *const AVCaptureDeviceTypeBuiltInWideAngleCamera = @"AVCaptureDeviceTypeBuiltInWideAngleCamera";
NSString *const AVCaptureDeviceTypeBuiltInTelephotoCamera = @"AVCaptureDeviceTypeBuiltInTelephotoCamera";
NSString *const AVCaptureDeviceTypeBuiltInUltraWideCamera = @"AVCaptureDeviceTypeBuiltInUltraWideCamera";
NSString *const AVCaptureDeviceTypeBuiltInDualCamera = @"AVCaptureDeviceTypeBuiltInDualCamera";
NSString *const AVCaptureDeviceTypeBuiltInDualWideCamera = @"AVCaptureDeviceTypeBuiltInDualWideCamera";
NSString *const AVCaptureDeviceTypeBuiltInTripleCamera = @"AVCaptureDeviceTypeBuiltInTripleCamera";
NSString *const AVCaptureDeviceTypeBuiltInTrueDepthCamera = @"AVCaptureDeviceTypeBuiltInTrueDepthCamera";
NSString *const AVCaptureDeviceTypeBuiltInLiDARDepthCamera = @"AVCaptureDeviceTypeBuiltInLiDARDepthCamera";
NSString *const AVCaptureDeviceTypeBuiltInMicrophone = @"AVCaptureDeviceTypeMicrophone";
NSString *const AVCaptureDeviceTypeMicrophone = @"AVCaptureDeviceTypeMicrophone";
NSString *const AVCaptureDeviceTypeExternalUnknown = @"AVCaptureDeviceTypeExternal";
NSString *const AVCaptureDeviceTypeExternal = @"AVCaptureDeviceTypeExternal";
NSString *const AVCaptureDeviceTypeContinuityCamera = @"AVCaptureDeviceTypeContinuityCamera";
NSString *const AVCaptureDeviceTypeDeskViewCamera = @"AVCaptureDeviceTypeDeskViewCamera";

@interface AVCaptureDeviceDiscoverySession : NSObject
@end
@implementation AVCaptureDeviceDiscoverySession
+ (id) discoverySessionWithDeviceTypes: (id) types mediaType: (id) mediaType position: (NSInteger) pos {
    return [[[self alloc] init] autorelease];
}
- (NSArray *) devices {
    return [NSArray array];
}
@end

@interface AVCapturePhotoOutput : NSObject
@end
@implementation AVCapturePhotoOutput
@end

@interface AVCapturePhotoSettings : NSObject
@end
@implementation AVCapturePhotoSettings
+ (id) photoSettings {
    return [[[self alloc] init] autorelease];
}
@end
