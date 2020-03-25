#ifndef COREAUDIOTYPES_H
#define COREAUDIOTYPES_H
#include <MacTypes.h>

#define COREAUDIOTYPES_VERSION 1052

typedef Float32 AudioSampleType;
typedef Float32 AudioUnitSampleType;

typedef struct 
{
	Float64 mSampleRate;
	UInt32 mFormatID;
	UInt32 mFormatFlags;
	UInt32 mBytesPerPacket; // 0 for VBR
	UInt32 mFramesPerPacket;
	UInt32 mBytesPerFrame;
	UInt32 mChannelsPerFrame;
	UInt32 mBitsPerChannel;
	UInt32 mReserved;
} AudioStreamBasicDescription;

typedef struct
{
	OSType mType;
	OSType mSubType;
	OSType mManufacturer;
} AudioClassDescription;

typedef struct
{
	UInt32 mNumberChannels;
	UInt32 mDataByteSize;
	void* mData;
} AudioBuffer;

typedef struct
{
	UInt32 mNumberBuffers;
	AudioBuffer mBuffers[1];
} AudioBufferList;

typedef struct
{
   Float64 mMinimum;
   Float64 mMaximum;
} AudioValueRange;

typedef struct
{
	SInt64 mStartOffset;
	UInt32 mVariableFramesInPacket; // 0 for constant streams
	UInt32 mDataByteSize;
} AudioStreamPacketDescription;

typedef struct
{
	SInt64 mSubframes, mSubframeDivisor;
	UInt32 mCounter, mType, mFlags;
	SInt16 mHours, mMinutes, mSeconds, mFrames;
} SMPTETime;

typedef struct
{
	Float64 mSampleTime;
	UInt64 mHostTime;
	Float64 mRateScalar;
	UInt64 mWordClockTime;
	SMPTETime mSMPTETime;
	UInt32 mFlags, mReserved;
} AudioTimeStamp;

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
#if defined(__BIG_ENDIAN__)
   kAudioFormatFlagsNativeEndian = kAudioFormatFlagIsBigEndian,
#else
   kAudioFormatFlagsNativeEndian = 0,
#endif
   kAudioFormatFlagsCanonical = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,
   kAudioFormatFlagsNativeFloatPacked = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,
	
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

enum {
   kAudioChannelLayoutTag_UseChannelDescriptions  = (0<<16) | 0,
   kAudioChannelLayoutTag_UseChannelBitmap        = (1<<16) | 0,
   
