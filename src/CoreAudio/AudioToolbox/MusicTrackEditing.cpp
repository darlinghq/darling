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

// Music track editing (MusicPlayer.h): moving, clearing, cutting, copying and merging the events
// in a range of time.
//
// A range is [start, end): an event at the start is in it, an event at the end is not, except that
// kMusicTimeStamp_EndOfTrack as the end selects every event from the start on, the last one
// included. The documentation requires the start to be less than the end.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <new>
#include "MusicSequenceImpl.h"

namespace
{

bool validRange(MusicTimeStamp start, MusicTimeStamp end)
{
	// GUESS: a range whose start is not less than its end, that holds a time that is not a number,
	// or that starts before zero (no event is there, and a cut would move events below zero) is
	// kAudio_ParamError; the documentation says only that the start must be less than the end.
	return !std::isnan(start) && !std::isnan(end) && start >= 0 && start < end;
}

bool inRange(const MusicEvent& event, MusicTimeStamp start, MusicTimeStamp end)
{
	return event.time >= start && (end == kMusicTimeStamp_EndOfTrack || event.time < end);
}

// The events of the track in the range, as copies.
std::vector<MusicEvent> copyRange(const OpaqueMusicTrack* track, MusicTimeStamp start, MusicTimeStamp end)
{
	std::vector<MusicEvent> copies;
	for (const MusicEvent& event : track->events)
	{
		if (inRange(event, start, end))
			copies.push_back(event);
	}
	return copies;
}

// Puts copies of the events in a track; an event of the range start goes to the insertion time.
void placeCopies(OpaqueMusicTrack* dest, std::vector<MusicEvent>& copies, MusicTimeStamp rangeStart, MusicTimeStamp insertTime)
{
	for (MusicEvent& copy : copies)
	{
		copy.time = copy.time - rangeStart + insertTime;
		musicimpl::insertEvent(dest, std::move(copy));
	}
}

void eraseRange(OpaqueMusicTrack* track, MusicTimeStamp start, MusicTimeStamp end)
{
	std::vector<MusicEvent>& events = track->events;
	events.erase(std::remove_if(events.begin(), events.end(),
		[&](const MusicEvent& e) { return inRange(e, start, end); }), events.end());
}

}

OSStatus MusicTrackMoveEvents(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime, MusicTimeStamp inMoveTime)
{
	if (!inTrack || !validRange(inStartTime, inEndTime) || !std::isfinite(inMoveTime))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// GUESS: a move that would put an event before time zero is kAudio_ParamError and moves nothing;
	// the alternative was to clamp such events to zero.
	for (const MusicEvent& event : inTrack->events)
	{
		if (inRange(event, inStartTime, inEndTime) && event.time + inMoveTime < 0)
			return kAudio_ParamError;
	}

	for (MusicEvent& event : inTrack->events)
	{
		if (inRange(event, inStartTime, inEndTime))
			event.time += inMoveTime;
	}

	// GUESS: events that end up with the same time stay in the order they had before the move.
	std::stable_sort(inTrack->events.begin(), inTrack->events.end(),
		[](const MusicEvent& a, const MusicEvent& b) { return a.time < b.time; });
	return noErr;
}

OSStatus MusicTrackClear(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime)
{
	if (!inTrack || !validRange(inStartTime, inEndTime))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	eraseRange(inTrack, inStartTime, inEndTime);
	return noErr;
}

OSStatus MusicTrackCut(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime)
{
	if (!inTrack || !validRange(inStartTime, inEndTime))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	eraseRange(inTrack, inStartTime, inEndTime);

	// What follows the range moves back by the length of the range.
	if (inEndTime != kMusicTimeStamp_EndOfTrack)
	{
		for (MusicEvent& event : inTrack->events)
		{
			if (event.time >= inEndTime)
				event.time -= inEndTime - inStartTime;
		}
	}
	return noErr;
}

OSStatus MusicTrackCopyInsert(MusicTrack inSourceTrack, MusicTimeStamp inSourceStartTime, MusicTimeStamp inSourceEndTime,
	MusicTrack inDestTrack, MusicTimeStamp inDestInsertTime)
{
	// GUESS: an insertion time below zero or not finite is kAudio_ParamError, like the time stamp of
	// a new event.
	if (!inSourceTrack || !inDestTrack || !validRange(inSourceStartTime, inSourceEndTime) ||
		!std::isfinite(inDestInsertTime) || inDestInsertTime < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// GUESS: with kMusicTimeStamp_EndOfTrack as the end, the length of the range is the length of
	// the source track minus the start; it is 0 when the range holds only events at the end of the
	// track, which are copied all the same, as a merge copies them. The alternative was the time of
	// the last event.
	const MusicTimeStamp rangeLength = inSourceEndTime == kMusicTimeStamp_EndOfTrack
		? musicimpl::trackLength(inSourceTrack) - inSourceStartTime
		: inSourceEndTime - inSourceStartTime;

	try
	{
		// Copied before the destination changes, as the two tracks may be the same one.
		std::vector<MusicEvent> copies = copyRange(inSourceTrack, inSourceStartTime, inSourceEndTime);
		if (copies.empty() && !(rangeLength > 0))
			return noErr;

		// The events at and after the insertion time move later to make room.
		for (MusicEvent& event : inDestTrack->events)
		{
			if (event.time >= inDestInsertTime)
				event.time += rangeLength;
		}
		placeCopies(inDestTrack, copies, inSourceStartTime, inDestInsertTime);
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}

OSStatus MusicTrackMerge(MusicTrack inSourceTrack, MusicTimeStamp inSourceStartTime, MusicTimeStamp inSourceEndTime,
	MusicTrack inDestTrack, MusicTimeStamp inDestInsertTime)
{
	if (!inSourceTrack || !inDestTrack || !validRange(inSourceStartTime, inSourceEndTime) ||
		!std::isfinite(inDestInsertTime) || inDestInsertTime < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		// The events already in the destination stay where they are. GUESS: a copied event with the same
		// time as one of them goes after it.
		std::vector<MusicEvent> copies = copyRange(inSourceTrack, inSourceStartTime, inSourceEndTime);
		placeCopies(inDestTrack, copies, inSourceStartTime, inDestInsertTime);
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}
