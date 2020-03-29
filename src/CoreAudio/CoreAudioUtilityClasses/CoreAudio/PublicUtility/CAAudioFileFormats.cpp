/*
     File: CAAudioFileFormats.cpp
 Abstract: CAAudioFileFormats.h
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
#include "CAAudioFileFormats.h"
#include <algorithm>
#include <ctype.h>

CAAudioFileFormats *CAAudioFileFormats::sInstance = NULL;

CAAudioFileFormats *CAAudioFileFormats::Instance(bool loadDataFormats)
{
	if (sInstance == NULL)
		sInstance = new CAAudioFileFormats(loadDataFormats);
	return sInstance;
}

/*
class CompareFileFormatNames {
public:
	bool	operator() (const CAAudioFileFormats::FileFormatInfo &a, const CAAudioFileFormats::FileFormatInfo &b)
	{
		return CFStringCompare(a.mFileTypeName, b.mFileTypeName, 
			kCFCompareCaseInsensitive | kCFCompareLocalized) == kCFCompareLessThan;
	}
};*/

static int CompareFileFormatNames(const void *va, const void *vb)
{
	CAAudioFileFormats::FileFormatInfo  *a = (CAAudioFileFormats::FileFormatInfo *)va, 
										*b = (CAAudioFileFormats::FileFormatInfo *)vb;
	return CFStringCompare(a->mFileTypeName, b->mFileTypeName, 
		kCFCompareCaseInsensitive | kCFCompareLocalized);
}

CAAudioFileFormats::CAAudioFileFormats(bool loadDataFormats) : 
	mNumFileFormats(0), mFileFormats(NULL)
{
	OSStatus err;
	UInt32 size;
	UInt32 *fileTypes = NULL;
	
	// get all file types
	err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_WritableTypes, 0, NULL, &size);
	if (err != noErr) goto bail;
	mNumFileFormats = size / sizeof(UInt32);
	mFileFormats = new FileFormatInfo[mNumFileFormats];
	fileTypes = new UInt32[mNumFileFormats];
	err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_WritableTypes, 0, NULL, &size, fileTypes);
	if (err != noErr) goto bail;
	
	// get info for each file type
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		OSType filetype = fileTypes[i];

		ffi->mFileTypeID = filetype;
		
		// file type name
		ffi->mFileTypeName = NULL;
		size = sizeof(CFStringRef);
		err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_FileTypeName, sizeof(UInt32), &filetype, &size, &ffi->mFileTypeName);
		if (err == noErr && ffi->mFileTypeName)
			CFRetain(ffi->mFileTypeName);
		
		// file extensions
		size = sizeof(CFArrayRef);
		err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_ExtensionsForType,
			sizeof(OSType), &filetype, &size, &ffi->mExtensions);
		if (err)
			ffi->mExtensions = NULL;
		
		// file data formats
		ffi->mNumDataFormats = 0;
		ffi->mDataFormats = NULL;
		
		if (loadDataFormats)
			ffi->LoadDataFormats();
	}

	// sort file formats by name
	qsort(mFileFormats, mNumFileFormats, sizeof(FileFormatInfo), CompareFileFormatNames);
bail:
	delete[] fileTypes;
}

