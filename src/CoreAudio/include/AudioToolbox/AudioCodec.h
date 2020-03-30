#ifndef _AUDIO_CODEC_H
#define _AUDIO_CODEC_H

#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioComponent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef AudioComponentInstance AudioCodec;
typedef UInt32 AudioCodecPropertyID;
struct AudioCodecMagicCookieInfo
{
 UInt32 mMagicCookieSize;
 const void* _Nullable mMagicCookie;
};
typedef struct AudioCodecMagicCookieInfo AudioCodecMagicCookieInfo;
enum
{
 kAudioDecoderComponentType = 'adec',
 kAudioEncoderComponentType = 'aenc',
 kAudioUnityCodecComponentType = 'acdc'
};
enum
{
 kAudioCodecPropertySupportedInputFormats = 'ifm#',
 kAudioCodecPropertySupportedOutputFormats = 'ofm#',
 kAudioCodecPropertyAvailableInputSampleRates = 'aisr',
 kAudioCodecPropertyAvailableOutputSampleRates = 'aosr',
 kAudioCodecPropertyAvailableBitRateRange = 'abrt',
 kAudioCodecPropertyMinimumNumberInputPackets = 'mnip',
 kAudioCodecPropertyMinimumNumberOutputPackets = 'mnop',
 kAudioCodecPropertyAvailableNumberChannels = 'cmnc',
 kAudioCodecPropertyDoesSampleRateConversion = 'lmrc',
 kAudioCodecPropertyAvailableInputChannelLayoutTags = 'aicl',
 kAudioCodecPropertyAvailableOutputChannelLayoutTags = 'aocl',
 kAudioCodecPropertyInputFormatsForOutputFormat = 'if4o',
 kAudioCodecPropertyOutputFormatsForInputFormat = 'of4i',
 kAudioCodecPropertyFormatInfo = 'acfi'
};
enum
{
 kAudioCodecPropertyInputBufferSize = 'tbuf',
 kAudioCodecPropertyPacketFrameSize = 'pakf',
 kAudioCodecPropertyHasVariablePacketByteSizes = 'vpk?',
 kAudioCodecPropertyMaximumPacketByteSize = 'pakb',
    kAudioCodecPropertyPacketSizeLimitForVBR = 'pakl',
 kAudioCodecPropertyCurrentInputFormat = 'ifmt',
 kAudioCodecPropertyCurrentOutputFormat = 'ofmt',
 kAudioCodecPropertyMagicCookie = 'kuki',
 kAudioCodecPropertyUsedInputBufferSize = 'ubuf',
 kAudioCodecPropertyIsInitialized = 'init',
 kAudioCodecPropertyCurrentTargetBitRate = 'brat',
   kAudioCodecPropertyCurrentInputSampleRate = 'cisr',
   kAudioCodecPropertyCurrentOutputSampleRate = 'cosr',
 kAudioCodecPropertyQualitySetting = 'srcq',
 kAudioCodecPropertyApplicableBitRateRange = 'brta',
    kAudioCodecPropertyRecommendedBitRateRange = 'brtr',
 kAudioCodecPropertyApplicableInputSampleRates = 'isra',
 kAudioCodecPropertyApplicableOutputSampleRates = 'osra',
 kAudioCodecPropertyPaddedZeros = 'pad0',
 kAudioCodecPropertyPrimeMethod = 'prmm',
 kAudioCodecPropertyPrimeInfo = 'prim',
 kAudioCodecPropertyCurrentInputChannelLayout = 'icl ',
 kAudioCodecPropertyCurrentOutputChannelLayout = 'ocl ',
 kAudioCodecPropertySettings = 'acs ',
 kAudioCodecPropertyFormatList = 'acfl',
 kAudioCodecPropertyBitRateControlMode = 'acbf',
 kAudioCodecPropertySoundQualityForVBR = 'vbrq',
 kAudioCodecPropertyDelayMode = 'dmod',
    kAudioCodecPropertyAdjustLocalQuality = '^qal',
    kAudioCodecPropertyProgramTargetLevel = 'pptl',
    kAudioCodecPropertyDynamicRangeControlMode = 'mdrc',
    kAudioCodecPropertyProgramTargetLevelConstant = 'ptlc',
};
enum
{
 kAudioCodecQuality_Max = 0x7F,
 kAudioCodecQuality_High = 0x60,
 kAudioCodecQuality_Medium = 0x40,
 kAudioCodecQuality_Low = 0x20,
 kAudioCodecQuality_Min = 0
};
enum
{
 kAudioCodecPrimeMethod_Pre = 0,
 kAudioCodecPrimeMethod_Normal = 1,
 kAudioCodecPrimeMethod_None = 2
};
enum
{
 kAudioCodecBitRateControlMode_Constant = 0,
 kAudioCodecBitRateControlMode_LongTermAverage = 1,
 kAudioCodecBitRateControlMode_VariableConstrained = 2,
 kAudioCodecBitRateControlMode_Variable = 3,
};
enum
{
    kAudioCodecDelayMode_Compatibility = 0,
    kAudioCodecDelayMode_Minimum = 1,
    kAudioCodecDelayMode_Optimal = 2
};
enum
{
 kProgramTargetLevel_None = 0,
 kProgramTargetLevel_Minus31dB = 1,
 kProgramTargetLevel_Minus23dB = 2,
 kProgramTargetLevel_Minus20dB = 3
};
enum
{
 kDynamicRangeControlMode_None = 0,
 kDynamicRangeControlMode_Light = 1,
 kDynamicRangeControlMode_Heavy = 2
};
typedef struct AudioCodecPrimeInfo
{
 UInt32 leadingFrames;
 UInt32 trailingFrames;
} AudioCodecPrimeInfo;
typedef UInt32 AudioSettingsFlags; enum {
 kAudioSettingsFlags_ExpertParameter = (1L << 0),
 kAudioSettingsFlags_InvisibleParameter = (1L << 1),
 kAudioSettingsFlags_MetaParameter = (1L << 2),
 kAudioSettingsFlags_UserInterfaceParameter = (1L << 3)
};
enum
{
 kAudioCodecProduceOutputPacketFailure = 1,
 kAudioCodecProduceOutputPacketSuccess = 2,
 kAudioCodecProduceOutputPacketSuccessHasMore = 3,
 kAudioCodecProduceOutputPacketNeedsMoreInputData = 4,
 kAudioCodecProduceOutputPacketAtEOF = 5
};
enum
{
 kAudioCodecGetPropertyInfoSelect = 0x0001,
 kAudioCodecGetPropertySelect = 0x0002,
 kAudioCodecSetPropertySelect = 0x0003,
 kAudioCodecInitializeSelect = 0x0004,
 kAudioCodecUninitializeSelect = 0x0005,
 kAudioCodecAppendInputDataSelect = 0x0006,
 kAudioCodecProduceOutputDataSelect = 0x0007,
 kAudioCodecResetSelect = 0x0008,
 kAudioCodecAppendInputBufferListSelect = 0x0009,
 kAudioCodecProduceOutputBufferListSelect = 0x000A
};
enum
{
 kAudioCodecNoError = 0,
 kAudioCodecUnspecifiedError = 'what',
 kAudioCodecUnknownPropertyError = 'who?',
 kAudioCodecBadPropertySizeError = '!siz',
 kAudioCodecIllegalOperationError = 'nope',
 kAudioCodecUnsupportedFormatError = '!dat',
 kAudioCodecStateError = '!stt',
 kAudioCodecNotEnoughBufferSpaceError = '!buf'
};
extern OSStatus
AudioCodecGetPropertyInfo( AudioCodec inCodec,
       AudioCodecPropertyID inPropertyID,
       UInt32* _Nullable outSize,
       Boolean* _Nullable outWritable);
