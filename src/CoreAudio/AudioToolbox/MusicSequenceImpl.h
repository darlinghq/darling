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

#ifndef MUSICSEQUENCEIMPL_H
#define MUSICSEQUENCEIMPL_H

#include <AudioToolbox/MusicPlayer.h>
#include <cstring>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// The data model behind MusicSequence, MusicTrack, MusicEventIterator and MusicPlayer. A
// sequence owns its tempo track and its other tracks; a track points back to the sequence it
// belongs to. Every entry point holds musicimpl::lock() for its whole duration.

struct OpaqueMusicSequence;

// One event of a track. The data is the event structure in the form MusicEventIteratorGetEventInfo
// hands out (for example a MIDINoteMessage, or a MIDIRawData followed by its bytes). The property
// list of an AUPreset event is retained for as long as the event lives.
class MusicEvent
{
public:
	MusicEvent(MusicTimeStamp time, MusicEventType type, const void* data, size_t size)
		: time(time), type(type), data(static_cast<const UInt8*>(data), static_cast<const UInt8*>(data) + size)
	{
		retainPreset();
	}

	MusicEvent(MusicTimeStamp time, MusicEventType type, std::vector<UInt8>&& data)
		: time(time), type(type), data(std::move(data))
	{
		retainPreset();
	}

	MusicEvent(const MusicEvent& other) : time(other.time), type(other.type), data(other.data)
	{
		retainPreset();
	}

	MusicEvent(MusicEvent&& other) noexcept : time(other.time), type(other.type), data(std::move(other.data))
	{
		other.type = kMusicEventType_NULL;
	}

	MusicEvent& operator=(MusicEvent other) noexcept
	{
		std::swap(time, other.time);
		std::swap(type, other.type);
		data.swap(other.data);
		return *this;
	}

	~MusicEvent()
	{
		if (type == kMusicEventType_AUPreset && preset())
			CFRelease(preset());
	}

	MusicTimeStamp time;
	MusicEventType type;
	std::vector<UInt8> data;

private:
	CFPropertyListRef preset() const
	{
		AUPresetEvent event;
		std::memcpy(&event, data.data(), sizeof(event));
		return event.preset;
	}

	void retainPreset() const
	{
		if (type == kMusicEventType_AUPreset && preset())
			CFRetain(preset());
	}
};

// Where a track sends its events: an audio unit node of the graph of the sequence, or a MIDI
// endpoint. A track that has none set reports the node kAUNodeInvalid (-1) as its destination.
enum class MusicDestination { none, node, endpoint };

struct OpaqueMusicTrack
{
	OpaqueMusicSequence* sequence = nullptr;

	MusicDestination destination = MusicDestination::none;
	AUNode destinationNode = -1;
	MIDIEndpointRef destinationEndpoint = 0;

	// The track properties of MusicTrackSetProperty. The length is only used when it was set.
	MusicTrackLoopInfo loop = { 0, 1 };
	MusicTimeStamp offsetTime = 0;
	Boolean muted = false;
	Boolean soloed = false;
	UInt32 automatedParameters = 0;
	bool hasLength = false;
	MusicTimeStamp length = 0;

	// In time order; events with the same time are in the order they were added.
	std::vector<MusicEvent> events;
};

// A position among the events of one track: index is the position of the current event, and the
// number of events when the iterator is beyond the last one.
struct OpaqueMusicEventIterator
{
	OpaqueMusicTrack* track = nullptr;
	size_t index = 0;
};

struct OpaqueMusicSequence
{
	OpaqueMusicSequence();

	OpaqueMusicTrack tempoTrack;

	// The number of players that have the sequence set; it cannot be disposed while there is one.
	int playerCount = 0;

	// The graph set by the client (not owned), or the default graph made on demand (owned, see
	// musicimpl::graphOf), and the callback for user events with its data.
	AUGraph graph = nullptr;
	bool graphOwned = false;
	MusicSequenceUserCallback userCallback = nullptr;
	void* userCallbackData = nullptr;

	// The sequence type; beats by default. Any value that was set is kept.
	MusicSequenceType type = kMusicSequenceType_Beats;

	// The ticks per quarter note of a file made from the sequence: 480 for a sequence made by hand.
	SInt16 timeResolution = 480;
	std::vector<std::unique_ptr<OpaqueMusicTrack>> tracks;
};

// A player: the sequence it plays and a position in beats. A stopped player stays at its position.
// A playing one advances from the anchor, the host time at which the position was last set: the
// position is where the sequence is after the seconds that passed since then, scaled by the rate.
struct OpaqueMusicPlayer
{
	OpaqueMusicSequence* sequence = nullptr;
	bool playing = false;
	MusicTimeStamp time = 0;
	UInt64 anchorHostTime = 0;
	Float64 rate = 1.0;
};

namespace musicimpl
{

inline std::mutex& lock()
{
	static std::mutex m;
	return m;
}

// The seconds at a position of the sequence in beats, and the position at a number of seconds,
// through the tempo map. The tempo of the first tempo event also holds before zero.
Float64 secondsForBeats(const OpaqueMusicSequence* sequence, MusicTimeStamp beats);
MusicTimeStamp beatsForSeconds(const OpaqueMusicSequence* sequence, Float64 seconds);

// The order of the events of a track: by time, with a time that is not a number after all others.
inline bool timeBefore(MusicTimeStamp a, MusicTimeStamp b)
{
	if (b != b)
		return a == a;
	return a < b;
}

// Inserts an event into the track, after the events that have the same time, and returns its
// position.
size_t insertEvent(OpaqueMusicTrack* track, MusicEvent&& event);

// Gives the event at index a new time and puts it where that time belongs: when the time is not
// earlier, before the events that have it, otherwise after them. Returns its new position.
size_t moveEvent(OpaqueMusicTrack* track, size_t index, MusicTimeStamp time);

// The graph of the sequence: the one set by the client, or else the default graph (a music device,
// a dynamics processor and the default output, made when first needed and owned by the sequence).
OSStatus graphOf(OpaqueMusicSequence* sequence, AUGraph* outGraph);

// Whether the node is a member of the graph of the sequence.
bool nodeInGraph(OpaqueMusicSequence* sequence, AUNode node);

// The length of a track: the end of its last event (a note lasts for its duration), or the length
// set on the track if that is greater.
MusicTimeStamp trackLength(const OpaqueMusicTrack* track);

// Validates an event of the given type and copies its structure, including the bytes that follow
// a variable length one, into out. The result is kAudioToolboxErr_InvalidEventType for a type
// that has no event structure and kAudio_ParamError for a structure that cannot be stored.
OSStatus makeEventData(MusicEventType type, const void* event, std::vector<UInt8>& out);

}

#endif
