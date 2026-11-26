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
// The tempo map is the tempo events (kMusicEventType_ExtendedTempo) of the tempo track, in beats per
// minute; other events in that track are ignored. A tempo holds from its time until the next tempo
// event, so of two at the same time the later one holds, and the first tempo also holds before its time
// (120 when there is none). The sequence type does not change any of this: on macOS 26.7.3 and 10.14.4 the
// conversions of a "seconds" or "samples" sequence read the tempo events as beats per minute too
// (tests/src/audiotoolbox_music_sequence_time.c); the type only decides which tempo events can be added.

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

Float64 beatsPerSecond(Float64 tempo)
{
	return tempo / 60.0;
}

// The tempo before the first tempo event, and in a sequence without one, is the 120 beats per minute the
// documentation gives for a new sequence, for every sequence type (observed on macOS).
const Float64 kDefaultTempo = 120.0;

std::vector<TempoPoint> tempoMap(const OpaqueMusicSequence* sequence)
{
	std::vector<TempoPoint> map;
	for (const MusicEvent& event : sequence->tempoTrack.events)
	{
		if (event.type != kMusicEventType_ExtendedTempo)
			continue;
		ExtendedTempoEvent tempo;
		std::memcpy(&tempo, event.data.data(), sizeof(tempo));
		map.push_back({ event.time, beatsPerSecond(tempo.bpm) });
	}
	if (map.empty() || map.front().time > 0)
		map.insert(map.begin(), { 0.0, beatsPerSecond(kDefaultTempo) });
	return map;
}

}

OSStatus MusicSequenceSetSequenceType(MusicSequence inSequence, MusicSequenceType inType)
{
	if (!inSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// QUIRK: seconds and samples are refused while the tempo track has a tempo event
	// (kAudioToolboxErr_InvalidSequenceType); any other value, one that is none of the three types
	// too, is accepted and kept. Evidence: black-box observation on macOS 26.7.3 and 10.14.4, tier 2.
	// Test: tests/src/audiotoolbox_music_sequence_time.c, "sequence types".
	if (inType == kMusicSequenceType_Seconds || inType == kMusicSequenceType_Samples)
	{
		for (const MusicEvent& event : inSequence->tempoTrack.events)
		{
			if (event.type == kMusicEventType_ExtendedTempo)
				return kAudioToolboxErr_InvalidSequenceType;
		}
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

Float64 musicimpl::secondsForBeats(const OpaqueMusicSequence* sequence, MusicTimeStamp beats)
{
	if (std::isnan(beats))
		return beats;
	const std::vector<TempoPoint> map = tempoMap(sequence);
	if (beats < 0)
		return beats / map.front().beatsPerSecond;

	Float64 seconds = 0;
	for (size_t i = 0; i < map.size() && map[i].time < beats; i++)
	{
		// The last tempo holds for ever.
		MusicTimeStamp end = i + 1 < map.size() ? std::min(map[i + 1].time, beats) : beats;
		seconds += (end - map[i].time) / map[i].beatsPerSecond;
	}
	return seconds;
}

MusicTimeStamp musicimpl::beatsForSeconds(const OpaqueMusicSequence* sequence, Float64 seconds)
{
	if (std::isnan(seconds))
		return seconds;
	const std::vector<TempoPoint> map = tempoMap(sequence);
	if (seconds < 0)
		return seconds * map.front().beatsPerSecond;

	Float64 remaining = seconds;
	for (size_t i = 0; i < map.size(); i++)
	{
		const Float64 length = i + 1 < map.size() ? (map[i + 1].time - map[i].time) / map[i].beatsPerSecond : INFINITY;
		if (remaining <= length)
			return map[i].time + remaining * map[i].beatsPerSecond;
		remaining -= length;
	}
	return 0;
}

OSStatus MusicSequenceGetSecondsForBeats(MusicSequence inSequence, MusicTimeStamp inBeats, Float64* outSeconds)
{
	// A time below zero is kAudio_ParamError and leaves the output alone; one that is not a number or is
	// infinite is converted like any other (the result is then not a number or infinite), as on macOS.
	if (!inSequence || !outSeconds || inBeats < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		*outSeconds = musicimpl::secondsForBeats(inSequence, inBeats);
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}

OSStatus MusicSequenceGetBeatsForSeconds(MusicSequence inSequence, Float64 inSeconds, MusicTimeStamp* outBeats)
{
	if (!inSequence || !outBeats || inSeconds < 0)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	try
	{
		*outBeats = musicimpl::beatsForSeconds(inSequence, inSeconds);
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
	return noErr;
}
