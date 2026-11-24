// Copyright (C) 2026 Lubos Dolezel
//
// The destinations of a MusicSequence and its tracks (MusicPlayer.h). A track sends its events to an
// audio unit node of the graph its sequence uses, or to a MIDI endpoint; MusicTrackGetDestNode on a
// track that has an endpoint (and the other way round) returns kAudioToolboxErr_IllegalTrackDestination;
// MusicSequenceSetMIDIEndpoint targets every track (not the tempo track) at the endpoint. The node
// has to be a member of the graph of the sequence. MusicSequenceSetUserCallback registers a callback
// that a NULL one removes. Nothing is played, so nothing reaches the node or the endpoint and the
// callback is not called. A sequence that has no graph of its own set creates a default one of a DLS
// music device, a dynamics processor and the default output, in that order. What the documentation
// leaves open (a track with no destination, a node that is not a member, what setting a graph does to
// the destinations) is what macOS 26.7.3 and 10.14.4 do, observed with these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void callback(void* data, MusicSequence seq, MusicTrack track, MusicTimeStamp time,
	const MusicEventUserData* event, MusicTimeStamp start, MusicTimeStamp end)
{
	(void) data; (void) seq; (void) track; (void) time; (void) event; (void) start; (void) end;
}

static int hasDescription(AUGraph graph, AUNode node, OSType type, OSType subType)
{
	AudioComponentDescription desc;
	return AUGraphNodeInfo(graph, node, &desc, NULL) == noErr && desc.componentType == type
		&& desc.componentSubType == subType && desc.componentManufacturer == kAudioUnitManufacturer_Apple;
}

