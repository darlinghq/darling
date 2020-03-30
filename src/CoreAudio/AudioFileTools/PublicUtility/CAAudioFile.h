/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	CAAudioFile.h
	
=============================================================================*/

#ifndef __CAAudioFile_h__
#define __CAAudioFile_h__

#include <AvailabilityMacros.h>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <AudioToolbox.h>
#endif

#include "CAStreamBasicDescription.h"
#include "CABufferList.h"
#include "CAAudioChannelLayout.h"
#include "CAXException.h"
#include "CAMath.h"

#ifndef CAAF_USE_EXTAUDIOFILE
// option: use AudioToolbox/ExtAudioFile.h? Only available on Tiger.
	#if MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_3
		// we are building software that must be deployable on Panther or earlier
		#define CAAF_USE_EXTAUDIOFILE 0
	#else
		// else we require Tiger and can use the API
		#define CAAF_USE_EXTAUDIOFILE 1
	#endif
#endif

#ifndef MAC_OS_X_VERSION_10_4
	// we have pre-Tiger headers; add our own declarations
	typedef UInt32 AudioFileTypeID;
	enum {
		kExtAudioFileError_InvalidProperty			= -66561,
		kExtAudioFileError_InvalidPropertySize		= -66562,
		kExtAudioFileError_NonPCMClientFormat		= -66563,
		kExtAudioFileError_InvalidChannelMap		= -66564,	// number of channels doesn't match format
		kExtAudioFileError_InvalidOperationOrder	= -66565,
		kExtAudioFileError_InvalidDataFormat		= -66566,
		kExtAudioFileError_MaxPacketSizeUnknown		= -66567,
		kExtAudioFileError_InvalidSeek				= -66568,	// writing, or offset out of bounds
		kExtAudioFileError_AsyncWriteTooLarge		= -66569,
		kExtAudioFileError_AsyncWriteBufferOverflow	= -66570	// an async write could not be completed in time
	};
#else
	#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
		#include <AudioToolbox/ExtendedAudioFile.h>
	#else
		#include "ExtendedAudioFile.h"
	#endif
#endif

// _______________________________________________________________________________________
// Wrapper class for an AudioFile, supporting encode/decode to/from a PCM client format
class CAAudioFile {
public:
	// implementation-independent helpers
	void	Open(const char *filePath) {
		FSRef fsref;
		XThrowIfError(FSPathMakeRef((UInt8 *)filePath, &fsref, NULL), "locate audio file");
		Open(fsref);
	}

	bool							HasConverter() const { return GetConverter() != NULL; }

	double  GetDurationSeconds() {
		double sr = GetFileDataFormat().mSampleRate;
		return fnonzero(sr) ? GetNumberFrames() / sr : 0.;
	}
				// will be 0 if the file's frames/packet is 0 (variable)
				// or the file's sample rate is 0 (unknown)

#if CAAF_USE_EXTAUDIOFILE
public:
	CAAudioFile() : mExtAF(NULL) { }
	virtual ~CAAudioFile() { if (mExtAF) Close(); }

	void	Open(const FSRef &fsref) {
				// open an existing file
		XThrowIfError(ExtAudioFileOpen(&fsref, &mExtAF), "ExtAudioFileOpen failed");
	}
	
	void	CreateNew(const FSRef &inParentDir, CFStringRef inFileName,	AudioFileTypeID inFileType, const AudioStreamBasicDescription &inStreamDesc, const AudioChannelLayout *inChannelLayout=NULL) {
		XThrowIfError(ExtAudioFileCreateNew(&inParentDir, inFileName, inFileType, &inStreamDesc, inChannelLayout, &mExtAF), "ExtAudioFileCreateNew failed");
	}

	void	Wrap(AudioFileID fileID, bool forWriting) {
				// use this to wrap an AudioFileID opened externally
		XThrowIfError(ExtAudioFileWrapAudioFileID(fileID, forWriting, &mExtAF), "ExtAudioFileWrapAudioFileID failed");
	}
	
	void	Close() {
		XThrowIfError(ExtAudioFileDispose(mExtAF), "ExtAudioFileClose failed");
		mExtAF = NULL;
	}

	const CAStreamBasicDescription &GetFileDataFormat() {
		UInt32 size = sizeof(mFileDataFormat);
		XThrowIfError(ExtAudioFileGetProperty(mExtAF, kExtAudioFileProperty_FileDataFormat, &size, &mFileDataFormat), "Couldn't get file's data format");
		return mFileDataFormat;
	}
	
