// Copyright (C) 2026 Lubos Dolezel
#import <Foundation/Foundation.h>

typedef NSString *AVAudioTimePitchAlgorithm NS_TYPED_ENUM;

extern const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmTimeDomain;
extern const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmSpectral;
extern const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmVarispeed;
// Deprecated: plays like TimeDomain.
extern const AVAudioTimePitchAlgorithm AVAudioTimePitchAlgorithmLowQualityZeroLatency;
