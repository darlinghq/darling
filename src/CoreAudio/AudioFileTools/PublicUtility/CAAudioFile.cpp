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
	CAAudioFile.cpp
	
=============================================================================*/

#include "CAAudioFile.h"

#if !CAAF_USE_EXTAUDIOFILE

#include "CAXException.h"
#include <algorithm>
#include "CAHostTimeBase.h"
#include "CADebugMacros.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <AudioFormat.h>
#endif

#if DEBUG
	//#define VERBOSE_IO 1
	//#define VERBOSE_CONVERTER 1
	//#define VERBOSE_CHANNELMAP 1
	//#define LOG_FUNCTION_ENTRIES 1

	#if VERBOSE_CHANNELMAP
		#include "CAChannelLayouts.h"	// this is in Source/Tests/AudioFileTools/Utility
	#endif
#endif

#if LOG_FUNCTION_ENTRIES
	class FunctionLogger {
	public:
		FunctionLogger(const char *name, const char *fmt=NULL, ...) : mName(name) {
			Indent();
			printf("-> %s ", name);
			if (fmt) {
				va_list args;
				va_start(args, fmt);
				vprintf(fmt, args);
				va_end(args);
			}
			printf("\n");
			++sIndent;
		}
		~FunctionLogger() {
			--sIndent;
			Indent();
			printf("<- %s\n", mName);
			if (sIndent == 0)
				printf("\n");
		}
		
		static void	Indent() {
			for (int i = sIndent; --i >= 0; ) {
				putchar(' '); putchar(' ');
			}
		}
		
		const char *mName;
		static int sIndent;
	};
	int FunctionLogger::sIndent = 0;

	#define LOG_FUNCTION(name, format, ...) FunctionLogger _flog(name, format, ## __VA_ARGS__);
#else
	#define LOG_FUNCTION(name, format, foo)
#endif

static const UInt32 kDefaultIOBufferSizeBytes = 0x10000;

#if CAAUDIOFILE_PROFILE
	#define StartTiming(af, starttime) UInt64 starttime = af->mProfiling ? CAHostTimeBase::GetTheCurrentTime() : 0
	#define ElapsedTime(af, starttime, counter) if (af->mProfiling) counter += (CAHostTimeBase::GetTheCurrentTime() - starttime)
#else
	#define StartTiming(af, starttime)
	#define ElapsedTime(af, starttime, counter)
#endif

#define kNoMoreInputRightNow 'nein'

// _______________________________________________________________________________________
//
CAAudioFile::CAAudioFile() :
	mAudioFile(0),
	mUseCache(false),
	mFinishingEncoding(false),
	mMode(kClosed),
	mFileDataOffset(-1),
	mFramesToSkipFollowingSeek(0),
	
	mClientOwnsIOBuffer(false),
	mPacketDescs(NULL),
	mNumPacketDescs(0),
	mConverter(NULL),
	mMagicCookie(NULL),
	mWriteBufferList(NULL)
#if CAAUDIOFILE_PROFILE
    ,
	mProfiling(false),
	mTicksInConverter(0),
	mTicksInReadInConverter(0),
	mTicksInIO(0),
	mInConverter(false)
#endif
{
	mIOBufferList.mBuffers[0].mData = NULL;
	mIOBufferList.mBuffers[0].mDataByteSize = 0;
	mClientMaxPacketSize = 0;
	mIOBufferSizeBytes = kDefaultIOBufferSizeBytes;
}

// _______________________________________________________________________________________
//
CAAudioFile::~CAAudioFile()
{
	Close();
}

