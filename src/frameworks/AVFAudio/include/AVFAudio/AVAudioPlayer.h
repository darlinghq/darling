/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

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

#ifndef AVFAUDIO_AVAUDIOPLAYER_H
#define AVFAUDIO_AVAUDIOPLAYER_H

#import <Foundation/Foundation.h>
#import <AVFAudio/AVAudioFormat.h>

@class AVAudioPlayer;

NS_ASSUME_NONNULL_BEGIN

@protocol AVAudioPlayerDelegate <NSObject>
@optional
// Called on the main thread when the sound has played to its end (or failed to), not when it is stopped.
- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;
- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(nullable NSError *)error;
@end

// Plays a sound from a file or from memory.
@interface AVAudioPlayer : NSObject
{
@private
	void *_core;
}

- (nullable instancetype)initWithContentsOfURL:(NSURL *)url error:(NSError **)outError;
- (nullable instancetype)initWithData:(NSData *)data error:(NSError **)outError;
- (nullable instancetype)initWithContentsOfURL:(NSURL *)url fileTypeHint:(nullable NSString *)utiString error:(NSError **)outError;
- (nullable instancetype)initWithData:(NSData *)data fileTypeHint:(nullable NSString *)utiString error:(NSError **)outError;

- (BOOL)prepareToPlay;
- (BOOL)play;
- (BOOL)playAtTime:(NSTimeInterval)time;
- (void)pause;
- (void)stop;

@property (readonly, getter=isPlaying) BOOL playing;
@property (readonly) NSUInteger numberOfChannels;
@property (readonly) NSTimeInterval duration;
@property (nullable, assign) id<AVAudioPlayerDelegate> delegate;
@property (readonly, nullable) NSURL *url;
@property (readonly, nullable) NSData *data;
@property float pan;
@property float volume;
- (void)setVolume:(float)volume fadeDuration:(NSTimeInterval)duration;
@property BOOL enableRate;
@property float rate;
@property NSTimeInterval currentTime;
@property (readonly) NSTimeInterval deviceCurrentTime;
@property NSInteger numberOfLoops;
@property (readonly) AVAudioFormat *format;
@property (getter=isMeteringEnabled) BOOL meteringEnabled;
- (void)updateMeters;
- (float)peakPowerForChannel:(NSUInteger)channelNumber;
- (float)averagePowerForChannel:(NSUInteger)channelNumber;
@property (readonly) NSDictionary<NSString *, id> *settings;

@end

NS_ASSUME_NONNULL_END

#endif