	const CAAudioChannelLayout &	GetFileChannelLayout() {
		return FetchChannelLayout(mFileChannelLayout, kExtAudioFileProperty_FileChannelLayout);
	}
	
	void	SetFileChannelLayout(const CAAudioChannelLayout &layout) {
		XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_FileChannelLayout, layout.Size(), &layout.Layout()), "Couldn't set file's channel layout");
		mFileChannelLayout = layout;
	}

	const CAStreamBasicDescription &GetClientDataFormat() {
		UInt32 size = sizeof(mClientDataFormat);
		XThrowIfError(ExtAudioFileGetProperty(mExtAF, kExtAudioFileProperty_ClientDataFormat, &size, &mClientDataFormat), "Couldn't get client data format");
		return mClientDataFormat;
	}
	
	const CAAudioChannelLayout &	GetClientChannelLayout() {
		return FetchChannelLayout(mClientChannelLayout, kExtAudioFileProperty_ClientChannelLayout);
	}
	
	void	SetClientFormat(const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout=NULL) {
		XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_ClientDataFormat, sizeof(dataFormat), &dataFormat), "Couldn't set client format");
		if (layout)
			SetClientChannelLayout(*layout);
	}
	
	void	SetClientChannelLayout(const CAAudioChannelLayout &layout) {
		XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_ClientChannelLayout, layout.Size(), &layout.Layout()), "Couldn't set client channel layout");
	}
	
	AudioConverterRef				GetConverter() const {
		UInt32 size = sizeof(AudioConverterRef);
		AudioConverterRef converter;
		XThrowIfError(ExtAudioFileGetProperty(mExtAF, kExtAudioFileProperty_AudioConverter, &size, &converter), "Couldn't get file's AudioConverter");
		return converter;
	}

	OSStatus	SetConverterProperty(AudioConverterPropertyID inPropertyID,	UInt32 inPropertyDataSize, const void *inPropertyData, bool inCanFail=false)
	{
		OSStatus err = AudioConverterSetProperty(GetConverter(), inPropertyID, inPropertyDataSize, inPropertyData);
		if (!inCanFail)
			XThrowIfError(err, "Couldn't set audio converter property");
		if (!err) {
			// must tell the file that we have changed the converter; a NULL converter config is sufficient
			CFPropertyListRef config = NULL;
			XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_ConverterConfig, sizeof(CFPropertyListRef), &config), "couldn't signal the file that the converter has changed");
		}
		return err;
	}
	
	SInt64		GetNumberFrames() {
		SInt64 length;
		UInt32 size = sizeof(SInt64);
		XThrowIfError(ExtAudioFileGetProperty(mExtAF, kExtAudioFileProperty_FileLengthFrames, &size, &length), "Couldn't get file's length");
		return length;
	}
	
	void		SetNumberFrames(SInt64 length) {
		XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_FileLengthFrames, sizeof(SInt64), &length), "Couldn't set file's length");
	}
	
	void		Seek(SInt64 pos) {
		XThrowIfError(ExtAudioFileSeek(mExtAF, pos), "Couldn't seek in audio file");
	}
	
	SInt64		Tell() {
		SInt64 pos;
		XThrowIfError(ExtAudioFileTell(mExtAF, &pos), "Couldn't get file's mark");
		return pos;
	}
	
	void		Read(UInt32 &ioFrames, AudioBufferList *ioData) {
		XThrowIfError(ExtAudioFileRead(mExtAF, &ioFrames, ioData), "Couldn't read audio file");
	}

	void		Write(UInt32 inFrames, const AudioBufferList *inData) {
		XThrowIfError(ExtAudioFileWrite(mExtAF, inFrames, inData), "Couldn't write audio file");
	}

	void		SetIOBufferSizeBytes(UInt32 bufferSizeBytes) {
		XThrowIfError(ExtAudioFileSetProperty(mExtAF, kExtAudioFileProperty_IOBufferSizeBytes, sizeof(UInt32), &bufferSizeBytes), "Couldn't set audio file's I/O buffer size");
	}

private:
	const CAAudioChannelLayout &	FetchChannelLayout(CAAudioChannelLayout &layoutObj, ExtAudioFilePropertyID propID) {
		UInt32 size;
		XThrowIfError(ExtAudioFileGetPropertyInfo(mExtAF, propID, &size, NULL), "Couldn't get info about channel layout");
		AudioChannelLayout *layout = (AudioChannelLayout *)malloc(size);
		OSStatus err = ExtAudioFileGetProperty(mExtAF, propID, &size, layout);
		if (err) {
			free(layout);
			XThrowIfError(err, "Couldn't get channel layout");
		}
		layoutObj = layout;
		free(layout);
		return layoutObj;
	}


