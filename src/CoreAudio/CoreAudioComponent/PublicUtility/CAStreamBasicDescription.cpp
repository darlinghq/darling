/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#include "CAStreamBasicDescription.h"
#include "CAMath.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreFoundation/CFByteOrder.h>
#else
	#include <CFByteOrder.h>
#endif

#pragma mark	This file needs to compile on earlier versions of the OS, so please keep that in mind when editing it

char *CAStringForOSType (OSType t, char *writeLocation, size_t bufsize)
{
	if (bufsize > 0) {
		char *p = writeLocation, *pend = writeLocation + bufsize;
		union { UInt32 i; unsigned char str[4]; } u;
		unsigned char *q = u.str;
		u.i = CFSwapInt32HostToBig(t);

		bool hasNonPrint = false;
		for (int i = 0; i < 4; ++i) {
			if (!(isprint(*q) && *q != '\\')) {
				hasNonPrint = true;
				break;
			}
            q++;
		}
        q = u.str;
		
		if (hasNonPrint)
			p += snprintf (p, pend - p, "0x");
		else if (p < pend)
			*p++ = '\'';
			
		for (int i = 0; i < 4 && p < pend; ++i) {
			if (hasNonPrint) {
				p += snprintf(p, pend - p, "%02X", *q++);
			} else {
				*p++ = *q++;
			}
		}
		if (!hasNonPrint && p < pend)
			*p++ = '\'';
		if (p >= pend) p -= 1;
		*p = '\0';
	}
	return writeLocation;
}


const AudioStreamBasicDescription	CAStreamBasicDescription::sEmpty = { 0.0, 0, 0, 0, 0, 0, 0, 0, 0 };

CAStreamBasicDescription::CAStreamBasicDescription() 
{ 
	memset (this, 0, sizeof(AudioStreamBasicDescription)); 
}
	
CAStreamBasicDescription::CAStreamBasicDescription(const AudioStreamBasicDescription &desc)
{
	SetFrom(desc);
}


CAStreamBasicDescription::CAStreamBasicDescription(double inSampleRate,		UInt32 inFormatID,
									UInt32 inBytesPerPacket,	UInt32 inFramesPerPacket,
									UInt32 inBytesPerFrame,		UInt32 inChannelsPerFrame,
									UInt32 inBitsPerChannel,	UInt32 inFormatFlags)
{
	mSampleRate = inSampleRate;
	mFormatID = inFormatID;
	mBytesPerPacket = inBytesPerPacket;
	mFramesPerPacket = inFramesPerPacket;
	mBytesPerFrame = inBytesPerFrame;
	mChannelsPerFrame = inChannelsPerFrame;
	mBitsPerChannel = inBitsPerChannel;
	mFormatFlags = inFormatFlags;
	mReserved = 0;
}

