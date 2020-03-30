#ifndef _EXTENDED_AUDIO_FILE_H
#define _EXTENDED_AUDIO_FILE_H

#include <CoreFoundation/CoreFoundation.h>
#include <AudioToolbox/AudioFile.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueExtAudioFile * ExtAudioFileRef;





typedef UInt32 ExtAudioFilePropertyID;
enum {
 kExtAudioFileProperty_FileDataFormat = 'ffmt',
 kExtAudioFileProperty_FileChannelLayout = 'fclo',
 kExtAudioFileProperty_ClientDataFormat = 'cfmt',
 kExtAudioFileProperty_ClientChannelLayout = 'cclo',
 kExtAudioFileProperty_CodecManufacturer = 'cman',


 kExtAudioFileProperty_AudioConverter = 'acnv',
 kExtAudioFileProperty_AudioFile = 'afil',
 kExtAudioFileProperty_FileMaxPacketSize = 'fmps',
 kExtAudioFileProperty_ClientMaxPacketSize = 'cmps',
 kExtAudioFileProperty_FileLengthFrames = '#frm',


 kExtAudioFileProperty_ConverterConfig = 'accf',
 kExtAudioFileProperty_IOBufferSizeBytes = 'iobs',
 kExtAudioFileProperty_IOBuffer = 'iobf',
 kExtAudioFileProperty_PacketTable = 'xpti'
};

enum {
 kExtAudioFileError_InvalidProperty = -66561,
 kExtAudioFileError_InvalidPropertySize = -66562,
 kExtAudioFileError_NonPCMClientFormat = -66563,
 kExtAudioFileError_InvalidChannelMap = -66564,
 kExtAudioFileError_InvalidOperationOrder = -66565,
 kExtAudioFileError_InvalidDataFormat = -66566,
 kExtAudioFileError_MaxPacketSizeUnknown = -66567,
 kExtAudioFileError_InvalidSeek = -66568,
 kExtAudioFileError_AsyncWriteTooLarge = -66569,
 kExtAudioFileError_AsyncWriteBufferOverflow = -66570
};
extern OSStatus
ExtAudioFileOpenURL( CFURLRef inURL,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile);
extern OSStatus
ExtAudioFileWrapAudioFileID(AudioFileID inFileID,
       Boolean inForWriting,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile);
extern OSStatus
ExtAudioFileCreateWithURL( CFURLRef inURL,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
                      UInt32 inFlags,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile);
extern OSStatus
ExtAudioFileOpen( const struct FSRef * inFSRef,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile);
extern OSStatus
ExtAudioFileCreateNew( const struct FSRef * inParentDir,
       CFStringRef inFileName,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile);
extern OSStatus
ExtAudioFileDispose( ExtAudioFileRef inExtAudioFile);
extern OSStatus
ExtAudioFileRead( ExtAudioFileRef inExtAudioFile,
       UInt32 * ioNumberFrames,
       AudioBufferList * ioData);
extern OSStatus
ExtAudioFileWrite( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * ioData);
extern OSStatus
ExtAudioFileWriteAsync( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * _Nullable ioData);
extern OSStatus
ExtAudioFileSeek( ExtAudioFileRef inExtAudioFile,
       SInt64 inFrameOffset);
extern OSStatus
ExtAudioFileTell( ExtAudioFileRef inExtAudioFile,
       SInt64 * outFrameOffset);
extern OSStatus
ExtAudioFileGetPropertyInfo(ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * _Nullable outSize,
       Boolean * _Nullable outWritable);
extern OSStatus
ExtAudioFileGetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * ioPropertyDataSize,
       void * outPropertyData);
extern OSStatus
ExtAudioFileSetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 inPropertyDataSize,
       const void * inPropertyData);

#ifdef __cplusplus
}
#endif

#endif
