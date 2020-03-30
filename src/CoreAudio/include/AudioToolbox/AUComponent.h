/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef AUCOMPONENT_H
#define AUCOMPONENT_H
#include <MacTypes.h>
#include <CoreFoundation/CFString.h>
#include <AudioToolbox/AudioComponent.h>
#include <CoreAudio/CoreAudioTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef AudioComponentInstance AudioUnit;
enum {
 kAudioUnitType_Output = 'auou',
 kAudioUnitType_MusicDevice = 'aumu',
 kAudioUnitType_MusicEffect = 'aumf',
 kAudioUnitType_FormatConverter = 'aufc',
 kAudioUnitType_Effect = 'aufx',
 kAudioUnitType_Mixer = 'aumx',
 kAudioUnitType_Panner = 'aupn',
 kAudioUnitType_Generator = 'augn',
 kAudioUnitType_OfflineEffect = 'auol',
 kAudioUnitType_MIDIProcessor = 'aumi'
};
enum {
 kAudioUnitManufacturer_Apple = 'appl'
};
enum {
 kAudioUnitSubType_GenericOutput = 'genr',
 kAudioUnitSubType_VoiceProcessingIO = 'vpio'
};
enum {
 kAudioUnitSubType_HALOutput = 'ahal',
 kAudioUnitSubType_DefaultOutput = 'def ',
 kAudioUnitSubType_SystemOutput = 'sys ',
};
enum {
 kAudioUnitSubType_DLSSynth = 'dls ',
 kAudioUnitSubType_Sampler = 'samp',
 kAudioUnitSubType_MIDISynth = 'msyn'
};
enum {
 kAudioUnitSubType_AUConverter = 'conv',
 kAudioUnitSubType_Varispeed = 'vari',
 kAudioUnitSubType_DeferredRenderer = 'defr',
 kAudioUnitSubType_Splitter = 'splt',
 kAudioUnitSubType_MultiSplitter = 'mspl',
 kAudioUnitSubType_Merger = 'merg',
 kAudioUnitSubType_NewTimePitch = 'nutp',
 kAudioUnitSubType_AUiPodTimeOther = 'ipto',
 kAudioUnitSubType_RoundTripAAC = 'raac',
};
enum {
 kAudioUnitSubType_TimePitch = 'tmpt'
};
enum {
 kAudioUnitSubType_PeakLimiter = 'lmtr',
 kAudioUnitSubType_DynamicsProcessor = 'dcmp',
 kAudioUnitSubType_LowPassFilter = 'lpas',
 kAudioUnitSubType_HighPassFilter = 'hpas',
 kAudioUnitSubType_BandPassFilter = 'bpas',
 kAudioUnitSubType_HighShelfFilter = 'hshf',
 kAudioUnitSubType_LowShelfFilter = 'lshf',
 kAudioUnitSubType_ParametricEQ = 'pmeq',
 kAudioUnitSubType_Distortion = 'dist',
 kAudioUnitSubType_Delay = 'dely',
 kAudioUnitSubType_SampleDelay = 'sdly',
 kAudioUnitSubType_NBandEQ = 'nbeq'
};
enum {
 kAudioUnitSubType_GraphicEQ = 'greq',
 kAudioUnitSubType_MultiBandCompressor = 'mcmp',
 kAudioUnitSubType_MatrixReverb = 'mrev',
 kAudioUnitSubType_Pitch = 'tmpt',
 kAudioUnitSubType_AUFilter = 'filt',
 kAudioUnitSubType_NetSend = 'nsnd',
 kAudioUnitSubType_RogerBeep = 'rogr'
};
enum {
 kAudioUnitSubType_MultiChannelMixer = 'mcmx',
 kAudioUnitSubType_MatrixMixer = 'mxmx',
    kAudioUnitSubType_SpatialMixer = '3dem',
};
enum {
 kAudioUnitSubType_StereoMixer = 'smxr',
 kAudioUnitSubType_3DMixer = '3dmx',

};
enum {
 kAudioUnitSubType_SphericalHeadPanner = 'sphr',
 kAudioUnitSubType_VectorPanner = 'vbas',
 kAudioUnitSubType_SoundFieldPanner = 'ambi',
 kAudioUnitSubType_HRTFPanner = 'hrtf'
};
enum {
 kAudioUnitSubType_NetReceive = 'nrcv',
 kAudioUnitSubType_ScheduledSoundPlayer = 'sspl',
 kAudioUnitSubType_AudioFilePlayer = 'afpl'
};
typedef UInt32 AudioUnitRenderActionFlags; enum
{
 kAudioUnitRenderAction_PreRender = (1UL << 2),
 kAudioUnitRenderAction_PostRender = (1UL << 3),
 kAudioUnitRenderAction_OutputIsSilence = (1UL << 4),
 kAudioOfflineUnitRenderAction_Preflight = (1UL << 5),
 kAudioOfflineUnitRenderAction_Render = (1UL << 6),
 kAudioOfflineUnitRenderAction_Complete = (1UL << 7),
 kAudioUnitRenderAction_PostRenderError = (1UL << 8),
 kAudioUnitRenderAction_DoNotCheckRenderArgs = (1UL << 9)
};
enum {
 kAudioUnitErr_InvalidProperty = -10879,
 kAudioUnitErr_InvalidParameter = -10878,
 kAudioUnitErr_InvalidElement = -10877,
 kAudioUnitErr_NoConnection = -10876,
 kAudioUnitErr_FailedInitialization = -10875,
 kAudioUnitErr_TooManyFramesToProcess = -10874,
 kAudioUnitErr_InvalidFile = -10871,
 kAudioUnitErr_UnknownFileType = -10870,
 kAudioUnitErr_FileNotSpecified = -10869,
 kAudioUnitErr_FormatNotSupported = -10868,
 kAudioUnitErr_Uninitialized = -10867,
 kAudioUnitErr_InvalidScope = -10866,
 kAudioUnitErr_PropertyNotWritable = -10865,
 kAudioUnitErr_CannotDoInCurrentContext = -10863,
 kAudioUnitErr_InvalidPropertyValue = -10851,
 kAudioUnitErr_PropertyNotInUse = -10850,
 kAudioUnitErr_Initialized = -10849,
 kAudioUnitErr_InvalidOfflineRender = -10848,
 kAudioUnitErr_Unauthorized = -10847,
    kAudioUnitErr_MIDIOutputBufferFull = -66753,
    kAudioComponentErr_InstanceInvalidated = -66749,
 kAudioUnitErr_RenderTimeout = -66745,
 kAudioUnitErr_ExtensionNotFound = -66744
};
typedef UInt32 AudioUnitPropertyID;

