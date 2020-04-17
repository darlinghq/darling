#ifndef AUDIOCONVERTER_H
#define AUDIOCONVERTER_H
#include <CoreServices/MacTypes.h>
#include <CoreAudio/CoreAudioTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueAudioConverter * AudioConverterRef;

typedef UInt32 AudioConverterPropertyID;
enum
{
    kAudioConverterPropertyMinimumInputBufferSize = 'mibs',
    kAudioConverterPropertyMinimumOutputBufferSize = 'mobs',
    kAudioConverterPropertyMaximumInputBufferSize = 'xibs',
    kAudioConverterPropertyMaximumInputPacketSize = 'xips',
    kAudioConverterPropertyMaximumOutputPacketSize = 'xops',
    kAudioConverterPropertyCalculateInputBufferSize = 'cibs',
    kAudioConverterPropertyCalculateOutputBufferSize = 'cobs',
    kAudioConverterPropertyInputCodecParameters = 'icdp',
    kAudioConverterPropertyOutputCodecParameters = 'ocdp',
    kAudioConverterSampleRateConverterAlgorithm = 'srci',
    kAudioConverterSampleRateConverterComplexity = 'srca',
    kAudioConverterSampleRateConverterQuality = 'srcq',
    kAudioConverterSampleRateConverterInitialPhase = 'srcp',
    kAudioConverterCodecQuality = 'cdqu',
    kAudioConverterPrimeMethod = 'prmm',
    kAudioConverterPrimeInfo = 'prim',
    kAudioConverterChannelMap = 'chmp',
    kAudioConverterDecompressionMagicCookie = 'dmgc',
    kAudioConverterCompressionMagicCookie = 'cmgc',
    kAudioConverterEncodeBitRate = 'brat',
    kAudioConverterEncodeAdjustableSampleRate = 'ajsr',
    kAudioConverterInputChannelLayout = 'icl ',
    kAudioConverterOutputChannelLayout = 'ocl ',
    kAudioConverterApplicableEncodeBitRates = 'aebr',
    kAudioConverterAvailableEncodeBitRates = 'vebr',
    kAudioConverterApplicableEncodeSampleRates = 'aesr',
    kAudioConverterAvailableEncodeSampleRates = 'vesr',
    kAudioConverterAvailableEncodeChannelLayoutTags = 'aecl',
    kAudioConverterCurrentOutputStreamDescription = 'acod',
    kAudioConverterCurrentInputStreamDescription = 'acid',
    kAudioConverterPropertySettings = 'acps',
    kAudioConverterPropertyBitDepthHint = 'acbd',
    kAudioConverterPropertyFormatList = 'flst'
};
enum
{
 kAudioConverterPropertyDithering = 'dith',
 kAudioConverterPropertyDitherBitDepth = 'dbit'
};
enum
{
 kDitherAlgorithm_TPDF = 1,
 kDitherAlgorithm_NoiseShaping = 2
};
enum
{
    kAudioConverterQuality_Max = 0x7F,
    kAudioConverterQuality_High = 0x60,
    kAudioConverterQuality_Medium = 0x40,
    kAudioConverterQuality_Low = 0x20,
    kAudioConverterQuality_Min = 0
};
enum
{
    kAudioConverterSampleRateConverterComplexity_Linear = 'line',
    kAudioConverterSampleRateConverterComplexity_Normal = 'norm',
    kAudioConverterSampleRateConverterComplexity_Mastering = 'bats',
    kAudioConverterSampleRateConverterComplexity_MinimumPhase = 'minp'
};
enum
{
    kConverterPrimeMethod_Pre = 0,
    kConverterPrimeMethod_Normal = 1,
    kConverterPrimeMethod_None = 2
};
struct AudioConverterPrimeInfo {
    UInt32 leadingFrames;
    UInt32 trailingFrames;
};
typedef struct AudioConverterPrimeInfo AudioConverterPrimeInfo;





