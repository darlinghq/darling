// Copyright (C) 2026 Lubos Dolezel
//
// MusicTrackMoveEvents, Clear, Cut, CopyInsert and Merge (MusicPlayer.h). A range is
// [start, end) and its start must be less than its end; kMusicTimeStamp_EndOfTrack as the end
// reaches the last event. Move shifts the events in the range, Clear removes them, Cut removes them
// and moves what follows back by the range, CopyInsert copies them into another track at a time
// and moves the events there at and after that time forward by the range, Merge copies them
// without moving anything. Checks marked GUESS pin what the documentation leaves open (whether
// the end of a range is included, the code for a bad range, the order of events with the same
// time, a move below zero, the length of a range that ends at the end of the track); they were not
// observed on macOS.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static void add(MusicTrack track, double time, UInt8 id)
{
	MIDINoteMessage note = { 0, id, 100, 0, 0.5f };
	CHECK(MusicTrackNewMIDINoteEvent(track, time, &note) == noErr);
}

// The events of a track as "id@time" in order, e.g. "1@0 2@1".
static void dump(MusicTrack track, char* out, size_t size)
{
	MusicEventIterator it = NULL;
	CHECK(NewMusicEventIterator(track, &it) == noErr);
	out[0] = 0;
	Boolean b = 0;
	while (MusicEventIteratorHasCurrentEvent(it, &b) == noErr && b)
	{
		MusicTimeStamp t;
		const MIDINoteMessage* note;
		CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, (const void**) &note, NULL) == noErr);
		char one[32];
		snprintf(one, sizeof(one), "%s%d@%g", out[0] ? " " : "", note->note, t);
		strncat(out, one, size - strlen(out) - 1);
		CHECK(MusicEventIteratorNextEvent(it) == noErr);
	}
	CHECK(DisposeMusicEventIterator(it) == noErr);
}

#define EXPECT(track, expected) do { char got[256]; dump(track, got, sizeof(got)); \
	if (strcmp(got, expected) != 0) { printf("FAIL %d: got \"%s\" expected \"%s\"\n", __LINE__, got, expected); g_failures++; } } while (0)

// A new track with the notes 1..5 at the times 0..4.
static MusicTrack makeTrack(MusicSequence seq)
{
	MusicTrack track = NULL;
	CHECK(MusicSequenceNewTrack(seq, &track) == noErr);
	for (int i = 0; i < 5; i++)
		add(track, i, i + 1);
	return track;
}