// _______________________________________________________________________________________
//
void	CAAudioFile::Close()
{
	LOG_FUNCTION("CAAudioFile::Close", NULL, NULL);
	if (mMode == kClosed)
		return;
	if (mMode == kWriting)
		FlushEncoder();
	CloseConverter();
	if (mAudioFile != 0 && mOwnOpenFile) {
		AudioFileClose(mAudioFile);
		mAudioFile = 0;
	}
	if (!mClientOwnsIOBuffer) {
		delete[] (Byte *)mIOBufferList.mBuffers[0].mData;
		mIOBufferList.mBuffers[0].mData = NULL;
		mIOBufferList.mBuffers[0].mDataByteSize = 0;
	}
	delete[] mPacketDescs;	mPacketDescs = NULL;	mNumPacketDescs = 0;
	delete[] mMagicCookie;	mMagicCookie = NULL;
	delete mWriteBufferList;	mWriteBufferList = NULL;
	mMode = kClosed;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::CloseConverter()
{
	if (mConverter) {
#if VERBOSE_CONVERTER
		printf("CAAudioFile %p : CloseConverter\n", this);
#endif
		AudioConverterDispose(mConverter);
		mConverter = NULL;
	}
}

// =======================================================================================

// _______________________________________________________________________________________
//
void	CAAudioFile::Open(const FSRef &fsref)
{
	LOG_FUNCTION("CAAudioFile::Open", "%p", this);
	XThrowIf(mMode != kClosed, kExtAudioFileError_InvalidOperationOrder, "file already open");
	mFSRef = fsref;
	XThrowIfError(AudioFileOpen(&mFSRef, fsRdPerm, 0, &mAudioFile), "open audio file");
	mOwnOpenFile = true;
	mMode = kReading;
	GetExistingFileInfo();
}

// _______________________________________________________________________________________
//
void	CAAudioFile::Wrap(AudioFileID fileID, bool forWriting)
{
	LOG_FUNCTION("CAAudioFile::Wrap", "%p", this);
	XThrowIf(mMode != kClosed, kExtAudioFileError_InvalidOperationOrder, "file already open");

	mAudioFile = fileID;
	mOwnOpenFile = false;
	mMode = forWriting ? kPreparingToWrite : kReading;
	GetExistingFileInfo();
	if (forWriting)
		FileFormatChanged();
}

// _______________________________________________________________________________________
//
void	CAAudioFile::CreateNew(const FSRef &parentDir, CFStringRef filename, AudioFileTypeID filetype, const AudioStreamBasicDescription &dataFormat, const AudioChannelLayout *layout)
{
	LOG_FUNCTION("CAAudioFile::CreateNew", "%p", this);
	XThrowIf(mMode != kClosed, kExtAudioFileError_InvalidOperationOrder, "file already open");
	
	mFileDataFormat = dataFormat;
	if (layout) {
		mFileChannelLayout = layout;
#if VERBOSE_CHANNELMAP
		printf("PrepareNew passed channel layout: %s\n", CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
	}
	mMode = kPreparingToCreate;
	FileFormatChanged(&parentDir, filename, filetype);
}

// _______________________________________________________________________________________
//
// called to create the file -- or update its format/channel layout/properties based on an encoder 
// setting change
void	CAAudioFile::FileFormatChanged(const FSRef *parentDir, CFStringRef filename, AudioFileTypeID filetype)
{
	LOG_FUNCTION("CAAudioFile::FileFormatChanged", "%p", this);
	XThrowIf(mMode != kPreparingToCreate && mMode != kPreparingToWrite, kExtAudioFileError_InvalidOperationOrder, "new file not prepared");
	
	UInt32 propertySize;
	OSStatus err;
	AudioStreamBasicDescription saveFileDataFormat = mFileDataFormat;
	
#if VERBOSE_CONVERTER
	mFileDataFormat.PrintFormat(stdout, "", "Specified file data format");
#endif
	
	// Find out the actual format the converter will produce. This is necessary in
	// case the bitrate has forced a lower sample rate, which needs to be set correctly
	// in the stream description passed to AudioFileCreate.
	if (mConverter != NULL) {
		propertySize = sizeof(AudioStreamBasicDescription);
		Float64 origSampleRate = mFileDataFormat.mSampleRate;
		XThrowIfError(AudioConverterGetProperty(mConverter, kAudioConverterCurrentOutputStreamDescription, &propertySize, &mFileDataFormat), "get audio converter's output stream description");
		// do the same for the channel layout being output by the converter
#if VERBOSE_CONVERTER
		mFileDataFormat.PrintFormat(stdout, "", "Converter output");
#endif
		if (fiszero(mFileDataFormat.mSampleRate))
			mFileDataFormat.mSampleRate = origSampleRate;
		err = AudioConverterGetPropertyInfo(mConverter, kAudioConverterOutputChannelLayout, &propertySize, NULL);
		if (err == noErr && propertySize > 0) {
			AudioChannelLayout *layout = static_cast<AudioChannelLayout *>(malloc(propertySize));
			err = AudioConverterGetProperty(mConverter, kAudioConverterOutputChannelLayout, &propertySize, layout);
			if (err) {
				free(layout);
				XThrow(err, "couldn't get audio converter's output channel layout");
			}
			mFileChannelLayout = layout;
#if VERBOSE_CHANNELMAP
			printf("got new file's channel layout from converter: %s\n", CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
			free(layout);
		}
	}
	
	// create the output file
	if (mMode == kPreparingToCreate) {
		CAStreamBasicDescription newFileDataFormat = mFileDataFormat;
		if (fiszero(newFileDataFormat.mSampleRate))
			newFileDataFormat.mSampleRate = 44100;	// just make something up for now
#if VERBOSE_CONVERTER
		newFileDataFormat.PrintFormat(stdout, "", "Applied to new file");
#endif
		XThrowIfError(AudioFileCreate(parentDir, filename, filetype, &newFileDataFormat, 0, &mFSRef, &mAudioFile), "create audio file");
		mMode = kPreparingToWrite;
		mOwnOpenFile = true;
	} else if (saveFileDataFormat != mFileDataFormat || fnotequal(saveFileDataFormat.mSampleRate, mFileDataFormat.mSampleRate)) {
		// second check must be explicit since operator== on ASBD treats SR of zero as "don't care"
		if (fiszero(mFileDataFormat.mSampleRate))
			mFileDataFormat.mSampleRate = mClientDataFormat.mSampleRate;
#if VERBOSE_CONVERTER
		mFileDataFormat.PrintFormat(stdout, "", "Applied to new file");
#endif
		XThrowIf(fiszero(mFileDataFormat.mSampleRate), kExtAudioFileError_InvalidDataFormat, "file's sample rate is 0");
		XThrowIfError(AudioFileSetProperty(mAudioFile, kAudioFilePropertyDataFormat, sizeof(AudioStreamBasicDescription), &mFileDataFormat), "couldn't update file's data format");
	}

	UInt32 deferSizeUpdates = 1;
	err = AudioFileSetProperty(mAudioFile, kAudioFilePropertyDeferSizeUpdates, sizeof(UInt32), &deferSizeUpdates);

	if (mConverter != NULL) {
		// encoder
		// get the magic cookie, if any, from the converter		
		delete[] mMagicCookie;	mMagicCookie = NULL;
		mMagicCookieSize = 0;

		err = AudioConverterGetPropertyInfo(mConverter, kAudioConverterCompressionMagicCookie, &propertySize, NULL);
		
		// we can get a noErr result and also a propertySize == 0
		// -- if the file format does support magic cookies, but this file doesn't have one.
		if (err == noErr && propertySize > 0) {
			mMagicCookie = new Byte[propertySize];
			XThrowIfError(AudioConverterGetProperty(mConverter, kAudioConverterCompressionMagicCookie, &propertySize, mMagicCookie), "get audio converter's magic cookie");
			mMagicCookieSize = propertySize;	// the converter lies and tell us the wrong size
			// now set the magic cookie on the output file
			UInt32 willEatTheCookie = false;
			// the converter wants to give us one; will the file take it?
			err = AudioFileGetPropertyInfo(mAudioFile, kAudioFilePropertyMagicCookieData,
					NULL, &willEatTheCookie);
			if (err == noErr && willEatTheCookie) {
#if VERBOSE_CONVERTER
				printf("Setting cookie on encoded file\n");
#endif
				XThrowIfError(AudioFileSetProperty(mAudioFile, kAudioFilePropertyMagicCookieData, mMagicCookieSize, mMagicCookie), "set audio file's magic cookie");
			}
		}
		
		// get maximum packet size
		propertySize = sizeof(UInt32);
		XThrowIfError(AudioConverterGetProperty(mConverter, kAudioConverterPropertyMaximumOutputPacketSize, &propertySize, &mFileMaxPacketSize), "get audio converter's maximum output packet size");

		AllocateBuffers(true /* okToFail */);
	} else {
		InitFileMaxPacketSize();
	}
	
	if (mFileChannelLayout.IsValid() && mFileChannelLayout.NumberChannels() > 2) {
		// don't bother tagging mono/stereo files
		UInt32 isWritable;
		err = AudioFileGetPropertyInfo(mAudioFile, kAudioFilePropertyChannelLayout, NULL, &isWritable);
		if (!err && isWritable) {
#if VERBOSE_CHANNELMAP
			printf("writing file's channel layout: %s\n", CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
			err = AudioFileSetProperty(mAudioFile, kAudioFilePropertyChannelLayout, 
				mFileChannelLayout.Size(), &mFileChannelLayout.Layout());
			if (err)
				CAXException::Warning("could not set the file's channel layout", err);
		} else {
#if VERBOSE_CHANNELMAP
			printf("file won't accept a channel layout (write)\n");
#endif
		}
	}
	
	UpdateClientMaxPacketSize();	// also sets mFrame0Offset
	mPacketMark = 0;
	mFrameMark = 0;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::InitFileMaxPacketSize()
{
	LOG_FUNCTION("CAAudioFile::InitFileMaxPacketSize", "%p", this);
	UInt32 propertySize = sizeof(UInt32);
	OSStatus err = AudioFileGetProperty(mAudioFile, kAudioFilePropertyMaximumPacketSize, 
		&propertySize, &mFileMaxPacketSize);
	if (err) {
		// workaround for 3361377: not all file formats' maximum packet sizes are supported
		if (!mFileDataFormat.IsPCM())
			XThrowIfError(err, "get audio file's maximum packet size");
		mFileMaxPacketSize = mFileDataFormat.mBytesPerFrame;
	}
	AllocateBuffers(true /* okToFail */);
}


// _______________________________________________________________________________________
//
SInt64  CAAudioFile::FileDataOffset()
{
	if (mFileDataOffset < 0) {
		UInt32 propertySize = sizeof(SInt64);
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyDataOffset, &propertySize, &mFileDataOffset), "couldn't get file's data offset");
	}
	return mFileDataOffset;
}

// _______________________________________________________________________________________
//
SInt64  CAAudioFile::GetNumberFrames() const
{
	AudioFilePacketTableInfo pti;
	UInt32 propertySize = sizeof(pti);
	OSStatus err = AudioFileGetProperty(mAudioFile, kAudioFilePropertyPacketTableInfo, &propertySize, &pti);
	if (err == noErr)
		return pti.mNumberValidFrames;
	return mFileDataFormat.mFramesPerPacket * GetNumberPackets() - mFrame0Offset;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::SetNumberFrames(SInt64 nFrames)
{
	XThrowIf(mFileDataFormat.mFramesPerPacket != 1, kExtAudioFileError_InvalidDataFormat, "SetNumberFrames only supported for PCM");
	XThrowIfError(AudioFileSetProperty(mAudioFile, kAudioFilePropertyAudioDataPacketCount, sizeof(SInt64), &nFrames), "Couldn't set number of packets on audio file");
}

// _______________________________________________________________________________________
//
// call for existing file, NOT new one - from Open() or Wrap()
void	CAAudioFile::GetExistingFileInfo()
{
	LOG_FUNCTION("CAAudioFile::GetExistingFileInfo", "%p", this);
	UInt32 propertySize;
	OSStatus err;
	
	// get mFileDataFormat
	propertySize = sizeof(AudioStreamBasicDescription);
	XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyDataFormat, &propertySize, &mFileDataFormat), "get audio file's data format");
	
	// get mFileChannelLayout
	err = AudioFileGetPropertyInfo(mAudioFile, kAudioFilePropertyChannelLayout, &propertySize, NULL);
	if (err == noErr && propertySize > 0) {
		AudioChannelLayout *layout = static_cast<AudioChannelLayout *>(malloc(propertySize));
		err = AudioFileGetProperty(mAudioFile, kAudioFilePropertyChannelLayout, &propertySize, layout);
		if (err == noErr) {
			mFileChannelLayout = layout;
#if VERBOSE_CHANNELMAP
			printf("existing file's channel layout: %s\n", CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
		}
		free(layout);
		XThrowIfError(err, "get audio file's channel layout");
	}
	if (mMode != kReading)
		return;
	
#if 0
	// get mNumberPackets
	propertySize = sizeof(mNumberPackets);
	XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyAudioDataPacketCount, &propertySize, &mNumberPackets), "get audio file's packet count");
#if VERBOSE_IO
	printf("CAAudioFile::GetExistingFileInfo: %qd packets\n", mNumberPackets);
#endif
#endif
	
	// get mMagicCookie
	err = AudioFileGetPropertyInfo(mAudioFile, kAudioFilePropertyMagicCookieData, &propertySize, NULL);
	if (err == noErr && propertySize > 0) {
		mMagicCookie = new Byte[propertySize];
		mMagicCookieSize = propertySize;
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyMagicCookieData, &propertySize, mMagicCookie), "get audio file's magic cookie");
	}
	InitFileMaxPacketSize();
	mPacketMark = 0;
	mFrameMark = 0;
	
	UpdateClientMaxPacketSize();
}

