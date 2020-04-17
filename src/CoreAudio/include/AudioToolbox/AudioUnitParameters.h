#ifndef _AUDIO_UNIT_PARAMETERS_H
#define _AUDIO_UNIT_PARAMETERS_H

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AUComponent.h>

enum {
 kAUGroupParameterID_Volume = 7,
 kAUGroupParameterID_Sustain = 64,
 kAUGroupParameterID_Sostenuto = 66,
 kAUGroupParameterID_AllNotesOff = 123,
 kAUGroupParameterID_ModWheel = 1,
 kAUGroupParameterID_PitchBend = 0xE0,
 kAUGroupParameterID_AllSoundOff = 120,
 kAUGroupParameterID_ResetAllControllers = 121,
 kAUGroupParameterID_Pan = 10,
 kAUGroupParameterID_Foot = 4,
 kAUGroupParameterID_ChannelPressure = 0xD0,
 kAUGroupParameterID_KeyPressure = 0xA0,
 kAUGroupParameterID_Expression = 11,
 kAUGroupParameterID_DataEntry = 6,

 kAUGroupParameterID_Volume_LSB = kAUGroupParameterID_Volume + 32,
 kAUGroupParameterID_ModWheel_LSB = kAUGroupParameterID_ModWheel + 32,
 kAUGroupParameterID_Pan_LSB = kAUGroupParameterID_Pan + 32,
 kAUGroupParameterID_Foot_LSB = kAUGroupParameterID_Foot + 32,
 kAUGroupParameterID_Expression_LSB = kAUGroupParameterID_Expression + 32,
 kAUGroupParameterID_DataEntry_LSB = kAUGroupParameterID_DataEntry + 32,

 kAUGroupParameterID_KeyPressure_FirstKey = 256,
 kAUGroupParameterID_KeyPressure_LastKey = 383
};
enum {

 kPannerParam_Gain = 0,


 kPannerParam_Azimuth = 1,

 kPannerParam_Elevation = 2,


 kPannerParam_Distance = 3,


 kPannerParam_CoordScale = 4,

 kPannerParam_RefDistance = 5,
};






enum {

    kSpatialMixerParam_Azimuth = 0,


    kSpatialMixerParam_Elevation = 1,


    kSpatialMixerParam_Distance = 2,


    kSpatialMixerParam_Gain = 3,


    kSpatialMixerParam_PlaybackRate = 4,


    kSpatialMixerParam_Enable = 5,


    kSpatialMixerParam_MinGain = 6,


    kSpatialMixerParam_MaxGain = 7,


    kSpatialMixerParam_ReverbBlend = 8,


    kSpatialMixerParam_GlobalReverbGain = 9,




    kSpatialMixerParam_OcclusionAttenuation = 10,




    kSpatialMixerParam_ObstructionAttenuation = 11
};


enum {

 kReverbParam_FilterFrequency = 14,


 kReverbParam_FilterBandwidth = 15,


 kReverbParam_FilterGain = 16,


    kReverbParam_FilterType = 17,


    kReverbParam_FilterEnable = 18
};



enum {

    k3DMixerParam_Azimuth = 0,


    k3DMixerParam_Elevation = 1,


    k3DMixerParam_Distance = 2,


    k3DMixerParam_Gain = 3,


    k3DMixerParam_PlaybackRate = 4,




    k3DMixerParam_ReverbBlend = 5,


    k3DMixerParam_GlobalReverbGain = 6,




    k3DMixerParam_OcclusionAttenuation = 7,




    k3DMixerParam_ObstructionAttenuation = 8,


    k3DMixerParam_MinGain = 9,


    k3DMixerParam_MaxGain = 10,







 k3DMixerParam_PreAveragePower = 1000,
 k3DMixerParam_PrePeakHoldLevel = 2000,
 k3DMixerParam_PostAveragePower = 3000,
 k3DMixerParam_PostPeakHoldLevel = 4000
};




