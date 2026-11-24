// Copyright (C) 2026 Lubos Dolezel
//
// MusicSequence and its tracks (MusicPlayer.h). A new sequence has only a tempo track, which the
// track count and the track accessors leave out. New tracks are appended; an index out of range,
// or a track that is not part of the sequence, is an error; removing a track renumbers the tracks
// after it. The result codes for an index out of range and for a track that is not in the sequence are
// what macOS 26.7.3 and 10.14.4 give, observed with these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

int main(void)
{
	// NULL arguments are rejected instead of dereferenced.
	CHECK(NewMusicSequence(NULL) != noErr);

	MusicSequence seq = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(seq != NULL);

	// Only the tempo track exists, and it does not count.
	UInt32 count = 99;
	CHECK(MusicSequenceGetTrackCount(seq, &count) == noErr);
	CHECK(count == 0);
	MusicTrack track = NULL;
	// An index that is out of range is kAudioToolboxErr_TrackIndexError (observed on macOS 26.7.3 and 10.14.4)
	CHECK(MusicSequenceGetIndTrack(seq, 0, &track) == kAudioToolboxErr_TrackIndexError);

	MusicTrack tempo = NULL;
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);
	CHECK(tempo != NULL);
	MusicSequence owner = NULL;
	CHECK(MusicTrackGetSequence(tempo, &owner) == noErr);
	CHECK(owner == seq);

	// New tracks are appended, in order.
	MusicTrack a = NULL, b = NULL, c = NULL;
	CHECK(MusicSequenceNewTrack(seq, &a) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &b) == noErr);
	CHECK(MusicSequenceNewTrack(seq, &c) == noErr);
	CHECK(a != NULL && b != NULL && c != NULL);
	CHECK(a != b && b != c && a != c);
	CHECK(a != tempo && b != tempo && c != tempo);
	CHECK(MusicSequenceGetTrackCount(seq, &count) == noErr);
	CHECK(count == 3);

	MusicTrack got = NULL;
	CHECK(MusicSequenceGetIndTrack(seq, 0, &got) == noErr && got == a);
	CHECK(MusicSequenceGetIndTrack(seq, 1, &got) == noErr && got == b);
	CHECK(MusicSequenceGetIndTrack(seq, 2, &got) == noErr && got == c);
	CHECK(MusicSequenceGetIndTrack(seq, 3, &got) == kAudioToolboxErr_TrackIndexError);

	UInt32 index = 99;
	CHECK(MusicSequenceGetTrackIndex(seq, a, &index) == noErr && index == 0);
	CHECK(MusicSequenceGetTrackIndex(seq, c, &index) == noErr && index == 2);

	// Every track reports the sequence it belongs to.
	owner = NULL;
	CHECK(MusicTrackGetSequence(b, &owner) == noErr && owner == seq);

	// A track of another sequence is not a member; the output is left alone.
	MusicSequence other = NULL;
	CHECK(NewMusicSequence(&other) == noErr);
	MusicTrack foreign = NULL;
	CHECK(MusicSequenceNewTrack(other, &foreign) == noErr);
	index = 77;
	CHECK(MusicSequenceGetTrackIndex(seq, foreign, &index) != noErr);
	CHECK(index == 77);
	// The code is kAudioToolboxErr_TrackNotFound (the documentation says only "an error"; observed on macOS)
	CHECK(MusicSequenceGetTrackIndex(seq, foreign, &index) == kAudioToolboxErr_TrackNotFound);
	// The tempo track is not one of the tracks that the index refers to.
	CHECK(MusicSequenceGetTrackIndex(seq, tempo, &index) == kAudioToolboxErr_TrackNotFound);

	// Removing a track renumbers the ones after it.
	CHECK(MusicSequenceDisposeTrack(seq, b) == noErr);
	CHECK(MusicSequenceGetTrackCount(seq, &count) == noErr && count == 2);
	CHECK(MusicSequenceGetIndTrack(seq, 0, &got) == noErr && got == a);
	CHECK(MusicSequenceGetIndTrack(seq, 1, &got) == noErr && got == c);
	CHECK(MusicSequenceGetTrackIndex(seq, c, &index) == noErr && index == 1);

	// A track cannot be removed from a sequence it is not in, and the tempo track cannot be removed.
	CHECK(MusicSequenceDisposeTrack(seq, foreign) == kAudio_ParamError);
	CHECK(MusicSequenceDisposeTrack(seq, tempo) == kAudio_ParamError);
	CHECK(MusicSequenceGetTrackCount(seq, &count) == noErr && count == 2);
	CHECK(MusicSequenceGetTrackCount(other, &count) == noErr && count == 1);

	// A new track is appended after the remaining ones.
	MusicTrack d = NULL;
	CHECK(MusicSequenceNewTrack(seq, &d) == noErr);
	CHECK(MusicSequenceGetTrackIndex(seq, d, &index) == noErr && index == 2);

	CHECK(MusicSequenceGetTrackCount(NULL, &count) != noErr);
	CHECK(MusicSequenceNewTrack(seq, NULL) != noErr);
	CHECK(MusicTrackGetSequence(NULL, &owner) != noErr);

	CHECK(DisposeMusicSequence(seq) == noErr);
	CHECK(DisposeMusicSequence(other) == noErr);
	CHECK(DisposeMusicSequence(NULL) != noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
