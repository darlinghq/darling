#ifndef COREAUDIOTYPES_H
#define COREAUDIOTYPES_H
#include <MacTypes.h>

#define COREAUDIOTYPES_VERSION 1052

enum
{
    kAudio_UnimplementedError = -4,
    kAudio_FileNotFoundError = -43,
    kAudio_FilePermissionError = -54,
    kAudio_TooManyFilesOpenError = -42,
    kAudio_BadFilePathError = '!pth',
    kAudio_ParamError = -50,
    kAudio_MemFullError = -108
};
struct AudioValueRange
{
    Float64 mMinimum;
    Float64 mMaximum;
};
typedef struct AudioValueRange AudioValueRange;
struct AudioValueTranslation
{
    void* _Nonnull mInputData;
    UInt32 mInputDataSize;
    void* _Nonnull mOutputData;
    UInt32 mOutputDataSize;
};
typedef struct AudioValueTranslation AudioValueTranslation;
struct AudioBuffer
{
    UInt32 mNumberChannels;
    UInt32 mDataByteSize;
    void* _Nullable mData;
};
typedef struct AudioBuffer AudioBuffer;
struct AudioBufferList
{
    UInt32 mNumberBuffers;
    AudioBuffer mBuffers[1];
};
typedef struct AudioBufferList AudioBufferList;
 typedef Float32 AudioSampleType;
 typedef Float32 AudioUnitSampleType;
typedef UInt32 AudioFormatID;





typedef UInt32 AudioFormatFlags;
struct AudioStreamBasicDescription
{
    Float64 mSampleRate;
    AudioFormatID mFormatID;
    AudioFormatFlags mFormatFlags;
    UInt32 mBytesPerPacket;
    UInt32 mFramesPerPacket;
    UInt32 mBytesPerFrame;
    UInt32 mChannelsPerFrame;
    UInt32 mBitsPerChannel;
    UInt32 mReserved;
};
typedef struct AudioStreamBasicDescription AudioStreamBasicDescription;







static const Float64 kAudioStreamAnyRate = 0.0;
enum
{
    kAudioFormatLinearPCM = 'lpcm',
    kAudioFormatAC3 = 'ac-3',
    kAudioFormat60958AC3 = 'cac3',
    kAudioFormatAppleIMA4 = 'ima4',
    kAudioFormatMPEG4AAC = 'aac ',
    kAudioFormatMPEG4CELP = 'celp',
    kAudioFormatMPEG4HVXC = 'hvxc',
    kAudioFormatMPEG4TwinVQ = 'twvq',
    kAudioFormatMACE3 = 'MAC3',
    kAudioFormatMACE6 = 'MAC6',
    kAudioFormatULaw = 'ulaw',
    kAudioFormatALaw = 'alaw',
    kAudioFormatQDesign = 'QDMC',
    kAudioFormatQDesign2 = 'QDM2',
    kAudioFormatQUALCOMM = 'Qclp',
    kAudioFormatMPEGLayer1 = '.mp1',
    kAudioFormatMPEGLayer2 = '.mp2',
    kAudioFormatMPEGLayer3 = '.mp3',
    kAudioFormatTimeCode = 'time',
    kAudioFormatMIDIStream = 'midi',
    kAudioFormatParameterValueStream = 'apvs',
    kAudioFormatAppleLossless = 'alac',
    kAudioFormatMPEG4AAC_HE = 'aach',
    kAudioFormatMPEG4AAC_LD = 'aacl',
    kAudioFormatMPEG4AAC_ELD = 'aace',
    kAudioFormatMPEG4AAC_ELD_SBR = 'aacf',
    kAudioFormatMPEG4AAC_ELD_V2 = 'aacg',
    kAudioFormatMPEG4AAC_HE_V2 = 'aacp',
    kAudioFormatMPEG4AAC_Spatial = 'aacs',
    kAudioFormatAMR = 'samr',
    kAudioFormatAMR_WB = 'sawb',
    kAudioFormatAudible = 'AUDB',
    kAudioFormatiLBC = 'ilbc',
    kAudioFormatDVIIntelIMA = 0x6D730011,
    kAudioFormatMicrosoftGSM = 0x6D730031,
    kAudioFormatAES3 = 'aes3',
    kAudioFormatEnhancedAC3 = 'ec-3',
    kAudioFormatFLAC = 'flac',
    kAudioFormatOpus = 'opus'
};
enum
{
    kAudioFormatFlagIsFloat = (1U << 0),
    kAudioFormatFlagIsBigEndian = (1U << 1),
    kAudioFormatFlagIsSignedInteger = (1U << 2),
    kAudioFormatFlagIsPacked = (1U << 3),
    kAudioFormatFlagIsAlignedHigh = (1U << 4),
    kAudioFormatFlagIsNonInterleaved = (1U << 5),
    kAudioFormatFlagIsNonMixable = (1U << 6),
    kAudioFormatFlagsAreAllClear = 0x80000000,

