#ifndef COREAUDIOTYPES_H
#define COREAUDIOTYPES_H
#include <CoreServices/MacTypes.h>

struct AudioStreamBasicDescription
{
	Float64 mSampleRate;
	UInt32 mFormatID;
	UInt32 mFormatFlags;
	UInt32 mBytesPerPacket;
	UInt32 mFramesPerPacket;
	UInt32 mBytesPerFrame;
	UInt32 mChannelsPerFrame;
	UInt32 mBitsPerChannel;
	UInt32 mReserved;
};

struct AudioClassDescription
{
	OSType mType;
	OSType mSubType;
	OSType mManufacturer;
};

struct AudioBuffer
{
	UInt32 mNumberChannels;
	UInt32 mDataByteSize;
	void* mData;
};

struct AudioBufferList
{
	UInt32 mNumberBuffers;
	AudioBuffer mBuffers[1];
};

struct AudioStreamPacketDescription
{
	SInt64 mStartOffset;
	UInt32 mVariableFramesInPacket; // 0 for constant streams
	UInt32 mDataByteSize;
};

struct SMPTETime
{
	SInt64 mSubframes, mSubframeDivisor;
	UInt32 mCounter, mType, mFlags;
	SInt16 mHours, mMinutes, mSeconds, mFrames;
};

struct AudioTimeStamp
{
	Float64 mSampleTime;
	UInt64 mHostTime;
	Float64 mRateScalar;
	UInt64 mWordClockTime;
	SMPTETime mSMPTETime;
	UInt32 mFlags, mReserved;
};

enum
{
	kAudioFormatLinearPCM               = 'lpcm',
	kAudioFormatAC3                     = 'ac-3',
	kAudioFormat60958AC3                = 'cac3',
	kAudioFormatAppleIMA4               = 'ima4',
	kAudioFormatMPEG4AAC                = 'aac ',
	kAudioFormatMPEG4CELP               = 'celp',
	kAudioFormatMPEG4HVXC               = 'hvxc',
	kAudioFormatMPEG4TwinVQ             = 'twvq',
	kAudioFormatMACE3                   = 'MAC3',
	kAudioFormatMACE6                   = 'MAC6',
	kAudioFormatULaw                    = 'ulaw',
	kAudioFormatALaw                    = 'alaw',
	kAudioFormatQDesign                 = 'QDMC',
	kAudioFormatQDesign2                = 'QDM2',
	kAudioFormatQUALCOMM                = 'Qclp',
	kAudioFormatMPEGLayer1              = '.mp1',
	kAudioFormatMPEGLayer2              = '.mp2',
	kAudioFormatMPEGLayer3              = '.mp3',
	kAudioFormatTimeCode                = 'time',
	kAudioFormatMIDIStream              = 'midi',
	kAudioFormatParameterValueStream    = 'apvs',
	kAudioFormatAppleLossless           = 'alac',
	kAudioFormatMPEG4AAC_HE             = 'aach',
	kAudioFormatMPEG4AAC_LD             = 'aacl',
	kAudioFormatMPEG4AAC_ELD            = 'aace',
	kAudioFormatMPEG4AAC_ELD_SBR        = 'aacf',
	kAudioFormatMPEG4AAC_ELD_V2         = 'aacg',    
	kAudioFormatMPEG4AAC_HE_V2          = 'aacp',
	kAudioFormatMPEG4AAC_Spatial        = 'aacs',
	kAudioFormatAMR                     = 'samr',
	kAudioFormatAudible                 = 'AUDB',
	kAudioFormatiLBC                    = 'ilbc',
	kAudioFormatDVIIntelIMA             = 0x6D730011,
	kAudioFormatMicrosoftGSM            = 0x6D730031,
	kAudioFormatAES3                    = 'aes3'
};

enum
{
	kAudioFormatFlagIsFloat                     = (1 << 0),     // 0x1
	kAudioFormatFlagIsBigEndian                 = (1 << 1),     // 0x2
	kAudioFormatFlagIsSignedInteger             = (1 << 2),     // 0x4
	kAudioFormatFlagIsPacked                    = (1 << 3),     // 0x8
	kAudioFormatFlagIsAlignedHigh               = (1 << 4),     // 0x10
	kAudioFormatFlagIsNonInterleaved            = (1 << 5),     // 0x20
	kAudioFormatFlagIsNonMixable                = (1 << 6),     // 0x40
	kAudioFormatFlagsAreAllClear                = (1 << 31),
	
