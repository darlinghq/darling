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

#import <AVFAudio/AVAudioFormat.h>
#import <AVFAudio/AVAudioSettings.h>
#import "AVFAudioStub.h"

@implementation AVAudioFormat

- (instancetype)initWithStreamDescription:(const AudioStreamBasicDescription *)asbd
{
	if (!asbd || asbd->mSampleRate <= 0 || asbd->mChannelsPerFrame == 0)
	{
		[self release];
		return nil;
	}
	if ((self = [super init]))
		_asbd = *asbd;
	return self;
}

- (instancetype)initWithStreamDescription:(const AudioStreamBasicDescription *)asbd channelLayout:(AVAudioChannelLayout *)layout
{
	// The channel layout is not kept: AVAudioChannelLayout is not implemented yet.
	return [self initWithStreamDescription:asbd];
}

- (instancetype)initStandardFormatWithSampleRate:(double)sampleRate channels:(AVAudioChannelCount)channels
{
	return [self initWithCommonFormat:AVAudioPCMFormatFloat32 sampleRate:sampleRate channels:channels interleaved:NO];
}

- (instancetype)initWithCommonFormat:(AVAudioCommonFormat)format sampleRate:(double)sampleRate channels:(AVAudioChannelCount)channels interleaved:(BOOL)interleaved
{
	UInt32 bits;
	UInt32 flags = kAudioFormatFlagIsPacked;

	switch (format)
	{
		case AVAudioPCMFormatFloat32: bits = 32; flags |= kAudioFormatFlagIsFloat; break;
		case AVAudioPCMFormatFloat64: bits = 64; flags |= kAudioFormatFlagIsFloat; break;
		case AVAudioPCMFormatInt16: bits = 16; flags |= kAudioFormatFlagIsSignedInteger; break;
		case AVAudioPCMFormatInt32: bits = 32; flags |= kAudioFormatFlagIsSignedInteger; break;
		default:
			[self release];
			return nil;
	}

	AudioStreamBasicDescription asbd = {};
	asbd.mSampleRate = sampleRate;
	asbd.mFormatID = kAudioFormatLinearPCM;
	asbd.mFormatFlags = flags | (interleaved ? 0 : kAudioFormatFlagIsNonInterleaved);
#if __BIG_ENDIAN__
	asbd.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
	asbd.mChannelsPerFrame = channels;
	asbd.mBitsPerChannel = bits;
	asbd.mFramesPerPacket = 1;
	asbd.mBytesPerFrame = asbd.mBytesPerPacket = (bits / 8) * (interleaved ? channels : 1);

	return [self initWithStreamDescription:&asbd];
}

- (instancetype)initWithSettings:(NSDictionary<NSString *, id> *)settings
{
	AudioStreamBasicDescription asbd = {};
	NSNumber* value;

	if ((value = settings[AVSampleRateKey]))
		asbd.mSampleRate = [value doubleValue];
	if ((value = settings[AVNumberOfChannelsKey]))
		asbd.mChannelsPerFrame = [value unsignedIntValue];
	asbd.mFormatID = (value = settings[AVFormatIDKey]) ? [value unsignedIntValue] : kAudioFormatLinearPCM;

	if (asbd.mFormatID == kAudioFormatLinearPCM)
	{
		const UInt32 bits = (value = settings[AVLinearPCMBitDepthKey]) ? [value unsignedIntValue] : 16;
		UInt32 flags = kAudioFormatFlagIsPacked;

		if ([settings[AVLinearPCMIsFloatKey] boolValue])
			flags |= kAudioFormatFlagIsFloat;
		else
			flags |= kAudioFormatFlagIsSignedInteger;
		if ([settings[AVLinearPCMIsBigEndianKey] boolValue])
			flags |= kAudioFormatFlagIsBigEndian;
		const BOOL nonInterleaved = [settings[AVLinearPCMIsNonInterleaved] boolValue];
		if (nonInterleaved)
			flags |= kAudioFormatFlagIsNonInterleaved;

		asbd.mFormatFlags = flags;
		asbd.mBitsPerChannel = bits;
		asbd.mFramesPerPacket = 1;
		asbd.mBytesPerFrame = asbd.mBytesPerPacket = (bits / 8) * (nonInterleaved ? 1 : asbd.mChannelsPerFrame);
	}

	return [self initWithStreamDescription:&asbd];
}

