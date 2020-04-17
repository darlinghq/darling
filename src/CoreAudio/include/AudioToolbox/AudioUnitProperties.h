#ifndef AUDIOUNITPROPERTIES_H
#define AUDIOUNITPROPERTIES_H
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AUComponent.h>
#include <CoreFoundation/CoreFoundation.h>

enum {
 kAudioUnitScope_Global = 0,
 kAudioUnitScope_Input = 1,
 kAudioUnitScope_Output = 2,
 kAudioUnitScope_Group = 3,
 kAudioUnitScope_Part = 4,
 kAudioUnitScope_Note = 5,
 kAudioUnitScope_Layer = 6,
 kAudioUnitScope_LayerItem = 7
};

enum
{

 kAudioUnitProperty_ClassInfo = 0,
 kAudioUnitProperty_MakeConnection = 1,
 kAudioUnitProperty_SampleRate = 2,
 kAudioUnitProperty_ParameterList = 3,
 kAudioUnitProperty_ParameterInfo = 4,
 kAudioUnitProperty_CPULoad = 6,
 kAudioUnitProperty_StreamFormat = 8,
 kAudioUnitProperty_ElementCount = 11,
 kAudioUnitProperty_Latency = 12,
 kAudioUnitProperty_SupportedNumChannels = 13,
 kAudioUnitProperty_MaximumFramesPerSlice = 14,
 kAudioUnitProperty_ParameterValueStrings = 16,
 kAudioUnitProperty_AudioChannelLayout = 19,
 kAudioUnitProperty_TailTime = 20,
 kAudioUnitProperty_BypassEffect = 21,
 kAudioUnitProperty_LastRenderError = 22,
 kAudioUnitProperty_SetRenderCallback = 23,
 kAudioUnitProperty_FactoryPresets = 24,
 kAudioUnitProperty_RenderQuality = 26,
 kAudioUnitProperty_HostCallbacks = 27,
 kAudioUnitProperty_InPlaceProcessing = 29,
 kAudioUnitProperty_ElementName = 30,
 kAudioUnitProperty_SupportedChannelLayoutTags = 32,
 kAudioUnitProperty_PresentPreset = 36,
 kAudioUnitProperty_DependentParameters = 45,
 kAudioUnitProperty_InputSamplesInOutput = 49,
 kAudioUnitProperty_ShouldAllocateBuffer = 51,
 kAudioUnitProperty_FrequencyResponse = 52,
 kAudioUnitProperty_ParameterHistoryInfo = 53,
 kAudioUnitProperty_NickName = 54,
 kAudioUnitProperty_OfflineRender = 37,
 kAudioUnitProperty_ParameterIDName = 34,
 kAudioUnitProperty_ParameterStringFromValue = 33,
 kAudioUnitProperty_ParameterClumpName = 35,
 kAudioUnitProperty_ParameterValueFromString = 38,
 kAudioUnitProperty_ContextName = 25,
 kAudioUnitProperty_PresentationLatency = 40,
 kAudioUnitProperty_ClassInfoFromDocument = 50,
 kAudioUnitProperty_RequestViewController = 56,
 kAudioUnitProperty_ParametersForOverview = 57,
 kAudioUnitProperty_SupportsMPE = 58,

 kAudioUnitProperty_FastDispatch = 5,
 kAudioUnitProperty_SetExternalBuffer = 15,
 kAudioUnitProperty_GetUIComponentList = 18,
 kAudioUnitProperty_CocoaUI = 31,
 kAudioUnitProperty_IconLocation = 39,
 kAudioUnitProperty_AUHostIdentifier = 46,

 kAudioUnitProperty_MIDIOutputCallbackInfo = 47,
 kAudioUnitProperty_MIDIOutputCallback = 48,
};
typedef struct AudioUnitConnection {
 AudioUnit _Nullable sourceAudioUnit;
 UInt32 sourceOutputNumber;
 UInt32 destInputNumber;
} AudioUnitConnection;

typedef struct AUChannelInfo {
 SInt16 inChannels;
 SInt16 outChannels;
} AUChannelInfo;


typedef struct AudioUnitExternalBuffer {
 Byte * buffer;
 UInt32 size;
} AudioUnitExternalBuffer;

