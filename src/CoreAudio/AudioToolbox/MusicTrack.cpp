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

// Music tracks (MusicPlayer.h): adding time-stamped events.
//
// A track keeps its events in time order. The event types and the layout of their data are those
// of MusicEventType and the event structures in MusicPlayer.h; a track stores a copy of the event
// the caller passes.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <new>
#include "MusicSequenceImpl.h"

size_t musicimpl::insertEvent(OpaqueMusicTrack* track, MusicEvent&& event)
{
	auto pos = std::upper_bound(track->events.begin(), track->events.end(), event.time,
		[](MusicTimeStamp time, const MusicEvent& e) { return time < e.time; });
	pos = track->events.insert(pos, std::move(event));
	return static_cast<size_t>(pos - track->events.begin());
}

namespace
{

// Copies an event structure of header bytes followed by count items of unit bytes. The size of an
// event is reported in a UInt32, so the whole must fit in one.
bool copyVariable(size_t header, UInt32 count, size_t unit, const void* event, std::vector<UInt8>& out)
{
	if (count > (0xFFFFFFFFu - header) / unit)
		return false;
	const UInt8* bytes = static_cast<const UInt8*>(event);
	out.assign(bytes, bytes + header + static_cast<size_t>(count) * unit);
	return true;
}

}

OSStatus musicimpl::makeEventData(MusicEventType type, const void* event, std::vector<UInt8>& out)
{
	if (!event)
		return kAudio_ParamError;

	switch (type)
	{
		case kMusicEventType_MIDINoteMessage:
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(MIDINoteMessage));
			return noErr;
		case kMusicEventType_MIDIChannelMessage:
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(MIDIChannelMessage));
			return noErr;
		case kMusicEventType_Parameter:
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(ParameterEvent));
			return noErr;
		case kMusicEventType_ExtendedTempo:
		{
			// GUESS: a tempo that is zero, negative or not a number cannot describe a rate of beats, so it is
			// kAudio_ParamError. The documentation does not name a valid range.
			const Float64 bpm = static_cast<const ExtendedTempoEvent*>(event)->bpm;
			if (!(bpm > 0) || !std::isfinite(bpm))
				return kAudio_ParamError;
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(ExtendedTempoEvent));
			return noErr;
		}
		case kMusicEventType_AUPreset:
		{
			// GUESS: an AU preset event without a property list is kAudio_ParamError, as it names nothing.
			if (!static_cast<const AUPresetEvent*>(event)->preset)
				return kAudio_ParamError;
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(AUPresetEvent));
			return noErr;
		}
		case kMusicEventType_MIDIRawData:
			if (!copyVariable(offsetof(MIDIRawData, data), static_cast<const MIDIRawData*>(event)->length, 1, event, out))
				return kAudio_ParamError;
			return noErr;
		case kMusicEventType_User:
			if (!copyVariable(offsetof(MusicEventUserData, data), static_cast<const MusicEventUserData*>(event)->length, 1, event, out))
				return kAudio_ParamError;
			return noErr;
		case kMusicEventType_Meta:
			if (!copyVariable(offsetof(MIDIMetaEvent, data), static_cast<const MIDIMetaEvent*>(event)->dataLength, 1, event, out))
				return kAudio_ParamError;
			return noErr;
		case kMusicEventType_ExtendedNote:
		{
			// extendedParams.argCount counts the pitch and the velocity, so it is at least 2 and the
			// controls that follow number argCount - 2 (MusicDevice.h).
			const ExtendedNoteOnEvent* note = static_cast<const ExtendedNoteOnEvent*>(event);
			const UInt32 argCount = note->extendedParams.argCount;
			if (argCount < 2)
				return kAudio_ParamError;
			const size_t header = offsetof(ExtendedNoteOnEvent, extendedParams) + offsetof(MusicDeviceNoteParams, mControls);
			if (!copyVariable(header, argCount - 2, sizeof(NoteParamsControlValue), event, out))
				return kAudio_ParamError;
			// GUESS: the stored structure is never smaller than ExtendedNoteOnEvent, so a client may
			// always read it as one; the unused part is zero. The alternative was the exact size.
			if (out.size() < sizeof(ExtendedNoteOnEvent))
				out.resize(sizeof(ExtendedNoteOnEvent), 0);
			return noErr;
		}
		default:
			return kAudioToolboxErr_InvalidEventType;
	}
}

namespace
{

// The documentation says only that a time stamp should not be less than zero, not what happens
// otherwise. GUESS: a negative or non-finite time stamp is kAudio_ParamError; the alternative
// was to store the event anyway.
bool validTimeStamp(MusicTimeStamp time)
{
	return std::isfinite(time) && time >= 0;
}

OSStatus addEvent(MusicTrack track, MusicTimeStamp time, MusicEventType type, const void* event)
{
	if (!track || !validTimeStamp(time))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		std::vector<UInt8> data;
		OSStatus status = musicimpl::makeEventData(type, event, data);
		if (status != noErr)
			return status;
		musicimpl::insertEvent(track, MusicEvent(time, type, std::move(data)));
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}

}

OSStatus MusicTrackNewMIDINoteEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDINoteMessage* inMessage)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_MIDINoteMessage, inMessage);
}

OSStatus MusicTrackNewMIDIChannelEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIChannelMessage* inMessage)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_MIDIChannelMessage, inMessage);
}

OSStatus MusicTrackNewMIDIRawDataEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIRawData* inRawData)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_MIDIRawData, inRawData);
}

OSStatus MusicTrackNewExtendedNoteEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const ExtendedNoteOnEvent* inInfo)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_ExtendedNote, inInfo);
}

OSStatus MusicTrackNewParameterEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const ParameterEvent* inInfo)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_Parameter, inInfo);
}

OSStatus MusicTrackNewExtendedTempoEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, Float64 inBPM)
{
	ExtendedTempoEvent event = { inBPM };
	return addEvent(inTrack, inTimeStamp, kMusicEventType_ExtendedTempo, &event);
}

OSStatus MusicTrackNewMetaEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MIDIMetaEvent* inMetaEvent)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_Meta, inMetaEvent);
}

OSStatus MusicTrackNewUserEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const MusicEventUserData* inUserData)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_User, inUserData);
}

OSStatus MusicTrackNewAUPresetEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, const AUPresetEvent* inPresetEvent)
{
	return addEvent(inTrack, inTimeStamp, kMusicEventType_AUPreset, inPresetEvent);
}