   kAudioChannelLayoutTag_Mono                    = (100<<16) | 1,
   kAudioChannelLayoutTag_Stereo                  = (101<<16) | 2,
   kAudioChannelLayoutTag_StereoHeadphones        = (102<<16) | 2,
   kAudioChannelLayoutTag_MatrixStereo            = (103<<16) | 2,
   kAudioChannelLayoutTag_MidSide                 = (104<<16) | 2,
   kAudioChannelLayoutTag_XY                      = (105<<16) | 2,
   kAudioChannelLayoutTag_Binaural                = (106<<16) | 2,
   kAudioChannelLayoutTag_Ambisonic_B_Format      = (107<<16) | 4,
   kAudioChannelLayoutTag_Quadraphonic            = (108<<16) | 4,
   kAudioChannelLayoutTag_Pentagonal              = (109<<16) | 5,
   kAudioChannelLayoutTag_Hexagonal               = (110<<16) | 6,
   kAudioChannelLayoutTag_Octagonal               = (111<<16) | 8,
   kAudioChannelLayoutTag_Cube                    = (112<<16) | 8,
   
   
   kAudioChannelLayoutTag_MPEG_1_0                = kAudioChannelLayoutTag_Mono,
   kAudioChannelLayoutTag_MPEG_2_0                = kAudioChannelLayoutTag_Stereo,
   kAudioChannelLayoutTag_MPEG_3_0_A              = (113<<16) | 3,
   kAudioChannelLayoutTag_MPEG_3_0_B              = (114<<16) | 3,
   kAudioChannelLayoutTag_MPEG_4_0_A              = (115<<16) | 4,
   kAudioChannelLayoutTag_MPEG_4_0_B              = (116<<16) | 4,
   kAudioChannelLayoutTag_MPEG_5_0_A              = (117<<16) | 5,
   kAudioChannelLayoutTag_MPEG_5_0_B              = (118<<16) | 5,
   kAudioChannelLayoutTag_MPEG_5_0_C              = (119<<16) | 5,
   kAudioChannelLayoutTag_MPEG_5_0_D              = (120<<16) | 5,
   kAudioChannelLayoutTag_MPEG_5_1_A              = (121<<16) | 6,
   kAudioChannelLayoutTag_MPEG_5_1_B              = (122<<16) | 6,
   kAudioChannelLayoutTag_MPEG_5_1_C              = (123<<16) | 6,
   kAudioChannelLayoutTag_MPEG_5_1_D              = (124<<16) | 6,
   kAudioChannelLayoutTag_MPEG_6_1_A              = (125<<16) | 7,
   kAudioChannelLayoutTag_MPEG_7_1_A              = (126<<16) | 8,
   kAudioChannelLayoutTag_MPEG_7_1_B              = (127<<16) | 8,
   kAudioChannelLayoutTag_MPEG_7_1_C              = (128<<16) | 8,
   kAudioChannelLayoutTag_Emagic_Default_7_1      = (129<<16) | 8,
   kAudioChannelLayoutTag_SMPTE_DTV               = (130<<16) | 8,
   
   
   kAudioChannelLayoutTag_ITU_1_0                 = kAudioChannelLayoutTag_Mono,
   kAudioChannelLayoutTag_ITU_2_0                 = kAudioChannelLayoutTag_Stereo,
   kAudioChannelLayoutTag_ITU_2_1                 = (131<<16) | 3,
   kAudioChannelLayoutTag_ITU_2_2                 = (132<<16) | 4,
   kAudioChannelLayoutTag_ITU_3_0                 =
   kAudioChannelLayoutTag_MPEG_3_0_A,
   kAudioChannelLayoutTag_ITU_3_1                 =
   kAudioChannelLayoutTag_MPEG_4_0_A,
   kAudioChannelLayoutTag_ITU_3_2                 =
   kAudioChannelLayoutTag_MPEG_5_0_A,
   kAudioChannelLayoutTag_ITU_3_2_1               =
   kAudioChannelLayoutTag_MPEG_5_1_A,
   kAudioChannelLayoutTag_ITU_3_4_1               =
   kAudioChannelLayoutTag_MPEG_7_1_C,
   
   
   kAudioChannelLayoutTag_DVD_0                   = kAudioChannelLayoutTag_Mono,
   kAudioChannelLayoutTag_DVD_1                   = kAudioChannelLayoutTag_Stereo,
   kAudioChannelLayoutTag_DVD_2                   = kAudioChannelLayoutTag_ITU_2_1,
   kAudioChannelLayoutTag_DVD_3                   = kAudioChannelLayoutTag_ITU_2_2,
   kAudioChannelLayoutTag_DVD_4                   = (133<<16) | 3,
   kAudioChannelLayoutTag_DVD_5                   = (134<<16) | 4,
   kAudioChannelLayoutTag_DVD_6                   = (135<<16) | 5,
   kAudioChannelLayoutTag_DVD_7                   =
   kAudioChannelLayoutTag_MPEG_3_0_A,
   kAudioChannelLayoutTag_DVD_8                   =
   kAudioChannelLayoutTag_MPEG_4_0_A,
   kAudioChannelLayoutTag_DVD_9                   =
   kAudioChannelLayoutTag_MPEG_5_0_A,
   kAudioChannelLayoutTag_DVD_10                  = (136<<16) | 4,
   kAudioChannelLayoutTag_DVD_11                  = (137<<16) | 5,
   kAudioChannelLayoutTag_DVD_12                  =
   kAudioChannelLayoutTag_MPEG_5_1_A,
   kAudioChannelLayoutTag_DVD_13                  = kAudioChannelLayoutTag_DVD_8,
   kAudioChannelLayoutTag_DVD_14                  = kAudioChannelLayoutTag_DVD_9,
   kAudioChannelLayoutTag_DVD_15                  = kAudioChannelLayoutTag_DVD_10,
   kAudioChannelLayoutTag_DVD_16                  = kAudioChannelLayoutTag_DVD_11,
   kAudioChannelLayoutTag_DVD_17                  = kAudioChannelLayoutTag_DVD_12,
   kAudioChannelLayoutTag_DVD_18                  = (138<<16) | 5,
   kAudioChannelLayoutTag_DVD_19                  =
   kAudioChannelLayoutTag_MPEG_5_0_B,
   kAudioChannelLayoutTag_DVD_20                  =
   kAudioChannelLayoutTag_MPEG_5_1_B,
   
   
   kAudioChannelLayoutTag_AudioUnit_4             =
   kAudioChannelLayoutTag_Quadraphonic,
   kAudioChannelLayoutTag_AudioUnit_5             =
   kAudioChannelLayoutTag_Pentagonal,
   kAudioChannelLayoutTag_AudioUnit_6             =
   kAudioChannelLayoutTag_Hexagonal,
   kAudioChannelLayoutTag_AudioUnit_8             =
   kAudioChannelLayoutTag_Octagonal,
   