typedef struct AURenderCallbackStruct {
 AURenderCallback _Nullable inputProc;
 void * _Nullable inputProcRefCon;
} AURenderCallbackStruct;
typedef struct AUPreset {
 SInt32 presetNumber;
 CFStringRef _Nullable presetName;
} AUPreset;


enum {
 kRenderQuality_Max = 127,
 kRenderQuality_High = 96,
 kRenderQuality_Medium = 64,
 kRenderQuality_Low = 32,
 kRenderQuality_Min = 0
};
enum {
 kNumberOfResponseFrequencies = 1024
};
typedef struct AudioUnitFrequencyResponseBin
{
 Float64 mFrequency;
 Float64 mMagnitude;
} AudioUnitFrequencyResponseBin;
typedef OSStatus (*HostCallback_GetBeatAndTempo)(void * _Nullable inHostUserData,
           Float64 * _Nullable outCurrentBeat,
           Float64 * _Nullable outCurrentTempo);
typedef OSStatus (*HostCallback_GetMusicalTimeLocation)(void * _Nullable inHostUserData,
            UInt32 * _Nullable outDeltaSampleOffsetToNextBeat,
            Float32 * _Nullable outTimeSig_Numerator,
            UInt32 * _Nullable outTimeSig_Denominator,
            Float64 * _Nullable outCurrentMeasureDownBeat);
typedef OSStatus (*HostCallback_GetTransportState)(void * _Nullable inHostUserData,
          Boolean * _Nullable outIsPlaying,
          Boolean * _Nullable outTransportStateChanged,
          Float64 * _Nullable outCurrentSampleInTimeLine,
          Boolean * _Nullable outIsCycling,
          Float64 * _Nullable outCycleStartBeat,
          Float64 * _Nullable outCycleEndBeat);
typedef OSStatus (*HostCallback_GetTransportState2)(void * _Nullable inHostUserData,
          Boolean * _Nullable outIsPlaying,
          Boolean * _Nullable outIsRecording,
          Boolean * _Nullable outTransportStateChanged,
          Float64 * _Nullable outCurrentSampleInTimeLine,
          Boolean * _Nullable outIsCycling,
          Float64 * _Nullable outCycleStartBeat,
          Float64 * _Nullable outCycleEndBeat);


typedef struct HostCallbackInfo {
 void * _Nullable hostUserData;
 HostCallback_GetBeatAndTempo _Nullable beatAndTempoProc;
    HostCallback_GetMusicalTimeLocation _Nullable musicalTimeLocationProc;
 HostCallback_GetTransportState _Nullable transportStateProc;
 HostCallback_GetTransportState2 _Nullable transportStateProc2;
} HostCallbackInfo;






typedef struct AUDependentParameter {
 AudioUnitScope mScope;
 AudioUnitParameterID mParameterID;
} AUDependentParameter;
typedef struct AudioUnitCocoaViewInfo {
 CFURLRef mCocoaAUViewBundleLocation;
 CFStringRef _Nonnull mCocoaAUViewClass[1];
} AudioUnitCocoaViewInfo;





typedef struct AUHostVersionIdentifier {
 CFStringRef hostName;
 UInt32 hostVersion;
} AUHostVersionIdentifier;





struct MIDIPacketList;





typedef OSStatus
(*AUMIDIOutputCallback)(void * _Nullable userData,
      const AudioTimeStamp * timeStamp,
      UInt32 midiOutNum,
      const struct MIDIPacketList * pktlist);






typedef struct AUMIDIOutputCallbackStruct {
 AUMIDIOutputCallback midiOutputCallback;
 void * _Nullable userData;
} AUMIDIOutputCallbackStruct;