void	CAAudioFileFormats::FileFormatInfo::LoadDataFormats()
{
	if (mDataFormats != NULL) return;
	
	UInt32 *writableFormats = NULL, *readableFormats = NULL;
	int nWritableFormats, nReadableFormats;
	// get all writable formats
	UInt32 size;
	OSStatus err = AudioFormatGetPropertyInfo(kAudioFormatProperty_EncodeFormatIDs, 0, NULL, &size);
	if (err != noErr) goto bail;
	nWritableFormats = size / sizeof(UInt32);
	writableFormats = new UInt32[nWritableFormats];
	err = AudioFormatGetProperty(kAudioFormatProperty_EncodeFormatIDs, 0, NULL, &size, writableFormats);
	if (err != noErr) goto bail;
	
	// get all readable formats
	err = AudioFormatGetPropertyInfo(kAudioFormatProperty_DecodeFormatIDs, 0, NULL, &size);
	if (err != noErr) goto bail;
	nReadableFormats = size / sizeof(UInt32);
	readableFormats = new UInt32[nReadableFormats];
	err = AudioFormatGetProperty(kAudioFormatProperty_DecodeFormatIDs, 0, NULL, &size, readableFormats);
	if (err != noErr) goto bail;
	
	err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_AvailableFormatIDs, sizeof(UInt32), &mFileTypeID, &size);
	if (err == noErr) {
		mNumDataFormats = size / sizeof(OSType);
		OSType *formatIDs = new OSType[mNumDataFormats];
		err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_AvailableFormatIDs,
			sizeof(UInt32), &mFileTypeID, &size, formatIDs);
		if (err == noErr) {
			mDataFormats = new DataFormatInfo[mNumDataFormats];
			for (int j = 0; j < mNumDataFormats; ++j) {
				int k;
				bool anyBigEndian = false, anyLittleEndian = false;
				DataFormatInfo *dfi = &mDataFormats[j];
				dfi->mFormatID = formatIDs[j];
				dfi->mReadable = (dfi->mFormatID == kAudioFormatLinearPCM);
				dfi->mWritable = (dfi->mFormatID == kAudioFormatLinearPCM);
				for (k = 0; k < nReadableFormats; ++k)
					if (readableFormats[k] == dfi->mFormatID) {
						dfi->mReadable = true;
						break;
					}
				for (k = 0; k < nWritableFormats; ++k)
					if (writableFormats[k] == dfi->mFormatID) {
						dfi->mWritable = true;
						break;
					}
				
				dfi->mNumVariants = 0;
				AudioFileTypeAndFormatID tf = { mFileTypeID, dfi->mFormatID };
				err = AudioFileGetGlobalInfoSize(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat,
					sizeof(AudioFileTypeAndFormatID), &tf, &size);
				if (err == noErr) {
					dfi->mNumVariants = size / sizeof(AudioStreamBasicDescription);
					dfi->mVariants = new AudioStreamBasicDescription[dfi->mNumVariants];
					err = AudioFileGetGlobalInfo(kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat,
						sizeof(AudioFileTypeAndFormatID), &tf, &size, dfi->mVariants);
					if (err) {
						dfi->mNumVariants = 0;
						delete[] dfi->mVariants;
						dfi->mVariants = NULL;
					} else {
						for (k = 0; k < dfi->mNumVariants; ++k) {
							AudioStreamBasicDescription *desc = &dfi->mVariants[k];
							if (desc->mBitsPerChannel > 8) {
								if (desc->mFormatFlags & kAudioFormatFlagIsBigEndian)
									anyBigEndian = true;
								else
									anyLittleEndian = true;
							}
						}
					}
				}
				
				dfi->mEitherEndianPCM = (anyBigEndian && anyLittleEndian);
			}
		}
		delete[] formatIDs;
	}
bail:
	delete[] readableFormats;
	delete[] writableFormats;
}