enum {

 kMultiChannelMixerParam_Volume = 0,

 kMultiChannelMixerParam_Enable = 1,

 kMultiChannelMixerParam_Pan = 2,





 kMultiChannelMixerParam_PreAveragePower = 1000,
 kMultiChannelMixerParam_PrePeakHoldLevel = 2000,
 kMultiChannelMixerParam_PostAveragePower = 3000,
 kMultiChannelMixerParam_PostPeakHoldLevel = 4000
};



enum {
 kMatrixMixerParam_Volume = 0,
 kMatrixMixerParam_Enable = 1,



 kMatrixMixerParam_PreAveragePower = 1000,
 kMatrixMixerParam_PrePeakHoldLevel = 2000,
 kMatrixMixerParam_PostAveragePower = 3000,
 kMatrixMixerParam_PostPeakHoldLevel = 4000,


 kMatrixMixerParam_PreAveragePowerLinear = 5000,
 kMatrixMixerParam_PrePeakHoldLevelLinear = 6000,
 kMatrixMixerParam_PostAveragePowerLinear = 7000,
 kMatrixMixerParam_PostPeakHoldLevelLinear = 8000
};




enum {

 kHALOutputParam_Volume = 14
};


enum {
 kTimePitchParam_Rate = 0,
 kTimePitchParam_Pitch = 1,
 kTimePitchParam_EffectBlend = 2
};


enum {


 kNewTimePitchParam_Rate = 0,



 kNewTimePitchParam_Pitch = 1,





 kNewTimePitchParam_Overlap = 4,






 kNewTimePitchParam_EnablePeakLocking = 6
};


enum {

 kAUSamplerParam_Gain = 900,


 kAUSamplerParam_CoarseTuning = 901,


 kAUSamplerParam_FineTuning = 902,


 kAUSamplerParam_Pan = 903
};







enum {

 kBandpassParam_CenterFrequency = 0,


 kBandpassParam_Bandwidth = 1
};


enum {

 kHipassParam_CutoffFrequency = 0,


 kHipassParam_Resonance = 1
};


enum {

 kLowPassParam_CutoffFrequency = 0,


 kLowPassParam_Resonance = 1
};


enum {

 kHighShelfParam_CutOffFrequency = 0,


 kHighShelfParam_Gain = 1
};


enum {

 kAULowShelfParam_CutoffFrequency = 0,


 kAULowShelfParam_Gain = 1
};


enum {

    kParametricEQParam_CenterFreq = 0,


    kParametricEQParam_Q = 1,


    kParametricEQParam_Gain = 2
};


enum {

 kLimiterParam_AttackTime = 0,


 kLimiterParam_DecayTime = 1,


 kLimiterParam_PreGain = 2
};







enum {

 kDynamicsProcessorParam_Threshold = 0,


 kDynamicsProcessorParam_HeadRoom = 1,


 kDynamicsProcessorParam_ExpansionRatio = 2,


 kDynamicsProcessorParam_ExpansionThreshold = 3,


 kDynamicsProcessorParam_AttackTime = 4,


 kDynamicsProcessorParam_ReleaseTime = 5,


 kDynamicsProcessorParam_MasterGain = 6,


 kDynamicsProcessorParam_CompressionAmount = 1000,
 kDynamicsProcessorParam_InputAmplitude = 2000,
 kDynamicsProcessorParam_OutputAmplitude = 3000
};



enum {

 kVarispeedParam_PlaybackRate = 0,

 kVarispeedParam_PlaybackCents = 1
};



enum {

 kDistortionParam_Delay = 0,

 kDistortionParam_Decay = 1,

 kDistortionParam_DelayMix = 2,


 kDistortionParam_Decimation = 3,

 kDistortionParam_Rounding = 4,

 kDistortionParam_DecimationMix = 5,


 kDistortionParam_LinearTerm = 6,

 kDistortionParam_SquaredTerm = 7,

