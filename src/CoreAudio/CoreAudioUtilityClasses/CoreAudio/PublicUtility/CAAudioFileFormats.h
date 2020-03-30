/*
     File: CAAudioFileFormats.h
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
#ifndef __CAAudioFileFormats_h__
#define __CAAudioFileFormats_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <AudioToolbox.h>
#endif
#include "CAStreamBasicDescription.h"

class CAAudioFileFormats {
public:
	enum { noErr = 0 };

	struct DataFormatInfo {
		DataFormatInfo() : mVariants(NULL) { }
		~DataFormatInfo() { delete[] mVariants; }
		
		UInt32							mFormatID;
		int								mNumVariants;
		AudioStreamBasicDescription *	mVariants;
		bool							mReadable;
		bool							mWritable;
		bool							mEitherEndianPCM;

#if DEBUG
		void	DebugPrint();
#endif
	};
	
	struct FileFormatInfo {
		FileFormatInfo() : mFileTypeName(NULL), mExtensions(NULL), mDataFormats(NULL) { }
		~FileFormatInfo() {
			delete[] mDataFormats;
			if (mFileTypeName)
				CFRelease(mFileTypeName);
			if (mExtensions)
				CFRelease(mExtensions);
		}
		
		AudioFileTypeID					mFileTypeID;
		CFStringRef						mFileTypeName;
		CFArrayRef						mExtensions;
		int								mNumDataFormats;
		DataFormatInfo *				mDataFormats;		// NULL until loaded!
		
		int		NumberOfExtensions() { return static_cast<int>(mExtensions ? CFArrayGetCount(mExtensions) : 0); }
		char *	GetExtension(int index, char *buf, int buflen) {
					CFStringRef cfext = (CFStringRef)CFArrayGetValueAtIndex(mExtensions, index);
					CFStringGetCString(cfext, buf, buflen, kCFStringEncodingUTF8);
					return buf;
				}
		bool	MatchExtension(CFStringRef testExt) {	// testExt should not include "."
					CFIndex n = NumberOfExtensions();
					for (CFIndex i = 0; i < n; ++i) {
						CFStringRef ext = (CFStringRef)CFArrayGetValueAtIndex(mExtensions, i);
						if (CFStringCompare(ext, testExt, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
							return true;
					}
					return false;
				}
		bool	AnyWritableFormats();
		void	LoadDataFormats();
		
#if DEBUG
		void	DebugPrint();
#endif
	};
	
private:	// use Instance()
	CAAudioFileFormats(bool loadDataFormats);
	~CAAudioFileFormats();
public:
	
	bool	InferDataFormatFromFileFormat(AudioFileTypeID filetype, CAStreamBasicDescription &fmt);
	
	bool	InferFileFormatFromFilename(const char *filename, AudioFileTypeID &filetype);

	bool	InferFileFormatFromFilename(CFStringRef filename, AudioFileTypeID &filetype);

	bool	InferFileFormatFromDataFormat(const CAStreamBasicDescription &fmt, AudioFileTypeID &filetype);

	bool	IsKnownDataFormat(UInt32 dataFormat);
	
#if DEBUG
	void	DebugPrint();
#endif

	int					mNumFileFormats;
	FileFormatInfo	*	mFileFormats;
	
	FileFormatInfo *	FindFileFormat(UInt32 formatID);

	static CAAudioFileFormats *	Instance(bool loadDataFormats=true);

private:	
	static CAAudioFileFormats *	sInstance;
};

char *	OSTypeToStr(char *buf, size_t bufsize, UInt32 t);
int		StrToOSType(const char *str, UInt32 &t);

#endif // __CAAudioFileFormats_h__