typedef struct AUInputSamplesInOutputCallbackStruct {
 AUInputSamplesInOutputCallback inputToOutputCallback;
 void * _Nullable userData;
} AUInputSamplesInOutputCallbackStruct;
typedef struct AudioUnitParameterHistoryInfo
{
 Float32 updatesPerSecond;
 Float32 historyDurationInSeconds;
} AudioUnitParameterHistoryInfo;
typedef UInt32 AudioUnitParameterUnit; enum
{
 kAudioUnitParameterUnit_Generic = 0,
 kAudioUnitParameterUnit_Indexed = 1,
 kAudioUnitParameterUnit_Boolean = 2,
 kAudioUnitParameterUnit_Percent = 3,
 kAudioUnitParameterUnit_Seconds = 4,
 kAudioUnitParameterUnit_SampleFrames = 5,
 kAudioUnitParameterUnit_Phase = 6,
 kAudioUnitParameterUnit_Rate = 7,
 kAudioUnitParameterUnit_Hertz = 8,
 kAudioUnitParameterUnit_Cents = 9,
 kAudioUnitParameterUnit_RelativeSemiTones = 10,
 kAudioUnitParameterUnit_MIDINoteNumber = 11,
 kAudioUnitParameterUnit_MIDIController = 12,
 kAudioUnitParameterUnit_Decibels = 13,
 kAudioUnitParameterUnit_LinearGain = 14,
 kAudioUnitParameterUnit_Degrees = 15,
 kAudioUnitParameterUnit_EqualPowerCrossfade = 16,
 kAudioUnitParameterUnit_MixerFaderCurve1 = 17,
 kAudioUnitParameterUnit_Pan = 18,
 kAudioUnitParameterUnit_Meters = 19,
 kAudioUnitParameterUnit_AbsoluteCents = 20,
 kAudioUnitParameterUnit_Octaves = 21,
 kAudioUnitParameterUnit_BPM = 22,
    kAudioUnitParameterUnit_Beats = 23,
 kAudioUnitParameterUnit_Milliseconds = 24,
 kAudioUnitParameterUnit_Ratio = 25,
 kAudioUnitParameterUnit_CustomUnit = 26
};
typedef UInt32 AudioUnitParameterOptions; enum
{
 kAudioUnitParameterFlag_CFNameRelease = (1UL << 4),

 kAudioUnitParameterFlag_OmitFromPresets = (1UL << 13),
 kAudioUnitParameterFlag_PlotHistory = (1UL << 14),
 kAudioUnitParameterFlag_MeterReadOnly = (1UL << 15),


 kAudioUnitParameterFlag_DisplayMask = (7UL << 16) | (1UL << 22),
 kAudioUnitParameterFlag_DisplaySquareRoot = (1UL << 16),
 kAudioUnitParameterFlag_DisplaySquared = (2UL << 16),
 kAudioUnitParameterFlag_DisplayCubed = (3UL << 16),
 kAudioUnitParameterFlag_DisplayCubeRoot = (4UL << 16),
 kAudioUnitParameterFlag_DisplayExponential = (5UL << 16),

 kAudioUnitParameterFlag_HasClump = (1UL << 20),
 kAudioUnitParameterFlag_ValuesHaveStrings = (1UL << 21),

 kAudioUnitParameterFlag_DisplayLogarithmic = (1UL << 22),

 kAudioUnitParameterFlag_IsHighResolution = (1UL << 23),
 kAudioUnitParameterFlag_NonRealTime = (1UL << 24),
 kAudioUnitParameterFlag_CanRamp = (1UL << 25),
 kAudioUnitParameterFlag_ExpertMode = (1UL << 26),
 kAudioUnitParameterFlag_HasCFNameString = (1UL << 27),
 kAudioUnitParameterFlag_IsGlobalMeta = (1UL << 28),
 kAudioUnitParameterFlag_IsElementMeta = (1UL << 29),
 kAudioUnitParameterFlag_IsReadable = (1UL << 30),
 kAudioUnitParameterFlag_IsWritable = (1UL << 31)
};
typedef struct AudioUnitParameterInfo
{
 char name[52];
 CFStringRef _Nullable unitName;
 UInt32 clumpID;
 CFStringRef _Nullable cfNameString;
 AudioUnitParameterUnit unit;
 AudioUnitParameterValue minValue;
 AudioUnitParameterValue maxValue;
 AudioUnitParameterValue defaultValue;
 AudioUnitParameterOptions flags;
} AudioUnitParameterInfo;





enum {
 kAudioUnitClumpID_System = 0
};



static __inline__ __attribute__((always_inline)) AudioUnitParameterOptions GetAudioUnitParameterDisplayType(AudioUnitParameterOptions flags)
{
 return flags & kAudioUnitParameterFlag_DisplayMask;
}