// note that the outgoing format will have zero for the sample rate, channels per frame, bytesPerPacket, bytesPerFrame
bool	CAAudioFileFormats::InferDataFormatFromFileFormat(AudioFileTypeID filetype, CAStreamBasicDescription &fmt)
{
	// if the file format only supports one data format
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		ffi->LoadDataFormats();
		if (ffi->mFileTypeID == filetype && ffi->mNumDataFormats > 0) {
			DataFormatInfo *dfi = &ffi->mDataFormats[0];
			if (ffi->mNumDataFormats > 1) {
				// file can contain multiple data formats. Take PCM if it's there.
				for (int j = 0; j < ffi->mNumDataFormats; ++j) {
					if (ffi->mDataFormats[j].mFormatID == kAudioFormatLinearPCM) {
						dfi = &ffi->mDataFormats[j];
						break;
					}
				}
			}
			memset(&fmt, 0, sizeof(fmt));
			fmt.mFormatID = dfi->mFormatID;
			if (dfi->mNumVariants > 0) {
				// take the first variant as a default
				fmt = dfi->mVariants[0];
				if (dfi->mNumVariants > 1 && dfi->mFormatID == kAudioFormatLinearPCM) {
					// look for a 16-bit variant as a better default
					for (int j = 0; j < dfi->mNumVariants; ++j) {
						AudioStreamBasicDescription *desc = &dfi->mVariants[j];
						if (desc->mBitsPerChannel == 16) {
							fmt = *desc;
							break;
						}
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool	CAAudioFileFormats::InferFileFormatFromFilename(CFStringRef filename, AudioFileTypeID &filetype)
{
	bool result = false;
	CFRange range = CFStringFind(filename, CFSTR("."), kCFCompareBackwards);
	if (range.location == kCFNotFound) return false;
	range.location += 1;
	range.length = CFStringGetLength(filename) - range.location;
	CFStringRef ext = CFStringCreateWithSubstring(NULL, filename, range);
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		if (ffi->MatchExtension(ext)) {
			filetype = ffi->mFileTypeID;
			result = true;
			break;
		}
	}
	CFRelease(ext);
	return result;
}

bool	CAAudioFileFormats::InferFileFormatFromFilename(const char *filename, AudioFileTypeID &filetype)
{
	if (filename == NULL) return false;
	CFStringRef cfname = CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
	bool result = InferFileFormatFromFilename(cfname, filetype);
	CFRelease(cfname);
	return result;
}

bool	CAAudioFileFormats::InferFileFormatFromDataFormat(const CAStreamBasicDescription &fmt, 
			AudioFileTypeID &filetype)
{
	// if there's exactly one file format that supports this data format
	FileFormatInfo *theFileFormat = NULL;
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		ffi->LoadDataFormats();
		DataFormatInfo *dfi = ffi->mDataFormats, *dfiend = dfi + ffi->mNumDataFormats;
		for ( ; dfi < dfiend; ++dfi)
			if (dfi->mFormatID == fmt.mFormatID) {
				if (theFileFormat != NULL)
					return false;	// ambiguous
				theFileFormat = ffi;	// got a candidate
			}
	}
	if (theFileFormat == NULL)
		return false;
	filetype = theFileFormat->mFileTypeID;
	return true;
}

bool	CAAudioFileFormats::IsKnownDataFormat(OSType dataFormat)
{
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		ffi->LoadDataFormats();
		DataFormatInfo *dfi = ffi->mDataFormats, *dfiend = dfi + ffi->mNumDataFormats;
		for ( ; dfi < dfiend; ++dfi)
			if (dfi->mFormatID == dataFormat)
				return true;
	}
	return false;
}

CAAudioFileFormats::FileFormatInfo *	CAAudioFileFormats::FindFileFormat(UInt32 formatID)
{
	for (int i = 0; i < mNumFileFormats; ++i) {
		FileFormatInfo *ffi = &mFileFormats[i];
		if (ffi->mFileTypeID == formatID)
			return ffi;
	}
	return NULL;
}

bool	CAAudioFileFormats::FileFormatInfo::AnyWritableFormats()
{
	LoadDataFormats();
	DataFormatInfo *dfi = mDataFormats, *dfiend = dfi + mNumDataFormats;
	for ( ; dfi < dfiend; ++dfi)
		if (dfi->mWritable)
			return true;
	return false;
}

char *OSTypeToStr(char *buf, size_t bufsize, OSType t)
{
	if (bufsize > 0) {
		char *p = buf, *pend = buf + bufsize;
		char str[4] = {0}, *q = str;
		*(UInt32 *)str = CFSwapInt32HostToBig(t);
		for (int i = 0; i < 4 && p < pend; ++i) {
			if (isprint(*q) && *q != '\\')
				*p++ = *q++;
			else {
				snprintf(p, pend - p, "\\x%02x", *q++);
				p += 4;
			}
		}
		if (p >= pend) p = pend - 1;
		*p = '\0';
	}
	return buf;
}

int		StrToOSType(const char *str, OSType &t)
{
	char buf[4];
	const char *p = str;
	int x;
	for (int i = 0; i < 4; ++i) {
		if (*p != '\\') {
			if ((buf[i] = *p++) == '\0') {
				// special-case for 'aac ': if we only got three characters, assume the last was a space
				if (i == 3) {
					--p;
					buf[i] = ' ';
					break;
				}
				goto fail;
			}
		} else {
			if (*++p != 'x') goto fail;
			if (sscanf(++p, "%02X", &x) != 1) goto fail;
			buf[i] = x;
			p += 2;
		}
	}
	t = CFSwapInt32BigToHost(*(UInt32 *)buf);
	return static_cast<int>(p - str);
fail:
	return 0;
}

#if DEBUG

void	CAAudioFileFormats::DebugPrint()
{
	for (int i = 0; i < mNumFileFormats; ++i)
		mFileFormats[i].DebugPrint();
}

void	CAAudioFileFormats::FileFormatInfo::DebugPrint()
{
	char ftype[20];
	char ftypename[64];
	CFStringGetCString(mFileTypeName, ftypename, sizeof(ftypename), kCFStringEncodingUTF8);
	printf("File type: '%s' = %s\n  Extensions:", OSTypeToStr(ftype, sizeof(ftype), mFileTypeID), ftypename);
	int i, n = NumberOfExtensions();
	for (i = 0; i < n; ++i) {
		GetExtension(i, ftype, sizeof(ftype));
		printf(" .%s", ftype);
	}
	LoadDataFormats();
	printf("\n  Formats:\n");
	for (i = 0; i < mNumDataFormats; ++i)
		mDataFormats[i].DebugPrint();
}

void	CAAudioFileFormats::DataFormatInfo::DebugPrint()
{
	char buf[20];
	static const char *ny[] = { "not ", "" };
	printf("    '%s': %sreadable %swritable\n", OSTypeToStr(buf, sizeof(buf), mFormatID), ny[mReadable], ny[mWritable]);
	for (int i = 0; i < mNumVariants; ++i) {
		CAStreamBasicDescription desc(mVariants[i]);
		desc.PrintFormat(stdout, "      ", "");
		//printf("        %d bytes/frame\n", desc.mBytesPerFrame);
	}
}
#endif

