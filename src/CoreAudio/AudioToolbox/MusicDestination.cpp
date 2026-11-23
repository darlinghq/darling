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

// Where the events of a sequence go (MusicPlayer.h): the audio processing graph of a sequence, the
// destination of each track (a node of that graph or a MIDI endpoint) and the callback for user
// events.
//
// These are stored and reported back; nothing plays, so nothing is sent to a node or an endpoint
// and the user callback is never called. A graph is not owned by the sequence: the caller keeps
// it alive for as long as the sequence has it.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include "MusicSequenceImpl.h"

OSStatus MusicSequenceSetAUGraph(MusicSequence inSequence, AUGraph inGraph)
{
	if (!inSequence)
		return kAudio_ParamError;

	// A NULL graph goes back to the default graph. The destinations that tracks have are not
	// checked again against a new graph.
	std::lock_guard<std::mutex> guard(musicimpl::lock());
	inSequence->graph = inGraph;
	return noErr;
}

OSStatus MusicSequenceGetAUGraph(MusicSequence inSequence, AUGraph* outGraph)
{
	if (!inSequence || !outGraph)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// The documentation says a sequence with no graph set creates a default one with a music device
	// and a dynamics processor. That graph is not created here, and the call says so.
	*outGraph = inSequence->graph;
	if (!inSequence->graph)
		return kAudio_UnimplementedError;
	return noErr;
}

OSStatus MusicSequenceSetMIDIEndpoint(MusicSequence inSequence, MIDIEndpointRef inEndpoint)
{
	if (!inSequence)
		return kAudio_ParamError;

	// The same as targeting every track of the sequence at the endpoint. GUESS: the tempo track is not
	// one of those tracks, as it is not among the tracks the other functions count and index (the
	// header says "all of the tracks"); the alternative was to include it.
	std::lock_guard<std::mutex> guard(musicimpl::lock());
	for (const std::unique_ptr<OpaqueMusicTrack>& track : inSequence->tracks)
	{
		track->destination = MusicDestination::endpoint;
		track->destinationEndpoint = inEndpoint;
	}
	return noErr;
}

OSStatus MusicSequenceSetUserCallback(MusicSequence inSequence, MusicSequenceUserCallback inCallback, void* inClientData)
{
	if (!inSequence)
		return kAudio_ParamError;

	// A NULL callback removes the registered one.
	std::lock_guard<std::mutex> guard(musicimpl::lock());
	inSequence->userCallback = inCallback;
	inSequence->userCallbackData = inCallback ? inClientData : nullptr;
	return noErr;
}

OSStatus MusicTrackSetDestNode(MusicTrack inTrack, AUNode inNode)
{
	if (!inTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// The node must be in the graph of the track's sequence. GUESS: with no graph set the node is in
	// none, so the code is the one for a node that is not found, kAUGraphErr_NodeNotFound, which is
	// also what the graph itself reports for a node that is not a member.
	if (!inTrack->sequence->graph)
		return kAUGraphErr_NodeNotFound;
	OSStatus status = AUGraphNodeInfo(inTrack->sequence->graph, inNode, nullptr, nullptr);
	if (status != noErr)
		return status;

	inTrack->destination = MusicDestination::node;
	inTrack->destinationNode = inNode;
	return noErr;
}

OSStatus MusicTrackSetDestMIDIEndpoint(MusicTrack inTrack, MIDIEndpointRef inEndpoint)
{
	if (!inTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	inTrack->destination = MusicDestination::endpoint;
	inTrack->destinationEndpoint = inEndpoint;
	return noErr;
}

OSStatus MusicTrackGetDestNode(MusicTrack inTrack, AUNode* outNode)
{
	if (!inTrack || !outNode)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// An endpoint destination is kAudioToolboxErr_IllegalTrackDestination, as documented. GUESS: a
	// track with no destination set is kAudioToolboxError_NoTrackDestination, the code the header
	// has for it; the documentation does not say which call reports it.
	switch (inTrack->destination)
	{
		case MusicDestination::node:
			*outNode = inTrack->destinationNode;
			return noErr;
		case MusicDestination::endpoint:
			return kAudioToolboxErr_IllegalTrackDestination;
		default:
			return kAudioToolboxError_NoTrackDestination;
	}
}

OSStatus MusicTrackGetDestMIDIEndpoint(MusicTrack inTrack, MIDIEndpointRef* outEndpoint)
{
	if (!inTrack || !outEndpoint)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	switch (inTrack->destination)
	{
		case MusicDestination::endpoint:
			*outEndpoint = inTrack->destinationEndpoint;
			return noErr;
		case MusicDestination::node:
			return kAudioToolboxErr_IllegalTrackDestination;
		default:
			return kAudioToolboxError_NoTrackDestination;
	}
}
