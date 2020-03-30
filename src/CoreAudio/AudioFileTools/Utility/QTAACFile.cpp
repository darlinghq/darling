/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	QTAACFile.cpp
	
=============================================================================*/

#include "QTAACFile.h"
#include "CAXException.h"
#include "CAStreamBasicDescription.h"


// _______________________________________________________________________________________
//
QTAACFile::QTAACFile() :
	mMovieResFile(-1),
	mMovie(NULL),
	mPacketHandle(NULL),
	mMagicCookie(NULL),
	mConverter(NULL),
	mFileMaxPacketSize(0)
{
}

// _______________________________________________________________________________________
//
QTAACFile::~QTAACFile()
{
	Close();
	CloseConverter();
	if (mPacketHandle)
		DisposeHandle(mPacketHandle);
}

// _______________________________________________________________________________________
//
void	QTAACFile::Close()
{
	if (mMovieResFile != -1) {
		CloseMovieFile(mMovieResFile);
		mMovieResFile = -1;
	}
	if (mMovie) {
		DisposeMovie(mMovie);
		mMovie = NULL;
	}
	if (mMagicCookie) {
		free(mMagicCookie);
		mMagicCookie = NULL;
	}
}

// _______________________________________________________________________________________
//
void	QTAACFile::CloseConverter()
{
	if (mConverter) {
		AudioConverterDispose(mConverter);
		mConverter = NULL;
	}
}

// _______________________________________________________________________________________
/*
	GetMP4DecoderConfig

	NOTE: This is designed to work only for a very specific case -- an MPEG-4/QuickTime movie
	containing AAC-encoded audio. It is adapted from the "SoundPlayer"QuickTime sample code,
	but with much generality removed.
	
	If the audio is not MPEG-4, then we'll fail immediately with an exception.
	
	If the audio is MPEG-4, but not AAC-encoded, then we will fail somewhere beyond this function, when passing the magic cookie (ESDS) to the AudioConverter.
*/
static void GetMP4DecoderConfig(Media inMedia, AudioStreamBasicDescription &outStreamFormat, char **outEsds, UInt32 *outEsdsSize)
{
	OSStatus err = noErr;
	Size size;
	Handle extension = NULL;
	SoundDescriptionHandle hSoundDescription = (SoundDescriptionHandle)NewHandle(0);
	struct MyMP4AudioAtom {
		UInt32  size;
		OSType  atomType;
		UInt32  version;
	};
	
	try {
		// get the description of the sample data
		GetMediaSampleDescription(inMedia, 1, (SampleDescriptionHandle)hSoundDescription);
		XThrowIfError(GetMoviesError(), "couldn't get media sample description");

		// other formats: exercise for the reader!
		OSType fmt = (*hSoundDescription)->dataFormat;
		XThrowIf(fmt != 'mp4a', -1, "can only decode MPEG-4/AAC");
		
		extension = NewHandle(0);
		
		// get the "magic" decompression atom
		// This extension to the SoundDescription information stores data specific to a given audio decompressor.
		// Some audio decompression algorithms require a set of out-of-stream values to configure the decompressor
		// which are stored in a siDecompressionParams atom. The contents of the siDecompressionParams atom are dependent
		// on the audio decompressor.

		err = GetSoundDescriptionExtension(hSoundDescription, &extension, siDecompressionParams);

		// if it doesn't have an atom, that's ok for some formats, but not for AAC
		XThrowIfError(err, "couldn't get sound description extension");

		size = GetHandleSize(extension);
		HLock(extension);
		UserDataAtom *atom = (UserDataAtom *)*extension;
		bool moreAtoms = true;
		do {
			long atomSize = EndianS32_BtoN(atom->size);
			
			XThrowIf(atomSize < 8, -1, "invalid sound description atom");
			switch (EndianU32_BtoN(atom->atomType)) {
			case 'esds':
				*outEsdsSize = atomSize - sizeof(MyMP4AudioAtom);
				*outEsds = (char *)malloc(*outEsdsSize);
				memcpy(*outEsds, (char *)atom + sizeof(MyMP4AudioAtom), *outEsdsSize);
				break;
			case kAudioTerminatorAtomType:
				moreAtoms = false;
				break;
			}
			atom = (UserDataAtom *)((char *)atom + atomSize);
		} while (moreAtoms);
		
		HUnlock(extension);

		// set up our stream description
		memset(&outStreamFormat, 0, sizeof(AudioStreamBasicDescription));
		outStreamFormat.mFormatID = kAudioFormatMPEG4AAC;
	}
	catch (...) {
		if (extension) DisposeHandle(extension);
		if (hSoundDescription) DisposeHandle((Handle)hSoundDescription);
		throw;
	}
	if (extension) DisposeHandle(extension);
	if (hSoundDescription) DisposeHandle((Handle)hSoundDescription);
}

