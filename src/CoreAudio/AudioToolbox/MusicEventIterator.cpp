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
	// GUESS: a time that is not a number is kAudio_ParamError; the documentation does not cover it.
	if (!inIterator || std::isnan(inTimeStamp))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	const std::vector<MusicEvent>& events = inIterator->track->events;

	if (inTimeStamp == kMusicTimeStamp_EndOfTrack)
	{
		inIterator->index = events.size();
		return noErr;
	}

	// The first event at the time, or else the first event after it; with several events at that
	// time, the first of them.
	auto pos = std::lower_bound(events.begin(), events.end(), inTimeStamp,
		[](const MusicEvent& e, MusicTimeStamp time) { return e.time < time; });
	inIterator->index = static_cast<size_t>(pos - events.begin());
	return noErr;
}

OSStatus MusicEventIteratorNextEvent(MusicEventIterator inIterator)
{
	if (!inIterator)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// Moving forward from the last event goes beyond it. GUESS: moving forward from beyond the last
	// event is kAudioToolboxErr_EndOfTrack and leaves the iterator where it is; the documentation
	// does not cover it. The alternative was noErr.
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

	// At the first event the iterator stays and the call fails. GUESS: the code is
	// kAudioToolboxErr_StartOfTrack; the documentation says only "an error".
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

	// GUESS: there is no event to describe beyond the last event; the code is
	// kAudioToolboxErr_EndOfTrack. The documentation does not cover it.
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