private:
	ExtAudioFileRef				mExtAF;

	CAStreamBasicDescription	mFileDataFormat;
	CAAudioChannelLayout		mFileChannelLayout;

	CAStreamBasicDescription	mClientDataFormat;
	CAAudioChannelLayout		mClientChannelLayout;
#endif

#if !CAAF_USE_EXTAUDIOFILE
	CAAudioFile();
	virtual ~CAAudioFile();

	// --- second-stage initializers ---
	// Use exactly one of the following:
	//		- Open
	//		- PrepareNew followed by Create
	//		- Wrap
	
	void	Open(const FSRef &fsref);
				// open an existing file

	void	CreateNew(const FSRef &inParentDir, CFStringRef inFileName,	AudioFileTypeID inFileType, const AudioStreamBasicDescription &inStreamDesc, const AudioChannelLayout *inChannelLayout=NULL);
	
	void	Wrap(AudioFileID fileID, bool forWriting);
				// use this to wrap an AudioFileID opened externally

	// ---

	void	Close();
				// In case you want to close the file before the destructor executes
	
	// --- Data formats ---

	// Allow specifying the file's channel layout. Must be called before SetClientFormat.
	// When writing, the specified channel layout is written to the file (if the file format supports
	// the channel layout). When reading, the specified layout overrides the one read from the file,
	// if any.
	void	SetFileChannelLayout(const CAAudioChannelLayout &layout);
	
	// This specifies the data format which the client will use for reading/writing the file,
	// which may be different from the file's format. An AudioConverter is created if necessary.
	// The client format must be linear PCM.
	void	SetClientFormat(const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout=NULL);
	void	SetClientDataFormat(const CAStreamBasicDescription &dataFormat) { SetClientFormat(dataFormat, NULL); }
	void	SetClientChannelLayout(const CAAudioChannelLayout &layout) { SetClientFormat(mClientDataFormat, &layout); }
	
	// Wrapping the underlying converter, if there is one
	OSStatus	SetConverterProperty(AudioConverterPropertyID	inPropertyID,
									UInt32						inPropertyDataSize,
									const void *				inPropertyData,
									bool						inCanFail = false);
	void		SetConverterConfig(CFArrayRef config) {
					SetConverterProperty(kAudioConverterPropertySettings, sizeof(config), &config); }
	CFArrayRef  GetConverterConfig();
	
	// --- I/O ---
	// All I/O is sequential, but you can seek to an arbitrary position when reading.
	// SeekToPacket and TellPacket's packet numbers are in the file's data format, not the client's.
	// However, ReadPackets/WritePackets use packet counts in the client data format.

	void	Read(UInt32 &ioNumFrames, AudioBufferList *ioData);
	void	Write(UInt32 numFrames, const AudioBufferList *data);

	// These can fail for files without a constant mFramesPerPacket
	void	Seek(SInt64 frameNumber);
	SInt64  Tell() const;	// frameNumber
	
	// --- Accessors ---
	// note: client parameters only valid if SetClientFormat has been called
	AudioFileID						GetAudioFileID() const { return mAudioFile; }
	const CAStreamBasicDescription &GetFileDataFormat() const { return mFileDataFormat; }
	const CAStreamBasicDescription &GetClientDataFormat() const { return mClientDataFormat; }
	const CAAudioChannelLayout &	GetFileChannelLayout() const { return mFileChannelLayout; }
	const CAAudioChannelLayout &	GetClientChannelLayout() const { return mClientChannelLayout; }
	AudioConverterRef				GetConverter() const { return mConverter; }

	UInt32	GetFileMaxPacketSize() const { return mFileMaxPacketSize; }
	UInt32	GetClientMaxPacketSize() const { return mClientMaxPacketSize; }
	SInt64	GetNumberPackets() const {
		SInt64 npackets;
		UInt32 propertySize = sizeof(npackets);
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyAudioDataPacketCount, &propertySize, &npackets), "get audio file's packet count");
		return npackets;
	}
	SInt64  GetNumberFrames() const;
				// will be 0 if the file's frames/packet is 0 (variable)
	void	SetNumberFrames(SInt64 length);	// should only be set on a PCM file
	
	// --- Tunable performance parameters ---
	void	SetUseCache(bool b) { mUseCache = b; }
	void	SetIOBufferSizeBytes(UInt32 bufferSizeBytes) { mIOBufferSizeBytes = bufferSizeBytes; }
	UInt32  GetIOBufferSizeBytes() { return mIOBufferSizeBytes; }
	void *  GetIOBuffer() { return mIOBufferList.mBuffers[0].mData; }
	void	SetIOBuffer(void *buf);
	
	// -- Profiling ---