static __inline__ __attribute__((always_inline)) AudioUnitParameterOptions SetAudioUnitParameterDisplayType(AudioUnitParameterOptions flags, AudioUnitParameterOptions displayType)
{
 return (flags & ~kAudioUnitParameterFlag_DisplayMask) | displayType;
}
enum {
 kAudioUnitParameterName_Full = -1
};





typedef struct AudioUnitParameterNameInfo {
 AudioUnitParameterID inID;
 SInt32 inDesiredLength;
 CFStringRef _Nullable outName;
} AudioUnitParameterIDName;





typedef struct AudioUnitParameterStringFromValue {
 AudioUnitParameterID inParamID;
 const AudioUnitParameterValue * inValue;
 CFStringRef _Nullable outString;
} AudioUnitParameterStringFromValue;





typedef struct AudioUnitParameterValueFromString {
 AudioUnitParameterID inParamID;
 CFStringRef inString;
 AudioUnitParameterValue outValue;
} AudioUnitParameterValueFromString;
enum {

 kAudioOutputUnitProperty_IsRunning = 2001
};
enum {
 kAudioUnitProperty_AllParameterMIDIMappings = 41,
 kAudioUnitProperty_AddParameterMIDIMapping = 42,
 kAudioUnitProperty_RemoveParameterMIDIMapping = 43,
 kAudioUnitProperty_HotMapParameterMIDIMapping = 44
};
typedef UInt32 AUParameterMIDIMappingFlags; enum {
 kAUParameterMIDIMapping_AnyChannelFlag = (1L << 0),
 kAUParameterMIDIMapping_AnyNoteFlag = (1L << 1),
 kAUParameterMIDIMapping_SubRange = (1L << 2),
 kAUParameterMIDIMapping_Toggle = (1L << 3),
 kAUParameterMIDIMapping_Bipolar = (1L << 4),
 kAUParameterMIDIMapping_Bipolar_On = (1L << 5)
};
typedef struct AUParameterMIDIMapping
{
 AudioUnitScope mScope;
 AudioUnitElement mElement;
 AudioUnitParameterID mParameterID;
 AUParameterMIDIMappingFlags mFlags;
 AudioUnitParameterValue mSubRangeMin;
 AudioUnitParameterValue mSubRangeMax;
 UInt8 mStatus;
 UInt8 mData1;
 UInt8 reserved1;
 UInt8 reserved2;
 UInt32 reserved3;
} AUParameterMIDIMapping;
enum {

 kMusicDeviceProperty_MIDIXMLNames = 1006,
 kMusicDeviceProperty_PartGroup = 1010,
 kMusicDeviceProperty_DualSchedulingMode = 1013,
 kMusicDeviceProperty_SupportsStartStopNote = 1014
};




enum {
 kMusicDeviceSampleFrameMask_SampleOffset = 0xFFFFFF,
 kMusicDeviceSampleFrameMask_IsScheduled = 0x01000000
};
enum {

 kAudioUnitOfflineProperty_InputSize = 3020,
 kAudioUnitOfflineProperty_OutputSize = 3021,
 kAudioUnitOfflineProperty_StartOffset = 3022,
 kAudioUnitOfflineProperty_PreflightRequirements = 3023,
 kAudioUnitOfflineProperty_PreflightName = 3024
};
enum {
 kOfflinePreflight_NotRequired = 0,
 kOfflinePreflight_Optional = 1,
 kOfflinePreflight_Required = 2
};
enum {

 kAudioUnitProperty_DistanceAttenuationData = 3600
} __attribute__((availability(macosx,introduced=10.5,deprecated=10.11)));




typedef struct AUDistanceAttenuationData
{
 UInt32 inNumberOfPairs;
 struct {
  Float32 inDistance;
  Float32 outGain;
 } pairs[1];
} AUDistanceAttenuationData __attribute__((availability(macosx,introduced=10.5,deprecated=10.11)));
enum {

  kAudioUnitMigrateProperty_FromPlugin = 4000,
 kAudioUnitMigrateProperty_OldAutomation = 4001
};




enum {
 kOtherPluginFormat_Undefined = 0,
 kOtherPluginFormat_kMAS = 1,
 kOtherPluginFormat_kVST = 2,
 kOtherPluginFormat_AU = 3
};
typedef struct AudioUnitOtherPluginDesc
{
 UInt32 format;
 AudioClassDescription plugin;
} AudioUnitOtherPluginDesc;