extern OSStatus
AudioCodecGetProperty( AudioCodec inCodec,
      AudioCodecPropertyID inPropertyID,
      UInt32* ioPropertyDataSize,
      void* outPropertyData);
extern OSStatus
AudioCodecSetProperty( AudioCodec inCodec,
      AudioCodecPropertyID inPropertyID,
      UInt32 inPropertyDataSize,
      const void* inPropertyData);
extern OSStatus
AudioCodecInitialize( AudioCodec inCodec,
      const AudioStreamBasicDescription* _Nullable inInputFormat,
      const AudioStreamBasicDescription* _Nullable inOutputFormat,
      const void* _Nullable inMagicCookie,
      UInt32 inMagicCookieByteSize);
extern OSStatus
AudioCodecUninitialize(AudioCodec inCodec);
extern OSStatus
AudioCodecAppendInputData( AudioCodec inCodec,
       const void* inInputData,
       UInt32* ioInputDataByteSize,
       UInt32* ioNumberPackets,
       const AudioStreamPacketDescription* _Nullable inPacketDescription);
extern OSStatus
AudioCodecProduceOutputPackets( AudioCodec inCodec,
        void* outOutputData,
        UInt32* ioOutputDataByteSize,
        UInt32* ioNumberPackets,
        AudioStreamPacketDescription* _Nullable outPacketDescription,
        UInt32* outStatus);

extern OSStatus
AudioCodecAppendInputBufferList( AudioCodec inCodec,
         const AudioBufferList * inBufferList,
         UInt32* ioNumberPackets,
         const AudioStreamPacketDescription* _Nullable inPacketDescription,
         UInt32* outBytesConsumed);

extern OSStatus
AudioCodecProduceOutputBufferList( AudioCodec inCodec,
         AudioBufferList * ioBufferList,
         UInt32* ioNumberPackets,
         AudioStreamPacketDescription* _Nullable outPacketDescription,
         UInt32* outStatus);