// =======================================================================================

// _______________________________________________________________________________________
//
void	CAAudioFile::SetFileChannelLayout(const CAAudioChannelLayout &layout)
{
	LOG_FUNCTION("CAAudioFile::SetFileChannelLayout", "%p", this);
	mFileChannelLayout = layout;
#if VERBOSE_CHANNELMAP
	printf("file channel layout set explicitly (%s): %s\n", mMode == kReading ? "read" : "write", CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
	if (mMode != kReading)
		FileFormatChanged();
}

// _______________________________________________________________________________________
//
void	CAAudioFile::SetClientFormat(const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout)
{
	LOG_FUNCTION("CAAudioFile::SetClientFormat", "%p", this);
	XThrowIf(!dataFormat.IsPCM(), kExtAudioFileError_NonPCMClientFormat, "non-PCM client format on audio file");
	
	bool dataFormatChanging = (mClientDataFormat.mFormatID == 0 || mClientDataFormat != dataFormat);
	
	if (dataFormatChanging) {
		CloseConverter();
		if (mWriteBufferList) {
			delete mWriteBufferList;
			mWriteBufferList = NULL;
		}
		mClientDataFormat = dataFormat;
	}
	
	if (layout && layout->IsValid()) {
		XThrowIf(layout->NumberChannels() != mClientDataFormat.NumberChannels(), kExtAudioFileError_InvalidChannelMap, "inappropriate channel map");
		mClientChannelLayout = *layout;
	}
	
	bool differentLayouts;
	if (mClientChannelLayout.IsValid()) {
		if (mFileChannelLayout.IsValid()) {
			differentLayouts = mClientChannelLayout.Tag() != mFileChannelLayout.Tag();
#if VERBOSE_CHANNELMAP
			printf("two valid layouts, %s\n", differentLayouts ? "different" : "same");
#endif
		} else {
			differentLayouts = false;
#if VERBOSE_CHANNELMAP
			printf("valid client layout, unknown file layout\n");
#endif
		}
	} else {
		differentLayouts = false;
#if VERBOSE_CHANNELMAP
		if (mFileChannelLayout.IsValid())
			printf("valid file layout, unknown client layout\n");
		else
			printf("two invalid layouts\n");
#endif
	}
	
	if (mClientDataFormat != mFileDataFormat || differentLayouts) {
		// We need an AudioConverter.
		if (mMode == kReading) {
			// file -> client (decode)
//mFileDataFormat.PrintFormat(  stdout, "", "File:   ");
//mClientDataFormat.PrintFormat(stdout, "", "Client: ");

			if (mConverter == NULL)
				XThrowIfError(AudioConverterNew(&mFileDataFormat, &mClientDataFormat, &mConverter),
				"create audio converter");
			
#if VERBOSE_CONVERTER
			printf("CAAudioFile %p -- created converter\n", this);
			CAShow(mConverter);
#endif
			// set the magic cookie, if any (for decode)
			if (mMagicCookie)
				SetConverterProperty(kAudioConverterDecompressionMagicCookie, mMagicCookieSize, mMagicCookie, mFileDataFormat.IsPCM());
					// we get cookies from some AIFF's but the converter barfs on them,
					// so we set canFail to true for PCM

			SetConverterChannelLayout(false, mFileChannelLayout);
			SetConverterChannelLayout(true, mClientChannelLayout);
			
			// propagate leading/trailing frame counts
			if (mFileDataFormat.mBitsPerChannel == 0) {
				UInt32 propertySize;
				OSStatus err;
				AudioFilePacketTableInfo pti;
				propertySize = sizeof(pti);
				err = AudioFileGetProperty(mAudioFile, kAudioFilePropertyPacketTableInfo, &propertySize, &pti);
				if (err == noErr && (pti.mPrimingFrames > 0 || pti.mRemainderFrames > 0)) {
					AudioConverterPrimeInfo primeInfo;
					primeInfo.leadingFrames = pti.mPrimingFrames;
					primeInfo.trailingFrames = pti.mRemainderFrames;
					/* ignore any error. better to play it at all than not. */
					/*err = */AudioConverterSetProperty(mConverter, kAudioConverterPrimeInfo, sizeof(primeInfo), &primeInfo);
					//XThrowIfError(err, "couldn't set prime info on converter");
				}
			}
		} else if (mMode == kPreparingToCreate || mMode == kPreparingToWrite) {
			// client -> file (encode)
			if (mConverter == NULL)
				XThrowIfError(AudioConverterNew(&mClientDataFormat, &mFileDataFormat, &mConverter), "create audio converter");
			mWriteBufferList = CABufferList::New("", mClientDataFormat);
			SetConverterChannelLayout(false, mClientChannelLayout);
			SetConverterChannelLayout(true, mFileChannelLayout);
			if (mMode == kPreparingToWrite)
				FileFormatChanged();
		} else
			XThrowIfError(kExtAudioFileError_InvalidOperationOrder, "audio file format not yet known");
	}
	UpdateClientMaxPacketSize();
}

// _______________________________________________________________________________________
//
OSStatus	CAAudioFile::SetConverterProperty(	
											AudioConverterPropertyID	inPropertyID,
											UInt32						inPropertyDataSize,
											const void*					inPropertyData,
											bool						inCanFail)
{
	OSStatus err = noErr;
	//LOG_FUNCTION("ExtAudioFile::SetConverterProperty", "%p %-4.4s", this, (char *)&inPropertyID);
	if (inPropertyID == kAudioConverterPropertySettings && *(CFPropertyListRef *)inPropertyData == NULL)
		;
	else {
		err = AudioConverterSetProperty(mConverter, inPropertyID, inPropertyDataSize, inPropertyData);
		if (!inCanFail) {
			XThrowIfError(err, "set audio converter property");
		}
	}
	UpdateClientMaxPacketSize();
	if (mMode == kPreparingToWrite)
		FileFormatChanged();
	return err;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::SetConverterChannelLayout(bool output, const CAAudioChannelLayout &layout)
{
	LOG_FUNCTION("CAAudioFile::SetConverterChannelLayout", "%p", this);
	OSStatus err;
	
	if (layout.IsValid()) {
#if VERBOSE_CHANNELMAP
		printf("Setting converter's %s channel layout: %s\n", output ? "output" : "input",
			CAChannelLayouts::ConstantToString(mFileChannelLayout.Tag()));
#endif
		if (output) {
			err = AudioConverterSetProperty(mConverter, kAudioConverterOutputChannelLayout,
				layout.Size(), &layout.Layout());
			XThrowIf(err && err != kAudioConverterErr_OperationNotSupported, err, "couldn't set converter's output channel layout");
		} else {
			err = AudioConverterSetProperty(mConverter, kAudioConverterInputChannelLayout,
				layout.Size(), &layout.Layout());
			XThrowIf(err && err != kAudioConverterErr_OperationNotSupported, err, "couldn't set converter's input channel layout");
		}
		if (mMode == kPreparingToWrite)
			FileFormatChanged();
	}
}

// _______________________________________________________________________________________
//
CFArrayRef  CAAudioFile::GetConverterConfig()
{
	CFArrayRef plist;
	UInt32 propertySize = sizeof(plist);
	XThrowIfError(AudioConverterGetProperty(mConverter, kAudioConverterPropertySettings, &propertySize, &plist), "get converter property settings");
	return plist;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::UpdateClientMaxPacketSize()
{
	LOG_FUNCTION("CAAudioFile::UpdateClientMaxPacketSize", "%p", this);
	mFrame0Offset = 0;
	if (mConverter != NULL) {
		AudioConverterPropertyID property = (mMode == kReading) ? 
			kAudioConverterPropertyMaximumOutputPacketSize :
			kAudioConverterPropertyMaximumInputPacketSize;
			
		UInt32 propertySize = sizeof(UInt32);
		XThrowIfError(AudioConverterGetProperty(mConverter, property, &propertySize, &mClientMaxPacketSize),
			"get audio converter's maximum packet size");
		
		if (mFileDataFormat.mBitsPerChannel == 0) {
			AudioConverterPrimeInfo primeInfo;
			propertySize = sizeof(primeInfo);
			OSStatus err = AudioConverterGetProperty(mConverter, kAudioConverterPrimeInfo, &propertySize, &primeInfo);
			if (err == noErr)
				mFrame0Offset = primeInfo.leadingFrames;
#if VERBOSE_CONVERTER
			printf("kAudioConverterPrimeInfo: err = %ld, leadingFrames = %ld\n", err, mFrame0Offset);
#endif
		}
	} else {
		mClientMaxPacketSize = mFileMaxPacketSize;
	}
}

// _______________________________________________________________________________________
//	Allocates: mIOBufferList, mIOBufferSizePackets, mPacketDescs
//	Dependent on: mFileMaxPacketSize, mIOBufferSizeBytes
void	CAAudioFile::AllocateBuffers(bool okToFail)
{
	LOG_FUNCTION("CAAudioFile::AllocateBuffers", "%p", this);
	if (mFileMaxPacketSize == 0) {
		if (okToFail)
			return;
		XThrowIf(true, kExtAudioFileError_MaxPacketSizeUnknown, "file's maximum packet size is 0");
	}
	UInt32 bufferSizeBytes = mIOBufferSizeBytes = std::max(mIOBufferSizeBytes, mFileMaxPacketSize);
		// must be big enough for at least one maximum size packet
	
	if (mIOBufferList.mBuffers[0].mDataByteSize != bufferSizeBytes) {
		mIOBufferList.mNumberBuffers = 1;
		mIOBufferList.mBuffers[0].mNumberChannels = mFileDataFormat.mChannelsPerFrame;
		if (!mClientOwnsIOBuffer) {
			//printf("reallocating I/O buffer\n");
			delete[] (Byte *)mIOBufferList.mBuffers[0].mData;
			mIOBufferList.mBuffers[0].mData = new Byte[bufferSizeBytes];
		}
		mIOBufferList.mBuffers[0].mDataByteSize = bufferSizeBytes;
		mIOBufferSizePackets = bufferSizeBytes / mFileMaxPacketSize;
	}
	
	UInt32 propertySize = sizeof(UInt32);
	UInt32 externallyFramed;
	XThrowIfError(AudioFormatGetProperty(kAudioFormatProperty_FormatIsExternallyFramed,
			sizeof(AudioStreamBasicDescription), &mFileDataFormat, &propertySize, &externallyFramed),
			"is format externally framed");
	if (mNumPacketDescs != (externallyFramed ? mIOBufferSizePackets : 0)) {
		delete[] mPacketDescs;
		mPacketDescs = NULL;
		mNumPacketDescs = 0;

		if (externallyFramed) {
			//printf("reallocating packet descs\n");
			mPacketDescs = new AudioStreamPacketDescription[mIOBufferSizePackets];
			mNumPacketDescs = mIOBufferSizePackets;
		}
	}
}

// _______________________________________________________________________________________
//
void	CAAudioFile::SetIOBuffer(void *buf)
{
	if (!mClientOwnsIOBuffer)
		delete[] (Byte *)mIOBufferList.mBuffers[0].mData;
	mIOBufferList.mBuffers[0].mData = buf;

	if (buf == NULL) {
		mClientOwnsIOBuffer = false;
		SetIOBufferSizeBytes(mIOBufferSizeBytes);
	} else {
		mClientOwnsIOBuffer = true;
		AllocateBuffers();
	}
//	printf("CAAudioFile::SetIOBuffer %p: %p, 0x%lx bytes, mClientOwns = %d\n", this, mIOBufferList.mBuffers[0].mData, mIOBufferSizeBytes, mClientOwnsIOBuffer);
}

// ===============================================================================

/*
For Tiger:
added kAudioFilePropertyPacketToFrame and kAudioFilePropertyFrameToPacket.
You pass in an AudioFramePacketTranslation struct, with the appropriate field filled in, to AudioFileGetProperty.

	kAudioFilePropertyPacketToFrame			=	'pkfr',
		// pass a AudioFramePacketTranslation with mPacket filled out and get mFrame back. mFrameOffsetInPacket is ignored.
	kAudioFilePropertyFrameToPacket			=	'frpk',
		// pass a AudioFramePacketTranslation with mFrame filled out and get mPacket and mFrameOffsetInPacket back.

struct AudioFramePacketTranslation
{
	SInt64 mFrame;
	SInt64 mPacket;
	UInt32 mFrameOffsetInPacket;
};
*/

SInt64  CAAudioFile::PacketToFrame(SInt64 packet) const
{
	AudioFramePacketTranslation trans;
	UInt32 propertySize;
	
	switch (mFileDataFormat.mFramesPerPacket) {
	case 1:
		return packet;
	case 0:
		trans.mPacket = packet;
		propertySize = sizeof(trans);
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyPacketToFrame, &propertySize, &trans),
			"packet <-> frame translation unimplemented for format with variable frames/packet");
		return trans.mFrame;
	}
	return packet * mFileDataFormat.mFramesPerPacket;
}

SInt64	CAAudioFile::FrameToPacket(SInt64 inFrame) const
{
	AudioFramePacketTranslation trans;
	UInt32 propertySize;
	
	switch (mFileDataFormat.mFramesPerPacket) {
	case 1:
		return inFrame;
	case 0:
		trans.mFrame = inFrame;
		propertySize = sizeof(trans);
		XThrowIfError(AudioFileGetProperty(mAudioFile, kAudioFilePropertyFrameToPacket, &propertySize, &trans),
			"packet <-> frame translation unimplemented for format with variable frames/packet");
		return trans.mPacket;
	}
	return inFrame / mFileDataFormat.mFramesPerPacket;
}

// _______________________________________________________________________________________
//

SInt64  CAAudioFile::Tell() const	// frameNumber
{
	return mFrameMark - mFrame0Offset;
}

void	CAAudioFile::SeekToPacket(SInt64 packetNumber)
{
#if VERBOSE_IO
	printf("CAAudioFile::SeekToPacket: %qd\n", packetNumber);
#endif
	XThrowIf(mMode != kReading || packetNumber < 0 /*|| packetNumber >= mNumberPackets*/ , kExtAudioFileError_InvalidSeek, "seek to packet in audio file");
	if (mPacketMark == packetNumber)
		return; // already there! don't reset converter
	mPacketMark = packetNumber;
	
	mFrameMark = PacketToFrame(packetNumber) - mFrame0Offset;
	mFramesToSkipFollowingSeek = 0;
	if (mConverter)
		// must reset -- if we reached end of stream. converter will no longer work otherwise
		AudioConverterReset(mConverter);
}

/*
	Example: AAC, 1024 frames/packet, 2112 frame offset
	
                                           2112
                                             |
    Absolute frames:  0       1024      2048 |    3072
                      +---------+---------+--|------+---------+---------+
    Packets:          |    0    |    1    |  | 2    |    3    |    4    |
                      +---------+---------+--|------+---------+---------+
    Client frames:  -2112   -1088       -64  |     960						SeekToFrame, TellFrame
                                             |
                                             0

	*   Offset between absolute and client frames is mFrame0Offset.
	*** mFrameMark is in client frames ***
	
	Examples:
		clientFrame					0		960		1000	1024
		absoluteFrame				2112	3072	3112	3136
		packet						0		0		0		1
		tempFrameMark*				-2112	-2112	-2112	-1088
		mFramesToSkipFollowingSeek	2112	3072	3112	2112
*/
void	CAAudioFile::Seek(SInt64 clientFrame)
{
	if (clientFrame == mFrameMark)
		return; // already there! don't reset converter

	//SInt64 absoluteFrame = clientFrame + mFrame0Offset;
	XThrowIf(mMode != kReading || clientFrame < 0 || !mClientDataFormat.IsPCM(), kExtAudioFileError_InvalidSeek, "seek to frame in audio file");

#if VERBOSE_IO
	SInt64 prevFrameMark = mFrameMark;
#endif
	
	SInt64 packet;
	packet = FrameToPacket(clientFrame);
	if (packet < 0)
		packet = 0;
	SeekToPacket(packet);
	// this will have backed up mFrameMark to match the beginning of the packet
	mFramesToSkipFollowingSeek = std::max(UInt32(clientFrame - mFrameMark), UInt32(0));
	mFrameMark = clientFrame;
	
#if VERBOSE_IO
	printf("CAAudioFile::SeekToFrame: frame %qd (from %qd), packet %qd, skip %ld frames\n", mFrameMark, prevFrameMark, packet, mFramesToSkipFollowingSeek);
#endif
}

// _______________________________________________________________________________________
//
void	CAAudioFile::Read(UInt32 &ioNumPackets, AudioBufferList *ioData)
			// May read fewer packets than requested if:
			//		buffer is not big enough
			//		file does not contain that many more packets
			// Note that eofErr is not fatal, just results in 0 packets returned
			// ioData's buffer sizes may be shortened
{
	XThrowIf(mClientMaxPacketSize == 0, kExtAudioFileError_MaxPacketSizeUnknown, "client maximum packet size is 0");
	if (mIOBufferList.mBuffers[0].mData == NULL) {
#if DEBUG
		printf("warning: CAAudioFile::AllocateBuffers called from ReadPackets\n");
#endif
		AllocateBuffers();
	}
	UInt32 bufferSizeBytes = ioData->mBuffers[0].mDataByteSize;
	UInt32 maxNumPackets = bufferSizeBytes / mClientMaxPacketSize;	
	// older versions of AudioConverterFillComplexBuffer don't do this, so do our own sanity check
	UInt32 nPackets = std::min(ioNumPackets, maxNumPackets);
	
	mMaxPacketsToRead = ~0UL;
	
	if (mClientDataFormat.mFramesPerPacket == 1) {  // PCM or equivalent
		while (mFramesToSkipFollowingSeek > 0) {
			UInt32 skipFrames = std::min(mFramesToSkipFollowingSeek, maxNumPackets);
			UInt32 framesPerPacket;
			if ((framesPerPacket=mFileDataFormat.mFramesPerPacket) > 0)
				mMaxPacketsToRead = (skipFrames + framesPerPacket - 1) / framesPerPacket;

			if (mConverter == NULL) {
				XThrowIfError(ReadInputProc(NULL, &skipFrames, ioData, NULL, this), "read audio file");
			} else {
#if CAAUDIOFILE_PROFILE
				mInConverter = true;
#endif
				StartTiming(this, fill);
				XThrowIfError(AudioConverterFillComplexBuffer(mConverter, ReadInputProc, this, &skipFrames, ioData, NULL), "convert audio packets (pcm read)");
				ElapsedTime(this, fill, mTicksInConverter);
#if CAAUDIOFILE_PROFILE
				mInConverter = false;
#endif
			}
			if (skipFrames == 0) {	// hit EOF
				ioNumPackets = 0;
				return;
			}
			mFrameMark += skipFrames;
#if VERBOSE_IO
			printf("CAAudioFile::ReadPackets: skipped %ld frames\n", skipFrames);
#endif

			mFramesToSkipFollowingSeek -= skipFrames;

			// restore mDataByteSize
			for (int i = ioData->mNumberBuffers; --i >= 0 ; )
				ioData->mBuffers[i].mDataByteSize = bufferSizeBytes;
		}
	}
	
	if (mFileDataFormat.mFramesPerPacket > 0)
		// don't read more packets than we are being asked to produce
		mMaxPacketsToRead = nPackets / mFileDataFormat.mFramesPerPacket + 1;
	if (mConverter == NULL) {
		XThrowIfError(ReadInputProc(NULL, &nPackets, ioData, NULL, this), "read audio file");
	} else {
#if CAAUDIOFILE_PROFILE
		mInConverter = true;
#endif
		StartTiming(this, fill);
		XThrowIfError(AudioConverterFillComplexBuffer(mConverter, ReadInputProc, this, &nPackets, ioData, NULL), "convert audio packets (read)");
		ElapsedTime(this, fill, mTicksInConverter);
#if CAAUDIOFILE_PROFILE
		mInConverter = false;
#endif
	}
	if (mClientDataFormat.mFramesPerPacket == 1)
		mFrameMark += nPackets;
	
	ioNumPackets = nPackets;
}

// _______________________________________________________________________________________
//
OSStatus CAAudioFile::ReadInputProc(	AudioConverterRef				inAudioConverter,
										UInt32*							ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription**	outDataPacketDescription,
										void*							inUserData)
{
	CAAudioFile *This = static_cast<CAAudioFile *>(inUserData);

#if 0
	SInt64 remainingPacketsInFile = This->mNumberPackets - This->mPacketMark;
	if (remainingPacketsInFile <= 0) {
		*ioNumberDataPackets = 0;
		ioData->mBuffers[0].mDataByteSize = 0;
		if (outDataPacketDescription)
			*outDataPacketDescription = This->mPacketDescs;
#if VERBOSE_IO
		printf("CAAudioFile::ReadInputProc: EOF\n");
#endif
		return noErr;	// not eofErr; EOF is signified by 0 packets/0 bytes
	}
#endif
	
	// determine how much to read
	AudioBufferList *readBuffer;
	UInt32 readPackets;
	if (inAudioConverter != NULL) {
		// getting called from converter, need to use our I/O buffer
		readBuffer = &This->mIOBufferList;
		readPackets = This->mIOBufferSizePackets;
	} else {
		// getting called directly from ReadPackets, use supplied buffer
		if (This->mFileMaxPacketSize == 0)
			return kExtAudioFileError_MaxPacketSizeUnknown;
		readBuffer = ioData;
		readPackets = std::min(*ioNumberDataPackets, readBuffer->mBuffers[0].mDataByteSize / This->mFileMaxPacketSize);
			// don't attempt to read more packets than will fit in the buffer
	}
	// don't try to read past EOF
//	if (readPackets > remainingPacketsInFile)
//		readPackets = remainingPacketsInFile;
	// don't read more packets than necessary to produce the requested amount of converted data
	if (readPackets > This->mMaxPacketsToRead) {
#if VERBOSE_IO
		printf("CAAudioFile::ReadInputProc: limiting read to %ld packets (from %ld)\n", This->mMaxPacketsToRead, readPackets);
#endif
		readPackets = This->mMaxPacketsToRead;
	}
	
	// read
	UInt32 bytesRead;
	OSStatus err;
	
	StartTiming(This, read);
	StartTiming(This, readinconv);
	err = AudioFileReadPackets(This->mAudioFile, This->mUseCache, &bytesRead, This->mPacketDescs, This->mPacketMark, &readPackets, readBuffer->mBuffers[0].mData);
#if CAAUDIOFILE_PROFILE
	if (This->mInConverter) ElapsedTime(This, readinconv, This->mTicksInReadInConverter);
#endif
	ElapsedTime(This, read, This->mTicksInIO);

	if (err) {
		DebugMessageN1("Error %ld from AudioFileReadPackets!!!\n", err);
		return err;
	}
	
#if VERBOSE_IO
	printf("CAAudioFile::ReadInputProc: read %ld packets (%qd-%qd), %ld bytes, err %ld\n", readPackets, This->mPacketMark, This->mPacketMark + readPackets, bytesRead, err);
#if VERBOSE_IO >= 2
	if (This->mPacketDescs) {
		for (UInt32 i = 0; i < readPackets; ++i) {
			printf("  read packet %qd : offset %qd, length %ld\n", This->mPacketMark + i, This->mPacketDescs[i].mStartOffset, This->mPacketDescs[i].mDataByteSize);
		}
	}
	printf("  read buffer:"); CAShowAudioBufferList(readBuffer, 0, 4);
#endif
#endif
	if (readPackets == 0) {
		*ioNumberDataPackets = 0;
		ioData->mBuffers[0].mDataByteSize = 0;
		return noErr;
	}

	if (outDataPacketDescription)
		*outDataPacketDescription = This->mPacketDescs;
	ioData->mBuffers[0].mDataByteSize = bytesRead;
	ioData->mBuffers[0].mData = readBuffer->mBuffers[0].mData;

	This->mPacketMark += readPackets;
	if (This->mClientDataFormat.mFramesPerPacket != 1) {	// for PCM client formats we update in Read
		// but for non-PCM client format (weird case) we must update here/now
		if (This->mFileDataFormat.mFramesPerPacket > 0)
			This->mFrameMark += readPackets * This->mFileDataFormat.mFramesPerPacket;
		else {
			for (UInt32 i = 0; i < readPackets; ++i)
				This->mFrameMark += This->mPacketDescs[i].mVariableFramesInPacket;
		}
	}
	*ioNumberDataPackets = readPackets;
	return noErr;
}

// _______________________________________________________________________________________
//
void	CAAudioFile::Write(UInt32 numPackets, const AudioBufferList *data)
{
	if (mIOBufferList.mBuffers[0].mData == NULL) {
#if DEBUG
		printf("warning: CAAudioFile::AllocateBuffers called from WritePackets\n");
#endif
		AllocateBuffers();
	}

	if (mMode == kPreparingToWrite)
		mMode = kWriting;
	else
		XThrowIf(mMode != kWriting, kExtAudioFileError_InvalidOperationOrder, "can't write to this file");
	if (mConverter != NULL) {
		mWritePackets = numPackets;
		mWriteBufferList->SetFrom(data);
		WritePacketsFromCallback(WriteInputProc, this);
	} else {
		StartTiming(this, write);
		XThrowIfError(AudioFileWritePackets(mAudioFile, mUseCache, data->mBuffers[0].mDataByteSize, 
						NULL, mPacketMark, &numPackets, data->mBuffers[0].mData),
						"write audio file");
		ElapsedTime(this, write, mTicksInIO);
#if VERBOSE_IO
		printf("CAAudioFile::WritePackets: wrote %ld packets at %qd, %ld bytes\n", numPackets, mPacketMark, data->mBuffers[0].mDataByteSize);
#endif
		//mNumberPackets = 
		mPacketMark += numPackets;
		if (mFileDataFormat.mFramesPerPacket > 0)
			mFrameMark += numPackets * mFileDataFormat.mFramesPerPacket;
		// else: shouldn't happen since we're only called when there's no converter
	}
}

// _______________________________________________________________________________________
//
void	CAAudioFile::FlushEncoder()
{
	if (mConverter != NULL) {
		mFinishingEncoding = true;
		WritePacketsFromCallback(WriteInputProc, this);
		mFinishingEncoding = false;

		// get priming info from converter, set it on the file
		if (mFileDataFormat.mBitsPerChannel == 0) {
			UInt32 propertySize;
			OSStatus err;
			AudioConverterPrimeInfo primeInfo;
			propertySize = sizeof(primeInfo);
	
			err = AudioConverterGetProperty(mConverter, kAudioConverterPrimeInfo, &propertySize, &primeInfo);
			if (err == noErr) {
				AudioFilePacketTableInfo pti;
				propertySize = sizeof(pti);
				err = AudioFileGetProperty(mAudioFile, kAudioFilePropertyPacketTableInfo, &propertySize, &pti);
				if (err == noErr) {
//printf("old packet table info: %qd valid, %ld priming, %ld remainder\n", pti.mNumberValidFrames, pti.mPrimingFrames, pti.mRemainderFrames);
					UInt64 totalFrames = pti.mNumberValidFrames + pti.mPrimingFrames + pti.mRemainderFrames;
					pti.mPrimingFrames = primeInfo.leadingFrames;
					pti.mRemainderFrames = primeInfo.trailingFrames;
					pti.mNumberValidFrames = totalFrames - pti.mPrimingFrames - pti.mRemainderFrames;
//printf("new packet table info: %qd valid, %ld priming, %ld remainder\n", pti.mNumberValidFrames, pti.mPrimingFrames, pti.mRemainderFrames);
					XThrowIfError(AudioFileSetProperty(mAudioFile, kAudioFilePropertyPacketTableInfo, sizeof(pti), &pti), "couldn't set packet table info on audio file");
				}
			}
		}
	}
}

// _______________________________________________________________________________________
//
OSStatus CAAudioFile::WriteInputProc(	AudioConverterRef				/*inAudioConverter*/,
										UInt32 *						ioNumberDataPackets,
										AudioBufferList*				ioData,
										AudioStreamPacketDescription **	outDataPacketDescription,
										void*							inUserData)
{
	CAAudioFile *This = static_cast<CAAudioFile *>(inUserData);
	if (This->mFinishingEncoding) {
		*ioNumberDataPackets = 0;
		ioData->mBuffers[0].mDataByteSize = 0;
		ioData->mBuffers[0].mData = NULL;
		if (outDataPacketDescription)
			*outDataPacketDescription = NULL;
		return noErr;
	}
	UInt32 numPackets = This->mWritePackets;
	if (numPackets == 0) {
		return kNoMoreInputRightNow;
	}
	This->mWriteBufferList->ToAudioBufferList(ioData);
	This->mWriteBufferList->BytesConsumed(numPackets * This->mClientDataFormat.mBytesPerFrame);
	*ioNumberDataPackets = numPackets;
	if (outDataPacketDescription)
		*outDataPacketDescription = NULL;
	This->mWritePackets -= numPackets;
	return noErr;
}

// _______________________________________________________________________________________
//
#if VERBOSE_IO
static void	hexdump(const void *addr, long len)
{
	const Byte *p = (Byte *)addr;
	UInt32 offset = 0;
	
	if (len > 0x400) len = 0x400;
	
	while (len > 0) {
		int n = len > 16 ? 16 : len;
		printf("%08lX:  ", offset);
		for (int i = 0; i < 16; ++i)
			if (i < n)
				printf("%02X ", p[i]);
			else printf("   ");
		for (int i = 0; i < 16; ++i)
			if (i < n)
				putchar(p[i] >= ' ' && p[i] < 127 ? p[i] : '.');
			else putchar(' ');
		putchar('\n');
		p += 16;
		len -= 16;
		offset += 16;
	}
}
#endif

// _______________________________________________________________________________________
//
void	CAAudioFile::WritePacketsFromCallback(
								AudioConverterComplexInputDataProc	inInputDataProc,
								void *								inInputDataProcUserData)
{
	while (true) {
		// keep writing until we exhaust the input (temporary stop), or produce no output (EOF)
		UInt32 numEncodedPackets = mIOBufferSizePackets;
		mIOBufferList.mBuffers[0].mDataByteSize = mIOBufferSizeBytes;
#if CAAUDIOFILE_PROFILE
		mInConverter = true;
#endif
		StartTiming(this, fill);
		OSStatus err = AudioConverterFillComplexBuffer(mConverter, inInputDataProc, inInputDataProcUserData, 
					&numEncodedPackets, &mIOBufferList, mPacketDescs);
		ElapsedTime(this, fill, mTicksInConverter);
#if CAAUDIOFILE_PROFILE
		mInConverter = false;
#endif
		XThrowIf(err != 0 && err != kNoMoreInputRightNow, err, "convert audio packets (write)");
		if (numEncodedPackets == 0)
			break;
		Byte *buf = (Byte *)mIOBufferList.mBuffers[0].mData;
#if VERBOSE_IO
		printf("CAAudioFile::WritePacketsFromCallback: wrote %ld packets, %ld bytes\n", numEncodedPackets, mIOBufferList.mBuffers[0].mDataByteSize);
		if (mPacketDescs) {
			for (UInt32 i = 0; i < numEncodedPackets; ++i) {
				printf("  write packet %qd : offset %qd, length %ld\n", mPacketMark + i, mPacketDescs[i].mStartOffset, mPacketDescs[i].mDataByteSize);
#if VERBOSE_IO >= 2
				hexdump(buf + mPacketDescs[i].mStartOffset, mPacketDescs[i].mDataByteSize);
#endif
			}
		}
#endif
		StartTiming(this, write);
		XThrowIfError(AudioFileWritePackets(mAudioFile, mUseCache, mIOBufferList.mBuffers[0].mDataByteSize, mPacketDescs, mPacketMark, &numEncodedPackets, buf), "write audio file");
		ElapsedTime(this, write, mTicksInIO);
		mPacketMark += numEncodedPackets;
		//mNumberPackets += numEncodedPackets;
		if (mFileDataFormat.mFramesPerPacket > 0)
			mFrameMark += numEncodedPackets * mFileDataFormat.mFramesPerPacket;
		else {
			for (UInt32 i = 0; i < numEncodedPackets; ++i)
				mFrameMark += mPacketDescs[i].mVariableFramesInPacket;
		}
		if (err == kNoMoreInputRightNow)
			break;
	}
}

#endif // !CAAF_USE_EXTAUDIOFILE