typedef struct AudioUnitParameterValueTranslation
{
 AudioUnitOtherPluginDesc otherDesc;
 UInt32 otherParamID;
 Float32 otherValue;
 AudioUnitParameterID auParamID;
 AudioUnitParameterValue auValue;
} AudioUnitParameterValueTranslation;






typedef struct AudioUnitPresetMAS_SettingData
{
 UInt32 isStockSetting;
 UInt32 settingID;
 UInt32 dataLen;
 UInt8 data[1];
} AudioUnitPresetMAS_SettingData;





typedef struct AudioUnitPresetMAS_Settings
{
 UInt32 manufacturerID;
 UInt32 effectID;
 UInt32 variantID;
 UInt32 settingsVersion;
 UInt32 numberOfSettings;
 AudioUnitPresetMAS_SettingData settings[1];
} AudioUnitPresetMAS_Settings;
enum {
 kAudioUnitProperty_SampleRateConverterComplexity = 3014
};
enum {
 kAudioUnitSampleRateConverterComplexity_Linear = 'line',
 kAudioUnitSampleRateConverterComplexity_Normal = 'norm',
 kAudioUnitSampleRateConverterComplexity_Mastering = 'bats'
};
enum {
 kAudioOutputUnitProperty_CurrentDevice = 2000,
 kAudioOutputUnitProperty_ChannelMap = 2002,
 kAudioOutputUnitProperty_EnableIO = 2003,
 kAudioOutputUnitProperty_StartTime = 2004,
 kAudioOutputUnitProperty_SetInputCallback = 2005,
 kAudioOutputUnitProperty_HasIO = 2006,
 kAudioOutputUnitProperty_StartTimestampsAtZero = 2007
};





typedef struct AudioOutputUnitStartAtTimeParams {

 AudioTimeStamp mTimestamp;
 UInt32 mFlags;
} AudioOutputUnitStartAtTimeParams;
enum {
 kAUVoiceIOProperty_BypassVoiceProcessing = 2100,
 kAUVoiceIOProperty_VoiceProcessingEnableAGC = 2101,
 kAUVoiceIOProperty_MuteOutput = 2104

};
enum {
 kAUVoiceIOProperty_VoiceProcessingQuality = 2103
} __attribute__((availability(macosx,introduced=10.7,deprecated=10.9)));
enum {
 kAUNBandEQProperty_NumberOfBands = 2200,
 kAUNBandEQProperty_MaxNumberOfBands = 2201,
 kAUNBandEQProperty_BiquadCoefficients = 2203
};
enum {
    kAUVoiceIOErr_UnexpectedNumberOfInputChannels = -66784,
};
enum {

 kAudioUnitProperty_MeteringMode = 3007,


 kAudioUnitProperty_MatrixLevels = 3006,
 kAudioUnitProperty_MatrixDimensions = 3009,
 kAudioUnitProperty_MeterClipping = 3011,


 kAudioUnitProperty_InputAnchorTimeStamp = 3016
};
typedef struct AudioUnitMeterClipping
{
 Float32 peakValueSinceLastCall;
 Boolean sawInfinity;
 Boolean sawNotANumber;
} AudioUnitMeterClipping;
enum {
 kAudioUnitProperty_ReverbRoomType = 10,
 kAudioUnitProperty_UsesInternalReverb = 1005,
 kAudioUnitProperty_SpatializationAlgorithm = 3000,
 kAudioUnitProperty_SpatialMixerDistanceParams = 3010,
 kAudioUnitProperty_SpatialMixerAttenuationCurve = 3013,
 kAudioUnitProperty_SpatialMixerRenderingFlags = 3003,
};




typedef UInt32 AUSpatializationAlgorithm; enum {
 kSpatializationAlgorithm_EqualPowerPanning = 0,
 kSpatializationAlgorithm_SphericalHead = 1,
 kSpatializationAlgorithm_HRTF = 2,
 kSpatializationAlgorithm_SoundField = 3,
 kSpatializationAlgorithm_VectorBasedPanning = 4,
 kSpatializationAlgorithm_StereoPassThrough = 5,
    kSpatializationAlgorithm_HRTFHQ = 6
};






