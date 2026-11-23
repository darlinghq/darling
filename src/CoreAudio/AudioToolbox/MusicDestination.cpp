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
// and the user callback is never called. A sequence that has no graph of its own set gets a default
// one the first time it is needed, as on macOS: a music device (DLS synth), a dynamics processor and
// the default output, connected in that order. A graph set by the client is not owned by the sequence
// (the caller keeps it alive); the default graph is, and goes with the sequence.
//
// What macOS does where the header is silent (macOS 26.7.3 and 10.14.4, tests/src/audiotoolbox_music_destinations.c):
// a track that has no destination set reports the node -1 (kAUNodeInvalid) and kAudioToolboxErr_IllegalTrackDestination
// as its endpoint; the other kind of destination is kAudioToolboxErr_IllegalTrackDestination too; a node that is
// not in the graph is kAudio_ParamError, but is kept as the destination all the same; setting a graph keeps the
// destinations of the tracks, and setting none (going back to a default graph) drops the nodes.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include "MusicSequenceImpl.h"

OSStatus musicimpl::graphOf(OpaqueMusicSequence* sequence, AUGraph* outGraph)
{
	if (!sequence->graph)
	{
		AUGraph graph = nullptr;
		OSStatus status = NewAUGraph(&graph);
		if (status != noErr)
			return status;

		AudioComponentDescription synth = { kAudioUnitType_MusicDevice, kAudioUnitSubType_DLSSynth, kAudioUnitManufacturer_Apple, 0, 0 };
		AudioComponentDescription output = { kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput, kAudioUnitManufacturer_Apple, 0, 0 };
		AudioComponentDescription dynamics = { kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor, kAudioUnitManufacturer_Apple, 0, 0 };
		AUNode synthNode = 0, outputNode = 0, dynamicsNode = 0;
		status = AUGraphAddNode(graph, &synth, &synthNode);
		if (status == noErr)
			status = AUGraphAddNode(graph, &output, &outputNode);
		if (status == noErr)
			status = AUGraphAddNode(graph, &dynamics, &dynamicsNode);
		if (status == noErr)
			status = AUGraphConnectNodeInput(graph, synthNode, 0, dynamicsNode, 0);
		if (status == noErr)
			status = AUGraphConnectNodeInput(graph, dynamicsNode, 0, outputNode, 0);
		if (status != noErr)
		{
			DisposeAUGraph(graph);
			return status;
		}
		sequence->graph = graph;
		sequence->graphOwned = true;
	}
	*outGraph = sequence->graph;
	return noErr;
}

bool musicimpl::nodeInGraph(OpaqueMusicSequence* sequence, AUNode node)
{
	AUGraph graph = nullptr;
	return graphOf(sequence, &graph) == noErr && AUGraphNodeInfo(graph, node, nullptr, nullptr) == noErr;
}

OSStatus MusicSequenceSetAUGraph(MusicSequence inSequence, AUGraph inGraph)
{
	if (!inSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A NULL graph goes back to the default graph, which is made again when it is needed.
	if (inSequence->graph && inSequence->graphOwned && inSequence->graph != inGraph)
		DisposeAUGraph(inSequence->graph);
	inSequence->graph = inGraph;
	inSequence->graphOwned = false;

	// QUIRK: the destinations of the tracks stay when a graph is set, a node that is not a member of it too; with
	// none (NULL) the node destinations are dropped. Evidence: black-box observation on macOS 26.7.3 and
	// 10.14.4, tier 2. Test: tests/src/audiotoolbox_music_destinations.c, "a graph of the client".
	if (!inGraph)
	{
		auto drop = [](OpaqueMusicTrack& track) {
			if (track.destination == MusicDestination::node)
			{
				track.destination = MusicDestination::none;
				track.destinationNode = -1;
			}
		};
		drop(inSequence->tempoTrack);
		for (const std::unique_ptr<OpaqueMusicTrack>& track : inSequence->tracks)
			drop(*track);
	}
	return noErr;
}

OSStatus MusicSequenceGetAUGraph(MusicSequence inSequence, AUGraph* outGraph)
{
	if (!inSequence || !outGraph)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	return musicimpl::graphOf(inSequence, outGraph);
}

OSStatus MusicSequenceSetMIDIEndpoint(MusicSequence inSequence, MIDIEndpointRef inEndpoint)
{
	if (!inSequence)
		return kAudio_ParamError;

	// The same as targeting every track of the sequence at the endpoint. The tempo track is not one of those
	// tracks, and tracks made later are not either (observed on macOS 26.7.3 and 10.14.4).
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

	// QUIRK: the node is kept as the destination even when it is not a member of the graph of the track's
	// sequence, in which case the result is kAudio_ParamError. Evidence: black-box observation on macOS
	// 26.7.3 and 10.14.4, tier 2. Test: tests/src/audiotoolbox_music_destinations.c, "a node that is not a member".
	inTrack->destination = MusicDestination::node;
	inTrack->destinationNode = inNode;
	return musicimpl::nodeInGraph(inTrack->sequence, inNode) ? noErr : kAudio_ParamError;
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

	// A track with an endpoint destination: kAudioToolboxErr_IllegalTrackDestination, as documented. One
	// with no destination set reports the node -1.
	if (inTrack->destination == MusicDestination::endpoint)
		return kAudioToolboxErr_IllegalTrackDestination;
	*outNode = inTrack->destinationNode;
	return noErr;
}

OSStatus MusicTrackGetDestMIDIEndpoint(MusicTrack inTrack, MIDIEndpointRef* outEndpoint)
{
	if (!inTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A track whose destination is a node, or that has none (macOS reports no destination as a node):
	// kAudioToolboxErr_IllegalTrackDestination.
	if (inTrack->destination != MusicDestination::endpoint)
		return kAudioToolboxErr_IllegalTrackDestination;
	if (!outEndpoint)
		return kAudio_ParamError;
	*outEndpoint = inTrack->destinationEndpoint;
	return noErr;
}
