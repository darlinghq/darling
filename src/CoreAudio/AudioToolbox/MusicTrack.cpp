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
#include <new>
#include "MusicSequenceImpl.h"

void musicimpl::insertEvent(OpaqueMusicTrack* track, MusicEvent&& event)
{
	auto pos = std::upper_bound(track->events.begin(), track->events.end(), event.time,
		[](MusicTimeStamp time, const MusicEvent& e) { return time < e.time; });
	track->events.insert(pos, std::move(event));
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

OSStatus addEvent(MusicTrack track, MusicTimeStamp time, MusicEventType type, const void* data, size_t size)
{
	if (!track || !data || !validTimeStamp(time))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		musicimpl::insertEvent(track, MusicEvent(time, type, data, size));
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
	return addEvent(inTrack, inTimeStamp, kMusicEventType_MIDINoteMessage, inMessage, sizeof(MIDINoteMessage));
}

OSStatus MusicTrackNewExtendedTempoEvent(MusicTrack inTrack, MusicTimeStamp inTimeStamp, Float64 inBPM)
{
	// GUESS: a tempo that is zero, negative or not a number cannot describe a rate of beats, so it is
	// kAudio_ParamError. The documentation does not name a valid range.
	if (!(inBPM > 0) || !std::isfinite(inBPM))
		return kAudio_ParamError;

	ExtendedTempoEvent event = { inBPM };
	return addEvent(inTrack, inTimeStamp, kMusicEventType_ExtendedTempo, &event, sizeof(event));
}
