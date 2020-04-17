#ifndef AUDIOFILE_H
#define AUDIOFILE_H

#include <CoreServices/MacTypes.h>
#include <CoreFoundation/CFURL.h>
#include <CoreAudio/AudioHardware.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef UInt32 AudioFileTypeID;
enum {
        kAudioFileAIFFType = 'AIFF',
        kAudioFileAIFCType = 'AIFC',
        kAudioFileWAVEType = 'WAVE',
        kAudioFileRF64Type = 'RF64',
        kAudioFileSoundDesigner2Type = 'Sd2f',
        kAudioFileNextType = 'NeXT',
        kAudioFileMP3Type = 'MPG3',
        kAudioFileMP2Type = 'MPG2',
        kAudioFileMP1Type = 'MPG1',
  kAudioFileAC3Type = 'ac-3',
        kAudioFileAAC_ADTSType = 'adts',
        kAudioFileMPEG4Type = 'mp4f',
        kAudioFileM4AType = 'm4af',
        kAudioFileM4BType = 'm4bf',
  kAudioFileCAFType = 'caff',
  kAudioFile3GPType = '3gpp',
  kAudioFile3GP2Type = '3gp2',
  kAudioFileAMRType = 'amrf',
  kAudioFileFLACType = 'flac'
};
enum {
        kAudioFileUnspecifiedError = 'wht?',
        kAudioFileUnsupportedFileTypeError = 'typ?',
        kAudioFileUnsupportedDataFormatError = 'fmt?',
        kAudioFileUnsupportedPropertyError = 'pty?',
        kAudioFileBadPropertySizeError = '!siz',
        kAudioFilePermissionsError = 'prm?',
        kAudioFileNotOptimizedError = 'optm',

        kAudioFileInvalidChunkError = 'chk?',
        kAudioFileDoesNotAllow64BitDataSizeError = 'off?',
        kAudioFileInvalidPacketOffsetError = 'pck?',
        kAudioFileInvalidFileError = 'dta?',
  kAudioFileOperationNotSupportedError = 0x6F703F3F,

  kAudioFileNotOpenError = -38,
  kAudioFileEndOfFileError = -39,
  kAudioFilePositionError = -40,
  kAudioFileFileNotFoundError = -43
};
typedef UInt32 AudioFileFlags; enum {
 kAudioFileFlags_EraseFile = 1,
 kAudioFileFlags_DontPageAlignAudioData = 2
};

typedef SInt8 AudioFilePermissions; enum {
 kAudioFileReadPermission = 0x01,
 kAudioFileWritePermission = 0x02,
 kAudioFileReadWritePermission = 0x03
};
typedef struct OpaqueAudioFileID *AudioFileID;




typedef UInt32 AudioFilePropertyID;
enum {
 kAudioFileLoopDirection_NoLooping = 0,
 kAudioFileLoopDirection_Forward = 1,
 kAudioFileLoopDirection_ForwardAndBackward = 2,
 kAudioFileLoopDirection_Backward = 3
};
struct AudioFile_SMPTE_Time
{
 SInt8 mHours;
 UInt8 mMinutes;
 UInt8 mSeconds;
 UInt8 mFrames;
 UInt32 mSubFrameSampleOffset;
};
typedef struct AudioFile_SMPTE_Time AudioFile_SMPTE_Time;







enum {
 kAudioFileMarkerType_Generic = 0,
};
struct AudioFileMarker
{
 Float64 mFramePosition;

 CFStringRef _Nullable mName;
 SInt32 mMarkerID;

