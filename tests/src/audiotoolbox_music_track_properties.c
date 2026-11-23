// Copyright (C) 2026 Lubos Dolezel
//
// MusicTrackSetProperty and MusicTrackGetProperty (MusicPlayer.h). Property values are read and
// written by reference and have fixed sizes; a Get without a buffer reports the size. Defaults:
// one loop repetition of no duration, offset 0, not muted, not soloed, no automated parameters; the time resolution of
// the tempo track is 480 for a sequence made by hand. The track length is the end of the last
// event (a note lasts for its duration) or the length set on the track, whichever is greater. A
// what the documentation leaves open (the code for a refused property, size or value, the size that
// is passed in) is what macOS 26.7.3 and 10.14.4 do, observed with these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static OSStatus getValue(MusicTrack track, UInt32 prop, void* value, UInt32 size)
{
	UInt32 length = size;
	OSStatus status = MusicTrackGetProperty(track, prop, value, &length);
	if (status == noErr)
		CHECK(length == size);
	return status;
}

static MusicTimeStamp trackLength(MusicTrack track)
{
	MusicTimeStamp length = -1;
	CHECK(getValue(track, kSequenceTrackProperty_TrackLength, &length, sizeof(length)) == noErr);
	return length;
}

int main(void)
{
	MusicSequence seq = NULL;
	MusicTrack track = NULL, tempo = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &track) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);

	// With no buffer the size is reported.
	UInt32 length = 0;
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_LoopInfo, NULL, &length) == noErr);
	CHECK(length == sizeof(MusicTrackLoopInfo));
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_OffsetTime, NULL, &length) == noErr);
	CHECK(length == sizeof(MusicTimeStamp));
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_MuteStatus, NULL, &length) == noErr);
	CHECK(length == sizeof(Boolean));
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_AutomatedParameters, NULL, &length) == noErr);
	CHECK(length == sizeof(UInt32));
	CHECK(MusicTrackGetProperty(tempo, kSequenceTrackProperty_TimeResolution, NULL, &length) == noErr);
	CHECK(length == sizeof(SInt16));

	// Defaults.
	MusicTrackLoopInfo loop = { 9, 9 };
	CHECK(getValue(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == noErr);
	// the default loop is no loop duration and one repetition (observed on macOS 26.7.3 and 10.14.4)
	CHECK(loop.loopDuration == 0 && loop.numberOfLoops == 1);
	MusicTimeStamp offset = 9;
	CHECK(getValue(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr && offset == 0);
	Boolean flag = 9;
	CHECK(getValue(track, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr && flag == 0);
	flag = 9;
	CHECK(getValue(track, kSequenceTrackProperty_SoloStatus, &flag, sizeof(flag)) == noErr && flag == 0);
	UInt32 automated = 9;
	CHECK(getValue(track, kSequenceTrackProperty_AutomatedParameters, &automated, sizeof(automated)) == noErr && automated == 0);
	CHECK(trackLength(track) == 0);
	SInt16 resolution = 0;
	CHECK(getValue(tempo, kSequenceTrackProperty_TimeResolution, &resolution, sizeof(resolution)) == noErr);
	CHECK(resolution == 480);
	// Valid on the tempo track only; another track gives kAudio_ParamError (observed on macOS).
	CHECK(getValue(track, kSequenceTrackProperty_TimeResolution, &resolution, sizeof(resolution)) == kAudio_ParamError);

	// Values round-trip, and each track has its own.
	MusicTrack other = NULL;
	CHECK(MusicSequenceNewTrack(seq, &other) == noErr);
	loop.loopDuration = 4.0; loop.numberOfLoops = 3;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == noErr);
	offset = 1.5;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr);
	flag = 1;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr);
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_SoloStatus, &flag, sizeof(flag)) == noErr);
	automated = 1;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_AutomatedParameters, &automated, sizeof(automated)) == noErr);

	MusicTrackLoopInfo gotLoop = { 0, 0 };
	CHECK(getValue(track, kSequenceTrackProperty_LoopInfo, &gotLoop, sizeof(gotLoop)) == noErr);
	CHECK(gotLoop.loopDuration == 4.0 && gotLoop.numberOfLoops == 3);
	CHECK(getValue(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr && offset == 1.5);
	flag = 0;
	CHECK(getValue(track, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr && flag == 1);
	flag = 0;
	CHECK(getValue(track, kSequenceTrackProperty_SoloStatus, &flag, sizeof(flag)) == noErr && flag == 1);
	automated = 0;
	CHECK(getValue(track, kSequenceTrackProperty_AutomatedParameters, &automated, sizeof(automated)) == noErr && automated == 1);
	flag = 9;
	CHECK(getValue(other, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr && flag == 0);

	// Looping is turned off by a loop duration of zero.
	loop.loopDuration = 0; loop.numberOfLoops = 0;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == noErr);
	CHECK(getValue(track, kSequenceTrackProperty_LoopInfo, &gotLoop, sizeof(gotLoop)) == noErr);
	CHECK(gotLoop.loopDuration == 0);

	// Track length: the end of the last event, a note lasting for its duration.
	MIDINoteMessage note = { 0, 60, 100, 0, 1.5f };
	CHECK(MusicTrackNewMIDINoteEvent(other, 2.0, &note) == noErr);
	CHECK(trackLength(other) == 3.5);
	MIDIChannelMessage channel = { 0xB0, 1, 2, 0 };
	CHECK(MusicTrackNewMIDIChannelEvent(other, 5.0, &channel) == noErr);
	CHECK(trackLength(other) == 5.0);
	ExtendedNoteOnEvent ext;
	memset(&ext, 0, sizeof(ext));
	ext.extendedParams.argCount = 2;
	ext.duration = 4.0f;
	CHECK(MusicTrackNewExtendedNoteEvent(other, 4.0, &ext) == noErr);
	CHECK(trackLength(other) == 8.0);
	// A length set on the track counts when it is the greater one; the track length is not lost
	// when it is shorter than the events.
	MusicTimeStamp set = 20.0;
	CHECK(MusicTrackSetProperty(other, kSequenceTrackProperty_TrackLength, &set, sizeof(set)) == noErr);
	CHECK(trackLength(other) == 20.0);
	set = 1.0;
	CHECK(MusicTrackSetProperty(other, kSequenceTrackProperty_TrackLength, &set, sizeof(set)) == noErr);
	CHECK(trackLength(other) == 8.0);
	// An event added afterwards extends it again.
	CHECK(MusicTrackNewMIDIChannelEvent(other, 9.0, &channel) == noErr);
	CHECK(trackLength(other) == 9.0);

	// QUIRK test: the length that is passed to Set and Get is not looked at, each property has the size it has
	// (observed on macOS 26.7.3 and 10.14.4): a Set with another size works, a Get with a small length (or none)
	// returns the whole value and the length of the property
	offset = 2.5;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, 4) == noErr);
	flag = 1;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_MuteStatus, &flag, 2) == noErr);
	offset = 7;
	length = 4;
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, &length) == noErr);
	CHECK(offset == 2.5 && length == sizeof(MusicTimeStamp));
	offset = 7;
	CHECK(MusicTrackGetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, NULL) == noErr);
	CHECK(offset == 2.5);

	// Refused: an unknown property, a read only one, NULL arguments, a negative loop duration or track length.
	// Everything else is accepted: an offset that is not a number or is below zero, a negative number of loops.
	CHECK(MusicTrackSetProperty(track, 99, &flag, sizeof(flag)) == kAudio_ParamError);
	resolution = 96;
	CHECK(MusicTrackSetProperty(tempo, kSequenceTrackProperty_TimeResolution, &resolution, sizeof(resolution)) == kAudio_ParamError);
	CHECK(getValue(tempo, kSequenceTrackProperty_TimeResolution, &resolution, sizeof(resolution)) == noErr && resolution == 480);
	offset = NAN;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr);
	CHECK(getValue(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr && isnan(offset));
	offset = -1.0;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr);
	CHECK(getValue(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr && offset == -1.0);
	offset = 1.5;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == noErr);
	loop.loopDuration = 3.0; loop.numberOfLoops = 2;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == noErr);
	loop.loopDuration = -1.0; loop.numberOfLoops = 1;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == kAudio_ParamError);
	CHECK(getValue(track, kSequenceTrackProperty_LoopInfo, &gotLoop, sizeof(gotLoop)) == noErr);
	CHECK(gotLoop.loopDuration == 3.0 && gotLoop.numberOfLoops == 2);
	loop.loopDuration = 1.0; loop.numberOfLoops = -1;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_LoopInfo, &loop, sizeof(loop)) == noErr);
	set = -1.0;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_TrackLength, &set, sizeof(set)) == kAudio_ParamError);
	// the flags are booleans
	flag = 7;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr);
	flag = 0;
	CHECK(getValue(track, kSequenceTrackProperty_MuteStatus, &flag, sizeof(flag)) == noErr && flag == 1);
	automated = 0xFFFFFFFF;
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_AutomatedParameters, &automated, sizeof(automated)) == noErr);
	automated = 0;
	CHECK(getValue(track, kSequenceTrackProperty_AutomatedParameters, &automated, sizeof(automated)) == noErr && automated == 1);
	CHECK(MusicTrackSetProperty(track, kSequenceTrackProperty_OffsetTime, NULL, sizeof(offset)) == kAudio_ParamError);
	CHECK(MusicTrackSetProperty(NULL, kSequenceTrackProperty_OffsetTime, &offset, sizeof(offset)) == kAudio_ParamError);
	CHECK(MusicTrackGetProperty(track, 99, NULL, &length) == kAudio_ParamError);
	CHECK(MusicTrackGetProperty(NULL, kSequenceTrackProperty_OffsetTime, &offset, &length) == kAudio_ParamError);

	CHECK(DisposeMusicSequence(seq) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