	kLinearPCMFormatFlagIsFloat                 = kAudioFormatFlagIsFloat,
	kLinearPCMFormatFlagIsBigEndian             = kAudioFormatFlagIsBigEndian,
	kLinearPCMFormatFlagIsSignedInteger         = kAudioFormatFlagIsSignedInteger,
	kLinearPCMFormatFlagIsPacked                = kAudioFormatFlagIsPacked,
	kLinearPCMFormatFlagIsAlignedHigh           = kAudioFormatFlagIsAlignedHigh,
	kLinearPCMFormatFlagIsNonInterleaved        = kAudioFormatFlagIsNonInterleaved,
	kLinearPCMFormatFlagIsNonMixable            = kAudioFormatFlagIsNonMixable,
	kLinearPCMFormatFlagsSampleFractionShift    = 7,
	kLinearPCMFormatFlagsSampleFractionMask     = (0x3F << kLinearPCMFormatFlagsSampleFractionShift),
	kLinearPCMFormatFlagsAreAllClear            = kAudioFormatFlagsAreAllClear,
	
	kAppleLosslessFormatFlag_16BitSourceData    = 1,
	kAppleLosslessFormatFlag_20BitSourceData    = 2,
	kAppleLosslessFormatFlag_24BitSourceData    = 3,
	kAppleLosslessFormatFlag_32BitSourceData    = 4
};

enum // typedef UInt32 AudioConverterPropertyID
{
	kAudioConverterPropertyMinimumInputBufferSize       = 'mibs',
	kAudioConverterPropertyMinimumOutputBufferSize      = 'mobs',
	kAudioConverterPropertyMaximumInputBufferSize       = 'xibs',
	kAudioConverterPropertyMaximumInputPacketSize       = 'xips',
	kAudioConverterPropertyMaximumOutputPacketSize      = 'xops',
	kAudioConverterPropertyCalculateInputBufferSize     = 'cibs',
	kAudioConverterPropertyCalculateOutputBufferSize    = 'cobs',
	kAudioConverterPropertyInputCodecParameters         = 'icdp',
	kAudioConverterPropertyOutputCodecParameters        = 'ocdp',
	kAudioConverterSampleRateConverterAlgorithm         = 'srci',
	kAudioConverterSampleRateConverterComplexity        = 'srca',
	kAudioConverterSampleRateConverterQuality           = 'srcq',
	kAudioConverterSampleRateConverterInitialPhase      = 'srcp',
	kAudioConverterCodecQuality                         = 'cdqu',
	kAudioConverterPrimeMethod                          = 'prmm',
	kAudioConverterPrimeInfo                            = 'prim',
	kAudioConverterChannelMap                           = 'chmp',
	kAudioConverterDecompressionMagicCookie             = 'dmgc',
	kAudioConverterCompressionMagicCookie               = 'cmgc',
	kAudioConverterEncodeBitRate                        = 'brat',
	kAudioConverterEncodeAdjustableSampleRate           = 'ajsr',
	kAudioConverterInputChannelLayout                   = 'icl ',
	kAudioConverterOutputChannelLayout                  = 'ocl ',
	kAudioConverterApplicableEncodeBitRates             = 'aebr',
	kAudioConverterAvailableEncodeBitRates              = 'vebr',
	kAudioConverterApplicableEncodeSampleRates          = 'aesr',
	kAudioConverterAvailableEncodeSampleRates           = 'vesr',
	kAudioConverterAvailableEncodeChannelLayoutTags     = 'aecl',
	kAudioConverterCurrentOutputStreamDescription       = 'acod',
	kAudioConverterCurrentInputStreamDescription        = 'acid',
	kAudioConverterPropertySettings                     = 'acps',
	kAudioConverterPropertyBitDepthHint                 = 'acbd',
	kAudioConverterPropertyFormatList                   = 'flst',
	kAudioConverterPropertyDithering                    = 'dith',
	kAudioConverterPropertyDitherBitDepth               = 'dbit'
};

#endif