typedef UInt32 AudioUnitScope;


typedef UInt32 AudioUnitElement;

typedef UInt32 AudioUnitParameterID;

typedef Float32 AudioUnitParameterValue;
typedef UInt32 AUParameterEventType; enum
{
 kParameterEvent_Immediate = 1,
 kParameterEvent_Ramped = 2
};
struct AudioUnitParameterEvent
{
 AudioUnitScope scope;
 AudioUnitElement element;
 AudioUnitParameterID parameter;

 AUParameterEventType eventType;

 union
 {
  struct
  {
   SInt32 startBufferOffset;
   UInt32 durationInFrames;
   AudioUnitParameterValue startValue;
   AudioUnitParameterValue endValue;
  } ramp;

  struct
  {
   UInt32 bufferOffset;
   AudioUnitParameterValue value;
  } immediate;

 } eventValues;
};
typedef struct AudioUnitParameterEvent AudioUnitParameterEvent;
struct AudioUnitParameter
{
 AudioUnit mAudioUnit;
 AudioUnitParameterID mParameterID;
 AudioUnitScope mScope;
 AudioUnitElement mElement;
};
typedef struct AudioUnitParameter AudioUnitParameter;
struct AudioUnitProperty
{
 AudioUnit mAudioUnit;
 AudioUnitPropertyID mPropertyID;
 AudioUnitScope mScope;
 AudioUnitElement mElement;
};
typedef struct AudioUnitProperty AudioUnitProperty;
typedef OSStatus
(*AURenderCallback)( void * inRefCon,
      AudioUnitRenderActionFlags * ioActionFlags,
      const AudioTimeStamp * inTimeStamp,
      UInt32 inBusNumber,
      UInt32 inNumberFrames,
      AudioBufferList * _Nullable ioData);
typedef void
(*AudioUnitPropertyListenerProc)( void * inRefCon,
         AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement);
typedef void
(*AUInputSamplesInOutputCallback)( void * inRefCon,
         const AudioTimeStamp * inOutputTimeStamp,
         Float64 inInputSample,
         Float64 inNumberInputSamples);
