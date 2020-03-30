/*
     File: CAExtAudioFile.h
 Abstract: Part of CoreAudio Utility Classes
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#ifndef __CAExtAudioFile_h__
#define __CAExtAudioFile_h__

#include <TargetConditionals.h>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/ExtendedAudioFile.h>
	#include <AudioToolbox/AudioConverter.h>
#else
	#include "ExtendedAudioFile.h"
	#include "AudioConverter.h"
#endif
#include "CAXException.h"
//#include "CAAutoDisposer.h"
#include "CAStreamBasicDescription.h"
#include "CAAudioChannelLayout.h"
#include "CACFObject.h"

// A C++ wrapper for ExtAudioFile
// Error returns throw CAXExceptions.
class CAExtAudioFile {
public:
	// instances are not automatically associated with open files.
	CAExtAudioFile() :
		mExtAudioFile(NULL) { }

	virtual ~CAExtAudioFile()
	{
		Close();
	}
	
	bool IsValid() const { return mExtAudioFile != NULL; }
	
	void	Open(const char* filename)
	{
		Close();
		CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (const UInt8*)filename, strlen(filename), false);
		XThrowIf(!url, -1, "couldn't convert path to CFURLRef");
		OSStatus res = ExtAudioFileOpenURL(url, &mExtAudioFile);
		if (res)
			CFRelease(url);
		Check(res, "ExtAudioFileOpenURL");
		CFRelease (url);
	}
	
	// this group of methods maps directly to the API other than OSStatus results translating into exceptions.
	// you must explicitly open, wrap or create a file.
	void	OpenURL(CFURLRef url)
	{
		Close();
		Check(ExtAudioFileOpenURL(url, &mExtAudioFile), "ExtAudioFileOpenURL");
	}
		
	void	WrapAudioFileID(AudioFileID inFileID, Boolean forWriting)
	{
		Close();
		Check(ExtAudioFileWrapAudioFileID(inFileID, forWriting, &mExtAudioFile), "ExtAudioFileWrapAudioFileID");
	}

	void	Create(const char *filePath, AudioFileTypeID filetype, const AudioStreamBasicDescription &streamDesc, const AudioChannelLayout *channelLayout, UInt32 flags)
	{
		CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)filePath, strlen(filePath), false);
		XThrowIf(!url, -1, "couldn't convert path to CFURLRef");
		Close();
		OSStatus res = ExtAudioFileCreateWithURL(url, filetype, &streamDesc, channelLayout, flags, &mExtAudioFile);
		if (res)
			CFRelease(url);
		Check(res, "ExtAudioFileCreateWithURL");
		CFRelease(url);
	}
	
	void	CreateWithURL(CFURLRef url, AudioFileTypeID filetype, const AudioStreamBasicDescription &streamDesc, const AudioChannelLayout *channelLayout, UInt32 flags)
	{
		Close();
		Check(ExtAudioFileCreateWithURL(url, filetype, &streamDesc, channelLayout, flags, &mExtAudioFile), "ExtAudioFileCreateWithURL");
	}

	// you may explicitly close a file, or have it closed automatically by the destructor.
	void	Close()
	{
		if (mExtAudioFile != NULL) {
			Check(ExtAudioFileDispose(mExtAudioFile), "ExtAudioFileClose");
			mExtAudioFile = NULL;
		}
	}
	
	void	Read(UInt32 &ioNumberFrames, AudioBufferList *ioData)
	{
		Check(ExtAudioFileRead(mExtAudioFile, &ioNumberFrames, ioData), "ExtAudioFileRead");
	}
	
	OSStatus	Write(UInt32 inNumberFrames, const AudioBufferList *ioData)
	{
		OSStatus err = ExtAudioFileWrite(mExtAudioFile, inNumberFrames, ioData);
		switch (err) {
			// noErr and certain special errors are returned without an exception
		case noErr:
			break;
	#if TARGET_OS_IPHONE
		case kExtAudioFileError_CodecUnavailableInputConsumed:
		case kExtAudioFileError_CodecUnavailableInputNotConsumed:
			break;
	#endif
		default:
			// throw an exception
			Check(err, "ExtAudioFileWrite");
			break;
		}
		return err;
	}
	
	void	WriteAsync(UInt32 inNumberFrames, const AudioBufferList *ioData)
	{
		Check(ExtAudioFileWriteAsync(mExtAudioFile, inNumberFrames, ioData), "ExtAudioFileWriteAsync");
	}
	
	void	Seek(SInt64 inFrameOffset)
	{
		Check(ExtAudioFileSeek(mExtAudioFile, inFrameOffset), "ExtAudioFileSeek");
	}
	
	SInt64	Tell() const
	{
		SInt64 pos;
		Check(ExtAudioFileTell(mExtAudioFile, &pos), "ExtAudioFileTell");
		return pos;
	}
	
	UInt32	GetPropertyInfo(ExtAudioFilePropertyID propid, Boolean *outWritable) const
	{
		UInt32 size;
		CheckProperty(ExtAudioFileGetPropertyInfo(mExtAudioFile, propid, &size, outWritable), "ExtAudioFileGetPropertyInfo", propid);
		return size;
	}
	
	void	GetProperty(ExtAudioFilePropertyID propid, UInt32 &ioSize, void *outData) const
	{
		CheckProperty(ExtAudioFileGetProperty(mExtAudioFile, propid, &ioSize, outData), "ExtAudioFileGetProperty", propid);
	}
	
	void	SetProperty(ExtAudioFilePropertyID propid, UInt32 size, const void *inData)
	{
		CheckProperty(ExtAudioFileSetProperty(mExtAudioFile, propid, size, inData), "ExtAudioFileSetProperty", propid);
	}
	
	const CAAudioChannelLayout &GetFileChannelLayout()
	{
		return FetchChannelLayout(mFileChannelLayout, kExtAudioFileProperty_FileChannelLayout);
	}

	void	SetFileChannelLayout(const CAAudioChannelLayout &layout) {
		SetProperty(kExtAudioFileProperty_FileChannelLayout, layout.Size(), &layout.Layout());
	}

	const CAStreamBasicDescription &GetFileDataFormat()
	{
		UInt32 size = sizeof(mFileDataFormat);
		GetProperty(kExtAudioFileProperty_FileDataFormat, size, &mFileDataFormat);
		return mFileDataFormat;
	}
	
	const CAStreamBasicDescription &GetClientDataFormat() {
		UInt32 size = sizeof(mClientDataFormat);
		GetProperty(kExtAudioFileProperty_ClientDataFormat, size, &mClientDataFormat);
		return mClientDataFormat;
	}
	

	void	SetClientFormat(const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout=NULL, UInt32 codecManuf=0) {
		if (codecManuf != 0)
			SetProperty('cman' /*kExtAudioFileProperty_CodecManufacturer*/, sizeof(codecManuf), &codecManuf);
		SetProperty(kExtAudioFileProperty_ClientDataFormat, sizeof(dataFormat), &dataFormat);
		if (layout)
			SetClientChannelLayout(*layout);
	}
	
	void	SetClientChannelLayout(const CAAudioChannelLayout &layout) {
		SetProperty(kExtAudioFileProperty_ClientChannelLayout, layout.Size(), &layout.Layout());
	}
	
	AudioConverterRef				GetConverter() const {
		UInt32 size = sizeof(AudioConverterRef);
		AudioConverterRef converter = NULL;
		GetProperty(kExtAudioFileProperty_AudioConverter, size, &converter);
		return converter;
	}

	bool	HasConverter() const { return GetConverter() != NULL; }
	
	OSStatus	SetConverterProperty(AudioConverterPropertyID inPropertyID,	UInt32 inPropertyDataSize, const void *inPropertyData, bool inCanFail=false)
	{
		OSStatus err = AudioConverterSetProperty(GetConverter(), inPropertyID, inPropertyDataSize, inPropertyData);
		if (!inCanFail)
			XThrowIfError(err, "Couldn't set audio converter property");
		if (!err) {
			// must tell the file that we have changed the converter; a NULL converter config is sufficient
			CFPropertyListRef config = NULL;
			SetProperty(kExtAudioFileProperty_ConverterConfig, sizeof(CFPropertyListRef), &config);
		}
		return err;
	}
	
	SInt64		GetNumberFrames() {
		SInt64 length;
		UInt32 size = sizeof(SInt64);
		GetProperty(kExtAudioFileProperty_FileLengthFrames, size, &length);
		return length;
	}
	