typedef UInt32 AUReverbRoomType; enum {
 kReverbRoomType_SmallRoom = 0,
 kReverbRoomType_MediumRoom = 1,
 kReverbRoomType_LargeRoom = 2,
 kReverbRoomType_MediumHall = 3,
 kReverbRoomType_LargeHall = 4,
 kReverbRoomType_Plate = 5,
 kReverbRoomType_MediumChamber = 6,
 kReverbRoomType_LargeChamber = 7,
 kReverbRoomType_Cathedral = 8,
 kReverbRoomType_LargeRoom2 = 9,
 kReverbRoomType_MediumHall2 = 10,
 kReverbRoomType_MediumHall3 = 11,
 kReverbRoomType_LargeHall2 = 12
};




typedef UInt32 AUSpatialMixerAttenuationCurve; enum {
 kSpatialMixerAttenuationCurve_Power = 0,
    kSpatialMixerAttenuationCurve_Exponential = 1,
    kSpatialMixerAttenuationCurve_Inverse = 2,
    kSpatialMixerAttenuationCurve_Linear = 3
};




typedef struct MixerDistanceParams {
 Float32 mReferenceDistance;
 Float32 mMaxDistance;
 Float32 mMaxAttenuation;
} MixerDistanceParams;




typedef UInt32 AUSpatialMixerRenderingFlags; enum {
 kSpatialMixerRenderingFlags_InterAuralDelay = (1L << 0),
 kSpatialMixerRenderingFlags_DistanceAttenuation = (1L << 2),
};
enum {
 kAudioUnitProperty_3DMixerDistanceParams = 3010,
 kAudioUnitProperty_3DMixerAttenuationCurve = 3013,
 kAudioUnitProperty_DopplerShift = 3002,
 kAudioUnitProperty_3DMixerRenderingFlags = 3003,
 kAudioUnitProperty_3DMixerDistanceAtten = 3004,
 kAudioUnitProperty_ReverbPreset = 3012
} __attribute__((availability(macosx,introduced=10.7,deprecated=10.11)));




typedef UInt32 AU3DMixerRenderingFlags; enum {
 k3DMixerRenderingFlags_InterAuralDelay = (1L << 0),
 k3DMixerRenderingFlags_DopplerShift = (1L << 1),
 k3DMixerRenderingFlags_DistanceAttenuation = (1L << 2),
 k3DMixerRenderingFlags_DistanceFilter = (1L << 3),
 k3DMixerRenderingFlags_DistanceDiffusion = (1L << 4),
 k3DMixerRenderingFlags_LinearDistanceAttenuation = (1L << 5),
 k3DMixerRenderingFlags_ConstantReverbBlend = (1L << 6)
};




typedef UInt32 AU3DMixerAttenuationCurve; enum {
 k3DMixerAttenuationCurve_Power = 0,
    k3DMixerAttenuationCurve_Exponential = 1,
    k3DMixerAttenuationCurve_Inverse = 2,
    k3DMixerAttenuationCurve_Linear = 3
};
enum {
 kAudioUnitProperty_ScheduleAudioSlice = 3300,
 kAudioUnitProperty_ScheduleStartTimeStamp = 3301,
 kAudioUnitProperty_CurrentPlayTime = 3302
};
typedef UInt32 AUScheduledAudioSliceFlags; enum {
    kScheduledAudioSliceFlag_Complete = 0x01,
    kScheduledAudioSliceFlag_BeganToRender = 0x02,
    kScheduledAudioSliceFlag_BeganToRenderLate = 0x04,

    kScheduledAudioSliceFlag_Loop = 0x08,
    kScheduledAudioSliceFlag_Interrupt = 0x10,
    kScheduledAudioSliceFlag_InterruptAtLoop = 0x20
};

typedef struct ScheduledAudioSlice ScheduledAudioSlice;



typedef void (*ScheduledAudioSliceCompletionProc)(void * _Nullable userData, ScheduledAudioSlice *bufferList);
struct ScheduledAudioSlice {
 AudioTimeStamp mTimeStamp;
 ScheduledAudioSliceCompletionProc _Nullable mCompletionProc;
 void * mCompletionProcUserData;
 AUScheduledAudioSliceFlags mFlags;
 UInt32 mReserved;
 void * _Nullable mReserved2;
 UInt32 mNumberFrames;
 AudioBufferList * mBufferList;
};
enum {
 kAudioUnitProperty_ScheduledFileIDs = 3310,
 kAudioUnitProperty_ScheduledFileRegion = 3311,
 kAudioUnitProperty_ScheduledFilePrime = 3312,
 kAudioUnitProperty_ScheduledFileBufferSizeFrames = 3313,
 kAudioUnitProperty_ScheduledFileNumberBuffers = 3314
};

