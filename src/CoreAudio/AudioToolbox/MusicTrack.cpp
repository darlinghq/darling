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
		[](MusicTimeStamp time, const MusicEvent& e) { return musicimpl::timeBefore(time, e.time); });
	pos = track->events.insert(pos, std::move(event));
	return static_cast<size_t>(pos - track->events.begin());
}

size_t musicimpl::moveEvent(OpaqueMusicTrack* track, size_t index, MusicTimeStamp time)
{
	// QUIRK: an event that is moved to a time that has events already goes before them when it moves
	// to a later time (or the same one) and after them when it moves to an earlier one; the other events
	// keep their order. Evidence: black-box observation on macOS 26.7.3 and 10.14.4, tier 2. Test:
	// tests/src/audiotoolbox_music_event_types.c, "SetEventTime ties".
	std::vector<MusicEvent>& events = track->events;
	MusicEvent event = std::move(events[index]);
	events.erase(events.begin() + index);
	const bool earlier = musicimpl::timeBefore(time, event.time);
	event.time = time;
	size_t pos = index;
	if (earlier)
	{
		while (pos > 0 && musicimpl::timeBefore(time, events[pos - 1].time))
			pos--;
	}
	else
	{
		while (pos < events.size() && musicimpl::timeBefore(events[pos].time, time))
			pos++;
	}
	events.insert(events.begin() + pos, std::move(event));
	return pos;
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
			// Any tempo is stored (macOS stores zero, negative and non-finite ones too; observed on 26.7.3 and 10.14.4).
			out.assign(static_cast<const UInt8*>(event), static_cast<const UInt8*>(event) + sizeof(ExtendedTempoEvent));
			return noErr;
		}
		case kMusicEventType_AUPreset:
		{
			// An AU preset event without a property list is kAudio_ParamError (observed on macOS 26.7.3).
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
			// extendedParams.argCount counts the pitch and the velocity, so the controls that follow number
			// argCount - 2 (MusicDevice.h), and the caller's structure holds at least that much, and
			// at least a whole ExtendedNoteOnEvent.
			//
			// QUIRK: the size of the event that is stored (and reported by MusicEventIteratorGetEventInfo) is 28
			// bytes plus 4 for each argument, for every argCount from 0 to 6 (36 for the pitch and velocity
			// alone, where sizeof(ExtendedNoteOnEvent) is 32, 44 for two controls, where the structure the
			// caller made is 40); the bytes past what the caller's structure holds are zero. Evidence:
			// black-box observation on macOS 26.7.3 and 10.14.4 x86_64, tier 2. Test:
			// tests/src/audiotoolbox_music_event_types.c, "stored size of an extended note".
			const ExtendedNoteOnEvent* note = static_cast<const ExtendedNoteOnEvent*>(event);
			const UInt32 argCount = note->extendedParams.argCount;
			if (argCount > (0xFFFFFFFFu - 28) / 8)
				return kAudio_ParamError;
			const size_t stored = 28 + static_cast<size_t>(argCount) * 4;
			const size_t header = offsetof(ExtendedNoteOnEvent, extendedParams) + offsetof(MusicDeviceNoteParams, mControls);
			const size_t held = std::max(sizeof(ExtendedNoteOnEvent),
				header + (argCount > 2 ? static_cast<size_t>(argCount - 2) * sizeof(NoteParamsControlValue) : 0));
			const UInt8* bytes = static_cast<const UInt8*>(event);
			out.assign(bytes, bytes + std::min(stored, held));
			out.resize(stored, 0);
			return noErr;
		}
		default:
			return kAudioToolboxErr_InvalidEventType;
	}
}

namespace
{

OSStatus addEvent(MusicTrack track, MusicTimeStamp time, MusicEventType type, const void* event)
{
	// The documentation says only that a time stamp should not be less than zero; a negative time, or
	// one that is not a number, is stored all the same (macOS 26.7.3 and 10.14.4 do so).
	if (!track)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		if (type == kMusicEventType_ExtendedTempo)
		{
			// QUIRK: a tempo event goes into the tempo track only: any other track gives
			// kAudioToolboxErr_IllegalTrackDestination; and in a sequence that is not of type beats, the tempo
			// track takes one tempo event only: a second one gives kAudioToolboxErr_InvalidSequenceType.
			// Evidence: black-box observation on macOS 26.7.3 and 10.14.4, tier 2. Test:
			// tests/src/audiotoolbox_music_event_iterator.c, "tempo events", and
			// tests/src/audiotoolbox_music_sequence_time.c.
			if (!track->sequence || track != &track->sequence->tempoTrack)
				return kAudioToolboxErr_IllegalTrackDestination;
			if (track->sequence->type != kMusicSequenceType_Beats)
			{
				for (const MusicEvent& e : track->events)
				{
					if (e.type == kMusicEventType_ExtendedTempo)
						return kAudioToolboxErr_InvalidSequenceType;
				}
			}
		}
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