protected:
	virtual void Check(OSStatus err, const char *func) const
	{
		if (err) {
			char txt[128];
			snprintf(txt, sizeof(txt), "%s failed", func);
			throw CAXException(txt, err);
		}
	}
	
	virtual void CheckProperty(OSStatus err, const char *func, UInt32 propid) const
	{
		if (err) {
			char txt[128];
			char propstr[] = CA4CCToCString(propid);
			snprintf(txt, sizeof(txt), "%s ('%s') failed", func, propstr);
			throw CAXException(txt, err);
		}
	}

	const CAAudioChannelLayout &	FetchChannelLayout(CAAudioChannelLayout &layoutObj, ExtAudioFilePropertyID propID) {
		UInt32 size = GetPropertyInfo(propID, NULL);
		CAAutoFree<AudioChannelLayout> layout;
		layout.allocBytes(size);
		GetProperty(propID, size, layout);
		layoutObj = layout;
		return layoutObj;
	}

private:
	CAExtAudioFile(const CAExtAudioFile &) { }	// prohibit
	CAExtAudioFile & operator = (const CAExtAudioFile &) { return *this; } // prohibit

private:
	ExtAudioFileRef				mExtAudioFile;
	
	// for convenience to the client, it helps if we hold onto some storage for these
	CAStreamBasicDescription	mFileDataFormat;
	CAAudioChannelLayout		mFileChannelLayout;
	CAStreamBasicDescription	mClientDataFormat;
};


#endif // __CAExtAudioFile_h__
