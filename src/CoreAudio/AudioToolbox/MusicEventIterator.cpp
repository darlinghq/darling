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

// Music event iterators (MusicPlayer.h).
//
// An iterator holds a position among the events of one track: an event, or the place just beyond
// the last event. As the documentation says, a track that is edited invalidates its iterators; the
// iterator here keeps its index, so after an edit it still lies within the events of the track but
// may point at another event. An iterator holds a pointer to its track and does not outlive it:
// after MusicSequenceDisposeTrack or DisposeMusicSequence it must be disposed and not used.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <new>
#include "MusicSequenceImpl.h"

OSStatus NewMusicEventIterator(MusicTrack inTrack, MusicEventIterator* outIterator)
{
	if (!inTrack || !outIterator)
		return kAudio_ParamError;

	OpaqueMusicEventIterator* iterator = new (std::nothrow) OpaqueMusicEventIterator;
	if (!iterator)
		return kAudio_MemFullError;

	// A new iterator points at the first event.
	iterator->track = inTrack;
	iterator->index = 0;
	*outIterator = iterator;
	return noErr;
}

OSStatus DisposeMusicEventIterator(MusicEventIterator inIterator)
{
	if (!inIterator)
		return kAudio_ParamError;

	delete inIterator;
	return noErr;
}

OSStatus MusicEventIteratorSeek(MusicEventIterator inIterator, MusicTimeStamp inTimeStamp)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	const std::vector<MusicEvent>& events = inIterator->track->events;

	if (inTimeStamp == kMusicTimeStamp_EndOfTrack)
	{
		inIterator->index = events.size();
		return noErr;
	}

	// QUIRK: a time that is not a number is not refused: the iterator goes to the first event (observed on
	// macOS 26.7.3 and 10.14.4, tier 2; test: tests/src/audiotoolbox_music_event_iterator.c, "seek to NaN").
	if (std::isnan(inTimeStamp))
	{
		inIterator->index = 0;
		return noErr;
	}

	// The first event at the time, or else the first event after it; with several events at that
	// time, the first of them.
	auto pos = std::lower_bound(events.begin(), events.end(), inTimeStamp,
		[](const MusicEvent& e, MusicTimeStamp time) { return musicimpl::timeBefore(e.time, time); });
	inIterator->index = static_cast<size_t>(pos - events.begin());
	return noErr;
}

OSStatus MusicEventIteratorNextEvent(MusicEventIterator inIterator)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// Moving forward from the last event goes beyond it. Moving forward from beyond the last event is
	// kAudioToolboxErr_EndOfTrack and leaves the iterator where it is (observed on macOS 26.7.3 and 10.14.4).
	if (inIterator->index >= inIterator->track->events.size())
		return kAudioToolboxErr_EndOfTrack;

	inIterator->index++;
	return noErr;
}

OSStatus MusicEventIteratorPreviousEvent(MusicEventIterator inIterator)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// At the first event the iterator stays and the call fails with kAudioToolboxErr_StartOfTrack (observed on
	// macOS 26.7.3 and 10.14.4).
	if (inIterator->index == 0)
		return kAudioToolboxErr_StartOfTrack;

	inIterator->index--;
	return noErr;
}

OSStatus MusicEventIteratorGetEventInfo(MusicEventIterator inIterator, MusicTimeStamp* outTimeStamp,
	MusicEventType* outEventType, const void** outEventData, UInt32* outEventDataSize)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	const std::vector<MusicEvent>& events = inIterator->track->events;

	// There is no event to describe beyond the last event: kAudioToolboxErr_EndOfTrack, and the outputs are not
	// written (observed on macOS 26.7.3 and 10.14.4).
	if (inIterator->index >= events.size())
		return kAudioToolboxErr_EndOfTrack;

	const MusicEvent& event = events[inIterator->index];
	if (outTimeStamp)
		*outTimeStamp = event.time;
	if (outEventType)
		*outEventType = event.type;
	if (outEventData)
		*outEventData = event.data.data();
	if (outEventDataSize)
		*outEventDataSize = static_cast<UInt32>(event.data.size());
	return noErr;
}

OSStatus MusicEventIteratorHasPreviousEvent(MusicEventIterator inIterator, Boolean* outHasPrevEvent)
{
	if (!inIterator || !outHasPrevEvent)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outHasPrevEvent = inIterator->index > 0;
	return noErr;
}

OSStatus MusicEventIteratorHasNextEvent(MusicEventIterator inIterator, Boolean* outHasNextEvent)
{
	if (!inIterator || !outHasNextEvent)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outHasNextEvent = inIterator->index + 1 < inIterator->track->events.size();
	return noErr;
}

OSStatus MusicEventIteratorHasCurrentEvent(MusicEventIterator inIterator, Boolean* outHasCurEvent)
{
	if (!inIterator || !outHasCurEvent)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outHasCurEvent = inIterator->index < inIterator->track->events.size();
	return noErr;
}

OSStatus MusicEventIteratorSetEventInfo(MusicEventIterator inIterator, MusicEventType inEventType, const void* inEventData)
{
	if (!inIterator || !inEventData)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	std::vector<MusicEvent>& events = inIterator->track->events;

	// With no event at the iterator, the code is kAudioToolboxErr_EndOfTrack, as for GetEventInfo (observed).
	if (inIterator->index >= events.size())
		return kAudioToolboxErr_EndOfTrack;

	// The new type and data replace the old ones; the time stamp and the position stay.
	try
	{
		std::vector<UInt8> data;
		OSStatus status = musicimpl::makeEventData(inEventType, inEventData, data);
		if (status != noErr)
			return status;
		events[inIterator->index] = MusicEvent(events[inIterator->index].time, inEventType, std::move(data));
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}

OSStatus MusicEventIteratorSetEventTime(MusicEventIterator inIterator, MusicTimeStamp inTimeStamp)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	std::vector<MusicEvent>& events = inIterator->track->events;

	// QUIRK: with no current event the call does nothing and returns noErr (the other editing call that
	// needs an event, SetEventInfo, gives kAudioToolboxErr_EndOfTrack). A time below zero or not a number is
	// accepted. The iterator keeps its position, which after the move may hold another event. Evidence:
	// black-box observation on macOS 26.7.3 and 10.14.4, tier 2. Test:
	// tests/src/audiotoolbox_music_event_types.c, "SetEventTime".
	if (inIterator->index >= events.size())
		return noErr;

	musicimpl::moveEvent(inIterator->track, inIterator->index, inTimeStamp);
	return noErr;
}

OSStatus MusicEventIteratorDeleteEvent(MusicEventIterator inIterator)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	std::vector<MusicEvent>& events = inIterator->track->events;

	// QUIRK: with no current event the call does nothing and returns noErr (observed on macOS 26.7.3 and
	// 10.14.4, tier 2; test: tests/src/audiotoolbox_music_event_types.c, "DeleteEvent").
	if (inIterator->index >= events.size())
		return noErr;

	// The iterator then points at the event that followed, or beyond the last event.
	events.erase(events.begin() + inIterator->index);
	return noErr;
}