int main(void)
{
	MusicSequence seq = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	MusicTrack t;

	// Move: the events in the range shift and the track stays in time order. GUESS: the end of the
	// range is not included.
	t = makeTrack(seq);
	CHECK(MusicTrackMoveEvents(t, 1, 3, 10) == noErr);
	EXPECT(t, "1@0 4@3 5@4 2@11 3@12");
	t = makeTrack(seq);
	CHECK(MusicTrackMoveEvents(t, 2, 4, -1.5) == noErr);
	EXPECT(t, "1@0 3@0.5 2@1 4@1.5 5@4");
	// GUESS: events that end up with the same time stay in the order they had before the move (4 was
	// after 2 before it, so it is after 2 now).
	t = makeTrack(seq);
	CHECK(MusicTrackMoveEvents(t, 3, 5, -2) == noErr);
	EXPECT(t, "1@0 2@1 4@1 3@2 5@2");
	// GUESS: a move below zero is kAudio_ParamError and moves nothing.
	t = makeTrack(seq);
	CHECK(MusicTrackMoveEvents(t, 0, 2, -0.5) == kAudio_ParamError);
	EXPECT(t, "1@0 2@1 3@2 4@3 5@4");
	CHECK(MusicTrackMoveEvents(t, 0, 5, NAN) == kAudio_ParamError);

	// The same with many events: 60 notes, the first 30 at time 10 and the rest at 20, are moved onto
	// time 10 and stay in their order.
	t = NULL;
	CHECK(MusicSequenceNewTrack(seq, &t) == noErr);
	for (int i = 0; i < 60; i++)
		add(t, i < 30 ? 10 : 20, i + 1);
	CHECK(MusicTrackMoveEvents(t, 15, 25, -10) == noErr);
	{
		MusicEventIterator many = NULL;
		CHECK(NewMusicEventIterator(t, &many) == noErr);
		for (int i = 0; i < 60; i++)
		{
			MusicTimeStamp when;
			const MIDINoteMessage* note;
			CHECK(MusicEventIteratorGetEventInfo(many, &when, NULL, (const void**) &note, NULL) == noErr);
			CHECK(when == 10 && note->note == i + 1);
			CHECK(MusicEventIteratorNextEvent(many) == noErr);
		}
		CHECK(DisposeMusicEventIterator(many) == noErr);
	}

	// Clear: the range [1, 3) holds the events at 1 and 2 only.
	t = makeTrack(seq);
	CHECK(MusicTrackClear(t, 1, 3) == noErr);
	EXPECT(t, "1@0 4@3 5@4");
	// kMusicTimeStamp_EndOfTrack reaches the last event.
	CHECK(MusicTrackClear(t, 3, kMusicTimeStamp_EndOfTrack) == noErr);
	EXPECT(t, "1@0");
	CHECK(MusicTrackClear(t, 0, kMusicTimeStamp_EndOfTrack) == noErr);
	EXPECT(t, "");

	// Cut: the range is removed and what follows moves back by its length.
	t = makeTrack(seq);
	CHECK(MusicTrackCut(t, 1, 3) == noErr);
	EXPECT(t, "1@0 4@1 5@2");
	t = makeTrack(seq);
	CHECK(MusicTrackCut(t, 2, kMusicTimeStamp_EndOfTrack) == noErr);
	EXPECT(t, "1@0 2@1");
	t = makeTrack(seq);
	CHECK(MusicTrackCut(t, 0.5, 2.5) == noErr);
	EXPECT(t, "1@0 4@1 5@2");

	// Bad ranges: the start must be less than the end. GUESS: kAudio_ParamError, nothing changes.
	t = makeTrack(seq);
	CHECK(MusicTrackClear(t, 3, 3) == kAudio_ParamError);
	CHECK(MusicTrackClear(t, 3, 1) == kAudio_ParamError);
	CHECK(MusicTrackCut(t, 2, 1) == kAudio_ParamError);
	CHECK(MusicTrackMoveEvents(t, 2, 2, 1) == kAudio_ParamError);
	CHECK(MusicTrackClear(t, NAN, 3) == kAudio_ParamError);
	CHECK(MusicTrackClear(NULL, 0, 1) == kAudio_ParamError);
	// GUESS: a range that starts below zero is refused too; a cut from -4 would put events below zero.
	CHECK(MusicTrackCut(t, -4, 2) == kAudio_ParamError);
	CHECK(MusicTrackClear(t, -1, 2) == kAudio_ParamError);
	CHECK(MusicTrackMoveEvents(t, -1, 2, 1) == kAudio_ParamError);
	CHECK(MusicTrackMerge(t, -1, 2, t, 0) == kAudio_ParamError);
	EXPECT(t, "1@0 2@1 3@2 4@3 5@4");

	// CopyInsert: copies the range into another track at a time and makes room there.
	MusicTrack src = makeTrack(seq);
	MusicTrack dst = NULL;
	CHECK(MusicSequenceNewTrack(seq, &dst) == noErr);
	add(dst, 0, 11); add(dst, 1, 12); add(dst, 2, 13);
	CHECK(MusicTrackCopyInsert(src, 1, 3, dst, 1) == noErr);
	EXPECT(dst, "11@0 2@1 3@2 12@3 13@4");
	EXPECT(src, "1@0 2@1 3@2 4@3 5@4");
	// Inserted elsewhere: the copy keeps its distance from the range start.
	CHECK(MusicTrackCopyInsert(src, 3, 5, dst, 10) == noErr);
	EXPECT(dst, "11@0 2@1 3@2 12@3 13@4 4@10 5@11");
	// Into the same track: the copy is taken before anything moves.
	t = makeTrack(seq);
	CHECK(MusicTrackCopyInsert(t, 0, 2, t, 2) == noErr);
	EXPECT(t, "1@0 2@1 1@2 2@3 3@4 4@5 5@6");
	// GUESS: a range that ends at the end of the track is as long as the track (the last note
	// lasts 0.5, so the track is 4.5 long) minus the start.
	src = makeTrack(seq);
	dst = NULL;
	CHECK(MusicSequenceNewTrack(seq, &dst) == noErr);
	add(dst, 0, 11);
	CHECK(MusicTrackCopyInsert(src, 3, kMusicTimeStamp_EndOfTrack, dst, 0) == noErr);
	EXPECT(dst, "4@0 5@1 11@1.5");
	// A range that holds only a zero-length event at the end of the track still copies it, as a merge
	// does (GUESS: the range is 0 long, so nothing in the destination moves).
	MusicTrack param = NULL, paramDst = NULL;
	CHECK(MusicSequenceNewTrack(seq, &param) == noErr && MusicSequenceNewTrack(seq, &paramDst) == noErr);
	ParameterEvent pe = { 1, kAudioUnitScope_Global, 0, 0.5f };
	CHECK(MusicTrackNewParameterEvent(param, 4, &pe) == noErr);
	add(paramDst, 0, 21);
	CHECK(MusicTrackCopyInsert(param, 4, kMusicTimeStamp_EndOfTrack, paramDst, 0) == noErr);
	MusicEventIterator pit = NULL;
	CHECK(NewMusicEventIterator(paramDst, &pit) == noErr);
	MusicTimeStamp pt = -1;
	MusicEventType ptype = 0;
	CHECK(MusicEventIteratorGetEventInfo(pit, &pt, &ptype, NULL, NULL) == noErr);
	CHECK(pt == 0 && ptype == kMusicEventType_MIDINoteMessage && MusicEventIteratorNextEvent(pit) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(pit, &pt, &ptype, NULL, NULL) == noErr && pt == 0 && ptype == kMusicEventType_Parameter);
	CHECK(DisposeMusicEventIterator(pit) == noErr);
	// Nothing to copy from a range past the end of the track.
	CHECK(MusicTrackCopyInsert(src, 9, kMusicTimeStamp_EndOfTrack, dst, 0) == noErr);
	EXPECT(dst, "4@0 5@1 11@1.5");
	CHECK(MusicTrackCopyInsert(src, 3, 1, dst, 0) == kAudio_ParamError);
	CHECK(MusicTrackCopyInsert(src, 1, 3, dst, -1) == kAudio_ParamError);
	CHECK(MusicTrackCopyInsert(src, 1, 3, NULL, 0) == kAudio_ParamError);

	// Merge: the events of the destination stay where they are.
	src = makeTrack(seq);
	dst = NULL;
	CHECK(MusicSequenceNewTrack(seq, &dst) == noErr);
	add(dst, 0, 11); add(dst, 2, 12);
	CHECK(MusicTrackMerge(src, 1, 3, dst, 0.5) == noErr);
	EXPECT(dst, "11@0 2@0.5 3@1.5 12@2");
	// GUESS: a merged event goes after the events that have its time.
	CHECK(MusicTrackMerge(src, 4, kMusicTimeStamp_EndOfTrack, dst, 2) == noErr);
	EXPECT(dst, "11@0 2@0.5 3@1.5 12@2 5@2");
	CHECK(MusicTrackMerge(src, 2, 2, dst, 0) == kAudio_ParamError);
	CHECK(MusicTrackMerge(src, 0, 1, dst, NAN) == kAudio_ParamError);

	CHECK(DisposeMusicSequence(seq) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
