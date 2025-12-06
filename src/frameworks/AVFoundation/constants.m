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
NSString *const AVCaptureDeviceWasConnectedNotification = @"AVCaptureDeviceWasConnectedNotification";
NSString *const AVCaptureDeviceWasDisconnectedNotification = @"AVCaptureDeviceWasDisconnectedNotification";
NSString *const AVCaptureSessionDidStartRunningNotification = @"AVCaptureSessionDidStartRunningNotification";
NSString *const AVCaptureSessionDidStopRunningNotification = @"AVCaptureSessionDidStopRunningNotification";
NSString *const AVCaptureSessionErrorKey = @"AVCaptureSessionErrorKey";
NSString *const AVCaptureSessionRuntimeErrorNotification = @"AVCaptureSessionRuntimeErrorNotification";
NSString *const AVChannelLayoutKey = @"AVChannelLayoutKey";
NSString *const AVEncoderBitRateChannelKey = @"AVEncoderBitRatePerChannelKey";
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
NSString *const AVNumberOfChannelsKey = @"AVNumberOfChannelsKey";
NSString *const AVPlayerItemDidPlayToEndTimeNotification = @"AVPlayerItemFailedToPlayToEndTimeNotification";
NSString *const AVPlayerItemFailedToPlayToEndTimeNotification = @"AVPlayerItemPlaybackStalledNotification";
NSString *const AVSampleRateKey = @"AVSampleRateKey";
NSString *const AVURLAssetPreferPreciseDurationAndTimingKey = @"AVURLAssetPreferPreciseDurationAndTimingKey";
NSString *const AVVideoAverageBitRateKey = @"AverageBitRate";
NSString *const AVVideoCodecAppleProRes422 = @"apcn";
NSString *const AVVideoCodecAppleProRes4444 = @"ap4h";
NSString *const AVVideoCodecH264 = @"avc1";
NSString *const AVVideoCodecHEVC = @"hvc1";
NSString *const AVVideoCodecJPEG = @"jpeg";
NSString *const AVVideoCodecKey = @"AVVideoCodecKey";
NSString *const AVVideoCompressionPropertiesKey = @"AVVideoCompressionPropertiesKey";
NSString *const AVVideoHeightKey = @"AVVideoHeightKey";
NSString *const AVVideoMaxKeyFrameIntervalKey = @"MaxKeyFrameInterval";
NSString *const AVVideoWidthKey = @"AVVideoWidthKey";

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

const AVLayerVideoGravity AVLayerVideoGravityResize = @"AVLayerVideoGravityResize";
const AVLayerVideoGravity AVLayerVideoGravityResizeAspect = @"AVLayerVideoGravityResizeAspect";
const AVLayerVideoGravity AVLayerVideoGravityResizeAspectFill = @"AVLayerVideoGravityResizeAspectFill";

const AVMediaType AVMediaTypeAudio = @"soun";
const AVMediaType AVMediaTypeMuxed = @"muxx";
const AVMediaType AVMediaTypeVideo = @"vide";

const AVMetadataKey AVMetadataCommonKeyAlbumName = @"albumName";
const AVMetadataKey AVMetadataCommonKeyArtist = @"artist";
const AVMetadataKey AVMetadataCommonKeyArtwork = @"artwork";
const AVMetadataKey AVMetadataCommonKeyDescription = @"description";
const AVMetadataKey AVMetadataCommonKeyTitle = @"title";

const AVMetadataKey AVMetadataID3MetadataKeyUserURL = @"WXXX";