 AudioFile_SMPTE_Time mSMPTETime;
 UInt32 mType;
 UInt16 mReserved;
 UInt16 mChannel;
};
typedef struct AudioFileMarker AudioFileMarker;
struct AudioFileMarkerList
{
 UInt32 mSMPTE_TimeType;
 UInt32 mNumberMarkers;
 AudioFileMarker mMarkers[1];
};
typedef struct AudioFileMarkerList AudioFileMarkerList;
static __inline__ __attribute__((always_inline)) size_t NumBytesToNumAudioFileMarkers(size_t inNumBytes)
{
 return (inNumBytes<__builtin_offsetof(AudioFileMarkerList, mMarkers[0]) ? 0 : (inNumBytes - __builtin_offsetof(AudioFileMarkerList, mMarkers[0])) / sizeof(AudioFileMarker));
}
static __inline__ __attribute__((always_inline)) size_t NumAudioFileMarkersToNumBytes(size_t inNumMarkers)
{
 return (__builtin_offsetof(AudioFileMarkerList, mMarkers) + (inNumMarkers) * sizeof(AudioFileMarker));
}
typedef UInt32 AudioFileRegionFlags; enum {
 kAudioFileRegionFlag_LoopEnable = 1,
 kAudioFileRegionFlag_PlayForward = 2,
 kAudioFileRegionFlag_PlayBackward = 4
};
struct AudioFileRegion
{
 UInt32 mRegionID;
 CFStringRef mName;
 AudioFileRegionFlags mFlags;
 UInt32 mNumberMarkers;
 AudioFileMarker mMarkers[1];
};
typedef struct AudioFileRegion AudioFileRegion;
struct AudioFileRegionList
{
 UInt32 mSMPTE_TimeType;
 UInt32 mNumberRegions;
 AudioFileRegion mRegions[1];
};
typedef struct AudioFileRegionList AudioFileRegionList;
static __inline__ __attribute__((always_inline)) AudioFileRegion *NextAudioFileRegion(const AudioFileRegion *inAFRegionPtr)
{
 return ((AudioFileRegion*)((char*)inAFRegionPtr + __builtin_offsetof(AudioFileRegion, mMarkers) + (inAFRegionPtr->mNumberMarkers)*sizeof(AudioFileMarker)));
}
struct AudioFramePacketTranslation
{
 SInt64 mFrame;
 SInt64 mPacket;
 UInt32 mFrameOffsetInPacket;
};
typedef struct AudioFramePacketTranslation AudioFramePacketTranslation;
typedef UInt32 AudioBytePacketTranslationFlags; enum {
 kBytePacketTranslationFlag_IsEstimate = 1
};
struct AudioBytePacketTranslation
{
 SInt64 mByte;
 SInt64 mPacket;
 UInt32 mByteOffsetInPacket;
 AudioBytePacketTranslationFlags mFlags;
};
typedef struct AudioBytePacketTranslation AudioBytePacketTranslation;
struct AudioFilePacketTableInfo
{
        SInt64 mNumberValidFrames;
        SInt32 mPrimingFrames;
        SInt32 mRemainderFrames;
};
typedef struct AudioFilePacketTableInfo AudioFilePacketTableInfo;
extern OSStatus
AudioFileCreateWithURL (CFURLRef inFileRef,
                    AudioFileTypeID inFileType,
                    const AudioStreamBasicDescription *inFormat,
                    AudioFileFlags inFlags,
                    AudioFileID _Nullable * _Nonnull outAudioFile);
extern OSStatus
AudioFileOpenURL ( CFURLRef inFileRef,
     AudioFilePermissions inPermissions,
     AudioFileTypeID inFileTypeHint,
     AudioFileID _Nullable * _Nonnull outAudioFile);
typedef OSStatus (*AudioFile_ReadProc)(
        void * inClientData,
        SInt64 inPosition,
        UInt32 requestCount,
        void * buffer,
        UInt32 * actualCount);
typedef OSStatus (*AudioFile_WriteProc)(
        void * inClientData,
        SInt64 inPosition,
        UInt32 requestCount,
        const void *buffer,
        UInt32 * actualCount);
typedef SInt64 (*AudioFile_GetSizeProc)(
        void * inClientData);
typedef OSStatus (*AudioFile_SetSizeProc)(
        void * inClientData,
        SInt64 inSize);
extern OSStatus
AudioFileInitializeWithCallbacks (
      void * inClientData,
      AudioFile_ReadProc inReadFunc,
      AudioFile_WriteProc inWriteFunc,
      AudioFile_GetSizeProc inGetSizeFunc,
      AudioFile_SetSizeProc inSetSizeFunc,
                        AudioFileTypeID inFileType,
                        const AudioStreamBasicDescription *inFormat,
                        AudioFileFlags inFlags,
                        AudioFileID _Nullable * _Nonnull outAudioFile);
extern OSStatus
AudioFileOpenWithCallbacks (
    void * inClientData,
    AudioFile_ReadProc inReadFunc,
    AudioFile_WriteProc _Nullable inWriteFunc,
    AudioFile_GetSizeProc inGetSizeFunc,
    AudioFile_SetSizeProc _Nullable inSetSizeFunc,
                AudioFileTypeID inFileTypeHint,
                AudioFileID _Nullable * _Nonnull outAudioFile);
extern OSStatus
AudioFileClose (AudioFileID inAudioFile);
extern OSStatus
AudioFileOptimize (AudioFileID inAudioFile);
extern OSStatus
AudioFileReadBytes ( AudioFileID inAudioFile,
                        Boolean inUseCache,
                        SInt64 inStartingByte,
                        UInt32 *ioNumBytes,
                        void *outBuffer);