// _______________________________________________________________________________________
//
void	QTAACFile::Open(const char *filename)
{
	FSRef theFSRef;
	Track theAudioTrack;
	FSSpec theFSSpec;
	
	// convert from POSIX full path to FSSpec
	XThrowIfError(FSPathMakeRef((UInt8 *)filename, &theFSRef, NULL), "locate file");
	XThrowIfError(FSGetCatalogInfo(&theFSRef, 0, NULL, NULL, &theFSSpec, NULL), "get FSSpec");

	// make a QuickTime movie (will open MP4 transparently)
	XThrowIfError(OpenMovieFile(&theFSSpec, &mMovieResFile, fsRdPerm), "OpenMovieFile failed");
	SInt16 nResID = 0;
	Str255          strName;
	Boolean         bWasChanged;
	XThrowIfError(NewMovieFromFile(&mMovie, mMovieResFile, &nResID, strName, newMovieActive, &bWasChanged), "NewMovieFromFile failed");

	// NOTE: we're assuming the data we want is in the movie's first enabled audio track
	theAudioTrack = GetMovieIndTrackType(mMovie, 1, SoundMediaType, movieTrackMediaType | movieTrackEnabledOnly);
	XThrowIf(theAudioTrack == NULL, -1, "movie contains no audio track");
	
	// find the track's media, and interrogate it for the audio format and magic cookie (decoder config)
	mMedia = GetTrackMedia(theAudioTrack);
	XThrowIf(mMedia == NULL, -1, "track contains no media");	
	GetMP4DecoderConfig(mMedia, mFileDataFormat, &mMagicCookie, &mMagicCookieSize);

	// Fill out the remaining fields of the ASBD by evaluating the magic cookie
	UInt32	size = sizeof(mFileDataFormat);
	XThrowIfError(AudioFormatGetProperty(kAudioFormatProperty_ASBDFromESDS, mMagicCookieSize, mMagicCookie, &size, &mFileDataFormat),
		"couldn't get ASBD from the Magic Cookie");

	// build a vector of times at which packets begin
	// so later we can randomly access the file by packet number
	TimeValue currentTime = 0;
	static const Fixed oneFixed = 0x10000;
	while (true) {
		TimeValue interestingTime, interestingDuration;
		GetMediaNextInterestingTime(mMedia, nextTimeMediaSample | nextTimeEdgeOK, currentTime, oneFixed, &interestingTime, &interestingDuration);
		if (interestingTime < 0)
			break;
		mPacketTimes.push_back(interestingTime);
		currentTime = interestingTime + 1;
	}
	mNumberPackets = mPacketTimes.size();
	mPacketMark = 0;
	mPacketHandle = NewHandle(1024);	// memory into which packets will be read
}

// _______________________________________________________________________________________
//
void	QTAACFile::SetClientFormat(const CAStreamBasicDescription &dataFormat)
{
	CloseConverter();

	XThrowIf(!dataFormat.IsPCM(), -1, "non-PCM client format on audio file");
	mClientDataFormat = dataFormat;
	
	if (mClientDataFormat != mFileDataFormat) {
		// We need an AudioConverter.
		// file -> client
		XThrowIfError(AudioConverterNew(&mFileDataFormat, &mClientDataFormat, &mConverter),
			"create audio converter");

		// set the magic cookie, if any (for decode)
		if (mMagicCookie)
			SetConverterProperty(kAudioConverterDecompressionMagicCookie,
				mMagicCookieSize, mMagicCookie, true);
	}
	UpdateClientMaxPacketSize();
}

// _______________________________________________________________________________________
//
OSStatus	QTAACFile::SetConverterProperty(AudioConverterPropertyID	inPropertyID,
											UInt32						inPropertyDataSize,
											const void*					inPropertyData,
											bool						inCanFail)
{
	OSStatus err = AudioConverterSetProperty(mConverter, inPropertyID, inPropertyDataSize, inPropertyData);
	if (!inCanFail) {
		XThrowIfError(err, "set audio converter property");
	}
	UpdateClientMaxPacketSize();
	return err;
}