    kLinearPCMFormatFlagIsFloat = kAudioFormatFlagIsFloat,
    kLinearPCMFormatFlagIsBigEndian = kAudioFormatFlagIsBigEndian,
    kLinearPCMFormatFlagIsSignedInteger = kAudioFormatFlagIsSignedInteger,
    kLinearPCMFormatFlagIsPacked = kAudioFormatFlagIsPacked,
    kLinearPCMFormatFlagIsAlignedHigh = kAudioFormatFlagIsAlignedHigh,
    kLinearPCMFormatFlagIsNonInterleaved = kAudioFormatFlagIsNonInterleaved,
    kLinearPCMFormatFlagIsNonMixable = kAudioFormatFlagIsNonMixable,
    kLinearPCMFormatFlagsSampleFractionShift = 7,
    kLinearPCMFormatFlagsSampleFractionMask = (0x3F << kLinearPCMFormatFlagsSampleFractionShift),
    kLinearPCMFormatFlagsAreAllClear = kAudioFormatFlagsAreAllClear,

    kAppleLosslessFormatFlag_16BitSourceData = 1,
    kAppleLosslessFormatFlag_20BitSourceData = 2,
    kAppleLosslessFormatFlag_24BitSourceData = 3,
    kAppleLosslessFormatFlag_32BitSourceData = 4
};
enum
{



    kAudioFormatFlagsNativeEndian = 0,


    kAudioFormatFlagsCanonical  = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,
    kAudioFormatFlagsAudioUnitCanonical  = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved,




    kAudioFormatFlagsNativeFloatPacked = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked
};
struct AudioStreamPacketDescription
{
    SInt64 mStartOffset;
    UInt32 mVariableFramesInPacket;
    UInt32 mDataByteSize;
};
typedef struct AudioStreamPacketDescription AudioStreamPacketDescription;
typedef UInt32 SMPTETimeType; enum
{
    kSMPTETimeType24 = 0,
    kSMPTETimeType25 = 1,
    kSMPTETimeType30Drop = 2,
    kSMPTETimeType30 = 3,
    kSMPTETimeType2997 = 4,
    kSMPTETimeType2997Drop = 5,
    kSMPTETimeType60 = 6,
    kSMPTETimeType5994 = 7,
    kSMPTETimeType60Drop = 8,
    kSMPTETimeType5994Drop = 9,
    kSMPTETimeType50 = 10,
    kSMPTETimeType2398 = 11
};
typedef UInt32 SMPTETimeFlags; enum
{
    kSMPTETimeUnknown = 0,
    kSMPTETimeValid = (1U << 0),
    kSMPTETimeRunning = (1U << 1)
};
struct SMPTETime
{
    SInt16 mSubframes;
    SInt16 mSubframeDivisor;
    UInt32 mCounter;
    SMPTETimeType mType;
    SMPTETimeFlags mFlags;
    SInt16 mHours;
    SInt16 mMinutes;
    SInt16 mSeconds;
    SInt16 mFrames;
};
typedef struct SMPTETime SMPTETime;
typedef UInt32 AudioTimeStampFlags; enum
{
    kAudioTimeStampNothingValid = 0,
    kAudioTimeStampSampleTimeValid = (1U << 0),
    kAudioTimeStampHostTimeValid = (1U << 1),
    kAudioTimeStampRateScalarValid = (1U << 2),
    kAudioTimeStampWordClockTimeValid = (1U << 3),
    kAudioTimeStampSMPTETimeValid = (1U << 4),
    kAudioTimeStampSampleHostTimeValid = (kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid)
};
struct AudioTimeStamp
{
    Float64 mSampleTime;
    UInt64 mHostTime;
    Float64 mRateScalar;
    UInt64 mWordClockTime;
    SMPTETime mSMPTETime;
    AudioTimeStampFlags mFlags;
    UInt32 mReserved;
};
typedef struct AudioTimeStamp AudioTimeStamp;
struct AudioClassDescription {
    OSType mType;
    OSType mSubType;
    OSType mManufacturer;
};
typedef struct AudioClassDescription AudioClassDescription;
typedef UInt32 AudioChannelLabel;





