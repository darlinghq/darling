/*
     File: CAStreamBasicDescription.h
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
#ifndef __CAStreamBasicDescription_h__
#define __CAStreamBasicDescription_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include "CoreAudioTypes.h"
	#include "CoreFoundation.h"
#endif

#include "CADebugMacros.h"
#include <string.h>	// for memset, memcpy
#include <stdio.h>	// for FILE *

#pragma mark	This file needs to compile on more earlier versions of the OS, so please keep that in mind when editing it

extern char *CAStringForOSType (OSType t, char *writeLocation, size_t bufsize);

// define Leopard specific symbols for backward compatibility if applicable
#if COREAUDIOTYPES_VERSION < 1050
typedef Float32 AudioSampleType;
enum { kAudioFormatFlagsCanonical = kAudioFormatFlagIsFloat | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked };
#endif
#if COREAUDIOTYPES_VERSION < 1051
typedef Float32 AudioUnitSampleType;
enum {
	kLinearPCMFormatFlagsSampleFractionShift    = 7,
	kLinearPCMFormatFlagsSampleFractionMask     = (0x3F << kLinearPCMFormatFlagsSampleFractionShift),
};
#endif

//	define the IsMixable format flag for all versions of the system
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
	enum { kIsNonMixableFlag = kAudioFormatFlagIsNonMixable };
#else
	enum { kIsNonMixableFlag = (1L << 6) };
#endif

//=============================================================================
//	CAStreamBasicDescription
//
//	This is a wrapper class for the AudioStreamBasicDescription struct.
//	It adds a number of convenience routines, but otherwise adds nothing
//	to the footprint of the original struct.
//=============================================================================
class CAStreamBasicDescription : 
	public AudioStreamBasicDescription
{

//	Constants
public:
	static const AudioStreamBasicDescription	sEmpty;
	
	enum CommonPCMFormat {
		kPCMFormatOther		= 0,
		kPCMFormatFloat32	= 1,
		kPCMFormatInt16		= 2,
		kPCMFormatFixed824	= 3,
		kPCMFormatFloat64	= 4
	};
	
//	Construction/Destruction
public:
	CAStreamBasicDescription();
	
	CAStreamBasicDescription(const AudioStreamBasicDescription &desc);
	
	CAStreamBasicDescription(		double inSampleRate,		UInt32 inFormatID,
									UInt32 inBytesPerPacket,	UInt32 inFramesPerPacket,
									UInt32 inBytesPerFrame,		UInt32 inChannelsPerFrame,
									UInt32 inBitsPerChannel,	UInt32 inFormatFlags);

	CAStreamBasicDescription(	double inSampleRate, UInt32 inNumChannels, CommonPCMFormat pcmf, bool inIsInterleaved) {
		unsigned wordsize;

		mSampleRate = inSampleRate;
		mFormatID = kAudioFormatLinearPCM;
		mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
		mFramesPerPacket = 1;
		mChannelsPerFrame = inNumChannels;
		mBytesPerFrame = mBytesPerPacket = 0;
		mReserved = 0;

		switch (pcmf) {
		default:
			return;
		case kPCMFormatFloat32:
			wordsize = 4;
			mFormatFlags |= kAudioFormatFlagIsFloat;
			break;
		case kPCMFormatFloat64:
			wordsize = 8;
			mFormatFlags |= kAudioFormatFlagIsFloat;
			break;
		case kPCMFormatInt16:
			wordsize = 2;
			mFormatFlags |= kAudioFormatFlagIsSignedInteger;
			break;
		case kPCMFormatFixed824:
			wordsize = 4;
			mFormatFlags |= kAudioFormatFlagIsSignedInteger | (24 << kLinearPCMFormatFlagsSampleFractionShift);
			break;
		}
		mBitsPerChannel = wordsize * 8;
		if (inIsInterleaved)
			mBytesPerFrame = mBytesPerPacket = wordsize * inNumChannels;
		else {
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
			mBytesPerFrame = mBytesPerPacket = wordsize;
		}
	}

//	Assignment
	CAStreamBasicDescription&	operator=(const AudioStreamBasicDescription& v) { SetFrom(v); return *this; }

	void	SetFrom(const AudioStreamBasicDescription &desc)
	{
		memcpy(this, &desc, sizeof(AudioStreamBasicDescription));
	}
	
	bool		FromText(const char *inTextDesc) { return FromText(inTextDesc, *this); }
	static bool	FromText(const char *inTextDesc, AudioStreamBasicDescription &outDesc);
					// return true if parsing was successful
	
	static const char *sTextParsingUsageString;
	
	// _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
	//
	// interrogation
	
	bool	IsPCM() const { return mFormatID == kAudioFormatLinearPCM; }
	
	bool	PackednessIsSignificant() const
	{
		Assert(IsPCM(), "PackednessIsSignificant only applies for PCM");
		return (SampleWordSize() << 3) != mBitsPerChannel;
	}
	
	bool	AlignmentIsSignificant() const
	{
		return PackednessIsSignificant() || (mBitsPerChannel & 7) != 0;
	}
	
	bool	IsInterleaved() const
	{
		return !(mFormatFlags & kAudioFormatFlagIsNonInterleaved);
	}
	
	bool	IsSignedInteger() const
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsSignedInteger);
	}
	
	bool	IsFloat() const
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsFloat);
	}
	
	bool	IsNativeEndian() const
	{
		return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian;
	}
	
	// for sanity with interleaved/deinterleaved possibilities, never access mChannelsPerFrame, use these:
	UInt32	NumberInterleavedChannels() const	{ return IsInterleaved() ? mChannelsPerFrame : 1; }	
	UInt32	NumberChannelStreams() const		{ return IsInterleaved() ? 1 : mChannelsPerFrame; }
	UInt32	NumberChannels() const				{ return mChannelsPerFrame; }
	UInt32	SampleWordSize() const				{ 
			return (mBytesPerFrame > 0 && NumberInterleavedChannels()) ? mBytesPerFrame / NumberInterleavedChannels() :  0;
	}

	UInt32	FramesToBytes(UInt32 nframes) const	{ return nframes * mBytesPerFrame; }
	UInt32	BytesToFrames(UInt32 nbytes) const	{
		Assert(mBytesPerFrame > 0, "bytesPerFrame must be > 0 in BytesToFrames");
		return nbytes / mBytesPerFrame;
	}
	
	bool	SameChannelsAndInterleaving(const CAStreamBasicDescription &a) const
	{
		return this->NumberChannels() == a.NumberChannels() && this->IsInterleaved() == a.IsInterleaved();
	}
	
	bool	IdentifyCommonPCMFormat(CommonPCMFormat &outFormat, bool *outIsInterleaved=NULL) const
	{	// return true if it's a valid PCM format.
	
		outFormat = kPCMFormatOther;
		// trap out patently invalid formats.
		if (mFormatID != kAudioFormatLinearPCM || mFramesPerPacket != 1 || mBytesPerFrame != mBytesPerPacket || mBitsPerChannel/8 > mBytesPerFrame || mChannelsPerFrame == 0)
			return false;
		bool interleaved = (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
		if (outIsInterleaved != NULL) *outIsInterleaved = interleaved;
		unsigned wordsize = mBytesPerFrame;
		if (interleaved) {
			if (wordsize % mChannelsPerFrame != 0) return false;
			wordsize /= mChannelsPerFrame;
		}
		
		if ((mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian
		&& wordsize * 8 == mBitsPerChannel) {
			// packed and native endian, good
			if (mFormatFlags & kLinearPCMFormatFlagIsFloat) {
				// float: reject nonsense bits
				if (mFormatFlags & (kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagsSampleFractionMask))
					return false;
				if (wordsize == 4)
					outFormat = kPCMFormatFloat32;
				if (wordsize == 8)
					outFormat = kPCMFormatFloat64;
			} else if (mFormatFlags & kLinearPCMFormatFlagIsSignedInteger) {
				// signed int
				unsigned fracbits = (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
				if (wordsize == 4 && fracbits == 24)
					outFormat = kPCMFormatFixed824;
				else if (wordsize == 2 && fracbits == 0)
					outFormat = kPCMFormatInt16;
			}
		}
		return true;
	}

	bool IsCommonFloat32(bool *outIsInterleaved=NULL) const {
		CommonPCMFormat fmt;
		return IdentifyCommonPCMFormat(fmt, outIsInterleaved) && fmt == kPCMFormatFloat32;
	}
	bool IsCommonFloat64(bool *outIsInterleaved=NULL) const {
		CommonPCMFormat fmt;
		return IdentifyCommonPCMFormat(fmt, outIsInterleaved) && fmt == kPCMFormatFloat64;
	}
	bool IsCommonFixed824(bool *outIsInterleaved=NULL) const {
		CommonPCMFormat fmt;
		return IdentifyCommonPCMFormat(fmt, outIsInterleaved) && fmt == kPCMFormatFixed824;
	}
	bool IsCommonInt16(bool *outIsInterleaved=NULL) const {
		CommonPCMFormat fmt;
		return IdentifyCommonPCMFormat(fmt, outIsInterleaved) && fmt == kPCMFormatInt16;
	}
	
	// _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
	//
	//	manipulation
	
	void	SetCanonical(UInt32 nChannels, bool interleaved)
				// note: leaves sample rate untouched
	{
		mFormatID = kAudioFormatLinearPCM;
		UInt32 sampleSize = SizeOf32(AudioSampleType);
		mFormatFlags = kAudioFormatFlagsCanonical;
		mBitsPerChannel = 8 * sampleSize;
		mChannelsPerFrame = nChannels;
		mFramesPerPacket = 1;
		if (interleaved)
			mBytesPerPacket = mBytesPerFrame = nChannels * sampleSize;
		else {
			mBytesPerPacket = mBytesPerFrame = sampleSize;
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		}
	}
	
	bool	IsCanonical() const
	{
		if (mFormatID != kAudioFormatLinearPCM) return false;
		UInt32 reqFormatFlags;
		UInt32 flagsMask = (kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagsSampleFractionMask);
		bool interleaved = (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
		unsigned sampleSize = SizeOf32(AudioSampleType);
		reqFormatFlags = kAudioFormatFlagsCanonical;
		UInt32 reqFrameSize = interleaved ? (mChannelsPerFrame * sampleSize) : sampleSize;

		return ((mFormatFlags & flagsMask) == reqFormatFlags
			&& mBitsPerChannel == 8 * sampleSize
			&& mFramesPerPacket == 1
			&& mBytesPerFrame == reqFrameSize
			&& mBytesPerPacket == reqFrameSize);
	}
	
	void	SetAUCanonical(UInt32 nChannels, bool interleaved)
	{
		mFormatID = kAudioFormatLinearPCM;
#if CA_PREFER_FIXED_POINT
		mFormatFlags = kAudioFormatFlagsCanonical | (kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift);
#else
		mFormatFlags = kAudioFormatFlagsCanonical;
#endif
		mChannelsPerFrame = nChannels;
		mFramesPerPacket = 1;
		mBitsPerChannel = 8 * SizeOf32(AudioUnitSampleType);
		if (interleaved)
			mBytesPerPacket = mBytesPerFrame = nChannels * SizeOf32(AudioUnitSampleType);
		else {
			mBytesPerPacket = mBytesPerFrame = SizeOf32(AudioUnitSampleType);
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		}
	}
	
	void	ChangeNumberChannels(UInt32 nChannels, bool interleaved)
				// alter an existing format
	{
		Assert(IsPCM(), "ChangeNumberChannels only works for PCM formats");
		UInt32 wordSize = SampleWordSize();	// get this before changing ANYTHING
		if (wordSize == 0)
			wordSize = (mBitsPerChannel + 7) / 8;
		mChannelsPerFrame = nChannels;
		mFramesPerPacket = 1;
		if (interleaved) {
			mBytesPerPacket = mBytesPerFrame = nChannels * wordSize;
			mFormatFlags &= ~static_cast<UInt32>(kAudioFormatFlagIsNonInterleaved);
		} else {
			mBytesPerPacket = mBytesPerFrame = wordSize;
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		}
	}
	
	// _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
	//
	//	other
	
	bool            IsEqual(const AudioStreamBasicDescription &other, bool interpretingWildcards=true) const;
	static bool     FlagIndependentEquivalence(const AudioStreamBasicDescription &x, const AudioStreamBasicDescription &y);
    static bool     IsFunctionallyEquivalent(const AudioStreamBasicDescription &x, const AudioStreamBasicDescription &y);
    
	void	Print() const {
		Print (stdout);
	}

	void	Print(FILE* file) const {
		PrintFormat (file, "", "AudioStreamBasicDescription:");	
	}

	void	PrintFormat(FILE *f, const char *indent, const char *name) const {
		char buf[256];
		fprintf(f, "%s%s %s\n", indent, name, AsString(buf, sizeof(buf)));
	}
	
	void	PrintFormat2(FILE *f, const char *indent, const char *name) const { // no trailing newline
		char buf[256];
		fprintf(f, "%s%s %s", indent, name, AsString(buf, sizeof(buf)));
	}

	char *	AsString(char *buf, size_t bufsize, bool brief=false) const;

	static void Print (const AudioStreamBasicDescription &inDesc) 
	{ 
		CAStreamBasicDescription desc(inDesc);
		desc.Print ();
	}
	
	OSStatus			Save(CFPropertyListRef *outData) const;
		
	OSStatus			Restore(CFPropertyListRef &inData);

//	Operations
	static bool			IsMixable(const AudioStreamBasicDescription& inDescription) { return (inDescription.mFormatID == kAudioFormatLinearPCM) && ((inDescription.mFormatFlags & kIsNonMixableFlag) == 0); }
	static void			NormalizeLinearPCMFormat(AudioStreamBasicDescription& ioDescription);
	static void			NormalizeLinearPCMFormat(bool inNativeEndian, AudioStreamBasicDescription& ioDescription);
	static void			ResetFormat(AudioStreamBasicDescription& ioDescription);
	static void			FillOutFormat(AudioStreamBasicDescription& ioDescription, const AudioStreamBasicDescription& inTemplateDescription);
	static void			GetSimpleName(const AudioStreamBasicDescription& inDescription, char* outName, UInt32 inMaxNameLength, bool inAbbreviate, bool inIncludeSampleRate = false);
    static void         ModifyFormatFlagsForMatching(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y, UInt32& xFlags, UInt32& yFlags, bool converterOnly);

#if CoreAudio_Debug
	static void			PrintToLog(const AudioStreamBasicDescription& inDesc);
#endif
};

bool		operator<(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y);
bool		operator==(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y);
#if TARGET_OS_MAC || (TARGET_OS_WIN32 && (_MSC_VER > 600))
inline bool	operator!=(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y) { return !(x == y); }
inline bool	operator<=(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y) { return (x < y) || (x == y); }
inline bool	operator>=(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y) { return !(x < y); }
inline bool	operator>(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y) { return !((x < y) || (x == y)); }
#endif

bool SanityCheck(const AudioStreamBasicDescription& x);


#endif // __CAStreamBasicDescription_h__
