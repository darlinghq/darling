/*
 This file is part of Darling.

 Copyright (C) 2025-2026 Darling Developers

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

#import <AVFAudio/AVAudioSettings.h>

/* Format settings keys */
NSString *const AVFormatIDKey = @"AVFormatIDKey";
NSString *const AVSampleRateKey = @"AVSampleRateKey";
NSString *const AVNumberOfChannelsKey = @"AVNumberOfChannelsKey";
NSString *const AVLinearPCMBitDepthKey = @"AVLinearPCMBitDepthKey";
NSString *const AVLinearPCMIsBigEndianKey = @"AVLinearPCMIsBigEndianKey";
NSString *const AVLinearPCMIsFloatKey = @"AVLinearPCMIsFloatKey";
NSString *const AVLinearPCMIsNonInterleaved = @"AVLinearPCMIsNonInterleaved";
NSString *const AVAudioFileTypeKey = @"AVAudioFileTypeKey";

/* Encoder settings keys */
NSString *const AVEncoderAudioQualityKey = @"AVEncoderAudioQualityKey";
NSString *const AVEncoderAudioQualityForVBRKey = @"AVEncoderAudioQualityForVBRKey";
NSString *const AVEncoderBitRateKey = @"AVEncoderBitRateKey";
NSString *const AVEncoderBitRatePerChannelKey = @"AVEncoderBitRatePerChannelKey";
NSString *const AVEncoderBitRateStrategyKey = @"AVEncoderBitRateStrategyKey";
NSString *const AVEncoderBitDepthHintKey = @"AVEncoderBitDepthHintKey";

/* Sample rate converter keys */
NSString *const AVSampleRateConverterAlgorithmKey = @"AVSampleRateConverterAlgorithmKey";
NSString *const AVSampleRateConverterAudioQualityKey = @"AVSampleRateConverterAudioQualityKey";

/* Channel layout key */
NSString *const AVChannelLayoutKey = @"AVChannelLayoutKey";
