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

// The time of a music sequence (MusicPlayer.h): the sequence type and the conversion between beats
// and seconds through the tempo map.
//
// The tempo map is the tempo events (kMusicEventType_ExtendedTempo) of the tempo track; other
// events in that track are ignored. A tempo holds from its time until the next tempo event, so of
// two at the same time the later one holds. With
// the sequence type "beats" a tempo is in beats per minute. With "seconds" the documentation says
// the tempo is 60 and a beat is a second, and with "samples" that it is a sample rate and a beat a
// sample. Both follow from reading the tempo as beats per minute and as beats per second.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>
#include "MusicSequenceImpl.h"

namespace
{

// From time (in beats) on, the sequence advances at beatsPerSecond.
struct TempoPoint
{
	MusicTimeStamp time;
	Float64 beatsPerSecond;
};

Float64 beatsPerSecond(MusicSequenceType type, Float64 tempo)
{
	return type == kMusicSequenceType_Samples ? tempo : tempo / 60.0;
}

// GUESS: the tempo before the first tempo event, and in a sequence without one, is the 120 beats
// per minute the documentation gives for a new sequence; a "seconds" sequence has 60 (a beat is a
// second) and a "samples" sequence 44100 (the documentation's own example of a sample rate).
Float64 defaultTempo(MusicSequenceType type)
{
	switch (type)
	{
		case kMusicSequenceType_Seconds: return 60.0;
		case kMusicSequenceType_Samples: return 44100.0;
		default: return 120.0;
	}
}

std::vector<TempoPoint> tempoMap(const OpaqueMusicSequence* sequence)
{
	std::vector<TempoPoint> map;
	for (const MusicEvent& event : sequence->tempoTrack.events)
	{
		if (event.type != kMusicEventType_ExtendedTempo)
			continue;
		ExtendedTempoEvent tempo;
		std::memcpy(&tempo, event.data.data(), sizeof(tempo));
		map.push_back({ event.time, beatsPerSecond(sequence->type, tempo.bpm) });
	}
	if (map.empty() || map.front().time > 0)
		map.insert(map.begin(), { 0.0, beatsPerSecond(sequence->type, defaultTempo(sequence->type)) });
	return map;
}

}

OSStatus MusicSequenceSetSequenceType(MusicSequence inSequence, MusicSequenceType inType)
{
	if (!inSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	switch (inType)
	{
		case kMusicSequenceType_Beats:
			break;
		case kMusicSequenceType_Seconds:
		case kMusicSequenceType_Samples:
			// Only while the sequence has no tempo events. GUESS: the code for refusing is
			// kAudioToolboxErr_InvalidSequenceType, as for a type that is none of the three.
			for (const MusicEvent& event : inSequence->tempoTrack.events)
			{
				if (event.type == kMusicEventType_ExtendedTempo)
					return kAudioToolboxErr_InvalidSequenceType;
			}
			break;
		default:
			return kAudioToolboxErr_InvalidSequenceType;
	}

	inSequence->type = inType;
	return noErr;
}

OSStatus MusicSequenceGetSequenceType(MusicSequence inSequence, MusicSequenceType* outType)
{
	if (!inSequence || !outType)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outType = inSequence->type;
	return noErr;
}

OSStatus MusicSequenceGetSecondsForBeats(MusicSequence inSequence, MusicTimeStamp inBeats, Float64* outSeconds)
{
	// GUESS: a time below zero or not finite is kAudio_ParamError, like a time stamp of an event.
	if (!inSequence || !outSeconds || !std::isfinite(inBeats) || inBeats < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		const std::vector<TempoPoint> map = tempoMap(inSequence);
		Float64 seconds = 0;
		for (size_t i = 0; i < map.size() && map[i].time < inBeats; i++)
		{
			// The last tempo holds for ever.
			MusicTimeStamp end = i + 1 < map.size() ? std::min(map[i + 1].time, inBeats) : inBeats;
			seconds += (end - map[i].time) / map[i].beatsPerSecond;
		}
		*outSeconds = seconds;
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}

OSStatus MusicSequenceGetBeatsForSeconds(MusicSequence inSequence, Float64 inSeconds, MusicTimeStamp* outBeats)
{
	if (!inSequence || !outBeats || !std::isfinite(inSeconds) || inSeconds < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		const std::vector<TempoPoint> map = tempoMap(inSequence);
		Float64 remaining = inSeconds;
		MusicTimeStamp beats = 0;
		for (size_t i = 0; i < map.size(); i++)
		{
			const MusicTimeStamp start = map[i].time;
			const Float64 length = i + 1 < map.size() ? (map[i + 1].time - start) / map[i].beatsPerSecond : INFINITY;
			if (remaining <= length)
			{
				beats = start + remaining * map[i].beatsPerSecond;
				break;
			}
			remaining -= length;
		}
		*outBeats = beats;
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}