typedef struct ScheduledAudioFileRegion ScheduledAudioFileRegion;



typedef void (*ScheduledAudioFileRegionCompletionProc)(void * _Nullable userData,
    ScheduledAudioFileRegion *fileRegion, OSStatus result);
struct ScheduledAudioFileRegion {
 AudioTimeStamp mTimeStamp;
 ScheduledAudioFileRegionCompletionProc _Nullable mCompletionProc;
 void * _Nullable mCompletionProcUserData;
 struct OpaqueAudioFileID * mAudioFile;
 UInt32 mLoopCount;
 SInt64 mStartFrame;
 UInt32 mFramesToPlay;
};
enum {
 kMusicDeviceProperty_UsesInternalReverb = kAudioUnitProperty_UsesInternalReverb,
 kMusicDeviceProperty_SoundBankData = 1008,
 kMusicDeviceProperty_StreamFromDisk = 1011,
 kMusicDeviceProperty_SoundBankFSRef = 1012
};
enum {
 kMusicDeviceProperty_InstrumentName = 1001,
 kMusicDeviceProperty_InstrumentNumber = 1004
};
enum {
 kMusicDeviceProperty_InstrumentCount = 1000,
 kMusicDeviceProperty_BankName = 1007,
 kMusicDeviceProperty_SoundBankURL = 1100
};
enum {
 kAUMIDISynthProperty_EnablePreload = 4119
};
enum {

 kAUSamplerProperty_LoadInstrument = 4102,
 kAUSamplerProperty_LoadAudioFiles = 4101
};
typedef struct AUSamplerInstrumentData {
 CFURLRef fileURL;
 UInt8 instrumentType;
 UInt8 bankMSB;
 UInt8 bankLSB;
 UInt8 presetID;
} AUSamplerInstrumentData;
enum
{
 kInstrumentType_DLSPreset = 1,
 kInstrumentType_SF2Preset = kInstrumentType_DLSPreset,
 kInstrumentType_AUPreset = 2,
 kInstrumentType_Audiofile = 3,
 kInstrumentType_EXS24 = 4
};

enum
{
 kAUSampler_DefaultPercussionBankMSB = 0x78,
 kAUSampler_DefaultMelodicBankMSB = 0x79,
 kAUSampler_DefaultBankLSB = 0x00
};
enum {
 kAudioUnitProperty_DeferredRendererPullSize = 3320,
 kAudioUnitProperty_DeferredRendererExtraLatency = 3321,
 kAudioUnitProperty_DeferredRendererWaitFrames = 3322
};
enum {
 kAUNetReceiveProperty_Hostname = 3511,
 kAUNetReceiveProperty_Password = 3512
};
enum {
 kAUNetSendProperty_PortNum = 3513,
 kAUNetSendProperty_TransmissionFormat = 3514,
 kAUNetSendProperty_TransmissionFormatIndex = 3515,
 kAUNetSendProperty_ServiceName = 3516,
 kAUNetSendProperty_Disconnect = 3517,
 kAUNetSendProperty_Password = 3518
};
enum {
 kAUNetSendPresetFormat_PCMFloat32 = 0,
 kAUNetSendPresetFormat_PCMInt24 = 1,
 kAUNetSendPresetFormat_PCMInt16 = 2,
 kAUNetSendPresetFormat_Lossless24 = 3,
 kAUNetSendPresetFormat_Lossless16 = 4,
 kAUNetSendPresetFormat_ULaw = 5,
 kAUNetSendPresetFormat_IMA4 = 6,
 kAUNetSendPresetFormat_AAC_128kbpspc = 7,
 kAUNetSendPresetFormat_AAC_96kbpspc = 8,
 kAUNetSendPresetFormat_AAC_80kbpspc = 9,
 kAUNetSendPresetFormat_AAC_64kbpspc = 10,
 kAUNetSendPresetFormat_AAC_48kbpspc = 11,
 kAUNetSendPresetFormat_AAC_40kbpspc = 12,
 kAUNetSendPresetFormat_AAC_32kbpspc = 13,
 kAUNetSendPresetFormat_AAC_LD_64kbpspc = 14,
 kAUNetSendPresetFormat_AAC_LD_48kbpspc = 15,
 kAUNetSendPresetFormat_AAC_LD_40kbpspc = 16,
 kAUNetSendPresetFormat_AAC_LD_32kbpspc = 17,
 kAUNetSendNumPresetFormats = 18
};
typedef struct AUNumVersion {

  UInt8 nonRelRev;
  UInt8 stage;
  UInt8 minorAndBugRev;
  UInt8 majorRev;
} AUNumVersion;