char *CAStreamBasicDescription::AsString(char *buf, size_t _bufsize, bool brief /*=false*/) const
{
	int bufsize = (int)_bufsize;	// must be signed to protect against overflow
	char *theBuffer = buf;
	int nc;
	char formatID[24];
	CAStringForOSType(mFormatID, formatID, sizeof(formatID));
	if (brief) {
		CommonPCMFormat com;
		bool interleaved;
		if (IdentifyCommonPCMFormat(com, &interleaved) && com != kPCMFormatOther) {
			const char *desc;
			switch (com) {
			case kPCMFormatInt16:
				desc = "Int16";
				break;
			case kPCMFormatInt32:
				desc = "Int32";
				break;
			case kPCMFormatFixed824:
				desc = "Int8.24";
				break;
			case kPCMFormatFloat32:
				desc = "Float32";
				break;
			case kPCMFormatFloat64:
				desc = "Float64";
				break;
			default:
				desc = NULL;
				break;
			}
			if (desc) {
				const char *inter ="";
				if (mChannelsPerFrame > 1)
					inter = !interleaved ? ", non-inter" : ", inter";
				snprintf(buf, static_cast<size_t>(bufsize), "%2d ch, %6.0f Hz, %s%s", (int)mChannelsPerFrame, mSampleRate, desc, inter);
				return theBuffer;
			}
		}
		if (mChannelsPerFrame == 0 && mSampleRate == 0.0 && mFormatID == 0) {
			snprintf(buf, static_cast<size_t>(bufsize), "%2d ch, %6.0f Hz", (int)mChannelsPerFrame, mSampleRate);
			return theBuffer;
		}
	}
	
	nc = snprintf(buf, static_cast<size_t>(bufsize), "%2d ch, %6.0f Hz, %s (0x%08X) ", (int)NumberChannels(), mSampleRate, formatID, (int)mFormatFlags);
	buf += nc; if ((bufsize -= nc) <= 0) goto exit;
	if (mFormatID == kAudioFormatLinearPCM) {
		bool isInt = !(mFormatFlags & kLinearPCMFormatFlagIsFloat);
		int wordSize = static_cast<int>(SampleWordSize());
		const char *endian = (wordSize > 1) ? 
			((mFormatFlags & kLinearPCMFormatFlagIsBigEndian) ? " big-endian" : " little-endian" ) : "";
		const char *sign = isInt ? 
			((mFormatFlags & kLinearPCMFormatFlagIsSignedInteger) ? " signed" : " unsigned") : "";
		const char *floatInt = isInt ? "integer" : "float";
		char packed[32];
		if (wordSize > 0 && PackednessIsSignificant()) {
			if (mFormatFlags & kLinearPCMFormatFlagIsPacked)
				snprintf(packed, sizeof(packed), "packed in %d bytes", wordSize);
			else
				snprintf(packed, sizeof(packed), "unpacked in %d bytes", wordSize);
		} else
			packed[0] = '\0';
		const char *align = (wordSize > 0 && AlignmentIsSignificant()) ?
			((mFormatFlags & kLinearPCMFormatFlagIsAlignedHigh) ? " high-aligned" : " low-aligned") : "";
		const char *deinter = (mFormatFlags & kAudioFormatFlagIsNonInterleaved) ? ", deinterleaved" : "";
		const char *commaSpace = (packed[0]!='\0') || (align[0]!='\0') ? ", " : "";
		char bitdepth[20];

		int fracbits = (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
		if (fracbits > 0)
			snprintf(bitdepth, sizeof(bitdepth), "%d.%d", (int)mBitsPerChannel - fracbits, fracbits);
		else
			snprintf(bitdepth, sizeof(bitdepth), "%d", (int)mBitsPerChannel);
		
		/*nc =*/ snprintf(buf, static_cast<size_t>(bufsize), "%s-bit%s%s %s%s%s%s%s",
			bitdepth, endian, sign, floatInt, 
			commaSpace, packed, align, deinter);
		// buf += nc; if ((bufsize -= nc) <= 0) goto exit;
	} else if (mFormatID == kAudioFormatAppleLossless) {
		int sourceBits = 0;
		switch (mFormatFlags)
		{
			case 1:	//	kAppleLosslessFormatFlag_16BitSourceData
				sourceBits = 16;
				break;
    		case 2:	//	kAppleLosslessFormatFlag_20BitSourceData
    			sourceBits = 20;
    			break;
    		case 3:	//	kAppleLosslessFormatFlag_24BitSourceData
    			sourceBits = 24;
    			break;
    		case 4:	//	kAppleLosslessFormatFlag_32BitSourceData
    			sourceBits = 32;
    			break;
		}
		if (sourceBits)
			nc = snprintf(buf, static_cast<size_t>(bufsize), "from %d-bit source, ", sourceBits);
		else
			nc = snprintf(buf, static_cast<size_t>(bufsize), "from UNKNOWN source bit depth, ");
		buf += nc; if ((bufsize -= nc) <= 0) goto exit;
		/*nc =*/ snprintf(buf, static_cast<size_t>(bufsize), "%d frames/packet", (int)mFramesPerPacket);
		//	buf += nc; if ((bufsize -= nc) <= 0) goto exit;
	}
	else
		/*nc =*/ snprintf(buf, static_cast<size_t>(bufsize), "%d bits/channel, %d bytes/packet, %d frames/packet, %d bytes/frame", 
			(int)mBitsPerChannel, (int)mBytesPerPacket, (int)mFramesPerPacket, (int)mBytesPerFrame);
exit:
	return theBuffer;
}

void	CAStreamBasicDescription::NormalizeLinearPCMFormat(AudioStreamBasicDescription& ioDescription)
{
	//  the only thing that changes is to make mixable linear PCM into the canonical linear PCM format
	if((ioDescription.mFormatID == kAudioFormatLinearPCM) && ((ioDescription.mFormatFlags & kIsNonMixableFlag) == 0))
	{
		//  the canonical linear PCM format
		ioDescription.mFormatFlags = kAudioFormatFlagsCanonical;
		ioDescription.mBytesPerPacket = SizeOf32(AudioSampleType) * ioDescription.mChannelsPerFrame;
		ioDescription.mFramesPerPacket = 1;
		ioDescription.mBytesPerFrame = SizeOf32(AudioSampleType) * ioDescription.mChannelsPerFrame;
		ioDescription.mBitsPerChannel = 8 * SizeOf32(AudioSampleType);
	}
}

void	CAStreamBasicDescription::NormalizeLinearPCMFormat(bool inNativeEndian, AudioStreamBasicDescription& ioDescription)
{
	//  the only thing that changes is to make mixable linear PCM into the canonical linear PCM format
	if((ioDescription.mFormatID == kAudioFormatLinearPCM) && ((ioDescription.mFormatFlags & kIsNonMixableFlag) == 0))
	{
		//  the canonical linear PCM format
		ioDescription.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
		if(inNativeEndian)
		{
#if TARGET_RT_BIG_ENDIAN
			ioDescription.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
		}
		else
		{
#if TARGET_RT_LITTLE_ENDIAN
			ioDescription.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
		}
		ioDescription.mBytesPerPacket = SizeOf32(AudioSampleType) * ioDescription.mChannelsPerFrame;
		ioDescription.mFramesPerPacket = 1;
		ioDescription.mBytesPerFrame = SizeOf32(AudioSampleType) * ioDescription.mChannelsPerFrame;
		ioDescription.mBitsPerChannel = 8 * SizeOf32(AudioSampleType);
	}
}

void	CAStreamBasicDescription::VirtualizeLinearPCMFormat(AudioStreamBasicDescription& ioDescription)
{
	//  the only thing that changes is to make mixable linear PCM into the HAL's virtual linear PCM format, which is Float32 currently
	if((ioDescription.mFormatID == kAudioFormatLinearPCM) && ((ioDescription.mFormatFlags & kIsNonMixableFlag) == 0))
	{
		//  the virtual linear PCM format
		ioDescription.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
		ioDescription.mBytesPerPacket = SizeOf32(Float32) * ioDescription.mChannelsPerFrame;
		ioDescription.mFramesPerPacket = 1;
		ioDescription.mBytesPerFrame = SizeOf32(Float32) * ioDescription.mChannelsPerFrame;
		ioDescription.mBitsPerChannel = 8 * SizeOf32(Float32);
	}
}

void	CAStreamBasicDescription::VirtualizeLinearPCMFormat(bool inNativeEndian, AudioStreamBasicDescription& ioDescription)
{
	//  the only thing that changes is to make mixable linear PCM into the HAL's virtual linear PCM format, which is Float32 currently
	if((ioDescription.mFormatID == kAudioFormatLinearPCM) && ((ioDescription.mFormatFlags & kIsNonMixableFlag) == 0))
	{
		//  the virtual linear PCM format
		ioDescription.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
		if(inNativeEndian)
		{
#if TARGET_RT_BIG_ENDIAN
			ioDescription.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
		}
		else
		{
#if TARGET_RT_LITTLE_ENDIAN
			ioDescription.mFormatFlags |= kAudioFormatFlagIsBigEndian;
#endif
		}
		ioDescription.mBytesPerPacket = SizeOf32(Float32) * ioDescription.mChannelsPerFrame;
		ioDescription.mFramesPerPacket = 1;
		ioDescription.mBytesPerFrame = SizeOf32(Float32) * ioDescription.mChannelsPerFrame;
		ioDescription.mBitsPerChannel = 8 * SizeOf32(Float32);
	}
}

void	CAStreamBasicDescription::ResetFormat(AudioStreamBasicDescription& ioDescription)
{
	ioDescription.mSampleRate = 0;
	ioDescription.mFormatID = 0;
	ioDescription.mBytesPerPacket = 0;
	ioDescription.mFramesPerPacket = 0;
	ioDescription.mBytesPerFrame = 0;
	ioDescription.mChannelsPerFrame = 0;
	ioDescription.mBitsPerChannel = 0;
	ioDescription.mFormatFlags = 0;
}

void	CAStreamBasicDescription::FillOutFormat(AudioStreamBasicDescription& ioDescription, const AudioStreamBasicDescription& inTemplateDescription)
{
	if(fiszero(ioDescription.mSampleRate))
	{
		ioDescription.mSampleRate = inTemplateDescription.mSampleRate;
	}
	if(ioDescription.mFormatID == 0)
	{
		ioDescription.mFormatID = inTemplateDescription.mFormatID;
	}
	if(ioDescription.mFormatFlags == 0)
	{
		ioDescription.mFormatFlags = inTemplateDescription.mFormatFlags;
	}
	if(ioDescription.mBytesPerPacket == 0)
	{
		ioDescription.mBytesPerPacket = inTemplateDescription.mBytesPerPacket;
	}
	if(ioDescription.mFramesPerPacket == 0)
	{
		ioDescription.mFramesPerPacket = inTemplateDescription.mFramesPerPacket;
	}
	if(ioDescription.mBytesPerFrame == 0)
	{
		ioDescription.mBytesPerFrame = inTemplateDescription.mBytesPerFrame;
	}
	if(ioDescription.mChannelsPerFrame == 0)
	{
		ioDescription.mChannelsPerFrame = inTemplateDescription.mChannelsPerFrame;
	}
	if(ioDescription.mBitsPerChannel == 0)
	{
		ioDescription.mBitsPerChannel = inTemplateDescription.mBitsPerChannel;
	}
}

void	CAStreamBasicDescription::GetSimpleName(const AudioStreamBasicDescription& inDescription, char* outName, UInt32 inMaxNameLength, bool inAbbreviate, bool inIncludeSampleRate)
{
	if(inIncludeSampleRate)
	{
		int theCharactersWritten = snprintf(outName, inMaxNameLength, "%.0f ", inDescription.mSampleRate);
		outName += theCharactersWritten;
		inMaxNameLength -= static_cast<UInt32>(theCharactersWritten);
	}
	
	switch(inDescription.mFormatID)
	{
		case kAudioFormatLinearPCM:
			{
				const char* theEndianString = NULL;
				if((inDescription.mFormatFlags & kAudioFormatFlagIsBigEndian) != 0)
				{
					#if	TARGET_RT_LITTLE_ENDIAN
						theEndianString = "Big Endian";
					#endif
				}
				else
				{
					#if	TARGET_RT_BIG_ENDIAN
						theEndianString = "Little Endian";
					#endif
				}
				
				const char* theKindString = NULL;
				if((inDescription.mFormatFlags & kAudioFormatFlagIsFloat) != 0)
				{
					theKindString = (inAbbreviate ? "Float" : "Floating Point");
				}
				else if((inDescription.mFormatFlags & kAudioFormatFlagIsSignedInteger) != 0)
				{
					theKindString = (inAbbreviate ? "SInt" : "Signed Integer");
				}
				else
				{
					theKindString = (inAbbreviate ? "UInt" : "Unsigned Integer");
				}
				
				const char* thePackingString = NULL;
				if((inDescription.mFormatFlags & kAudioFormatFlagIsPacked) == 0)
				{
					if((inDescription.mFormatFlags & kAudioFormatFlagIsAlignedHigh) != 0)
					{
						thePackingString = "High";
					}
					else
					{
						thePackingString = "Low";
					}
				}
				
				const char* theMixabilityString = NULL;
				if((inDescription.mFormatFlags & kIsNonMixableFlag) == 0)
				{
					theMixabilityString = "Mixable";
				}
				else
				{
					theMixabilityString = "Unmixable";
				}
				
				if(inAbbreviate)
				{
					if(theEndianString != NULL)
					{
						if(thePackingString != NULL)
						{
							snprintf(outName, inMaxNameLength, "%s %d Ch %s %s %s%d/%s%d", theMixabilityString, (int)inDescription.mChannelsPerFrame, theEndianString, thePackingString, theKindString, (int)inDescription.mBitsPerChannel, theKindString, (int)(inDescription.mBytesPerFrame / inDescription.mChannelsPerFrame) * 8);
						}
						else
						{
							snprintf(outName, inMaxNameLength, "%s %d Ch %s %s%d", theMixabilityString, (int)inDescription.mChannelsPerFrame, theEndianString, theKindString, (int)inDescription.mBitsPerChannel);
						}
					}
					else
					{
						if(thePackingString != NULL)
						{
							snprintf(outName, inMaxNameLength, "%s %d Ch %s %s%d/%s%d", theMixabilityString, (int)inDescription.mChannelsPerFrame, thePackingString, theKindString, (int)inDescription.mBitsPerChannel, theKindString, (int)((inDescription.mBytesPerFrame / inDescription.mChannelsPerFrame) * 8));
						}
						else
						{
							snprintf(outName, inMaxNameLength, "%s %d Ch %s%d", theMixabilityString, (int)inDescription.mChannelsPerFrame, theKindString, (int)inDescription.mBitsPerChannel);
						}
					}
				}
				else
				{
					if(theEndianString != NULL)
					{
						if(thePackingString != NULL)
						{
							snprintf(outName, inMaxNameLength, "%s %d Channel %d Bit %s %s Aligned %s in %d Bits", theMixabilityString, (int)inDescription.mChannelsPerFrame, (int)inDescription.mBitsPerChannel, theEndianString, theKindString, thePackingString, (int)(inDescription.mBytesPerFrame / inDescription.mChannelsPerFrame) * 8);
						}
						else
						{
							snprintf(outName, inMaxNameLength, "%s %d Channel %d Bit %s %s", theMixabilityString, (int)inDescription.mChannelsPerFrame, (int)inDescription.mBitsPerChannel, theEndianString, theKindString);
						}
					}
					else
					{
						if(thePackingString != NULL)
						{
							snprintf(outName, inMaxNameLength, "%s %d Channel %d Bit %s Aligned %s in %d Bits", theMixabilityString, (int)inDescription.mChannelsPerFrame, (int)inDescription.mBitsPerChannel, theKindString, thePackingString, (int)(inDescription.mBytesPerFrame / inDescription.mChannelsPerFrame) * 8);
						}
						else
						{
							snprintf(outName, inMaxNameLength, "%s %d Channel %d Bit %s", theMixabilityString, (int)inDescription.mChannelsPerFrame, (int)inDescription.mBitsPerChannel, theKindString);
						}
					}
				}
			}
			break;
		
		case kAudioFormatAC3:
			strlcpy(outName, "AC-3", sizeof(outName));
			break;
		
		case kAudioFormat60958AC3:
			strlcpy(outName, "AC-3 for SPDIF", sizeof(outName));
			break;
		
		default:
			CACopy4CCToCString(outName, inDescription.mFormatID);
			break;
	};
}

#if CoreAudio_Debug
#include "CALogMacros.h"

void	CAStreamBasicDescription::PrintToLog(const AudioStreamBasicDescription& inDesc)
{
	PrintFloat		("  Sample Rate:        ", inDesc.mSampleRate);
	Print4CharCode	("  Format ID:          ", inDesc.mFormatID);
	PrintHex		("  Format Flags:       ", inDesc.mFormatFlags);
	PrintInt		("  Bytes per Packet:   ", inDesc.mBytesPerPacket);
	PrintInt		("  Frames per Packet:  ", inDesc.mFramesPerPacket);
	PrintInt		("  Bytes per Frame:    ", inDesc.mBytesPerFrame);
	PrintInt		("  Channels per Frame: ", inDesc.mChannelsPerFrame);
	PrintInt		("  Bits per Channel:   ", inDesc.mBitsPerChannel);
}
#endif

bool	operator<(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y)
{
	bool theAnswer = false;
	bool isDone = false;
	
	//	note that if either side is 0, that field is skipped
	
	//	format ID is the first order sort
	if((!isDone) && ((x.mFormatID != 0) && (y.mFormatID != 0)))
	{
		if(x.mFormatID != y.mFormatID)
		{
			//	formats are sorted numerically except that linear
			//	PCM is always first
			if(x.mFormatID == kAudioFormatLinearPCM)
			{
				theAnswer = true;
			}
			else if(y.mFormatID == kAudioFormatLinearPCM)
			{
				theAnswer = false;
			}
			else
			{
				theAnswer = x.mFormatID < y.mFormatID;
			}
			isDone = true;
		}
	}
	
	
	//  mixable is always better than non-mixable for linear PCM and should be the second order sort item
	if((!isDone) && ((x.mFormatID == kAudioFormatLinearPCM) && (y.mFormatID == kAudioFormatLinearPCM)))
	{
		if(((x.mFormatFlags & kIsNonMixableFlag) == 0) && ((y.mFormatFlags & kIsNonMixableFlag) != 0))
		{
			theAnswer = true;
			isDone = true;
		}
		else if(((x.mFormatFlags & kIsNonMixableFlag) != 0) && ((y.mFormatFlags & kIsNonMixableFlag) == 0))
		{
			theAnswer = false;
			isDone = true;
		}
	}
	
	//	floating point vs integer for linear PCM only
	if((!isDone) && ((x.mFormatID == kAudioFormatLinearPCM) && (y.mFormatID == kAudioFormatLinearPCM)))
	{
		if((x.mFormatFlags & kAudioFormatFlagIsFloat) != (y.mFormatFlags & kAudioFormatFlagIsFloat))
		{
			//	floating point is better than integer
			theAnswer = y.mFormatFlags & kAudioFormatFlagIsFloat;
			isDone = true;
		}
	}
	
	//	bit depth
	if((!isDone) && ((x.mBitsPerChannel != 0) && (y.mBitsPerChannel != 0)))
	{
		if(x.mBitsPerChannel != y.mBitsPerChannel)
		{
			//	deeper bit depths are higher quality
			theAnswer = x.mBitsPerChannel < y.mBitsPerChannel;
			isDone = true;
		}
	}
	
	//	sample rate
	if((!isDone) && fnonzero(x.mSampleRate) && fnonzero(y.mSampleRate))
	{
		if(fnotequal(x.mSampleRate, y.mSampleRate))
		{
			//	higher sample rates are higher quality
			theAnswer = x.mSampleRate < y.mSampleRate;
			isDone = true;
		}
	}
	
	//	number of channels
	if((!isDone) && ((x.mChannelsPerFrame != 0) && (y.mChannelsPerFrame != 0)))
	{
		if(x.mChannelsPerFrame != y.mChannelsPerFrame)
		{
			//	more channels is higher quality
			theAnswer = x.mChannelsPerFrame < y.mChannelsPerFrame;
			//isDone = true;
		}
	}
	
	return theAnswer;
}

UInt32 CAStreamBasicDescription::GetRegularizedFormatFlags(bool forHardware) const
{
	UInt32 result = mFormatFlags;

	if (IsPCM()) {
		// First, if there are bits other than AllClear set, clear it because it's lying.
		if (result & ~kAudioFormatFlagsAreAllClear)
			result &= ~kAudioFormatFlagsAreAllClear;
	
		// If not forHardware, remove the mixability flag.
		if (!forHardware)
			result &= ~kLinearPCMFormatFlagIsNonMixable;
		
		// If the format has no extra bits, then it is packed.
		if (!PackednessIsSignificant())
			result |= kLinearPCMFormatFlagIsPacked;

		// Remove the high-aligned flag if alignment is irrelevant.
		if (!AlignmentIsSignificant())
			result &= ~kLinearPCMFormatFlagIsAlignedHigh;

		// Remove the signed integer bit if it's float
		if (result & kLinearPCMFormatFlagIsFloat)
			result &= ~kLinearPCMFormatFlagIsSignedInteger;
		
		// If the bit depth is 8 bits or less and the format is packed, we don't care about endianness
		if (mBitsPerChannel <= 8 && (result & kLinearPCMFormatFlagIsPacked))
			result &= kAudioFormatFlagIsBigEndian;
		
		// If there is 1 channel, we don't care about non-interleavedness.
		if (mChannelsPerFrame == 1)
			result &= ~kLinearPCMFormatFlagIsNonInterleaved;
		
		// Finally, if the bits really are all 0, set the AllClear flag.
		if (result == 0)
			result = kAudioFormatFlagsAreAllClear;
	}
	return result;
}

// private
bool CAStreamBasicDescription::EquivalentFormatFlags(const AudioStreamBasicDescription &x, const AudioStreamBasicDescription &y, bool forHardware, bool usingWildcards)
{
	if (usingWildcards)
	{
		// if either of the formats is a wildcard, we don't care about the flags
		// if either of the flags is a wildcard, we have matched
		if (x.mFormatID == 0 || y.mFormatID == 0 || x.mFormatFlags == 0 || y.mFormatFlags == 0)
		{
			return true;
		}
    }

	if (x.mFormatID != kAudioFormatLinearPCM) // we already know the formatID's match and have taken wildcards out of the picture.
		return x.mFormatFlags == y.mFormatFlags;
	
	// It is safe to down-cast from AudioStreamBasicDescription to its C++ wrapper.
	// The cast could be avoided with a copy, but here, efficiency matters.
	const CAStreamBasicDescription &a = *static_cast<const CAStreamBasicDescription *>(&x);
	const CAStreamBasicDescription &b = *static_cast<const CAStreamBasicDescription *>(&y);
	
	return a.GetRegularizedFormatFlags(forHardware) == b.GetRegularizedFormatFlags(forHardware);
}

bool	CAStreamBasicDescription::IsExactlyEqual(const AudioStreamBasicDescription &x, const AudioStreamBasicDescription &y)
{
	// mReserved didn't exist in early versions of OS X; we want to ignore differences there.
	// The structure is properly packed up until that point, so the shortcut of using memcmp()
	// instead of individual field comparisons is safe.
	return memcmp(&x, &y, offsetof(AudioStreamBasicDescription, mReserved)) == 0;
}

#define MATCH_WITH_WILDCARD(name) ((x.name) == 0 || (y.name) == 0 || (x.name) == (y.name))

bool	CAStreamBasicDescription::IsEquivalent(const AudioStreamBasicDescription &x, const AudioStreamBasicDescription &y, ComparisonOptions options)
{
	if (options & kCompareUsingWildcards) {
		return
			//	check the sample rate
			(fiszero(x.mSampleRate) || fiszero(y.mSampleRate) || fequal(x.mSampleRate, y.mSampleRate))
			
			//	check the format ids
			&& MATCH_WITH_WILDCARD(mFormatID)
				
			//	check the bytes per packet
			&& MATCH_WITH_WILDCARD(mBytesPerPacket)
			
			//	check the frames per packet
			&& MATCH_WITH_WILDCARD(mFramesPerPacket)
			
			//	check the bytes per frame
			&& MATCH_WITH_WILDCARD(mBytesPerFrame)
			
			//	check the channels per frame
			&& MATCH_WITH_WILDCARD(mChannelsPerFrame)
			
			//	check the bits per channel
			&& MATCH_WITH_WILDCARD(mBitsPerChannel)

			// Only if we get this far, do the work of matching the format flags
			&& EquivalentFormatFlags(x, y, options & kCompareForHardware, /*usingWildcards=*/true);
	} else {
		return	x.mSampleRate == y.mSampleRate
			&&	x.mFormatID == y.mFormatID
			&&	x.mBytesPerPacket == y.mBytesPerPacket
			&&	x.mFramesPerPacket == y.mFramesPerPacket
			&&	x.mChannelsPerFrame == y.mChannelsPerFrame
			&&	x.mBitsPerChannel == y.mBitsPerChannel
			&&	EquivalentFormatFlags(x, y, options & kCompareForHardware, /*usingWildcards=*/false);
	}
}

// DEPRECATED.
bool	operator==(const AudioStreamBasicDescription& x, const AudioStreamBasicDescription& y)
{
	return CAStreamBasicDescription::IsEquivalent(x, y, CAStreamBasicDescription::kCompareUsingWildcards | CAStreamBasicDescription::kCompareForHardware);
}

// To be deprecated.
bool	CAStreamBasicDescription::IsEqual(const AudioStreamBasicDescription &other, bool interpretingWildcards) const
{
	if (interpretingWildcards)
		return CAStreamBasicDescription::IsEquivalent(*this, other, CAStreamBasicDescription::kCompareUsingWildcards | CAStreamBasicDescription::kCompareForHardware);
	return IsExactlyEqual(*this, other);
}

// DEPRECATED.
bool	CAStreamBasicDescription::IsEqual(const AudioStreamBasicDescription &other) const
{
	return CAStreamBasicDescription::IsEquivalent(*this, other, CAStreamBasicDescription::kCompareUsingWildcards | CAStreamBasicDescription::kCompareForHardware);
}

bool SanityCheck(const AudioStreamBasicDescription& x)
{
	// This function returns false if there are sufficiently insane values in any field.
	// It is very conservative so even some very unlikely values will pass.
	// This is just meant to catch the case where the data from a file is corrupted.
	
	return 
		(x.mSampleRate >= 0.)	
		&& (x.mSampleRate < 3e6)	// SACD sample rate is 2.8224 MHz
		&& (x.mBytesPerPacket < 1000000)
		&& (x.mFramesPerPacket < 1000000)
		&& (x.mBytesPerFrame < 1000000)
		&& (x.mChannelsPerFrame > 0)
		&& (x.mChannelsPerFrame <= 1024)
		&& (x.mBitsPerChannel <= 1024)
		&& (x.mFormatID != 0)
		&& !(x.mFormatID == kAudioFormatLinearPCM && (x.mFramesPerPacket != 1 || x.mBytesPerPacket != x.mBytesPerFrame));
}

bool CAStreamBasicDescription::FromText(const char *inTextDesc, AudioStreamBasicDescription &fmt)
{
	const char *p = inTextDesc;
	
	memset(&fmt, 0, sizeof(fmt));

	bool isPCM = true;	// until proven otherwise
	UInt32 pcmFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;

	if (p[0] == '-')	// previously we required a leading dash on PCM formats
		++p;

	if (p[0] == 'B' && p[1] == 'E') {
		pcmFlags |= kLinearPCMFormatFlagIsBigEndian;
		p += 2;
	} else if (p[0] == 'L' && p[1] == 'E') {
		p += 2;
	} else {
		// default is native-endian
#if TARGET_RT_BIG_ENDIAN
		pcmFlags |= kLinearPCMFormatFlagIsBigEndian;
#endif
	}
	if (p[0] == 'F') {
		pcmFlags = (pcmFlags & ~static_cast<UInt32>(kAudioFormatFlagIsSignedInteger)) | kAudioFormatFlagIsFloat;
		++p;
	} else {
		if (p[0] == 'U') {
			pcmFlags &= ~static_cast<UInt32>(kAudioFormatFlagIsSignedInteger);
			++p;
		}
		if (p[0] == 'I')
			++p;
		else {
			// it's not PCM; presumably some other format (NOT VALIDATED; use AudioFormat for that)
			isPCM = false;
			p = inTextDesc;	// go back to the beginning
			char buf[4] = { ' ',' ',' ',' ' };
			for (int i = 0; i < 4; ++i) {
				if (*p != '\\') {
					if ((buf[i] = *p++) == '\0') {
						// special-case for 'aac'
						if (i != 3) return false;
						--p;	// keep pointing at the terminating null
						buf[i] = ' ';
						break;
					}
				} else {
					// "\xNN" is a hex byte
					if (*++p != 'x') return false;
					int x;
					if (sscanf(++p, "%02X", &x) != 1) return false;
					buf[i] = static_cast<char>(x);
					p += 2;
				}
			}
			
			if (strchr("-@/#", buf[3])) {
				// further special-casing for 'aac'
				buf[3] = ' ';
				--p;
			}
			
			memcpy(&fmt.mFormatID, buf, 4);
			fmt.mFormatID = CFSwapInt32BigToHost(fmt.mFormatID);
		}
	}
	
	if (isPCM) {
		fmt.mFormatID = kAudioFormatLinearPCM;
		fmt.mFormatFlags = pcmFlags;
		fmt.mFramesPerPacket = 1;
		fmt.mChannelsPerFrame = 1;
		UInt32 bitdepth = 0, fracbits = 0;
		while (isdigit(*p))
			bitdepth = 10 * bitdepth + static_cast<UInt32>(*p++ - '0');
		if (*p == '.') {
			++p;
			if (!isdigit(*p)) {
				fprintf(stderr, "Expected fractional bits following '.'\n");
				goto Bail;
			}
			while (isdigit(*p))
				fracbits = 10 * fracbits + static_cast<UInt32>(*p++ - '0');
			bitdepth += fracbits;
			fmt.mFormatFlags |= (fracbits << kLinearPCMFormatFlagsSampleFractionShift);
		}
		fmt.mBitsPerChannel = bitdepth;
		fmt.mBytesPerPacket = fmt.mBytesPerFrame = (bitdepth + 7) / 8;
		if (bitdepth & 7) {
			// assume unpacked. (packed odd bit depths are describable but not supported in AudioConverter.)
			fmt.mFormatFlags &= ~static_cast<UInt32>(kLinearPCMFormatFlagIsPacked);
			// alignment matters; default to high-aligned. use ':L_' for low.
			fmt.mFormatFlags |= kLinearPCMFormatFlagIsAlignedHigh;
		}
	}
	if (*p == '@') {
		++p;
		while (isdigit(*p))
			fmt.mSampleRate = 10 * fmt.mSampleRate + (*p++ - '0');
	}
	if (*p == '/') {
		UInt32 flags = 0;
		while (true) {
			char c = *++p;
			if (c >= '0' && c <= '9')
				flags = (flags << 4) | static_cast<UInt32>(c - '0');
			else if (c >= 'A' && c <= 'F')
				flags = (flags << 4) | static_cast<UInt32>(c - 'A' + 10);
			else if (c >= 'a' && c <= 'f')
				flags = (flags << 4) | static_cast<UInt32>(c - 'a' + 10);
			else break;
		}
		fmt.mFormatFlags = flags;
	}
	if (*p == '#') {
		++p;
		while (isdigit(*p))
			fmt.mFramesPerPacket = 10 * fmt.mFramesPerPacket + static_cast<UInt32>(*p++ - '0');
	}
	if (*p == ':') {
		++p;
		fmt.mFormatFlags &= ~static_cast<UInt32>(kLinearPCMFormatFlagIsPacked);
		if (*p == 'L')
			fmt.mFormatFlags &= ~static_cast<UInt32>(kLinearPCMFormatFlagIsAlignedHigh);
		else if (*p == 'H')
			fmt.mFormatFlags |= kLinearPCMFormatFlagIsAlignedHigh;
		else
			goto Bail;
		++p;
		UInt32 bytesPerFrame = 0;
		while (isdigit(*p))
			bytesPerFrame = 10 * bytesPerFrame + static_cast<UInt32>(*p++ - '0');
		fmt.mBytesPerFrame = fmt.mBytesPerPacket = bytesPerFrame;
	}
	if (*p == ',') {
		++p;
		int ch = 0;
		while (isdigit(*p))
			ch = 10 * ch + (*p++ - '0');
		fmt.mChannelsPerFrame = static_cast<UInt32>(ch);
		if (*p == 'D') {
			++p;
			if (fmt.mFormatID != kAudioFormatLinearPCM) {
				fprintf(stderr, "non-interleaved flag invalid for non-PCM formats\n");
				goto Bail;
			}
			fmt.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		} else {
			if (*p == 'I') ++p;	// default
			if (fmt.mFormatID == kAudioFormatLinearPCM)
				fmt.mBytesPerPacket = fmt.mBytesPerFrame *= static_cast<UInt32>(ch);
		}
	}
	if (*p != '\0') {
		fprintf(stderr, "extra characters at end of format string: %s\n", p);
		goto Bail;
	}
	return true;

Bail:
	fprintf(stderr, "Invalid format string: %s\n", inTextDesc);
	fprintf(stderr, "Syntax of format strings is: \n");
	return false;
}

const char *CAStreamBasicDescription::sTextParsingUsageString = 
	"format[@sample_rate_hz][/format_flags][#frames_per_packet][:LHbytesPerFrame][,channelsDI].\n"
	"Format for PCM is [-][BE|LE]{F|I|UI}{bitdepth}; else a 4-char format code (e.g. aac, alac).\n";