extern OSStatus
AudioFileWriteBytes ( AudioFileID inAudioFile,
                        Boolean inUseCache,
                        SInt64 inStartingByte,
                        UInt32 *ioNumBytes,
                        const void *inBuffer);
extern OSStatus
AudioFileReadPacketData ( AudioFileID inAudioFile,
                         Boolean inUseCache,
                         UInt32 * ioNumBytes,
                         AudioStreamPacketDescription * _Nullable outPacketDescriptions,
                         SInt64 inStartingPacket,
                         UInt32 * ioNumPackets,
                         void * _Nullable outBuffer);
extern OSStatus
AudioFileReadPackets ( AudioFileID inAudioFile,
                        Boolean inUseCache,
                        UInt32 * outNumBytes,
                        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
                        SInt64 inStartingPacket,
                        UInt32 * ioNumPackets,
                        void * _Nullable outBuffer);
extern OSStatus
AudioFileWritePackets ( AudioFileID inAudioFile,
                        Boolean inUseCache,
                        UInt32 inNumBytes,
                        const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
                        SInt64 inStartingPacket,
                        UInt32 *ioNumPackets,
                        const void *inBuffer);
extern OSStatus
AudioFileCountUserData ( AudioFileID inAudioFile,
       UInt32 inUserDataID,
       UInt32 *outNumberItems);
extern OSStatus
AudioFileGetUserDataSize ( AudioFileID inAudioFile,
       UInt32 inUserDataID,
       UInt32 inIndex,
       UInt32 *outUserDataSize);
extern OSStatus
AudioFileGetUserData ( AudioFileID inAudioFile,
      UInt32 inUserDataID,
      UInt32 inIndex,
      UInt32 *ioUserDataSize,
      void *outUserData);
extern OSStatus
AudioFileSetUserData ( AudioFileID inAudioFile,
      UInt32 inUserDataID,
      UInt32 inIndex,
      UInt32 inUserDataSize,
      const void *inUserData);
extern OSStatus
AudioFileRemoveUserData ( AudioFileID inAudioFile,
      UInt32 inUserDataID,
      UInt32 inIndex);
enum
{
 kAudioFilePropertyFileFormat = 'ffmt',
 kAudioFilePropertyDataFormat = 'dfmt',
 kAudioFilePropertyIsOptimized = 'optm',
 kAudioFilePropertyMagicCookieData = 'mgic',
 kAudioFilePropertyAudioDataByteCount = 'bcnt',
 kAudioFilePropertyAudioDataPacketCount = 'pcnt',
 kAudioFilePropertyMaximumPacketSize = 'psze',
 kAudioFilePropertyDataOffset = 'doff',
 kAudioFilePropertyChannelLayout = 'cmap',
 kAudioFilePropertyDeferSizeUpdates = 'dszu',
 kAudioFilePropertyDataFormatName = 'fnme',
 kAudioFilePropertyMarkerList = 'mkls',
 kAudioFilePropertyRegionList = 'rgls',
 kAudioFilePropertyPacketToFrame = 'pkfr',
 kAudioFilePropertyFrameToPacket = 'frpk',
 kAudioFilePropertyPacketToByte = 'pkby',
 kAudioFilePropertyByteToPacket = 'bypk',
 kAudioFilePropertyChunkIDs = 'chid',
 kAudioFilePropertyInfoDictionary = 'info',
 kAudioFilePropertyPacketTableInfo = 'pnfo',
 kAudioFilePropertyFormatList = 'flst',
 kAudioFilePropertyPacketSizeUpperBound = 'pkub',
 kAudioFilePropertyReserveDuration = 'rsrv',
 kAudioFilePropertyEstimatedDuration = 'edur',
 kAudioFilePropertyBitRate = 'brat',
 kAudioFilePropertyID3Tag = 'id3t',
 kAudioFilePropertySourceBitDepth = 'sbtd',
 kAudioFilePropertyAlbumArtwork = 'aart',
    kAudioFilePropertyAudioTrackCount = 'atct',
 kAudioFilePropertyUseAudioTrack = 'uatk'
};
extern OSStatus
AudioFileGetPropertyInfo( AudioFileID inAudioFile,
                                AudioFilePropertyID inPropertyID,
                                UInt32 * _Nullable outDataSize,
                                UInt32 * _Nullable isWritable);
extern OSStatus
AudioFileGetProperty( AudioFileID inAudioFile,
                        AudioFilePropertyID inPropertyID,
                        UInt32 *ioDataSize,
                        void *outPropertyData);
extern OSStatus
AudioFileSetProperty( AudioFileID inAudioFile,
                        AudioFilePropertyID inPropertyID,
                        UInt32 inDataSize,
                        const void *inPropertyData);
