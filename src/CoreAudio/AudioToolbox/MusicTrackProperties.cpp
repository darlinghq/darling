/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

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

// Music track properties (MusicPlayer.h): MusicTrackSetProperty and MusicTrackGetProperty.
//
// Property values are read and written by reference and have a fixed size. The properties are
// stored on the track; nothing plays yet, so they have no effect other than being read back
// (and the track length, which follows the events).

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include "MusicSequenceImpl.h"

namespace
{

// How long a note lasts is part of the length of its track.
MusicTimeStamp eventEnd(const MusicEvent& event)
{
	Float32 duration = 0;
	if (event.type == kMusicEventType_MIDINoteMessage)
	{
		MIDINoteMessage note;
		std::memcpy(&note, event.data.data(), sizeof(note));
		duration = note.duration;
	}
	else if (event.type == kMusicEventType_ExtendedNote)
	{
		ExtendedNoteOnEvent note;
		std::memcpy(&note, event.data.data(), offsetof(ExtendedNoteOnEvent, extendedParams));
		duration = note.duration;
	}
	return event.time + duration;
}

bool isTempoTrack(const OpaqueMusicTrack* track)
{
	return track->sequence && track == &track->sequence->tempoTrack;
}

template <typename T>
bool takeValue(const void* data, UInt32 length, T& value)
{
	if (length != sizeof(T))
		return false;
	std::memcpy(&value, data, sizeof(T));
	return true;
}

}

MusicTimeStamp musicimpl::trackLength(const OpaqueMusicTrack* track)
{
	MusicTimeStamp length = track->hasLength ? track->length : 0;
	for (const MusicEvent& event : track->events)
		length = std::max(length, eventEnd(event));
	return length;
}

OSStatus MusicTrackSetProperty(MusicTrack inTrack, UInt32 inPropertyID, void* inData, UInt32 inLength)
{
	// GUESS: a NULL value, a size that is not the size of the property, an unknown property and a
	// value that makes no sense (a negative or non-finite loop duration or length, a non-finite
	// offset, a negative loop count) are all kAudio_ParamError; the documentation says only that
	// properties have fixed sizes. A negative offset is accepted: it moves the track earlier.
	if (!inTrack || !inData)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	switch (inPropertyID)
	{
		case kSequenceTrackProperty_LoopInfo:
		{
			MusicTrackLoopInfo loop;
			if (!takeValue(inData, inLength, loop) || !std::isfinite(loop.loopDuration) || loop.loopDuration < 0 || loop.numberOfLoops < 0)
				return kAudio_ParamError;
			inTrack->loop = loop;
			return noErr;
		}
		case kSequenceTrackProperty_OffsetTime:
		{
			MusicTimeStamp offset;
			if (!takeValue(inData, inLength, offset) || !std::isfinite(offset))
				return kAudio_ParamError;
			inTrack->offsetTime = offset;
			return noErr;
		}
		case kSequenceTrackProperty_MuteStatus:
		{
			Boolean muted;
			if (!takeValue(inData, inLength, muted))
				return kAudio_ParamError;
			inTrack->muted = muted != 0;
			return noErr;
		}
		case kSequenceTrackProperty_SoloStatus:
		{
			Boolean soloed;
			if (!takeValue(inData, inLength, soloed))
				return kAudio_ParamError;
			inTrack->soloed = soloed != 0;
			return noErr;
		}
		case kSequenceTrackProperty_AutomatedParameters:
		{
			UInt32 automated;
			if (!takeValue(inData, inLength, automated))
				return kAudio_ParamError;
			inTrack->automatedParameters = automated;
			return noErr;
		}
		case kSequenceTrackProperty_TrackLength:
		{
			MusicTimeStamp length;
			if (!takeValue(inData, inLength, length) || !std::isfinite(length) || length < 0)
				return kAudio_ParamError;
			inTrack->hasLength = true;
			inTrack->length = length;
			return noErr;
		}
		default:
			// The time resolution is read only, so setting it is refused too.
			return kAudio_ParamError;
	}
}

OSStatus MusicTrackGetProperty(MusicTrack inTrack, UInt32 inPropertyID, void* outData, UInt32* ioLength)
{
	if (!inTrack || !ioLength)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// Each value is staged in a buffer of its own size so that the common code can report the size
	// and copy it.
	union
	{
		MusicTrackLoopInfo loop;
		MusicTimeStamp time;
		Boolean flag;
		UInt32 count;
		SInt16 resolution;
	} value;
	UInt32 size;

	switch (inPropertyID)
	{
		case kSequenceTrackProperty_LoopInfo:
			value.loop = inTrack->loop;
			size = sizeof(value.loop);
			break;
		case kSequenceTrackProperty_OffsetTime:
			value.time = inTrack->offsetTime;
			size = sizeof(value.time);
			break;
		case kSequenceTrackProperty_MuteStatus:
			value.flag = inTrack->muted;
			size = sizeof(value.flag);
			break;
		case kSequenceTrackProperty_SoloStatus:
			value.flag = inTrack->soloed;
			size = sizeof(value.flag);
			break;
		case kSequenceTrackProperty_AutomatedParameters:
			value.count = inTrack->automatedParameters;
			size = sizeof(value.count);
			break;
		case kSequenceTrackProperty_TrackLength:
			value.time = musicimpl::trackLength(inTrack);
			size = sizeof(value.time);
			break;
		case kSequenceTrackProperty_TimeResolution:
			// The documentation says the value is only valid on the tempo track. GUESS: on any other
			// track this is kAudio_ParamError.
			if (!isTempoTrack(inTrack))
				return kAudio_ParamError;
			value.resolution = inTrack->sequence->timeResolution;
			size = sizeof(value.resolution);
			break;
		default:
			return kAudio_ParamError;
	}

	// With no buffer only the size is reported. GUESS: a buffer smaller than the value is
	// kAudio_ParamError and nothing is copied.
	if (outData)
	{
		if (*ioLength < size)
			return kAudio_ParamError;
		std::memcpy(outData, &value, size);
	}
	*ioLength = size;
	return noErr;
}