extern const CFStringRef kAudioComponentRegistrationsChangedNotification;
extern const CFStringRef kAudioComponentInstanceInvalidationNotification;
extern OSStatus
AudioUnitInitialize( AudioUnit inUnit);

extern OSStatus
AudioUnitUninitialize( AudioUnit inUnit);

extern OSStatus
AudioUnitGetPropertyInfo( AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         UInt32 * _Nullable outDataSize,
         Boolean * _Nullable outWritable);

extern OSStatus
AudioUnitGetProperty( AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         void * outData,
         UInt32 * ioDataSize);

extern OSStatus
AudioUnitSetProperty( AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         const void * _Nullable inData,
         UInt32 inDataSize);

extern OSStatus
AudioUnitAddPropertyListener( AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitPropertyListenerProc inProc,
         void * _Nullable inProcUserData);

extern OSStatus
AudioUnitRemovePropertyListenerWithUserData(
         AudioUnit inUnit,
         AudioUnitPropertyID inID,
         AudioUnitPropertyListenerProc inProc,
         void * _Nullable inProcUserData);

extern OSStatus
AudioUnitAddRenderNotify( AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData);

extern OSStatus
AudioUnitRemoveRenderNotify( AudioUnit inUnit,
         AURenderCallback inProc,
         void * _Nullable inProcUserData);

extern OSStatus
AudioUnitGetParameter( AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue * outValue);

extern OSStatus
AudioUnitSetParameter( AudioUnit inUnit,
         AudioUnitParameterID inID,
         AudioUnitScope inScope,
         AudioUnitElement inElement,
         AudioUnitParameterValue inValue,
         UInt32 inBufferOffsetInFrames);

extern OSStatus
AudioUnitScheduleParameters( AudioUnit inUnit,
         const AudioUnitParameterEvent * inParameterEvent,
         UInt32 inNumParamEvents);

extern OSStatus
AudioUnitRender( AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inOutputBusNumber,
         UInt32 inNumberFrames,
         AudioBufferList * ioData);

extern OSStatus
AudioUnitProcess ( AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inNumberFrames,
         AudioBufferList * ioData);

extern OSStatus
AudioUnitProcessMultiple( AudioUnit inUnit,
         AudioUnitRenderActionFlags * _Nullable ioActionFlags,
         const AudioTimeStamp * inTimeStamp,
         UInt32 inNumberFrames,
         UInt32 inNumberInputBufferLists,
         const AudioBufferList * _Nonnull * _Nonnull inInputBufferLists,
         UInt32 inNumberOutputBufferLists,
         AudioBufferList * _Nonnull * _Nonnull ioOutputBufferLists);
extern OSStatus
AudioUnitReset( AudioUnit inUnit,
         AudioUnitScope inScope,
         AudioUnitElement inElement);
extern OSStatus
AudioUnitExtensionSetComponentList(CFStringRef extensionIdentifier, _Nullable CFArrayRef audioComponentInfo);

extern _Nullable CFArrayRef
AudioUnitExtensionCopyComponentList(CFStringRef extensionIdentifier);

enum {
 kAudioUnitRange = 0x0000,
 kAudioUnitInitializeSelect = 0x0001,
 kAudioUnitUninitializeSelect = 0x0002,
 kAudioUnitGetPropertyInfoSelect = 0x0003,
 kAudioUnitGetPropertySelect = 0x0004,
 kAudioUnitSetPropertySelect = 0x0005,
 kAudioUnitAddPropertyListenerSelect = 0x000A,
 kAudioUnitRemovePropertyListenerSelect = 0x000B,
 kAudioUnitRemovePropertyListenerWithUserDataSelect = 0x0012,
 kAudioUnitAddRenderNotifySelect = 0x000F,
 kAudioUnitRemoveRenderNotifySelect = 0x0010,
 kAudioUnitGetParameterSelect = 0x0006,
 kAudioUnitSetParameterSelect = 0x0007,
 kAudioUnitScheduleParametersSelect = 0x0011,
 kAudioUnitRenderSelect = 0x000E,
 kAudioUnitResetSelect = 0x0009,
 kAudioUnitComplexRenderSelect = 0x0013,
 kAudioUnitProcessSelect = 0x0014,
 kAudioUnitProcessMultipleSelect = 0x0015
};