enum
{
 kAudioFileGlobalInfo_ReadableTypes = 'afrf',
 kAudioFileGlobalInfo_WritableTypes = 'afwf',
 kAudioFileGlobalInfo_FileTypeName = 'ftnm',
 kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat = 'sdid',
 kAudioFileGlobalInfo_AvailableFormatIDs = 'fmid',

 kAudioFileGlobalInfo_AllExtensions = 'alxt',
 kAudioFileGlobalInfo_AllHFSTypeCodes = 'ahfs',
 kAudioFileGlobalInfo_AllUTIs = 'auti',
 kAudioFileGlobalInfo_AllMIMETypes = 'amim',

 kAudioFileGlobalInfo_ExtensionsForType = 'fext',
 kAudioFileGlobalInfo_HFSTypeCodesForType = 'fhfs',
 kAudioFileGlobalInfo_UTIsForType = 'futi',
 kAudioFileGlobalInfo_MIMETypesForType = 'fmim',

 kAudioFileGlobalInfo_TypesForMIMEType = 'tmim',
 kAudioFileGlobalInfo_TypesForUTI = 'tuti',
 kAudioFileGlobalInfo_TypesForHFSTypeCode = 'thfs',
 kAudioFileGlobalInfo_TypesForExtension = 'text'
};
struct AudioFileTypeAndFormatID
{
 AudioFileTypeID mFileType;
 UInt32 mFormatID;
};
typedef struct AudioFileTypeAndFormatID AudioFileTypeAndFormatID;
extern OSStatus
AudioFileGetGlobalInfoSize( AudioFilePropertyID inPropertyID,
                                UInt32 inSpecifierSize,
                                void * _Nullable inSpecifier,
                                UInt32 *outDataSize);
extern OSStatus
AudioFileGetGlobalInfo( AudioFilePropertyID inPropertyID,
        UInt32 inSpecifierSize,
                                void * _Nullable inSpecifier,
                          UInt32 *ioDataSize,
                          void *outPropertyData);



struct FSRef;
extern OSStatus
AudioFileCreate ( const struct FSRef *inParentRef,
                    CFStringRef inFileName,
                    AudioFileTypeID inFileType,
                    const AudioStreamBasicDescription *inFormat,
                    AudioFileFlags inFlags,
                    struct FSRef *outNewFileRef,
                    AudioFileID _Nullable * _Nonnull outAudioFile);
extern OSStatus
AudioFileInitialize ( const struct FSRef *inFileRef,
                        AudioFileTypeID inFileType,
                        const AudioStreamBasicDescription *inFormat,
                        AudioFileFlags inFlags,
                        AudioFileID _Nullable * _Nonnull outAudioFile);
extern OSStatus
AudioFileOpen ( const struct FSRef *inFileRef,
                AudioFilePermissions inPermissions,
                AudioFileTypeID inFileTypeHint,
                AudioFileID _Nullable * _Nonnull outAudioFile);

#define kAFInfoDictionary_Album                                                 "album"
#define kAFInfoDictionary_ApproximateDurationInSeconds  "approximate duration in seconds"
#define kAFInfoDictionary_Artist                        "artist"
#define kAFInfoDictionary_ChannelLayout                                 "channel layout"
#define kAFInfoDictionary_Comments                                              "comments"
#define kAFInfoDictionary_Composer                                              "composer"
#define kAFInfoDictionary_Copyright                                             "copyright"
#define kAFInfoDictionary_EncodingApplication           "encoding application"
#define kAFInfoDictionary_Genre                                                 "genre"
#define kAFInfoDictionary_ISRC                                                  "ISRC"                                  // International Standard Recording Code
#define kAFInfoDictionary_KeySignature                                  "key signature"
#define kAFInfoDictionary_Lyricist                                              "lyricist"
#define kAFInfoDictionary_NominalBitRate                "nominal bit rate"
#define kAFInfoDictionary_RecordedDate                                  "recorded date"
#define kAFInfoDictionary_SourceBitDepth                                "source bit depth"
#define kAFInfoDictionary_SourceEncoder                                 "source encoder"
#define kAFInfoDictionary_SubTitle                                              "subtitle"
#define kAFInfoDictionary_Tempo                                                 "tempo"
#define kAFInfoDictionary_TimeSignature                                 "time signature"
#define kAFInfoDictionary_Title                                                 "title"
#define kAFInfoDictionary_TrackNumber                   "track number"
#define kAFInfoDictionary_Year                                                  "year"


#ifdef __cplusplus
}
#endif

#endif