   kAudioChannelLayoutTag_AudioUnit_5_0           =
   kAudioChannelLayoutTag_MPEG_5_0_B,
   kAudioChannelLayoutTag_AudioUnit_6_0           = (139<<16) | 6,
   kAudioChannelLayoutTag_AudioUnit_7_0           = (140<<16) | 7,
   kAudioChannelLayoutTag_AudioUnit_7_0_Front     = (148<<16) | 7,
   kAudioChannelLayoutTag_AudioUnit_5_1           =
   kAudioChannelLayoutTag_MPEG_5_1_A,
   kAudioChannelLayoutTag_AudioUnit_6_1           =
   kAudioChannelLayoutTag_MPEG_6_1_A,
   kAudioChannelLayoutTag_AudioUnit_7_1           =
   kAudioChannelLayoutTag_MPEG_7_1_C,
   kAudioChannelLayoutTag_AudioUnit_7_1_Front     =
   kAudioChannelLayoutTag_MPEG_7_1_A,
   
   kAudioChannelLayoutTag_AAC_3_0                 =
   kAudioChannelLayoutTag_MPEG_3_0_B,
   kAudioChannelLayoutTag_AAC_Quadraphonic        =
   kAudioChannelLayoutTag_Quadraphonic,
   kAudioChannelLayoutTag_AAC_4_0                 =
   kAudioChannelLayoutTag_MPEG_4_0_B,
   kAudioChannelLayoutTag_AAC_5_0                 =
   kAudioChannelLayoutTag_MPEG_5_0_D,
   kAudioChannelLayoutTag_AAC_5_1                 =
   kAudioChannelLayoutTag_MPEG_5_1_D,
   kAudioChannelLayoutTag_AAC_6_0                 = (141<<16) | 6,
   kAudioChannelLayoutTag_AAC_6_1                 = (142<<16) | 7,
   kAudioChannelLayoutTag_AAC_7_0                 = (143<<16) | 7,
   kAudioChannelLayoutTag_AAC_7_1                 =
   kAudioChannelLayoutTag_MPEG_7_1_B,
   kAudioChannelLayoutTag_AAC_Octagonal           = (144<<16) | 8,
   
   kAudioChannelLayoutTag_TMH_10_2_std            = (145<<16) | 16,
   kAudioChannelLayoutTag_TMH_10_2_full           = (146<<16) | 21,
   