 kDistortionParam_CubicTerm = 8,

 kDistortionParam_PolynomialMix = 9,


 kDistortionParam_RingModFreq1 = 10,

 kDistortionParam_RingModFreq2 = 11,

 kDistortionParam_RingModBalance = 12,

 kDistortionParam_RingModMix = 13,


 kDistortionParam_SoftClipGain = 14,


 kDistortionParam_FinalMix = 15
};


enum {

 kDelayParam_WetDryMix = 0,


 kDelayParam_DelayTime = 1,


 kDelayParam_Feedback = 2,


 kDelayParam_LopassCutoff = 3
};
enum {

 kAUNBandEQParam_GlobalGain = 0,


 kAUNBandEQParam_BypassBand = 1000,


 kAUNBandEQParam_FilterType = 2000,


 kAUNBandEQParam_Frequency = 3000,


 kAUNBandEQParam_Gain = 4000,


 kAUNBandEQParam_Bandwidth = 5000
};
enum {
 kAUNBandEQFilterType_Parametric = 0,
 kAUNBandEQFilterType_2ndOrderButterworthLowPass = 1,
 kAUNBandEQFilterType_2ndOrderButterworthHighPass = 2,
 kAUNBandEQFilterType_ResonantLowPass = 3,
 kAUNBandEQFilterType_ResonantHighPass = 4,
 kAUNBandEQFilterType_BandPass = 5,
 kAUNBandEQFilterType_BandStop = 6,
 kAUNBandEQFilterType_LowShelf = 7,
 kAUNBandEQFilterType_HighShelf = 8,
 kAUNBandEQFilterType_ResonantLowShelf = 9,
 kAUNBandEQFilterType_ResonantHighShelf = 10,

 kNumAUNBandEQFilterTypes = 11
};



enum {

 kRoundTripAACParam_Format = 0,


 kRoundTripAACParam_EncodingStrategy = 1,


 kRoundTripAACParam_RateOrQuality = 2,


 kRoundTripAACParam_BitRate = 1,
 kRoundTripAACParam_Quality = 2,
 kRoundTripAACParam_CompressedFormatSampleRate = 3
};





enum {

 kGraphicEQParam_NumberOfBands = 10000
};


enum {

 kReverbParam_DryWetMix = 0,


 kReverbParam_SmallLargeMix = 1,


 kReverbParam_SmallSize = 2,


 kReverbParam_LargeSize = 3,


 kReverbParam_PreDelay = 4,


 kReverbParam_LargeDelay = 5,


 kReverbParam_SmallDensity = 6,


 kReverbParam_LargeDensity = 7,


 kReverbParam_LargeDelayRange = 8,


 kReverbParam_SmallBrightness = 9,


 kReverbParam_LargeBrightness = 10,


 kReverbParam_SmallDelayRange = 11,


 kReverbParam_ModulationRate = 12,


 kReverbParam_ModulationDepth = 13,
};


enum {

 kMultibandCompressorParam_Pregain = 0,

 kMultibandCompressorParam_Postgain = 1,

 kMultibandCompressorParam_Crossover1 = 2,

 kMultibandCompressorParam_Crossover2 = 3,

 kMultibandCompressorParam_Crossover3 = 4,

 kMultibandCompressorParam_Threshold1 = 5,

 kMultibandCompressorParam_Threshold2 = 6,

 kMultibandCompressorParam_Threshold3 = 7,

 kMultibandCompressorParam_Threshold4 = 8,

 kMultibandCompressorParam_Headroom1 = 9,

 kMultibandCompressorParam_Headroom2 = 10,

 kMultibandCompressorParam_Headroom3 = 11,

 kMultibandCompressorParam_Headroom4 = 12,

 kMultibandCompressorParam_AttackTime = 13,

 kMultibandCompressorParam_ReleaseTime = 14,

 kMultibandCompressorParam_EQ1 = 15,