typedef UInt32 AudioChannelLayoutTag;
enum
{
    kAudioChannelLabel_Unknown = 0xFFFFFFFF,
    kAudioChannelLabel_Unused = 0,
    kAudioChannelLabel_UseCoordinates = 100,

    kAudioChannelLabel_Left = 1,
    kAudioChannelLabel_Right = 2,
    kAudioChannelLabel_Center = 3,
    kAudioChannelLabel_LFEScreen = 4,
    kAudioChannelLabel_LeftSurround = 5,
    kAudioChannelLabel_RightSurround = 6,
    kAudioChannelLabel_LeftCenter = 7,
    kAudioChannelLabel_RightCenter = 8,
    kAudioChannelLabel_CenterSurround = 9,
    kAudioChannelLabel_LeftSurroundDirect = 10,
    kAudioChannelLabel_RightSurroundDirect = 11,
    kAudioChannelLabel_TopCenterSurround = 12,
    kAudioChannelLabel_VerticalHeightLeft = 13,
    kAudioChannelLabel_VerticalHeightCenter = 14,
    kAudioChannelLabel_VerticalHeightRight = 15,

    kAudioChannelLabel_TopBackLeft = 16,
    kAudioChannelLabel_TopBackCenter = 17,
    kAudioChannelLabel_TopBackRight = 18,

    kAudioChannelLabel_RearSurroundLeft = 33,
    kAudioChannelLabel_RearSurroundRight = 34,
    kAudioChannelLabel_LeftWide = 35,
    kAudioChannelLabel_RightWide = 36,
    kAudioChannelLabel_LFE2 = 37,
    kAudioChannelLabel_LeftTotal = 38,
    kAudioChannelLabel_RightTotal = 39,
    kAudioChannelLabel_HearingImpaired = 40,
    kAudioChannelLabel_Narration = 41,
    kAudioChannelLabel_Mono = 42,
    kAudioChannelLabel_DialogCentricMix = 43,

    kAudioChannelLabel_CenterSurroundDirect = 44,

    kAudioChannelLabel_Haptic = 45,


    kAudioChannelLabel_Ambisonic_W = 200,
    kAudioChannelLabel_Ambisonic_X = 201,
    kAudioChannelLabel_Ambisonic_Y = 202,
    kAudioChannelLabel_Ambisonic_Z = 203,


    kAudioChannelLabel_MS_Mid = 204,
    kAudioChannelLabel_MS_Side = 205,


    kAudioChannelLabel_XY_X = 206,
    kAudioChannelLabel_XY_Y = 207,


    kAudioChannelLabel_HeadphonesLeft = 301,
    kAudioChannelLabel_HeadphonesRight = 302,
    kAudioChannelLabel_ClickTrack = 304,
    kAudioChannelLabel_ForeignLanguage = 305,


    kAudioChannelLabel_Discrete = 400,


    kAudioChannelLabel_Discrete_0 = (1U<<16) | 0,
    kAudioChannelLabel_Discrete_1 = (1U<<16) | 1,
    kAudioChannelLabel_Discrete_2 = (1U<<16) | 2,
    kAudioChannelLabel_Discrete_3 = (1U<<16) | 3,
    kAudioChannelLabel_Discrete_4 = (1U<<16) | 4,
    kAudioChannelLabel_Discrete_5 = (1U<<16) | 5,
    kAudioChannelLabel_Discrete_6 = (1U<<16) | 6,
    kAudioChannelLabel_Discrete_7 = (1U<<16) | 7,
    kAudioChannelLabel_Discrete_8 = (1U<<16) | 8,
    kAudioChannelLabel_Discrete_9 = (1U<<16) | 9,
    kAudioChannelLabel_Discrete_10 = (1U<<16) | 10,
    kAudioChannelLabel_Discrete_11 = (1U<<16) | 11,
    kAudioChannelLabel_Discrete_12 = (1U<<16) | 12,
    kAudioChannelLabel_Discrete_13 = (1U<<16) | 13,
    kAudioChannelLabel_Discrete_14 = (1U<<16) | 14,
    kAudioChannelLabel_Discrete_15 = (1U<<16) | 15,
    kAudioChannelLabel_Discrete_65535 = (1U<<16) | 65535,


