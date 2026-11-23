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
// kMusicTimeStamp_EndOfTrack (the largest double) as the end selects every event from the start on.
// What is not documented follows macOS 26.7.3 and 10.14.4 (tests/src/audiotoolbox_music_track_editing.c):
// a start that is not a number or is below zero, or after the end, is kAudio_ParamError; a range that is
// empty (start equal to end) does nothing for Move, Clear and Cut and is kAudio_ParamError for CopyInsert and
// Merge, which also refuse a range that holds no events. A move may put events below zero or at a time that
// is not a number. Copy and Merge are given as much of the end of track as the end says: the length of the
// range is the end minus the start.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <new>
#include "MusicSequenceImpl.h"

namespace
{

bool validRange(MusicTimeStamp start, MusicTimeStamp end)
{
	return !std::isnan(start) && !std::isnan(end) && start >= 0 && !(start > end);
}

bool inRange(const MusicEvent& event, MusicTimeStamp start, MusicTimeStamp end)
{
	return event.time >= start && event.time < end;
}

// An event at the very end of the range is in it when the range reaches the end of the track.
bool inRangeToEnd(const MusicEvent& event, MusicTimeStamp start, MusicTimeStamp end)
{
	return end == kMusicTimeStamp_EndOfTrack ? event.time >= start : inRange(event, start, end);
}

// The events of the track in the range, as copies.
std::vector<MusicEvent> copyRange(const OpaqueMusicTrack* track, MusicTimeStamp start, MusicTimeStamp end)
{
	std::vector<MusicEvent> copies;
	for (const MusicEvent& event : track->events)
	{
		if (inRangeToEnd(event, start, end))
			copies.push_back(event);
	}
	return copies;
}

bool eventBefore(const MusicEvent& a, const MusicEvent& b)
{
	return musicimpl::timeBefore(a.time, b.time);
}

// Puts copies of the events in a track, before the events that already have the same time; an event
// at the range start goes to the insertion time. The copies keep their order among themselves.
void placeCopies(OpaqueMusicTrack* dest, std::vector<MusicEvent>& copies, MusicTimeStamp rangeStart, MusicTimeStamp insertTime)
{
	for (MusicEvent& copy : copies)
		copy.time = copy.time - rangeStart + insertTime;
	std::stable_sort(copies.begin(), copies.end(), eventBefore);
	std::vector<MusicEvent> merged;
	merged.reserve(dest->events.size() + copies.size());
	std::merge(std::make_move_iterator(copies.begin()), std::make_move_iterator(copies.end()),
		std::make_move_iterator(dest->events.begin()), std::make_move_iterator(dest->events.end()),
		std::back_inserter(merged), eventBefore);
	dest->events = std::move(merged);
}

void eraseRange(OpaqueMusicTrack* track, MusicTimeStamp start, MusicTimeStamp end)
{
	std::vector<MusicEvent>& events = track->events;
	events.erase(std::remove_if(events.begin(), events.end(),
		[&](const MusicEvent& e) { return inRangeToEnd(e, start, end); }), events.end());
}

}

OSStatus MusicTrackMoveEvents(MusicTrack inTrack, MusicTimeStamp inStartTime, MusicTimeStamp inEndTime, MusicTimeStamp inMoveTime)
{
	if (!inTrack || !validRange(inStartTime, inEndTime))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	for (MusicEvent& event : inTrack->events)
	{
		if (inRangeToEnd(event, inStartTime, inEndTime))
			event.time += inMoveTime;
	}

	// Events that end up with the same time stay in the order they had before the move.
	std::stable_sort(inTrack->events.begin(), inTrack->events.end(), eventBefore);
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
	// The insertion time must be a number and not below zero; the range must be a range (start before end).
	if (!inSourceTrack || !inDestTrack || !validRange(inSourceStartTime, inSourceEndTime) || inSourceStartTime == inSourceEndTime ||
		!(inDestInsertTime >= 0))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	try
	{
		// Copied before the destination changes, as the two tracks may be the same one. A range with no
		// events in it is kAudio_ParamError.
		std::vector<MusicEvent> copies = copyRange(inSourceTrack, inSourceStartTime, inSourceEndTime);
		if (copies.empty())
			return kAudio_ParamError;

		// The events at and after the insertion time move later to make room, by the length of the range
		// (with kMusicTimeStamp_EndOfTrack as the end that is as much as a double holds).
		const MusicTimeStamp rangeLength = inSourceEndTime - inSourceStartTime;
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
	// As for CopyInsert, except that any insertion time is accepted, below zero or not a number too.
	if (!inSourceTrack || !inDestTrack || !validRange(inSourceStartTime, inSourceEndTime) || inSourceStartTime == inSourceEndTime)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		// The events already in the destination stay where they are; a copied event goes before the
		// events that have its time.
		std::vector<MusicEvent> copies = copyRange(inSourceTrack, inSourceStartTime, inSourceEndTime);
		if (copies.empty())
			return kAudio_ParamError;
		placeCopies(inDestTrack, copies, inSourceStartTime, inDestInsertTime);
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}