int main(void)
{
	MusicSequence seq = NULL;
	MusicTrack a = NULL, b = NULL, tempo = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &a) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &b) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);

	// A track with no destination reports the node -1 (and, with it, no endpoint).
	AUNode node = 0;
	MIDIEndpointRef endpoint = 0;
	CHECK(MusicTrackGetDestNode(a, &node) == noErr && node == -1);
	CHECK(MusicTrackGetDestNode(tempo, &node) == noErr && node == -1);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackGetDestMIDIEndpoint(tempo, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);

	// Endpoints.
	CHECK(MusicTrackSetDestMIDIEndpoint(a, 42) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 42);
	CHECK(MusicTrackGetDestNode(a, &node) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);

	// The default graph: a DLS music device (node 1), the default output (2) and a dynamics processor (3), the device
	// connected to the processor and that to the output; not open, and the same graph on the next call.
	AUGraph graph = NULL;
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == noErr && graph != NULL);
	UInt32 count = 0;
	CHECK(AUGraphGetNodeCount(graph, &count) == noErr && count == 3);
	CHECK(hasDescription(graph, 1, kAudioUnitType_MusicDevice, kAudioUnitSubType_DLSSynth));
	CHECK(hasDescription(graph, 2, kAudioUnitType_Output, kAudioUnitSubType_DefaultOutput));
	CHECK(hasDescription(graph, 3, kAudioUnitType_Effect, kAudioUnitSubType_DynamicsProcessor));
	CHECK(AUGraphGetNumberOfInteractions(graph, &count) == noErr && count == 2);
	AUNodeInteraction first, second;
	CHECK(AUGraphGetInteractionInfo(graph, 0, &first) == noErr && AUGraphGetInteractionInfo(graph, 1, &second) == noErr);
	CHECK(first.nodeInteractionType == kAUNodeInteraction_Connection && second.nodeInteractionType == kAUNodeInteraction_Connection);
	CHECK(first.nodeInteraction.connection.sourceNode == 1 && first.nodeInteraction.connection.destNode == 3);
	CHECK(second.nodeInteraction.connection.sourceNode == 3 && second.nodeInteraction.connection.destNode == 2);
	Boolean isOpen = 1;
	CHECK(AUGraphIsOpen(graph, &isOpen) == noErr && isOpen == 0);
	AUGraph again = NULL;
	CHECK(MusicSequenceGetAUGraph(seq, &again) == noErr && again == graph);

	// Nodes of the default graph are destinations. QUIRK test: a node that is not a member is kAudio_ParamError, and
	// is the destination all the same.
	CHECK(MusicTrackSetDestNode(b, 3) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == 3);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackSetDestNode(b, 99) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == 99);
	CHECK(MusicTrackSetDestNode(b, 0) == kAudio_ParamError);
	CHECK(MusicTrackSetDestNode(b, -1) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == -1);
	CHECK(MusicTrackSetDestNode(b, 2) == noErr);

	// Setting the destination of a track again replaces it, from a node to an endpoint too.
	CHECK(MusicTrackSetDestMIDIEndpoint(b, 7) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == kAudioToolboxErr_IllegalTrackDestination);
	CHECK(MusicTrackSetDestNode(b, 1) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);

	// A graph of the client. QUIRK test: the destinations of the tracks stay, a node that is not a member of the
	// graph and an endpoint too.
	AUGraph own = NULL;
	CHECK(NewAUGraph(&own) == noErr);
	AudioComponentDescription desc = { kAudioUnitType_Output, kAudioUnitSubType_GenericOutput, kAudioUnitManufacturer_Apple, 0, 0 };
	AUNode member = 0;
	CHECK(AUGraphAddNode(own, &desc, &member) == noErr && member == 1);
	CHECK(MusicTrackSetDestNode(b, 99) == kAudio_ParamError);
	CHECK(MusicSequenceSetAUGraph(seq, own) == noErr);
	graph = NULL;
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == noErr && graph == own);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == 99);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 42);
	CHECK(MusicTrackSetDestNode(b, member) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == member);
	CHECK(MusicTrackSetDestNode(a, member + 100) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(a, &node) == noErr && node == member + 100);
	CHECK(MusicTrackSetDestMIDIEndpoint(a, 42) == noErr);

	// The endpoint of a whole sequence: every track that there is. Not the tempo track, and not a track made later.
	CHECK(MusicSequenceSetMIDIEndpoint(seq, 99) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(a, &endpoint) == noErr && endpoint == 99);
	CHECK(MusicTrackGetDestMIDIEndpoint(b, &endpoint) == noErr && endpoint == 99);
	CHECK(MusicTrackGetDestMIDIEndpoint(tempo, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);
	MusicTrack later = NULL;
	CHECK(MusicSequenceNewTrack(seq, &later) == noErr);
	CHECK(MusicTrackGetDestMIDIEndpoint(later, &endpoint) == kAudioToolboxErr_IllegalTrackDestination);

	// Setting no graph goes back to a default one (a new one), and drops the node destinations.
	CHECK(MusicTrackSetDestNode(b, member) == noErr);
	CHECK(MusicSequenceSetAUGraph(seq, NULL) == noErr);
	CHECK(MusicTrackGetDestNode(b, &node) == noErr && node == -1);
	graph = NULL;
	CHECK(MusicSequenceGetAUGraph(seq, &graph) == noErr && graph != NULL && graph != own);
	CHECK(AUGraphGetNodeCount(graph, &count) == noErr && count == 3);

	// The user callback: registered and removed with no effect on anything that can be read back.
	CHECK(MusicSequenceSetUserCallback(seq, callback, &g_failures) == noErr);
	CHECK(MusicSequenceSetUserCallback(seq, NULL, NULL) == noErr);

	CHECK(MusicTrackSetDestNode(NULL, 1) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(a, NULL) == kAudio_ParamError);
	CHECK(MusicTrackGetDestNode(NULL, &node) == kAudio_ParamError);
	CHECK(MusicSequenceSetMIDIEndpoint(NULL, 1) == kAudio_ParamError);
	CHECK(MusicSequenceGetAUGraph(seq, NULL) == kAudio_ParamError);
	CHECK(MusicSequenceGetAUGraph(NULL, &graph) == kAudio_ParamError);
	CHECK(MusicSequenceSetAUGraph(NULL, NULL) == kAudio_ParamError);
	CHECK(MusicSequenceSetUserCallback(NULL, callback, NULL) == kAudio_ParamError);
	CHECK(MusicTrackSetDestMIDIEndpoint(NULL, 1) == kAudio_ParamError);

	// The default graph goes with its sequence; one the client made does not
	CHECK(DisposeMusicSequence(seq) == noErr);
	CHECK(DisposeAUGraph(own) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