   kAudioChannelLayoutTag_AC3_1_0_1               = (149<<16) | 2,
   kAudioChannelLayoutTag_AC3_3_0                 = (150<<16) | 3,
   kAudioChannelLayoutTag_AC3_3_1                 = (151<<16) | 4,
   kAudioChannelLayoutTag_AC3_3_0_1               = (152<<16) | 4,
   kAudioChannelLayoutTag_AC3_2_1_1               = (153<<16) | 4,
   kAudioChannelLayoutTag_AC3_3_1_1               = (154<<16) | 5,
   
   kAudioChannelLayoutTag_EAC_6_0_A               = (155<<16) | 6,
   kAudioChannelLayoutTag_EAC_7_0_A               = (156<<16) | 7,
   kAudioChannelLayoutTag_EAC3_6_1_A              = (157<<16) | 7,
   kAudioChannelLayoutTag_EAC3_6_1_B              = (158<<16) | 7,
   kAudioChannelLayoutTag_EAC3_6_1_C              = (159<<16) | 7,
   kAudioChannelLayoutTag_EAC3_7_1_A              = (160<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_B              = (161<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_C              = (162<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_D              = (163<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_E              = (164<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_F              = (165<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_G              = (166<<16) | 8,
   kAudioChannelLayoutTag_EAC3_7_1_H              = (167<<16) | 8,
   
   kAudioChannelLayoutTag_DTS_3_1                 = (168<<16) | 4,
   kAudioChannelLayoutTag_DTS_4_1                 = (169<<16) | 5,
   kAudioChannelLayoutTag_DTS_6_0_A               = (170<<16) | 6,
   kAudioChannelLayoutTag_DTS_6_0_B               = (171<<16) | 6,
   kAudioChannelLayoutTag_DTS_6_0_C               = (172<<16) | 6,
   kAudioChannelLayoutTag_DTS_6_1_A               = (173<<16) | 7,
   kAudioChannelLayoutTag_DTS_6_1_B               = (174<<16) | 7,
   kAudioChannelLayoutTag_DTS_6_1_C               = (175<<16) | 7,
   kAudioChannelLayoutTag_DTS_6_1_D               = (182<<16) | 7,
   kAudioChannelLayoutTag_DTS_7_0                 = (176<<16) | 7,
   kAudioChannelLayoutTag_DTS_7_1                 = (177<<16) | 8,
   kAudioChannelLayoutTag_DTS_8_0_A               = (178<<16) | 8,
   kAudioChannelLayoutTag_DTS_8_0_B               = (179<<16) | 8,
   kAudioChannelLayoutTag_DTS_8_1_A               = (180<<16) | 9,
   kAudioChannelLayoutTag_DTS_8_1_B               = (181<<16) | 9,
   
   kAudioChannelLayoutTag_DiscreteInOrder         = (147<<16) | 0,
   kAudioChannelLayoutTag_Unknown                 = 0xFFFF0000
};

enum {
	kAudioConverterErr_PropertyNotSupported = 'prop',
	kAudioConverterErr_BadPropertySizeError = '!siz',
	kAudioConverterErr_FormatNotSupported = 'fmt?',
	kAudioConverterErr_OperationNotSupported = 0x6F703F3F,
	kAudioConverterErr_UnspecifiedError = 'what',
};

typedef UInt32 AudioChannelLabel;
typedef UInt32 AudioChannelLayoutTag;

struct AudioChannelDescription
{
	AudioChannelLabel mChannelLabel;
	UInt32 mChannelFlags;
	Float32 mCoordinates[3];
};

struct AudioChannelLayout
{
	AudioChannelLayoutTag mChannelLayoutTag;
	UInt32 mChannelBitmap;
	UInt32 mNumberChannelDescriptions;
	AudioChannelDescription mChannelDescriptions[1];
};

struct AudioValueTranslation
{
   void* mInputData;
   UInt32 mInputDataSize;
   void* mOutputData;
   UInt32 mOutputDataSize;
};

enum
{
    kAudioChannelLabel_Unknown                  = 0xFFFFFFFF,
    kAudioChannelLabel_Unused                   = 0,
    kAudioChannelLabel_UseCoordinates           = 100,

    kAudioChannelLabel_Left                     = 1,
    kAudioChannelLabel_Right                    = 2,
    kAudioChannelLabel_Center                   = 3,
    kAudioChannelLabel_LFEScreen                = 4,
    kAudioChannelLabel_LeftSurround             = 5,
    kAudioChannelLabel_RightSurround            = 6,
    kAudioChannelLabel_LeftCenter               = 7,
    kAudioChannelLabel_RightCenter              = 8,
    kAudioChannelLabel_CenterSurround           = 9, 
    kAudioChannelLabel_LeftSurroundDirect       = 10,
    kAudioChannelLabel_RightSurroundDirect      = 11,
    kAudioChannelLabel_TopCenterSurround        = 12,
    kAudioChannelLabel_VerticalHeightLeft       = 13,
    kAudioChannelLabel_VerticalHeightCenter     = 14,
    kAudioChannelLabel_VerticalHeightRight      = 15,

    kAudioChannelLabel_TopBackLeft              = 16,
    kAudioChannelLabel_TopBackCenter            = 17,
    kAudioChannelLabel_TopBackRight             = 18,

    kAudioChannelLabel_RearSurroundLeft         = 33,
    kAudioChannelLabel_RearSurroundRight        = 34,
    kAudioChannelLabel_LeftWide                 = 35,
    kAudioChannelLabel_RightWide                = 36,
    kAudioChannelLabel_LFE2                     = 37,
    kAudioChannelLabel_LeftTotal                = 38,
    kAudioChannelLabel_RightTotal               = 39,
    kAudioChannelLabel_HearingImpaired          = 40,
    kAudioChannelLabel_Narration                = 41,
    kAudioChannelLabel_Mono                     = 42,
    kAudioChannelLabel_DialogCentricMix         = 43,

    kAudioChannelLabel_CenterSurroundDirect     = 44,
    
    kAudioChannelLabel_Haptic                   = 45,


    kAudioChannelLabel_Ambisonic_W              = 200,
    kAudioChannelLabel_Ambisonic_X              = 201,
    kAudioChannelLabel_Ambisonic_Y              = 202,
    kAudioChannelLabel_Ambisonic_Z              = 203,

    kAudioChannelLabel_MS_Mid                   = 204,
    kAudioChannelLabel_MS_Side                  = 205,

    kAudioChannelLabel_XY_X                     = 206,
    kAudioChannelLabel_XY_Y                     = 207,

    kAudioChannelLabel_HeadphonesLeft           = 301,
    kAudioChannelLabel_HeadphonesRight          = 302,
    kAudioChannelLabel_ClickTrack               = 304,
    kAudioChannelLabel_ForeignLanguage          = 305,

    kAudioChannelLabel_Discrete                 = 400,

    kAudioChannelLabel_Discrete_0               = (1U<<16) | 0,
    kAudioChannelLabel_Discrete_1               = (1U<<16) | 1,
    kAudioChannelLabel_Discrete_2               = (1U<<16) | 2,
    kAudioChannelLabel_Discrete_3               = (1U<<16) | 3,
    kAudioChannelLabel_Discrete_4               = (1U<<16) | 4,
    kAudioChannelLabel_Discrete_5               = (1U<<16) | 5,
    kAudioChannelLabel_Discrete_6               = (1U<<16) | 6,
    kAudioChannelLabel_Discrete_7               = (1U<<16) | 7,
    kAudioChannelLabel_Discrete_8               = (1U<<16) | 8,
    kAudioChannelLabel_Discrete_9               = (1U<<16) | 9,
    kAudioChannelLabel_Discrete_10              = (1U<<16) | 10,
    kAudioChannelLabel_Discrete_11              = (1U<<16) | 11,
    kAudioChannelLabel_Discrete_12              = (1U<<16) | 12,
    kAudioChannelLabel_Discrete_13              = (1U<<16) | 13,
    kAudioChannelLabel_Discrete_14              = (1U<<16) | 14,
    kAudioChannelLabel_Discrete_15              = (1U<<16) | 15,
    kAudioChannelLabel_Discrete_65535           = (1U<<16) | 65535
};

#endif

