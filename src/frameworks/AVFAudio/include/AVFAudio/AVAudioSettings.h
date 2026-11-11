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

#import <Foundation/Foundation.h>

/* Format settings keys */
extern NSString *const AVFormatIDKey;
extern NSString *const AVSampleRateKey;
extern NSString *const AVNumberOfChannelsKey;
extern NSString *const AVLinearPCMBitDepthKey;
extern NSString *const AVLinearPCMIsBigEndianKey;
extern NSString *const AVLinearPCMIsFloatKey;
extern NSString *const AVLinearPCMIsNonInterleaved;
extern NSString *const AVAudioFileTypeKey;

/* Encoder settings keys */
extern NSString *const AVEncoderAudioQualityKey;
extern NSString *const AVEncoderAudioQualityForVBRKey;
extern NSString *const AVEncoderBitRateKey;
extern NSString *const AVEncoderBitRatePerChannelKey;
extern NSString *const AVEncoderBitRateStrategyKey;
extern NSString *const AVEncoderBitDepthHintKey;

/* Sample rate converter keys */
extern NSString *const AVSampleRateConverterAlgorithmKey;
extern NSString *const AVSampleRateConverterAudioQualityKey;

/* Channel layout key */
extern NSString *const AVChannelLayoutKey;