#if CAAUDIOFILE_PROFILE
	void	EnableProfiling(bool b) { mProfiling = b; }
	UInt64	TicksInConverter() const { return (mTicksInConverter > 0) ? (mTicksInConverter - mTicksInReadInConverter) : 0; }
	UInt64	TicksInIO() const { return mTicksInIO; }
#endif
	
// _______________________________________________________________________________________
private:
	SInt64  FileDataOffset();
	void	SeekToPacket(SInt64 packetNumber);
	SInt64	TellPacket() const { return mPacketMark; }  // will be imprecise if SeekToFrame was called
	
	void	SetConverterChannelLayout(bool output, const CAAudioChannelLayout &layout);
	void	WritePacketsFromCallback(
									AudioConverterComplexInputDataProc	inInputDataProc,
									void *								inInputDataProcUserData);
				// will use I/O buffer size
	void	InitFileMaxPacketSize();
	void	FileFormatChanged(const FSRef *parentDir=0, CFStringRef filename=0, AudioFileTypeID filetype=0);

	void	GetExistingFileInfo();
	void	FlushEncoder();
	void	CloseConverter();
	void	UpdateClientMaxPacketSize();
	void	AllocateBuffers(bool okToFail=false);
	SInt64  PacketToFrame(SInt64 packet) const;
	SInt64	FrameToPacket(SInt64 inFrame) const;

	static OSStatus ReadInputProc(		AudioConverterRef				inAudioConverter,
										UInt32*							ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription**	outDataPacketDescription,
										void*							inUserData);	

	static OSStatus WriteInputProc(		AudioConverterRef				inAudioConverter,
										UInt32*							ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription**	outDataPacketDescription,
										void*							inUserData);	
// _______________________________________________________________________________________
private:

	// the file
	FSRef						mFSRef;
	AudioFileID					mAudioFile;
	bool						mOwnOpenFile;
	bool						mUseCache;
	bool						mFinishingEncoding;
	enum { kClosed, kReading, kPreparingToCreate, kPreparingToWrite, kWriting } mMode;
	
//	SInt64						mNumberPackets;		// in file's format
	SInt64						mFileDataOffset;
	SInt64						mPacketMark;		// in file's format
	SInt64						mFrameMark;			// this may be offset from the start of the file
													// by the codec's latency; i.e. our frame 0 could
													// lie at frame 2112 of a decoded AAC file
	SInt32						mFrame0Offset;
	UInt32						mFramesToSkipFollowingSeek;
	
	// buffers
	UInt32						mIOBufferSizeBytes;
	UInt32						mIOBufferSizePackets;
	AudioBufferList				mIOBufferList;		// only one buffer -- USE ACCESSOR so it can be lazily initialized
	bool						mClientOwnsIOBuffer;
	AudioStreamPacketDescription *mPacketDescs;
	UInt32						mNumPacketDescs;
	
	// formats/conversion
	AudioConverterRef			mConverter;
	CAStreamBasicDescription	mFileDataFormat;
	CAStreamBasicDescription	mClientDataFormat;
	CAAudioChannelLayout		mFileChannelLayout;
	CAAudioChannelLayout		mClientChannelLayout;
	UInt32						mFileMaxPacketSize;
	UInt32						mClientMaxPacketSize;
	
	// cookie
	Byte *						mMagicCookie;
	UInt32						mMagicCookieSize;
	
	// for ReadPackets
	UInt32						mMaxPacketsToRead;
	
	// for WritePackets
	UInt32						mWritePackets;
	CABufferList *				mWriteBufferList;
	
#if CAAUDIOFILE_PROFILE
	// performance
	bool						mProfiling;
	UInt64						mTicksInConverter;
	UInt64						mTicksInReadInConverter;
	UInt64						mTicksInIO;
	bool						mInConverter;
#endif

#endif // CAAF_USE_EXTAUDIOFILE
};

#endif // __CAAudioFile_h__
