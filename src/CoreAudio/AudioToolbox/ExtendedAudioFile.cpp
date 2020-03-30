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


#include <AudioToolbox/ExtendedAudioFile.h>
#include <CoreServices/MacErrors.h>

// TODO: Implement with the help of CAAudioFile?

OSStatus ExtAudioFileOpenURL( CFURLRef inURL,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileWrapAudioFileID(AudioFileID inFileID,
       Boolean inForWriting,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileCreateWithURL( CFURLRef inURL,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
                      UInt32 inFlags,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileOpen( const struct FSRef * inFSRef,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileCreateNew( const struct FSRef * inParentDir,
       CFStringRef inFileName,
       AudioFileTypeID inFileType,
       const AudioStreamBasicDescription * inStreamDesc,
       const AudioChannelLayout * _Nullable inChannelLayout,
       ExtAudioFileRef _Nullable * _Nonnull outExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileDispose( ExtAudioFileRef inExtAudioFile)
{
	return unimpErr;
}

OSStatus ExtAudioFileRead( ExtAudioFileRef inExtAudioFile,
       UInt32 * ioNumberFrames,
       AudioBufferList * ioData)
{
	return unimpErr;
}

OSStatus ExtAudioFileWrite( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * ioData)
{
	return unimpErr;
}

OSStatus ExtAudioFileWriteAsync( ExtAudioFileRef inExtAudioFile,
       UInt32 inNumberFrames,
       const AudioBufferList * _Nullable ioData)
{
	return unimpErr;
}

OSStatus ExtAudioFileSeek( ExtAudioFileRef inExtAudioFile,
       SInt64 inFrameOffset)
{
	return unimpErr;
}

OSStatus ExtAudioFileTell( ExtAudioFileRef inExtAudioFile,
       SInt64 * outFrameOffset)
{
	return unimpErr;
}

OSStatus ExtAudioFileGetPropertyInfo(ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * _Nullable outSize,
       Boolean * _Nullable outWritable)
{
	return unimpErr;
}

OSStatus ExtAudioFileGetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 * ioPropertyDataSize,
       void * outPropertyData)
{
	return unimpErr;
}

OSStatus ExtAudioFileSetProperty( ExtAudioFileRef inExtAudioFile,
       ExtAudioFilePropertyID inPropertyID,
       UInt32 inPropertyDataSize,
       const void * inPropertyData)
{
	return unimpErr;
}
