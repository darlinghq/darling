// Copyright (C) 2026 Lubos Dolezel
//
// The destinations of a MusicSequence and its tracks (MusicPlayer.h). A track sends its events to an
// audio unit node of the graph its sequence uses, or to a MIDI endpoint; MusicTrackGetDestNode on a
// track that has an endpoint (and the other way round) returns kAudioToolboxErr_IllegalTrackDestination;
// MusicSequenceSetMIDIEndpoint targets every track (not the tempo track) at the endpoint. The node
// has to be a member of the graph of the sequence. MusicSequenceSetUserCallback registers a callback
// that a NULL one removes. Nothing is played, so nothing reaches the node or the endpoint and the
// callback is not called. Checks marked GUESS pin what the documentation leaves open (a track with
// no destination, a node with no graph, a sequence with no graph); not observed on macOS.
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void callback(void* data, MusicSequence seq, MusicTrack track, MusicTimeStamp time,
	const MusicEventUserData* event, MusicTimeStamp start, MusicTimeStamp end)
{
	(void) data; (void) seq; (void) track; (void) time; (void) event; (void) start; (void) end;
}

int main(void)
{
	MusicSequence seq = NULL;
	MusicTrack a = NULL, b = NULL, tempo = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &a) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &b) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);

	// GUESS: a track with no destination reports kAudioToolboxError_NoTrackDestination.
	AUNode node = 0;
	MIDIEndpointRef endpoint = 0;
	CHECK(MusicTrackGetDestNode(a, &node) == kAudioToolboxError_NoTrackDestination);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == kAudioToolboxError_NoTrackDestination);

	// Endpoints.
	CHECK(MusicTrackSetDestMIDIEndpoint(a, 42) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 42);
	CHECK(MusicTrackGetDestNode(a, &node) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxError_NoTrackDestination);

	// GUESS: with no graph set no node is a member, and the code is kAUGraphErr_NodeNotFound.
	CHECK(MusicTrackSetDestNode(b, 1) == kAUGraphErr_NodeNotFound);
	CHECK(MusicTrackGetDestNode(b, &node) == kAudioToolboxError_NoTrackDestination);

	// GUESS: the default graph of a sequence with none set is not created; the call says so.
	AUGraph graph = (AUGraph) 1;
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == kAudio_UnimplementedError);
	CHECK(graph == NULL);

	// A graph and its nodes.
	AUGraph own = NULL;
	CHECK(NewAUGraph(&own) == noErr);
	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_GenericOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AUNode member = 0;
	CHECK(AUGraphAddNode(own, &desc, &member) == noErr);
	CHECK(MusicSequenceSetAUGraph(seq, own) == noErr);
	graph = NULL;
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == noErr && graph == own);

	CHECK(MusicTrackSetDestNode(b, member) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == member);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);
	// A node that is not in the graph is refused and the destination stays.
	CHECK(MusicTrackSetDestNode(a, member + 100) == kAUGraphErr_NodeNotFound);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 42);

	// Setting the destination of a track again replaces it, from a node to an endpoint too.
	CHECK(MusicTrackSetDestMIDIEndpoint(b, 7) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackSetDestNode(b, member) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);

	// The endpoint of a whole sequence: every track. GUESS: not the tempo track.
	CHECK(MusicSequenceSetMIDIEndpoint(seq, 99) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 99);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == noErr && endpoint == 99);
	CHECK(MusicTrackGetDestMIDIEndpoint(tempo, &endpoint) == kAudioToolboxError_NoTrackDestination);

	// Setting no graph goes back to the default one.
	CHECK(MusicSequenceSetAUGraph(seq, NULL) == noErr);
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == kAudio_UnimplementedError);

	// The user callback: registered and removed with no effect on anything that can be read back.
	CHECK(MusicSequenceSetUserCallback(seq, callback, &g_failures) == noErr);
	CHECK(MusicSequenceSetUserCallback(seq, NULL, NULL) == noErr);

	CHECK(MusicTrackSetDestNode(NULL, 1) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(a, NULL) == kAudio_ParamError);
	CHECK(MusicSequenceSetMIDIEndpoint(NULL, 1) == kAudio_ParamError);
	CHECK(MusicSequenceGetAUGraph(seq, NULL) == kAudio_ParamError);
	CHECK(MusicSequenceSetUserCallback(NULL, callback, NULL) == kAudio_ParamError);

	CHECK(DisposeMusicSequence(seq) == noErr);
	CHECK(DisposeAUGraph(own) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