typedef OSStatus
(*AudioUnitInitializeProc) (void *self);

typedef OSStatus
(*AudioUnitUninitializeProc) (void *self);

typedef OSStatus
(*AudioUnitGetPropertyInfoProc) (void *self, AudioUnitPropertyID prop, AudioUnitScope scope,
         AudioUnitElement elem, UInt32 * _Nullable outDataSize, Boolean * _Nullable outWritable);

typedef OSStatus
(*AudioUnitGetPropertyProc) (void *self, AudioUnitPropertyID inID, AudioUnitScope inScope,
         AudioUnitElement inElement, void *outData, UInt32 *ioDataSize);

typedef OSStatus
(*AudioUnitSetPropertyProc) (void *self, AudioUnitPropertyID inID, AudioUnitScope inScope,
         AudioUnitElement inElement, const void *inData, UInt32 inDataSize);

typedef OSStatus
(*AudioUnitAddPropertyListenerProc) (void *self, AudioUnitPropertyID prop,
         AudioUnitPropertyListenerProc proc, void *userData);

typedef OSStatus
(*AudioUnitRemovePropertyListenerProc) (void *self, AudioUnitPropertyID prop,
         AudioUnitPropertyListenerProc proc);

typedef OSStatus
(*AudioUnitRemovePropertyListenerWithUserDataProc) (void *self, AudioUnitPropertyID prop,
         AudioUnitPropertyListenerProc proc, void * _Nullable userData);

typedef OSStatus
(*AudioUnitAddRenderNotifyProc) (void *self, AURenderCallback proc, void * _Nullable userData);

typedef OSStatus
(*AudioUnitRemoveRenderNotifyProc) (void *self, AURenderCallback proc, void * _Nullable userData);

typedef OSStatus
(*AudioUnitScheduleParametersProc) (void *self, const AudioUnitParameterEvent *events, UInt32 numEvents);

typedef OSStatus
(*AudioUnitResetProc) (void *self, AudioUnitScope inScope, AudioUnitElement inElement);

typedef OSStatus
(*AudioUnitComplexRenderProc) (void *self, AudioUnitRenderActionFlags * _Nullable ioActionFlags, const AudioTimeStamp *inTimeStamp,
         UInt32 inOutputBusNumber, UInt32 inNumberOfPackets, UInt32 *outNumberOfPackets,
         AudioStreamPacketDescription *outPacketDescriptions, AudioBufferList *ioData,
         void *outMetadata, UInt32 *outMetadataByteSize);

typedef OSStatus
(*AudioUnitProcessProc) (void *self, AudioUnitRenderActionFlags * _Nullable ioActionFlags, const AudioTimeStamp *inTimeStamp,
         UInt32 inNumberFrames, AudioBufferList *ioData);

typedef OSStatus
(*AudioUnitProcessMultipleProc) (void *self, AudioUnitRenderActionFlags * _Nullable ioActionFlags, const AudioTimeStamp *inTimeStamp,
         UInt32 inNumberFrames, UInt32 inNumberInputBufferLists, const AudioBufferList * _Nonnull * _Nonnull inInputBufferLists,
         UInt32 inNumberOutputBufferLists, AudioBufferList * _Nonnull * _Nonnull ioOutputBufferLists);
typedef OSStatus
(*AudioUnitGetParameterProc)( void * inComponentStorage,
        AudioUnitParameterID inID,
        AudioUnitScope inScope,
        AudioUnitElement inElement,
        AudioUnitParameterValue * outValue);
typedef OSStatus
(*AudioUnitSetParameterProc)( void * inComponentStorage,
        AudioUnitParameterID inID,
        AudioUnitScope inScope,
        AudioUnitElement inElement,
        AudioUnitParameterValue inValue,
        UInt32 inBufferOffsetInFrames);
typedef OSStatus
(*AudioUnitRenderProc)( void * inComponentStorage,
        AudioUnitRenderActionFlags * _Nullable ioActionFlags,
        const AudioTimeStamp * inTimeStamp,
        UInt32 inOutputBusNumber,
        UInt32 inNumberFrames,
        AudioBufferList * ioData);

enum {
 kAudioUnitErr_IllegalInstrument = -10873,
 kAudioUnitErr_InstrumentTypeNotFound = -10872,
} __attribute__((deprecated));

#ifdef __cplusplus
}
#endif

#endif

