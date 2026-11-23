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
// Property values are read and written by reference and have a fixed size (which macOS does not check against the one
// the caller passes, and neither does this). The properties are
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
	// QUIRK: the size that is passed is not looked at: each property has its own size, whatever the caller
	// says (observed on macOS 26.7.3 and 10.14.4, tier 2; test: tests/src/audiotoolbox_music_track_properties.c,
	// "sizes"). Values are checked as little as there: the offset may be anything, a loop duration only
	// not negative, and a track length not negative. The mute, solo and automation flags keep one bit of
	// truth.
	(void)inLength;
	if (!inTrack || !inData)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	switch (inPropertyID)
	{
		case kSequenceTrackProperty_LoopInfo:
		{
			MusicTrackLoopInfo loop;
			std::memcpy(&loop, inData, sizeof(loop));
			if (loop.loopDuration < 0)
				return kAudio_ParamError;
			inTrack->loop = loop;
			return noErr;
		}
		case kSequenceTrackProperty_OffsetTime:
			std::memcpy(&inTrack->offsetTime, inData, sizeof(inTrack->offsetTime));
			return noErr;
		case kSequenceTrackProperty_MuteStatus:
			inTrack->muted = *static_cast<const Boolean*>(inData) != 0;
			return noErr;
		case kSequenceTrackProperty_SoloStatus:
			inTrack->soloed = *static_cast<const Boolean*>(inData) != 0;
			return noErr;
		case kSequenceTrackProperty_AutomatedParameters:
		{
			UInt32 automated;
			std::memcpy(&automated, inData, sizeof(automated));
			inTrack->automatedParameters = automated != 0;
			return noErr;
		}
		case kSequenceTrackProperty_TrackLength:
		{
			MusicTimeStamp length;
			std::memcpy(&length, inData, sizeof(length));
			if (length < 0)
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
	if (!inTrack)
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
			// The documentation says the value is only valid on the tempo track; on any other track this is
			// kAudio_ParamError (observed on macOS 26.7.3 and 10.14.4).
			if (!isTempoTrack(inTrack))
				return kAudio_ParamError;
			value.resolution = inTrack->sequence->timeResolution;
			size = sizeof(value.resolution);
			break;
		default:
			return kAudio_ParamError;
	}

	// QUIRK: the length passed in is not looked at and may be NULL: the value is copied whole when there is
	// a buffer, and the length, when there is one, is set to the size of the value (observed on macOS 26.7.3
	// and 10.14.4, tier 2; test: tests/src/audiotoolbox_music_track_properties.c, "sizes").
	if (outData)
		std::memcpy(outData, &value, size);
	if (ioLength)
		*ioLength = size;
	return noErr;
}
