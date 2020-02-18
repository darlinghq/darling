#ifndef AUDIOUNITPROPERTIES_H
#define AUDIOUNITPROPERTIES_H

enum {
        kAudioOutputUnitProperty_CurrentDevice                  = 2000,
        kAudioOutputUnitProperty_ChannelMap                             = 2002,
        kAudioOutputUnitProperty_EnableIO                               = 2003,
        kAudioOutputUnitProperty_StartTime                              = 2004,
        kAudioOutputUnitProperty_SetInputCallback               = 2005,
        kAudioOutputUnitProperty_HasIO                                  = 2006,
        kAudioOutputUnitProperty_StartTimestampsAtZero  = 2007
};

enum {
        kAudioUnitScope_Global          = 0,
        kAudioUnitScope_Input           = 1,
        kAudioUnitScope_Output          = 2,
        kAudioUnitScope_Group           = 3,
        kAudioUnitScope_Part            = 4,
        kAudioUnitScope_Note            = 5,
        kAudioUnitScope_Layer           = 6,
        kAudioUnitScope_LayerItem       = 7
};

enum
{
        kAudioUnitProperty_ClassInfo                                    = 0,
        kAudioUnitProperty_MakeConnection                               = 1,
        kAudioUnitProperty_SampleRate                                   = 2,
        kAudioUnitProperty_ParameterList                                = 3,
        kAudioUnitProperty_ParameterInfo                                = 4,
        kAudioUnitProperty_CPULoad                                              = 6,
        kAudioUnitProperty_StreamFormat                                 = 8,
        kAudioUnitProperty_ElementCount                                 = 11,
        kAudioUnitProperty_Latency                                              = 12,
        kAudioUnitProperty_SupportedNumChannels                 = 13,
        kAudioUnitProperty_MaximumFramesPerSlice                = 14,
        kAudioUnitProperty_ParameterValueStrings                = 16,
        kAudioUnitProperty_AudioChannelLayout                   = 19,  
        kAudioUnitProperty_TailTime                                             = 20,
        kAudioUnitProperty_BypassEffect                                 = 21,
        kAudioUnitProperty_LastRenderError                              = 22,
        kAudioUnitProperty_SetRenderCallback                    = 23,
        kAudioUnitProperty_FactoryPresets                               = 24,
		kAudioUnitProperty_ContextName                          = 25,
        kAudioUnitProperty_RenderQuality                                = 26,
        kAudioUnitProperty_InPlaceProcessing                    = 29,
        kAudioUnitProperty_ElementName                                  = 30,
        kAudioUnitProperty_SupportedChannelLayoutTags   = 32,
        kAudioUnitProperty_PresentPreset                                = 36,
        kAudioUnitProperty_ShouldAllocateBuffer                 = 51,
        kAudioUnitProperty_ParameterHistoryInfo                 = 53
};

enum
{
        kSpatializationAlgorithm_EqualPowerPanning = 0,
        kSpatializationAlgorithm_SphericalHead,
        kSpatializationAlgorithm_HRTF,
        kSpatializationAlgorithm_SoundField,
        kSpatializationAlgorithm_VectorBasedPanning             = 4,
        kSpatializationAlgorithm_StereoPassThrough,
        kSpatializationAlgorithm_HRTFHQ,
};

enum
{
        k3DMixerRenderingFlags_InterAuralDelay                  = (1L << 0),
        k3DMixerRenderingFlags_DopplerShift                             = (1L << 1),
        k3DMixerRenderingFlags_DistanceAttenuation              = (1L << 2),
        k3DMixerRenderingFlags_DistanceFilter                   = (1L << 3),
        k3DMixerRenderingFlags_DistanceDiffusion                = (1L << 4),
        k3DMixerRenderingFlags_LinearDistanceAttenuation = (1L << 5),
        k3DMixerRenderingFlags_ConstantReverbBlend              = (1L << 6)
};

enum
{
        kAudioUnitProperty_3DMixerDistanceParams                = 3010,
        kAudioUnitProperty_3DMixerAttenuationCurve              = 3013,
        kAudioUnitProperty_DopplerShift                                 = 3002,
        kAudioUnitProperty_3DMixerRenderingFlags                = 3003,
        kAudioUnitProperty_3DMixerDistanceAtten                 = 3004,
        kAudioUnitProperty_ReverbPreset                                 = 3012,
};

enum
{
        kAudioUnitProperty_ReverbRoomType                               = 10,
        kAudioUnitProperty_UsesInternalReverb                   = 1005,
        kAudioUnitProperty_SpatializationAlgorithm              = 3000,
        kAudioUnitProperty_SpatialMixerDistanceParams   = 3010,
        kAudioUnitProperty_SpatialMixerAttenuationCurve = 3013,
        kAudioUnitProperty_SpatialMixerRenderingFlags   = 3003,
};

typedef struct MixerDistanceParams {
        Float32 mReferenceDistance;
        Float32 mMaxDistance;
        Float32 mMaxAttenuation;
} MixerDistanceParams;


struct AURenderCallbackStruct
{
	AURenderCallback inputProc;
	void *inputProcRefCon;
};
typedef struct AURenderCallbackStruct AURenderCallbackStruct;

struct AudioUnitConnection
{
	AudioUnit sourceAudioUnit;
	UInt32 sourceOutputNumber;
	UInt32 destInputNumber;
};
typedef struct AudioUnitConnection AudioUnitConnection;

#endif

