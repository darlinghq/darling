/*
     File: AudioFileFormat.h
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
#ifndef _AudioFileFormat_H_
#define _AudioFileFormat_H_

#include "AudioFileObject.h"

// AudioFileFormat is a factory class for AudioFileObjects. 

// UncertainResult is needed for DataIsThisFormat. 
// In the case of SoundDesigner 2 we can't determine if the data is SoundDesigner 2 or not.

typedef int UncertainResult;

enum {
	kFalse = 0,
	kTrue = 1,
	kCantDetermine = -1
};

class AudioFileHandle;
class AudioFileFormat;
class AudioFileFormatComponent;

class AudioFileFormatBase
{
public:
	AudioFileFormatBase(UInt32 inFileType);
	virtual ~AudioFileFormatBase();
	
	// return true if extension is of this format type
	virtual Boolean ExtensionIsThisFormat(CFStringRef inExtension) = 0;
	
	virtual UncertainResult FileDataIsThisFormat(
				UInt32								/*inDataByteSize*/,
				const void*							/*inData*/) = 0;
	
	// support SoundDesigner II files while minimizing opening and closing files.
	virtual Boolean ResourceIsThisFormat(const FSRef* /*inRef*/) { return false; }

	UInt32 GetFileType() const { return mFileType; }
	
	virtual UInt32 CanRead() const { return 1; }
	virtual UInt32 CanWrite() const { return 1; }
	virtual UInt32 HasResourceFork() const { return 0; }
	
	virtual void GetExtensions(CFArrayRef *outArray) = 0;
	virtual void GetUTIs(CFArrayRef *outArray) {}
	virtual void GetMIMETypes(CFArrayRef *outArray) {}
	virtual void GetFileTypeName(CFStringRef *outName) = 0;
	virtual OSStatus GetAvailableFormatIDs(UInt32* ioDataSize, void* outPropertyData) = 0;
	virtual OSStatus GetAvailableStreamDescriptions(UInt32 inFormatID, UInt32* ioDataSize, void* outPropertyData) = 0;
	virtual OSStatus GetHFSCodes(UInt32* ioDataSize, void* outPropertyData);
	
	virtual AudioFileFormat* AsAudioFileFormat() { return NULL; }
	virtual AudioFileFormatComponent* AsAudioFileFormatComponent() { return NULL; }
		
private:
	 UInt32 mFileType;
};

class AudioFileStreamObject;

class AudioFileFormat : public AudioFileFormatBase
{
public:
	AudioFileFormat(UInt32 inFileType);
	virtual ~AudioFileFormat();
	
	// create an AudioFileObject for this format type.
	virtual AudioFileObject* New() = 0; 
	virtual AudioFileStreamObject* NewStream() { return NULL; }
	
	// return true if file is of this format type
	virtual UncertainResult FileDataIsThisFormat(UInt32 inDataByteSize, const void* inData) = 0;				

	virtual AudioFileFormat* AsAudioFileFormat() { return this; }
};


#endif