- (const AudioStreamBasicDescription *)streamDescription
{
	return &_asbd;
}

- (double)sampleRate
{
	return _asbd.mSampleRate;
}

- (AVAudioChannelCount)channelCount
{
	return _asbd.mChannelsPerFrame;
}

- (BOOL)isInterleaved
{
	return !(_asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
}

- (AVAudioCommonFormat)commonFormat
{
	if (_asbd.mFormatID != kAudioFormatLinearPCM || !(_asbd.mFormatFlags & kAudioFormatFlagIsPacked))
		return AVAudioOtherFormat;
#if __BIG_ENDIAN__
	if (!(_asbd.mFormatFlags & kAudioFormatFlagIsBigEndian))
#else
	if (_asbd.mFormatFlags & kAudioFormatFlagIsBigEndian)
#endif
		return AVAudioOtherFormat;

	if (_asbd.mFormatFlags & kAudioFormatFlagIsFloat)
	{
		if (_asbd.mBitsPerChannel == 32)
			return AVAudioPCMFormatFloat32;
		if (_asbd.mBitsPerChannel == 64)
			return AVAudioPCMFormatFloat64;
	}
	else if (_asbd.mFormatFlags & kAudioFormatFlagIsSignedInteger)
	{
		if (_asbd.mBitsPerChannel == 16)
			return AVAudioPCMFormatInt16;
		if (_asbd.mBitsPerChannel == 32)
			return AVAudioPCMFormatInt32;
	}
	return AVAudioOtherFormat;
}

- (BOOL)isStandard
{
	return self.commonFormat == AVAudioPCMFormatFloat32 && !self.interleaved;
}

- (AVAudioChannelLayout *)channelLayout
{
	return nil;
}

- (NSDictionary<NSString *, id> *)settings
{
	NSMutableDictionary* settings = [NSMutableDictionary dictionary];

	settings[AVFormatIDKey] = @(_asbd.mFormatID);
	settings[AVSampleRateKey] = @(_asbd.mSampleRate);
	settings[AVNumberOfChannelsKey] = @(_asbd.mChannelsPerFrame);

	if (_asbd.mFormatID == kAudioFormatLinearPCM)
	{
		settings[AVLinearPCMBitDepthKey] = @(_asbd.mBitsPerChannel);
		settings[AVLinearPCMIsBigEndianKey] = @((_asbd.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0);
		settings[AVLinearPCMIsFloatKey] = @((_asbd.mFormatFlags & kAudioFormatFlagIsFloat) != 0);
		settings[AVLinearPCMIsNonInterleaved] = @((_asbd.mFormatFlags & kAudioFormatFlagIsNonInterleaved) != 0);
	}
	return settings;
}

- (BOOL)isEqual:(id)other
{
	if (other == self)
		return YES;
	if (![other isKindOfClass:[AVAudioFormat class]])
		return NO;

	const AudioStreamBasicDescription* o = [(AVAudioFormat*) other streamDescription];
	return memcmp(o, &_asbd, sizeof(_asbd)) == 0;
}

- (NSUInteger)hash
{
	return (NSUInteger) _asbd.mSampleRate ^ (_asbd.mFormatID << 3) ^ (_asbd.mChannelsPerFrame << 7) ^ _asbd.mFormatFlags;
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

+ (BOOL)supportsSecureCoding
{
	return YES;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
	[coder encodeObject:[self settings] forKey:@"AVAudioFormatSettings"];
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
	NSDictionary* settings = [coder decodeObjectOfClasses:[NSSet setWithObjects:[NSDictionary class], [NSString class], [NSNumber class], nil]
		forKey:@"AVAudioFormatSettings"];
	return settings ? [self initWithSettings:settings] : nil;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"<AVAudioFormat %p: %u ch, %.0f Hz, format %08x>", self,
		(unsigned) _asbd.mChannelsPerFrame, _asbd.mSampleRate, (unsigned) _asbd.mFormatID];
}

// Methods of the real class that are not implemented yet still have to be accepted.
- (NSMethodSignature *)methodSignatureForSelector:(SEL)aSelector
{
	return AVAudioStubSignature(aSelector);
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
	NSLog(@"Stub called: %@ in %@", NSStringFromSelector([anInvocation selector]), [self class]);
}

@end