    kAudioChannelLabel_HOA_ACN = 500,


    kAudioChannelLabel_HOA_ACN_0 = (2U << 16) | 0,
    kAudioChannelLabel_HOA_ACN_1 = (2U << 16) | 1,
    kAudioChannelLabel_HOA_ACN_2 = (2U << 16) | 2,
    kAudioChannelLabel_HOA_ACN_3 = (2U << 16) | 3,
    kAudioChannelLabel_HOA_ACN_4 = (2U << 16) | 4,
    kAudioChannelLabel_HOA_ACN_5 = (2U << 16) | 5,
    kAudioChannelLabel_HOA_ACN_6 = (2U << 16) | 6,
    kAudioChannelLabel_HOA_ACN_7 = (2U << 16) | 7,
    kAudioChannelLabel_HOA_ACN_8 = (2U << 16) | 8,
    kAudioChannelLabel_HOA_ACN_9 = (2U << 16) | 9,
    kAudioChannelLabel_HOA_ACN_10 = (2U << 16) | 10,
    kAudioChannelLabel_HOA_ACN_11 = (2U << 16) | 11,
    kAudioChannelLabel_HOA_ACN_12 = (2U << 16) | 12,
    kAudioChannelLabel_HOA_ACN_13 = (2U << 16) | 13,
    kAudioChannelLabel_HOA_ACN_14 = (2U << 16) | 14,
    kAudioChannelLabel_HOA_ACN_15 = (2U << 16) | 15,
    kAudioChannelLabel_HOA_ACN_65024 = (2U << 16) | 65024
};






typedef UInt32 AudioChannelBitmap; enum
{
    kAudioChannelBit_Left = (1U<<0),
    kAudioChannelBit_Right = (1U<<1),
    kAudioChannelBit_Center = (1U<<2),
    kAudioChannelBit_LFEScreen = (1U<<3),
    kAudioChannelBit_LeftSurround = (1U<<4),
    kAudioChannelBit_RightSurround = (1U<<5),
    kAudioChannelBit_LeftCenter = (1U<<6),
    kAudioChannelBit_RightCenter = (1U<<7),
    kAudioChannelBit_CenterSurround = (1U<<8),
    kAudioChannelBit_LeftSurroundDirect = (1U<<9),
    kAudioChannelBit_RightSurroundDirect = (1U<<10),
    kAudioChannelBit_TopCenterSurround = (1U<<11),
    kAudioChannelBit_VerticalHeightLeft = (1U<<12),
    kAudioChannelBit_VerticalHeightCenter = (1U<<13),
    kAudioChannelBit_VerticalHeightRight = (1U<<14),
    kAudioChannelBit_TopBackLeft = (1U<<15),
    kAudioChannelBit_TopBackCenter = (1U<<16),
    kAudioChannelBit_TopBackRight = (1U<<17)
};
typedef UInt32 AudioChannelFlags; enum
{
    kAudioChannelFlags_AllOff = 0,
    kAudioChannelFlags_RectangularCoordinates = (1U<<0),
    kAudioChannelFlags_SphericalCoordinates = (1U<<1),
    kAudioChannelFlags_Meters = (1U<<2)
};
typedef UInt32 AudioChannelCoordinateIndex; enum
{
    kAudioChannelCoordinates_LeftRight = 0,
    kAudioChannelCoordinates_BackFront = 1,
    kAudioChannelCoordinates_DownUp = 2,
    kAudioChannelCoordinates_Azimuth = 0,
    kAudioChannelCoordinates_Elevation = 1,
    kAudioChannelCoordinates_Distance = 2
};






enum
{
    kAudioChannelLayoutTag_UseChannelDescriptions = (0U<<16) | 0,
    kAudioChannelLayoutTag_UseChannelBitmap = (1U<<16) | 0,