// _______________________________________________________________________________________
//
void	QTAACFile::UpdateClientMaxPacketSize()
{
	if (mConverter != NULL) {
		AudioConverterPropertyID property = 
			kAudioConverterPropertyMaximumOutputPacketSize;
			
		UInt32 propertySize = sizeof(UInt32);
		XThrowIfError(AudioConverterGetProperty(mConverter, property, &propertySize, &mClientMaxPacketSize),
			"get audio converter's maximum packet size");
	} else
		mClientMaxPacketSize = mFileMaxPacketSize;
	XThrowIf(mClientMaxPacketSize == 0, -1, "client maximum packet size is 0");
}

// _______________________________________________________________________________________
//
//	This function reads the next packet from the movie. It always reads only 1 packet,
//	regardless of how many were asked for -- whether it's called either as a callback from
//	AudioConverterFillComplexBuffer, or directly from ReadPackets (below), the caller
//	will just call again if it wants more.
OSStatus QTAACFile::ReadInputProc(  AudioConverterRef				inAudioConverter,
									UInt32*							ioNumberDataPackets,
									AudioBufferList*				ioData,
									AudioStreamPacketDescription**	outDataPacketDescription,
									void*							inUserData)
{
	QTAACFile *This = static_cast<QTAACFile *>(inUserData);
	
	UInt32 remainingPacketsInFile = This->mNumberPackets - This->mPacketMark;
	if (remainingPacketsInFile == 0) {
		// end of file
		*ioNumberDataPackets = 0;
		ioData->mBuffers[0].mDataByteSize = 0;
		return noErr;	// EOF is signified by 0 packets/0 bytes/noErr
	}
	
	UInt32 ioPackets = 1;

	// don't try to read past EOF
	if (ioPackets > remainingPacketsInFile)
		ioPackets = remainingPacketsInFile;

	long bytesRead;
	OSStatus err;
	
	HUnlock(This->mPacketHandle);
	err = GetMediaSample(This->mMedia,			// specifies the media for this operation
			 This->mPacketHandle,			// function returns the sample data into this handle
			 0,								// maximum number of bytes of sample data to be returned
			 &bytesRead,					// the number of bytes of sample data returned
			 This->mPacketTimes[This->mPacketMark],		
											// starting time of the sample to be retrieved (must be in Media's TimeScale)
			 NULL,							// indicates the actual time of the returned sample data
			 NULL,							// duration of each sample in the media
			 NULL,							// sample description corresponding to the returned sample data (NULL to ignore)
			 NULL,							// index value to the sample description that corresponds to the returned sample data (NULL to ignore)
			 0,								// maximum number of samples to be returned (0 to use a value that is appropriate for the media)
			NULL,							// number of samples it actually returned
			 NULL);							// flags that describe the sample (NULL to ignore)		
	
	if (err)
		return err;

	if (outDataPacketDescription) {
		This->mPacketDescription.mStartOffset = 0;
		This->mPacketDescription.mVariableFramesInPacket = 0;
		This->mPacketDescription.mDataByteSize = bytesRead;
		*outDataPacketDescription = &This->mPacketDescription;
	}
	
	HLock(This->mPacketHandle);
	ioData->mBuffers[0].mDataByteSize = bytesRead;
	ioData->mBuffers[0].mData = *This->mPacketHandle;
	This->mPacketMark += ioPackets;
	*ioNumberDataPackets = ioPackets;
	return noErr;
}

// _______________________________________________________________________________________
//
void	QTAACFile::ReadPackets(UInt32 &ioNumPackets, AudioBufferList *ioData)
{
	// older versions of AudioConverterFillComplexBuffer don't do this, so do our own sanity check
	UInt32 maxNumPackets = ioData->mBuffers[0].mDataByteSize / mClientMaxPacketSize;	
	if (ioNumPackets > maxNumPackets)
		ioNumPackets = maxNumPackets;
	
	if (mConverter == NULL)
		XThrowIfError(ReadInputProc(NULL, &ioNumPackets, ioData, NULL, this), "read audio file");
	else {
		XThrowIfError(AudioConverterFillComplexBuffer(mConverter, ReadInputProc, this, &ioNumPackets, ioData, NULL),
			"qt convert audio packets");
	}
}
