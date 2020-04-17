#ifndef _AUDIO_FILE_COMPONENT_H
#define _AUDIO_FILE_COMPONENT_H

#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioComponent.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef AudioComponentInstance AudioFileComponent;




typedef UInt32 AudioFileComponentPropertyID;
extern OSStatus
AudioFileComponentCreateURL (
       AudioFileComponent inComponent,
       CFURLRef inFileRef,
       const AudioStreamBasicDescription *inFormat,
       UInt32 inFlags);
extern OSStatus
AudioFileComponentOpenURL (
     AudioFileComponent inComponent,
     CFURLRef inFileRef,
     SInt8 inPermissions,
     int inFileDescriptor);
extern OSStatus
AudioFileComponentOpenWithCallbacks(
        AudioFileComponent inComponent,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc);
extern OSStatus
AudioFileComponentInitializeWithCallbacks(
        AudioFileComponent inComponent,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc,
        UInt32 inFileType,
        const AudioStreamBasicDescription *inFormat,
        UInt32 inFlags);
extern OSStatus
AudioFileComponentCloseFile(
        AudioFileComponent inComponent);







extern OSStatus
AudioFileComponentOptimize(
        AudioFileComponent inComponent);
extern OSStatus
AudioFileComponentReadBytes(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        void *outBuffer);
extern OSStatus
AudioFileComponentWriteBytes(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        const void *inBuffer);
extern OSStatus
AudioFileComponentReadPackets(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 *outNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer);
extern OSStatus
AudioFileComponentReadPacketData(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 *ioNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer);
extern OSStatus
AudioFileComponentWritePackets(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 inNumBytes,
        const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        const void *inBuffer);
extern OSStatus
AudioFileComponentGetPropertyInfo(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 * _Nullable outPropertySize,
        UInt32 * _Nullable outWritable);
extern OSStatus
AudioFileComponentGetProperty(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData);
extern OSStatus
AudioFileComponentSetProperty(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inPropertyDataSize,
        const void *inPropertyData);
extern OSStatus
AudioFileComponentCountUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 *outNumberItems);
extern OSStatus
AudioFileComponentGetUserDataSize(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *outUserDataSize);
extern OSStatus
AudioFileComponentGetUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *ioUserDataSize,
        void *outUserData);
extern OSStatus
AudioFileComponentSetUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 inUserDataSize,
        const void *inUserData);
extern OSStatus
AudioFileComponentRemoveUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex);
extern OSStatus
AudioFileComponentExtensionIsThisFormat(
        AudioFileComponent inComponent,
        CFStringRef inExtension,
        UInt32 *outResult);
extern OSStatus
AudioFileComponentFileDataIsThisFormat(
        AudioFileComponent inComponent,
        UInt32 inDataByteSize,
        const void* inData,
        UInt32 *outResult);
extern OSStatus
AudioFileComponentFileIsThisFormat(
        AudioFileComponent inComponent,
        SInt16 inFileRefNum,
        UInt32 *outResult);
extern OSStatus
AudioFileComponentDataIsThisFormat(
        AudioFileComponent inComponent,
        void * _Nullable inClientData,
        AudioFile_ReadProc _Nullable inReadFunc,
        AudioFile_WriteProc _Nullable inWriteFunc,
        AudioFile_GetSizeProc _Nullable inGetSizeFunc,
        AudioFile_SetSizeProc _Nullable inSetSizeFunc,
        UInt32 *outResult);
extern OSStatus
AudioFileComponentGetGlobalInfoSize(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *outPropertySize);
extern OSStatus
AudioFileComponentGetGlobalInfo(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData);
enum
{
 kAudioFileComponent_CanRead = 'cnrd',
 kAudioFileComponent_CanWrite = 'cnwr',
 kAudioFileComponent_FileTypeName = 'ftnm',
 kAudioFileComponent_UTIsForType = 'futi',
 kAudioFileComponent_MIMETypesForType = 'fmim',
 kAudioFileComponent_ExtensionsForType = 'fext',
 kAudioFileComponent_AvailableFormatIDs = 'fmid',
 kAudioFileComponent_AvailableStreamDescriptionsForFormat = 'sdid',
 kAudioFileComponent_FastDispatchTable = 'fdft',
 kAudioFileComponent_HFSTypeCodesForType = 'fhfs'
};





enum
{
 kAudioFileCreateSelect = 0x0001,
 kAudioFileOpenSelect = 0x0002,
 kAudioFileInitializeSelect = 0x0003,
 kAudioFileOpenWithCallbacksSelect = 0x0004,
 kAudioFileInitializeWithCallbacksSelect = 0x0005,
 kAudioFileCloseSelect = 0x0006,
 kAudioFileOptimizeSelect = 0x0007,
 kAudioFileReadBytesSelect = 0x0008,
 kAudioFileWriteBytesSelect = 0x0009,
 kAudioFileReadPacketsSelect = 0x000A,
 kAudioFileWritePacketsSelect = 0x000B,
 kAudioFileGetPropertyInfoSelect = 0x000C,
 kAudioFileGetPropertySelect = 0x000D,
 kAudioFileSetPropertySelect = 0x000E,