    kAudioChannelLayoutTag_Mono = (100U<<16) | 1,
    kAudioChannelLayoutTag_Stereo = (101U<<16) | 2,
    kAudioChannelLayoutTag_StereoHeadphones = (102U<<16) | 2,
    kAudioChannelLayoutTag_MatrixStereo = (103U<<16) | 2,
    kAudioChannelLayoutTag_MidSide = (104U<<16) | 2,
    kAudioChannelLayoutTag_XY = (105U<<16) | 2,
    kAudioChannelLayoutTag_Binaural = (106U<<16) | 2,
    kAudioChannelLayoutTag_Ambisonic_B_Format = (107U<<16) | 4,

    kAudioChannelLayoutTag_Quadraphonic = (108U<<16) | 4,
    kAudioChannelLayoutTag_Pentagonal = (109U<<16) | 5,
    kAudioChannelLayoutTag_Hexagonal = (110U<<16) | 6,
    kAudioChannelLayoutTag_Octagonal = (111U<<16) | 8,
    kAudioChannelLayoutTag_Cube = (112U<<16) | 8,



    kAudioChannelLayoutTag_MPEG_1_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_MPEG_2_0 = kAudioChannelLayoutTag_Stereo,
    kAudioChannelLayoutTag_MPEG_3_0_A = (113U<<16) | 3,
    kAudioChannelLayoutTag_MPEG_3_0_B = (114U<<16) | 3,
    kAudioChannelLayoutTag_MPEG_4_0_A = (115U<<16) | 4,
    kAudioChannelLayoutTag_MPEG_4_0_B = (116U<<16) | 4,
    kAudioChannelLayoutTag_MPEG_5_0_A = (117U<<16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_B = (118U<<16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_C = (119U<<16) | 5,
    kAudioChannelLayoutTag_MPEG_5_0_D = (120U<<16) | 5,
    kAudioChannelLayoutTag_MPEG_5_1_A = (121U<<16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_B = (122U<<16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_C = (123U<<16) | 6,
    kAudioChannelLayoutTag_MPEG_5_1_D = (124U<<16) | 6,
    kAudioChannelLayoutTag_MPEG_6_1_A = (125U<<16) | 7,
    kAudioChannelLayoutTag_MPEG_7_1_A = (126U<<16) | 8,
    kAudioChannelLayoutTag_MPEG_7_1_B = (127U<<16) | 8,
    kAudioChannelLayoutTag_MPEG_7_1_C = (128U<<16) | 8,
    kAudioChannelLayoutTag_Emagic_Default_7_1 = (129U<<16) | 8,
    kAudioChannelLayoutTag_SMPTE_DTV = (130U<<16) | 8,



    kAudioChannelLayoutTag_ITU_1_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_ITU_2_0 = kAudioChannelLayoutTag_Stereo,

    kAudioChannelLayoutTag_ITU_2_1 = (131U<<16) | 3,
    kAudioChannelLayoutTag_ITU_2_2 = (132U<<16) | 4,
    kAudioChannelLayoutTag_ITU_3_0 = kAudioChannelLayoutTag_MPEG_3_0_A,
    kAudioChannelLayoutTag_ITU_3_1 = kAudioChannelLayoutTag_MPEG_4_0_A,

    kAudioChannelLayoutTag_ITU_3_2 = kAudioChannelLayoutTag_MPEG_5_0_A,
    kAudioChannelLayoutTag_ITU_3_2_1 = kAudioChannelLayoutTag_MPEG_5_1_A,
    kAudioChannelLayoutTag_ITU_3_4_1 = kAudioChannelLayoutTag_MPEG_7_1_C,


    kAudioChannelLayoutTag_DVD_0 = kAudioChannelLayoutTag_Mono,
    kAudioChannelLayoutTag_DVD_1 = kAudioChannelLayoutTag_Stereo,
    kAudioChannelLayoutTag_DVD_2 = kAudioChannelLayoutTag_ITU_2_1,
    kAudioChannelLayoutTag_DVD_3 = kAudioChannelLayoutTag_ITU_2_2,
    kAudioChannelLayoutTag_DVD_4 = (133U<<16) | 3,
    kAudioChannelLayoutTag_DVD_5 = (134U<<16) | 4,
    kAudioChannelLayoutTag_DVD_6 = (135U<<16) | 5,
    kAudioChannelLayoutTag_DVD_7 = kAudioChannelLayoutTag_MPEG_3_0_A,
    kAudioChannelLayoutTag_DVD_8 = kAudioChannelLayoutTag_MPEG_4_0_A,
    kAudioChannelLayoutTag_DVD_9 = kAudioChannelLayoutTag_MPEG_5_0_A,
    kAudioChannelLayoutTag_DVD_10 = (136U<<16) | 4,
    kAudioChannelLayoutTag_DVD_11 = (137U<<16) | 5,
    kAudioChannelLayoutTag_DVD_12 = kAudioChannelLayoutTag_MPEG_5_1_A,

    kAudioChannelLayoutTag_DVD_13 = kAudioChannelLayoutTag_DVD_8,
    kAudioChannelLayoutTag_DVD_14 = kAudioChannelLayoutTag_DVD_9,
    kAudioChannelLayoutTag_DVD_15 = kAudioChannelLayoutTag_DVD_10,
    kAudioChannelLayoutTag_DVD_16 = kAudioChannelLayoutTag_DVD_11,
    kAudioChannelLayoutTag_DVD_17 = kAudioChannelLayoutTag_DVD_12,
    kAudioChannelLayoutTag_DVD_18 = (138U<<16) | 5,
    kAudioChannelLayoutTag_DVD_19 = kAudioChannelLayoutTag_MPEG_5_0_B,
    kAudioChannelLayoutTag_DVD_20 = kAudioChannelLayoutTag_MPEG_5_1_B,



    kAudioChannelLayoutTag_AudioUnit_4 = kAudioChannelLayoutTag_Quadraphonic,
    kAudioChannelLayoutTag_AudioUnit_5 = kAudioChannelLayoutTag_Pentagonal,
    kAudioChannelLayoutTag_AudioUnit_6 = kAudioChannelLayoutTag_Hexagonal,
    kAudioChannelLayoutTag_AudioUnit_8 = kAudioChannelLayoutTag_Octagonal,

    kAudioChannelLayoutTag_AudioUnit_5_0 = kAudioChannelLayoutTag_MPEG_5_0_B,
    kAudioChannelLayoutTag_AudioUnit_6_0 = (139U<<16) | 6,
    kAudioChannelLayoutTag_AudioUnit_7_0 = (140U<<16) | 7,
    kAudioChannelLayoutTag_AudioUnit_7_0_Front = (148U<<16) | 7,
    kAudioChannelLayoutTag_AudioUnit_5_1 = kAudioChannelLayoutTag_MPEG_5_1_A,
    kAudioChannelLayoutTag_AudioUnit_6_1 = kAudioChannelLayoutTag_MPEG_6_1_A,
    kAudioChannelLayoutTag_AudioUnit_7_1 = kAudioChannelLayoutTag_MPEG_7_1_C,
    kAudioChannelLayoutTag_AudioUnit_7_1_Front = kAudioChannelLayoutTag_MPEG_7_1_A,

    kAudioChannelLayoutTag_AAC_3_0 = kAudioChannelLayoutTag_MPEG_3_0_B,
    kAudioChannelLayoutTag_AAC_Quadraphonic = kAudioChannelLayoutTag_Quadraphonic,
    kAudioChannelLayoutTag_AAC_4_0 = kAudioChannelLayoutTag_MPEG_4_0_B,
    kAudioChannelLayoutTag_AAC_5_0 = kAudioChannelLayoutTag_MPEG_5_0_D,
    kAudioChannelLayoutTag_AAC_5_1 = kAudioChannelLayoutTag_MPEG_5_1_D,
    kAudioChannelLayoutTag_AAC_6_0 = (141U<<16) | 6,
    kAudioChannelLayoutTag_AAC_6_1 = (142U<<16) | 7,
    kAudioChannelLayoutTag_AAC_7_0 = (143U<<16) | 7,
    kAudioChannelLayoutTag_AAC_7_1 = kAudioChannelLayoutTag_MPEG_7_1_B,
    kAudioChannelLayoutTag_AAC_7_1_B = (183U<<16) | 8,
    kAudioChannelLayoutTag_AAC_7_1_C = (184U<<16) | 8,
    kAudioChannelLayoutTag_AAC_Octagonal = (144U<<16) | 8,

    kAudioChannelLayoutTag_TMH_10_2_std = (145U<<16) | 16,
    kAudioChannelLayoutTag_TMH_10_2_full = (146U<<16) | 21,

    kAudioChannelLayoutTag_AC3_1_0_1 = (149U<<16) | 2,
    kAudioChannelLayoutTag_AC3_3_0 = (150U<<16) | 3,
    kAudioChannelLayoutTag_AC3_3_1 = (151U<<16) | 4,
    kAudioChannelLayoutTag_AC3_3_0_1 = (152U<<16) | 4,
    kAudioChannelLayoutTag_AC3_2_1_1 = (153U<<16) | 4,
    kAudioChannelLayoutTag_AC3_3_1_1 = (154U<<16) | 5,

    kAudioChannelLayoutTag_EAC_6_0_A = (155U<<16) | 6,
    kAudioChannelLayoutTag_EAC_7_0_A = (156U<<16) | 7,

    kAudioChannelLayoutTag_EAC3_6_1_A = (157U<<16) | 7,
    kAudioChannelLayoutTag_EAC3_6_1_B = (158U<<16) | 7,
    kAudioChannelLayoutTag_EAC3_6_1_C = (159U<<16) | 7,
    kAudioChannelLayoutTag_EAC3_7_1_A = (160U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_B = (161U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_C = (162U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_D = (163U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_E = (164U<<16) | 8,

    kAudioChannelLayoutTag_EAC3_7_1_F = (165U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_G = (166U<<16) | 8,
    kAudioChannelLayoutTag_EAC3_7_1_H = (167U<<16) | 8,

    kAudioChannelLayoutTag_DTS_3_1 = (168U<<16) | 4,
    kAudioChannelLayoutTag_DTS_4_1 = (169U<<16) | 5,
    kAudioChannelLayoutTag_DTS_6_0_A = (170U<<16) | 6,
    kAudioChannelLayoutTag_DTS_6_0_B = (171U<<16) | 6,
    kAudioChannelLayoutTag_DTS_6_0_C = (172U<<16) | 6,
    kAudioChannelLayoutTag_DTS_6_1_A = (173U<<16) | 7,
    kAudioChannelLayoutTag_DTS_6_1_B = (174U<<16) | 7,
    kAudioChannelLayoutTag_DTS_6_1_C = (175U<<16) | 7,
    kAudioChannelLayoutTag_DTS_7_0 = (176U<<16) | 7,
    kAudioChannelLayoutTag_DTS_7_1 = (177U<<16) | 8,
    kAudioChannelLayoutTag_DTS_8_0_A = (178U<<16) | 8,
    kAudioChannelLayoutTag_DTS_8_0_B = (179U<<16) | 8,
    kAudioChannelLayoutTag_DTS_8_1_A = (180U<<16) | 9,
    kAudioChannelLayoutTag_DTS_8_1_B = (181U<<16) | 9,
    kAudioChannelLayoutTag_DTS_6_1_D = (182U<<16) | 7,

    kAudioChannelLayoutTag_HOA_ACN_SN3D = (190U<<16) | 0,

    kAudioChannelLayoutTag_HOA_ACN_N3D = (191U<<16) | 0,


    kAudioChannelLayoutTag_DiscreteInOrder = (147U<<16) | 0,
    kAudioChannelLayoutTag_Unknown = 0xFFFF0000
};
struct AudioChannelDescription
{
    AudioChannelLabel mChannelLabel;
    AudioChannelFlags mChannelFlags;
    Float32 mCoordinates[3];
};
typedef struct AudioChannelDescription AudioChannelDescription;
struct AudioChannelLayout
{
    AudioChannelLayoutTag mChannelLayoutTag;
    AudioChannelBitmap mChannelBitmap;
    UInt32 mNumberChannelDescriptions;
    AudioChannelDescription mChannelDescriptions[1];
};
typedef struct AudioChannelLayout AudioChannelLayout;
    static __inline__ __attribute__((always_inline)) UInt32 AudioChannelLayoutTag_GetNumberOfChannels(AudioChannelLayoutTag inLayoutTag) { return (UInt32)(inLayoutTag & 0x0000FFFF); }
typedef long MPEG4ObjectID; enum
{
    kMPEG4Object_AAC_Main = 1,
    kMPEG4Object_AAC_LC = 2,
    kMPEG4Object_AAC_SSR = 3,
    kMPEG4Object_AAC_LTP = 4,
    kMPEG4Object_AAC_SBR = 5,
    kMPEG4Object_AAC_Scalable = 6,
    kMPEG4Object_TwinVQ = 7,
    kMPEG4Object_CELP = 8,
    kMPEG4Object_HVXC = 9
};

#endif