typedef struct AUHostIdentifier {
 CFStringRef hostName;
 AUNumVersion hostVersion;
} AUHostIdentifier;



enum {
 kAudioUnitParameterFlag_Global = (1L << 0),
 kAudioUnitParameterFlag_Input = (1L << 1),
 kAudioUnitParameterFlag_Output = (1L << 2),
 kAudioUnitParameterFlag_Group = (1L << 3)
};

enum {
 kAudioUnitParameterFlag_HasName = kAudioUnitParameterFlag_ValuesHaveStrings
};

enum {

 kAudioUnitProperty_SRCAlgorithm = 9,
 kAudioUnitProperty_MIDIControlMapping = 17,
 kAudioUnitProperty_CurrentPreset = 28,

 kAudioUnitProperty_ParameterValueName = kAudioUnitProperty_ParameterStringFromValue,
 kAudioUnitProperty_BusCount = kAudioUnitProperty_ElementCount,

 kAudioOfflineUnitProperty_InputSize = kAudioUnitOfflineProperty_InputSize,
 kAudioOfflineUnitProperty_OutputSize = kAudioUnitOfflineProperty_OutputSize
};

enum {
 kAudioUnitSRCAlgorithm_Polyphase = 'poly',
 kAudioUnitSRCAlgorithm_MediumQuality = 'csrc'
};



typedef struct AudioUnitMIDIControlMapping
{
 UInt16 midiNRPN;
 UInt8 midiControl;
 UInt8 scope;
 AudioUnitElement element;
 AudioUnitParameterID parameter;
} AudioUnitMIDIControlMapping;


typedef struct AudioUnitParameterValueName {
 AudioUnitParameterID inParamID;
 const Float32 * _Nonnull inValue;
 CFStringRef outName;
} AudioUnitParameterValueName;




enum {
 kMusicDeviceProperty_GroupOutputBus = 1002,
 kMusicDeviceProperty_SoundBankFSSpec = 1003,
 kAudioUnitProperty_PannerMode = 3008
};

enum {
 kAudioUnitProperty_SpeakerConfiguration = 3001
};



enum {
 kSpeakerConfiguration_HeadPhones = 0,
 kSpeakerConfiguration_Stereo = 1,
 kSpeakerConfiguration_Quad = 2,
 kSpeakerConfiguration_5_0 = 3,
 kSpeakerConfiguration_5_1 = kSpeakerConfiguration_5_0
};





typedef struct AUSamplerBankPresetData {
 CFURLRef bankURL;
 UInt8 bankMSB;
 UInt8 bankLSB;
 UInt8 presetID;
 UInt8 reserved;
} AUSamplerBankPresetData;

enum {
 kAUSamplerProperty_LoadPresetFromBank = 4100,
 kAUSamplerProperty_BankAndPreset = kAUSamplerProperty_LoadPresetFromBank
};

#define kAUPresetVersionKey             "version"
#define kAUPresetTypeKey                        "type"
#define kAUPresetSubtypeKey             "subtype"
#define kAUPresetManufacturerKey        "manufacturer"
#define kAUPresetDataKey                        "data"
#define kAUPresetNameKey                        "name"
#define kAUPresetRenderQualityKey       "render-quality"
#define kAUPresetCPULoadKey                     "cpu-load"
#define kAUPresetElementNameKey         "element-name"
#define kAUPresetExternalFileRefs       "file-references"
#define kAUPresetPartKey                        "part"

#endif