extern OSStatus
AudioCodecReset(AudioCodec inCodec);


typedef OSStatus
(*AudioCodecGetPropertyInfoProc)(void *self, AudioCodecPropertyID inPropertyID, UInt32 * _Nullable outSize, Boolean * _Nullable outWritable);

typedef OSStatus
(*AudioCodecGetPropertyProc)(void *self, AudioCodecPropertyID inPropertyID, UInt32 *ioPropertyDataSize,
        void *outPropertyData);

typedef OSStatus
(*AudioCodecSetPropertyProc)(void *self, AudioCodecPropertyID inPropertyID, UInt32 inPropertyDataSize,
        const void *inPropertyData);

typedef OSStatus
(*AudioCodecInitializeProc)(void *self, const AudioStreamBasicDescription * _Nullable inInputFormat,
        const AudioStreamBasicDescription * _Nullable inOutputFormat, const void * _Nullable inMagicCookie,
        UInt32 inMagicCookieByteSize);

typedef OSStatus
(*AudioCodecUninitializeProc)(void *self);

typedef OSStatus
(*AudioCodecAppendInputDataProc)(void *self, const void *inInputData, UInt32 *ioInputDataByteSize, UInt32 *ioNumberPackets,
        const AudioStreamPacketDescription * _Nullable inPacketDescription);

typedef OSStatus
(*AudioCodecProduceOutputPacketsProc)(void *self, void *outOutputData, UInt32 *ioOutputDataByteSize, UInt32 *ioNumberPackets,
        AudioStreamPacketDescription * _Nullable outPacketDescription, UInt32 *outStatus);

typedef OSStatus
(*AudioCodecResetProc)(void *self);

typedef OSStatus
(*AudioCodecAppendInputBufferListProc)(void *self, const AudioBufferList *ioBufferList, UInt32 *inNumberPackets,
        const AudioStreamPacketDescription * _Nullable inPacketDescription, UInt32 *outBytesConsumed);

typedef OSStatus
(*AudioCodecProduceOutputBufferListProc)(void *self, AudioBufferList *ioBufferList, UInt32 *ioNumberPackets,
        AudioStreamPacketDescription *_Nullable outPacketDescription, UInt32 *outStatus);
enum
{
 kAudioCodecPropertyMinimumDelayMode = 'mdel'
};
enum
{
 kAudioCodecPropertyNameCFString = 'lnam',
 kAudioCodecPropertyManufacturerCFString = 'lmak',
 kAudioCodecPropertyFormatCFString = 'lfor'
};
enum
{
 kAudioCodecPropertyRequiresPacketDescription = 'pakd',
 kAudioCodecPropertyAvailableBitRates = 'brt#',
 kAudioCodecExtendFrequencies = 'acef',
 kAudioCodecUseRecommendedSampleRate = 'ursr',
 kAudioCodecOutputPrecedence = 'oppr',
 kAudioCodecBitRateFormat = kAudioCodecPropertyBitRateControlMode,
 kAudioCodecDoesSampleRateConversion = kAudioCodecPropertyDoesSampleRateConversion,
 kAudioCodecInputFormatsForOutputFormat = kAudioCodecPropertyInputFormatsForOutputFormat,
 kAudioCodecOutputFormatsForInputFormat = kAudioCodecPropertyOutputFormatsForInputFormat,
 kAudioCodecPropertyInputChannelLayout = kAudioCodecPropertyCurrentInputChannelLayout,
 kAudioCodecPropertyOutputChannelLayout = kAudioCodecPropertyCurrentOutputChannelLayout,
 kAudioCodecPropertyAvailableInputChannelLayouts = kAudioCodecPropertyAvailableInputChannelLayoutTags,
 kAudioCodecPropertyAvailableOutputChannelLayouts = kAudioCodecPropertyAvailableOutputChannelLayoutTags,
 kAudioCodecPropertyZeroFramesPadded = kAudioCodecPropertyPaddedZeros
};
enum
{
 kAudioCodecBitRateFormat_CBR = kAudioCodecBitRateControlMode_Constant,
 kAudioCodecBitRateFormat_ABR = kAudioCodecBitRateControlMode_LongTermAverage,
 kAudioCodecBitRateFormat_VBR = kAudioCodecBitRateControlMode_VariableConstrained
};
enum
{
 kAudioCodecOutputPrecedenceNone = 0,
 kAudioCodecOutputPrecedenceBitRate = 1,
 kAudioCodecOutputPrecedenceSampleRate = 2
};
typedef struct AudioCodecMagicCookieInfo MagicCookieInfo __attribute__((deprecated));
enum
{
 kHintBasic = 0,
 kHintAdvanced = 1,
 kHintHidden = 2
};

#ifdef __cplusplus
}
#endif

#endif