enum
{
    kAudioConverterErr_FormatNotSupported = 'fmt?',
    kAudioConverterErr_OperationNotSupported = 0x6F703F3F,
    kAudioConverterErr_PropertyNotSupported = 'prop',
    kAudioConverterErr_InvalidInputSize = 'insz',
    kAudioConverterErr_InvalidOutputSize = 'otsz',

    kAudioConverterErr_UnspecifiedError = 'what',
    kAudioConverterErr_BadPropertySizeError = '!siz',
    kAudioConverterErr_RequiresPacketDescriptionsError = '!pkd',
    kAudioConverterErr_InputSampleRateOutOfRange = '!isr',
    kAudioConverterErr_OutputSampleRateOutOfRange = '!osr'
};
extern OSStatus
AudioConverterNew( const AudioStreamBasicDescription * inSourceFormat,
                        const AudioStreamBasicDescription * inDestinationFormat,
                        AudioConverterRef _Nullable * _Nonnull outAudioConverter);
extern OSStatus
AudioConverterNewSpecific( const AudioStreamBasicDescription * inSourceFormat,
                            const AudioStreamBasicDescription * inDestinationFormat,
                            UInt32 inNumberClassDescriptions,
                            const AudioClassDescription * inClassDescriptions,
                            AudioConverterRef _Nullable * _Nonnull outAudioConverter);
extern OSStatus
AudioConverterDispose( AudioConverterRef inAudioConverter);
extern OSStatus
AudioConverterReset( AudioConverterRef inAudioConverter);
extern OSStatus
AudioConverterGetPropertyInfo( AudioConverterRef inAudioConverter,
                                AudioConverterPropertyID inPropertyID,
                                UInt32 * _Nullable outSize,
                                Boolean * _Nullable outWritable);
extern OSStatus
AudioConverterGetProperty( AudioConverterRef inAudioConverter,
                            AudioConverterPropertyID inPropertyID,
                            UInt32 * ioPropertyDataSize,
                            void * outPropertyData);
extern OSStatus
AudioConverterSetProperty( AudioConverterRef inAudioConverter,
                            AudioConverterPropertyID inPropertyID,
                            UInt32 inPropertyDataSize,
                            const void * inPropertyData);
typedef OSStatus
(*AudioConverterInputDataProc)( AudioConverterRef inAudioConverter,
                                UInt32 * ioDataSize,
                                void * _Nonnull * _Nonnull outData,
                                void * _Nullable inUserData);
extern OSStatus
AudioConverterFillBuffer( AudioConverterRef inAudioConverter,
                            AudioConverterInputDataProc inInputDataProc,
                            void * _Nullable inInputDataProcUserData,
                            UInt32 * ioOutputDataSize,
                            void * outOutputData);
extern OSStatus
AudioConverterConvertBuffer( AudioConverterRef inAudioConverter,
                                UInt32 inInputDataSize,
                                const void * inInputData,
                                UInt32 * ioOutputDataSize,
                                void * outOutputData);
typedef OSStatus
(*AudioConverterComplexInputDataProc)( AudioConverterRef inAudioConverter,
                                        UInt32 * ioNumberDataPackets,
                                        AudioBufferList * ioData,
                                        AudioStreamPacketDescription * _Nullable * _Nullable outDataPacketDescription,
                                        void * _Nullable inUserData);
extern OSStatus
AudioConverterFillComplexBuffer( AudioConverterRef inAudioConverter,
                                    AudioConverterComplexInputDataProc inInputDataProc,
                                    void * _Nullable inInputDataProcUserData,
                                    UInt32 * ioOutputDataPacketSize,
                                    AudioBufferList * outOutputData,
                                    AudioStreamPacketDescription * _Nullable outPacketDescription);
extern OSStatus
AudioConverterConvertComplexBuffer( AudioConverterRef inAudioConverter,
                                    UInt32 inNumberPCMFrames,
                                    const AudioBufferList * inInputData,
                                    AudioBufferList * outOutputData);


#ifdef __cplusplus
}
#endif


#endif

