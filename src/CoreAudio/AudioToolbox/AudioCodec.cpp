/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#include <AudioToolbox/AudioCodec.h>
#include <CarbonCore/MacErrors.h>
#include "ComponentDispatch.h"

// An AudioCodec is a component instance (AudioCodec.h), so each call goes to the selector
// the codec plug-in side maps it to, the way the AudioUnit and AudioFileComponent calls do.
// Producing output packets is selected by kAudioCodecProduceOutputDataSelect.

OSStatus
AudioCodecGetPropertyInfo(AudioCodec inCodec,
	AudioCodecPropertyID inPropertyID,
	UInt32* outSize,
	Boolean* outWritable)
{
	return dispatchCall(inCodec, kAudioCodecGetPropertyInfoSelect, inPropertyID, outSize, outWritable);
}

OSStatus
AudioCodecGetProperty(AudioCodec inCodec,
	AudioCodecPropertyID inPropertyID,
	UInt32* ioPropertyDataSize,
	void* outPropertyData)
{
	return dispatchCall(inCodec, kAudioCodecGetPropertySelect, inPropertyID, ioPropertyDataSize, outPropertyData);
}

OSStatus
AudioCodecSetProperty(AudioCodec inCodec,
	AudioCodecPropertyID inPropertyID,
	UInt32 inPropertyDataSize,
	const void* inPropertyData)
{
	return dispatchCall(inCodec, kAudioCodecSetPropertySelect, inPropertyID, inPropertyDataSize, inPropertyData);
}

OSStatus
AudioCodecInitialize(AudioCodec inCodec,
	const AudioStreamBasicDescription* inInputFormat,
	const AudioStreamBasicDescription* inOutputFormat,
	const void* inMagicCookie,
	UInt32 inMagicCookieByteSize)
{
	return dispatchCall(inCodec, kAudioCodecInitializeSelect, inInputFormat, inOutputFormat, inMagicCookie, inMagicCookieByteSize);
}

OSStatus
AudioCodecUninitialize(AudioCodec inCodec)
{
	return dispatchCall(inCodec, kAudioCodecUninitializeSelect);
}

OSStatus
AudioCodecAppendInputData(AudioCodec inCodec,
	const void* inInputData,
	UInt32* ioInputDataByteSize,
	UInt32* ioNumberPackets,
	const AudioStreamPacketDescription* inPacketDescription)
{
	return dispatchCall(inCodec, kAudioCodecAppendInputDataSelect, inInputData, ioInputDataByteSize, ioNumberPackets, inPacketDescription);
}

OSStatus
AudioCodecProduceOutputPackets(AudioCodec inCodec,
	void* outOutputData,
	UInt32* ioOutputDataByteSize,
	UInt32* ioNumberPackets,
	AudioStreamPacketDescription* outPacketDescription,
	UInt32* outStatus)
{
	return dispatchCall(inCodec, kAudioCodecProduceOutputDataSelect, outOutputData, ioOutputDataByteSize, ioNumberPackets, outPacketDescription, outStatus);
}

OSStatus
AudioCodecAppendInputBufferList(AudioCodec inCodec,
	const AudioBufferList* inBufferList,
	UInt32* ioNumberPackets,
	const AudioStreamPacketDescription* inPacketDescription,
	UInt32* outBytesConsumed)
{
	return dispatchCall(inCodec, kAudioCodecAppendInputBufferListSelect, inBufferList, ioNumberPackets, inPacketDescription, outBytesConsumed);
}

OSStatus
AudioCodecProduceOutputBufferList(AudioCodec inCodec,
	AudioBufferList* ioBufferList,
	UInt32* ioNumberPackets,
	AudioStreamPacketDescription* outPacketDescription,
	UInt32* outStatus)
{
	return dispatchCall(inCodec, kAudioCodecProduceOutputBufferListSelect, ioBufferList, ioNumberPackets, outPacketDescription, outStatus);
}

OSStatus
AudioCodecReset(AudioCodec inCodec)
{
	return dispatchCall(inCodec, kAudioCodecResetSelect);
}