 kAudioFileExtensionIsThisFormatSelect = 0x000F,
 kAudioFileFileIsThisFormatSelect = 0x0010,
 kAudioFileDataIsThisFormatSelect = 0x0011,
 kAudioFileGetGlobalInfoSizeSelect = 0x0012,
 kAudioFileGetGlobalInfoSelect = 0x0013,

 kAudioFileCountUserDataSelect = 0x0014,
 kAudioFileGetUserDataSizeSelect = 0x0015,
 kAudioFileGetUserDataSelect = 0x0016,
 kAudioFileSetUserDataSelect = 0x0017,
 kAudioFileRemoveUserDataSelect = 0x0018,
 kAudioFileCreateURLSelect = 0x0019,
 kAudioFileOpenURLSelect = 0x001A,
 kAudioFileFileDataIsThisFormatSelect = 0x001B,
 kAudioFileReadPacketDataSelect = 0x001C,
};
typedef OSStatus (*ReadBytesFDF)( void *inComponentStorage,
         Boolean inUseCache,
         SInt64 inStartingByte,
         UInt32 *ioNumBytes,
         void *outBuffer);

typedef OSStatus (*WriteBytesFDF)( void *inComponentStorage,
         Boolean inUseCache,
         SInt64 inStartingByte,
         UInt32 *ioNumBytes,
         const void *inBuffer);

typedef OSStatus (*ReadPacketsFDF)( void *inComponentStorage,
         Boolean inUseCache,
         UInt32 *outNumBytes,
         AudioStreamPacketDescription * _Nullable outPacketDescriptions,
         SInt64 inStartingPacket,
         UInt32 *ioNumPackets,
         void *outBuffer);

typedef OSStatus (*ReadPacketDataFDF)( void *inComponentStorage,
         Boolean inUseCache,
         UInt32 *ioNumBytes,
         AudioStreamPacketDescription * _Nullable outPacketDescriptions,
         SInt64 inStartingPacket,
         UInt32 *ioNumPackets,
         void *outBuffer);

typedef OSStatus (*WritePacketsFDF)( void *inComponentStorage,
          Boolean inUseCache,
          UInt32 inNumBytes,
          const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
          SInt64 inStartingPacket,
          UInt32 *ioNumPackets,
          const void *inBuffer);

typedef OSStatus (*GetPropertyInfoFDF)( void *inComponentStorage,
          AudioFilePropertyID inPropertyID,
          UInt32 * _Nullable outDataSize,
          UInt32 * _Nullable isWritable);

typedef OSStatus (*GetPropertyFDF)( void *inComponentStorage,
          AudioFilePropertyID inPropertyID,
          UInt32 *ioDataSize,
          void *ioPropertyData);

typedef OSStatus (*SetPropertyFDF)( void *inComponentStorage,
          AudioFilePropertyID inPropertyID,
          UInt32 inDataSize,
          const void *inPropertyData);

typedef OSStatus (*CountUserDataFDF)( void *inComponentStorage,
          UInt32 inUserDataID,
          UInt32 *outNumberItems);

typedef OSStatus (*GetUserDataSizeFDF)( void *inComponentStorage,
          UInt32 inUserDataID,
          UInt32 inIndex,
          UInt32 *outDataSize);

typedef OSStatus (*GetUserDataFDF)( void *inComponentStorage,
          UInt32 inUserDataID,
          UInt32 inIndex,
          UInt32 *ioUserDataSize,
          void *outUserData);

typedef OSStatus (*SetUserDataFDF)( void *inComponentStorage,
          UInt32 inUserDataID,
          UInt32 inIndex,
          UInt32 inUserDataSize,
          const void *inUserData);
typedef struct AudioFileFDFTable
{
 void *mComponentStorage;
 ReadBytesFDF mReadBytesFDF;
 WriteBytesFDF mWriteBytesFDF;
 ReadPacketsFDF mReadPacketsFDF;
 WritePacketsFDF mWritePacketsFDF;

 GetPropertyInfoFDF mGetPropertyInfoFDF;
 GetPropertyFDF mGetPropertyFDF;
 SetPropertyFDF mSetPropertyFDF;

 CountUserDataFDF mCountUserDataFDF;
 GetUserDataSizeFDF mGetUserDataSizeFDF;
 GetUserDataFDF mGetUserDataFDF;
 SetUserDataFDF mSetUserDataFDF;
} AudioFileFDFTable;