 kMultibandCompressorParam_EQ2 = 16,

 kMultibandCompressorParam_EQ3 = 17,

 kMultibandCompressorParam_EQ4 = 18,



 kMultibandCompressorParam_CompressionAmount1 = 1000,

 kMultibandCompressorParam_CompressionAmount2 = 2000,

 kMultibandCompressorParam_CompressionAmount3 = 3000,

 kMultibandCompressorParam_CompressionAmount4 = 4000,


 kMultibandCompressorParam_InputAmplitude1 = 5000,

 kMultibandCompressorParam_InputAmplitude2 = 6000,

 kMultibandCompressorParam_InputAmplitude3 = 7000,

 kMultibandCompressorParam_InputAmplitude4 = 8000,


 kMultibandCompressorParam_OutputAmplitude1 = 9000,

 kMultibandCompressorParam_OutputAmplitude2 = 10000,

 kMultibandCompressorParam_OutputAmplitude3 = 11000,

 kMultibandCompressorParam_OutputAmplitude4 = 12000
};


enum
{

 kMultibandFilter_LowFilterType = 0,

 kMultibandFilter_LowFrequency = 1,

 kMultibandFilter_LowGain = 2,


 kMultibandFilter_CenterFreq1 = 3,

 kMultibandFilter_CenterGain1 = 4,

 kMultibandFilter_Bandwidth1 = 5,


 kMultibandFilter_CenterFreq2 = 6,

 kMultibandFilter_CenterGain2 = 7,

 kMultibandFilter_Bandwidth2 = 8,


 kMultibandFilter_CenterFreq3 = 9,

 kMultibandFilter_CenterGain3 = 10,

 kMultibandFilter_Bandwidth3 = 11,


 kMultibandFilter_HighFilterType = 12,

 kMultibandFilter_HighFrequency = 13,

 kMultibandFilter_HighGain = 14
};


enum {

 kRogerBeepParam_InGateThreshold = 0,

 kRogerBeepParam_InGateThresholdTime = 1,

 kRogerBeepParam_OutGateThreshold = 2,

 kRogerBeepParam_OutGateThresholdTime = 3,

 kRogerBeepParam_Sensitivity = 4,

 kRogerBeepParam_RogerType = 5,

 kRogerBeepParam_RogerGain = 6
};




enum {

 kStereoMixerParam_Volume = 0,


 kStereoMixerParam_Pan = 1,







 kStereoMixerParam_PreAveragePower = 1000,
 kStereoMixerParam_PrePeakHoldLevel = 2000,
 kStereoMixerParam_PostAveragePower = 3000,
 kStereoMixerParam_PostPeakHoldLevel = 4000
};


enum {

 kAUNetReceiveParam_Status = 0,
 kAUNetReceiveParam_NumParameters = 1
};


enum {

 kAUNetSendParam_Status = 0,
 kAUNetSendParam_NumParameters = 1
};



enum {
 kAUNetStatus_NotConnected = 0,
 kAUNetStatus_Connected = 1,
 kAUNetStatus_Overflow = 2,
 kAUNetStatus_Underflow = 3,
 kAUNetStatus_Connecting = 4,
 kAUNetStatus_Listening = 5
};



enum {

 kMusicDeviceParam_Tuning = 0,


 kMusicDeviceParam_Volume = 1,


 kMusicDeviceParam_ReverbVolume = 2
};
enum {
 kRandomParam_BoundA = 0,
 kRandomParam_BoundB = 1,
 kRandomParam_Curve = 2
};






enum {

 kReverb2Param_DryWetMix = 0,

 kReverb2Param_Gain = 1,


 kReverb2Param_MinDelayTime = 2,

 kReverb2Param_MaxDelayTime = 3,

 kReverb2Param_DecayTimeAt0Hz = 4,

 kReverb2Param_DecayTimeAtNyquist = 5,

 kReverb2Param_RandomizeReflections = 6,
};


#endif