typedef struct AudioFileFDFTableExtended
{
 void *mComponentStorage;
 ReadBytesFDF mReadBytesFDF;
 WriteBytesFDF mWriteBytesFDF;
 ReadPacketsFDF mReadPacketsFDF;
 WritePacketsFDF mWritePacketsFDF;

 GetPropertyInfoFDF mGetPropertyInfoFDF;
 GetPropertyFDF mGetPropertyFDF;
 SetPropertyFDF mSetPropertyFDF;

 CountUserDataFDF mCountUserDataFDF;
 GetUserDataSizeFDF mGetUserDataSizeFDF;
 GetUserDataFDF mGetUserDataFDF;
 SetUserDataFDF mSetUserDataFDF;

 ReadPacketDataFDF mReadPacketDataFDF;
} AudioFileFDFTableExtended;







struct FSRef;
extern OSStatus
AudioFileComponentCreate(
        AudioFileComponent inComponent,
        const struct FSRef *inParentRef,
        CFStringRef inFileName,
        const AudioStreamBasicDescription *inFormat,
        UInt32 inFlags,
        struct FSRef *outNewFileRef);
extern OSStatus
AudioFileComponentInitialize(
        AudioFileComponent inComponent,
        const struct FSRef *inFileRef,
        const AudioStreamBasicDescription *inFormat,
        UInt32 inFlags);
extern OSStatus
AudioFileComponentOpenFile(
        AudioFileComponent inComponent,
        const struct FSRef *inFileRef,
        SInt8 inPermissions,
        SInt16 inRefNum);







typedef OSStatus (*AudioFileComponentCreateURLProc)(
        void *self,
        CFURLRef inFileRef,
        const AudioStreamBasicDescription* inFormat,
        UInt32 inFlags);
typedef OSStatus (*AudioFileComponentOpenURLProc)(
        void *self,
        CFURLRef inFileRef,
        SInt8 inPermissions,
        int inFileDescriptor);

typedef OSStatus (*AudioFileComponentOpenWithCallbacksProc)(
        void *self,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc);

typedef OSStatus (*AudioFileComponentInitializeWithCallbacksProc)(
        void *self,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc,
        UInt32 inFileType,
        const AudioStreamBasicDescription *inFormat,
        UInt32 inFlags);

typedef OSStatus (*AudioFileComponentCloseProc)(
        void *self);

typedef OSStatus (*AudioFileComponentOptimizeProc)(
        void *self);

typedef OSStatus (*AudioFileComponentReadBytesProc)(
        void *self,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        void *outBuffer);

typedef OSStatus (*AudioFileComponentWriteBytesProc)(
        void *self,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        const void *inBuffer);

typedef OSStatus (*AudioFileComponentReadPacketsProc)(
        void *self,
        Boolean inUseCache,
        UInt32 *outNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer);


typedef OSStatus (*AudioFileComponentReadPacketDataProc)(
        void *self,
        Boolean inUseCache,
        UInt32 *ioNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer);

typedef OSStatus (*AudioFileComponentWritePacketsProc)(
        void *self,
        Boolean inUseCache,
        UInt32 inNumBytes,
        const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        const void *inBuffer);


typedef OSStatus (*AudioFileComponentGetPropertyInfoProc)(
        void *self,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 * _Nullable outPropertySize,
        UInt32 * _Nullable outWritable);


typedef OSStatus (*AudioFileComponentGetPropertyProc)(
        void *self,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData);

typedef OSStatus (*AudioFileComponentSetPropertyProc)(
        void *self,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inPropertyDataSize,
        const void *inPropertyData);


typedef OSStatus (*AudioFileComponentCountUserDataProc)(
        void *self,
        UInt32 inUserDataID,
        UInt32 *outNumberItems);

typedef OSStatus (*AudioFileComponentGetUserDataSizeProc)(
        void *self,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *outUserDataSize);

typedef OSStatus (*AudioFileComponentGetUserDataProc)(
        void *self,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *ioUserDataSize,
        void *outUserData);

typedef OSStatus (*AudioFileComponentSetUserDataProc)(
        void *self,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 inUserDataSize,
        const void *inUserData);


typedef OSStatus (*AudioFileComponentRemoveUserDataProc)(
        void *self,
        UInt32 inUserDataID,
        UInt32 inIndex);

typedef OSStatus (*AudioFileComponentExtensionIsThisFormatProc)(
        void *self,
        CFStringRef inExtension,
        UInt32 *outResult);


typedef OSStatus (*AudioFileComponentFileDataIsThisFormatProc)(
        void *self,
        UInt32 inDataByteSize,
        const void* inData,
        UInt32 *outResult);


typedef OSStatus (*AudioFileComponentGetGlobalInfoSizeProc)(
        void *self,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *outPropertySize);

typedef OSStatus (*AudioFileComponentGetGlobalInfoProc)(
        void *self,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData);

#ifdef __cplusplus
}
#endif

#endif
