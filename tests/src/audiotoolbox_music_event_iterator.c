// Copyright (C) 2026 Lubos Dolezel
//
// MusicTrack events and MusicEventIterator (MusicPlayer.h): events are kept in time order and an
// iterator walks them forwards and backwards. A new iterator points at the first event; Seek
// points at the first event at or after a time, and kMusicTimeStamp_EndOfTrack positions the
// iterator just beyond the last event, from where MusicEventIteratorPreviousEvent backs up to the
// last event; stepping from the last event goes beyond it; the previous event of the first event
// is an error and leaves the iterator unchanged. The track keeps a copy of an added event. The result
// codes and orders that the documentation leaves open, and the arguments that are accepted although they
// make little sense (a time below zero, a tempo of 0), are what macOS 26.7.3 and 10.14.4 do, observed with
// these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static Boolean has(OSStatus (*f)(MusicEventIterator, Boolean*), MusicEventIterator it)
{
	Boolean b = 99;
	CHECK(f(it, &b) == noErr);
	CHECK(b == 0 || b == 1);
	return b;
}

int main(void)
{
	MusicSequence seq = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	MusicTrack track = NULL;
	CHECK(MusicSequenceNewTrack(seq, &track) == noErr);

	// An empty track: no current, next or previous event.
	MusicEventIterator it = NULL;
	CHECK(NewMusicEventIterator(track, &it) == noErr && it != NULL);
	CHECK(!has(MusicEventIteratorHasCurrentEvent, it));
	CHECK(!has(MusicEventIteratorHasNextEvent, it));
	CHECK(!has(MusicEventIteratorHasPreviousEvent, it));
	CHECK(MusicEventIteratorGetEventInfo(it, NULL, NULL, NULL, NULL) != noErr);
	// Past the last event the code is kAudioToolboxErr_EndOfTrack (observed on macOS 26.7.3 and 10.14.4).
	CHECK(MusicEventIteratorGetEventInfo(it, NULL, NULL, NULL, NULL) == kAudioToolboxErr_EndOfTrack);
	CHECK(MusicEventIteratorNextEvent(it) == kAudioToolboxErr_EndOfTrack);
	// Stepping back from the first position is kAudioToolboxErr_StartOfTrack (observed on macOS).
	CHECK(MusicEventIteratorPreviousEvent(it) == kAudioToolboxErr_StartOfTrack);
	CHECK(DisposeMusicEventIterator(it) == noErr);

	// Events added out of order; two share the time 1.
	MIDINoteMessage n2 = { 1, 62, 90, 0, 2.0f };
	MIDINoteMessage n0 = { 0, 60, 100, 5, 1.0f };
	MIDINoteMessage n1a = { 2, 64, 80, 0, 0.5f };
	MIDINoteMessage n1b = { 3, 65, 70, 0, 0.25f };
	MIDINoteMessage nh = { 4, 66, 60, 0, 0.75f };
	CHECK(MusicTrackNewMIDINoteEvent(track, 2.0, &n2) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(track, 0.0, &n0) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(track, 1.0, &n1a) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(track, 0.5, &nh) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(track, 1.0, &n1b) == noErr);
	// QUIRK test: a tempo event is only for the tempo track, any other track gives
	// kAudioToolboxErr_IllegalTrackDestination (observed on macOS 26.7.3 and 10.14.4)
	CHECK(MusicTrackNewExtendedTempoEvent(track, 0.5, 90.0) == kAudioToolboxErr_IllegalTrackDestination);

	// The track keeps a copy of what was passed.
	n0.note = 1;

	CHECK(NewMusicEventIterator(track, &it) == noErr);
	MusicTimeStamp times[8];
	MusicEventType types[8];
	const void* data[8];
	UInt32 sizes[8];
	int n = 0;
	while (has(MusicEventIteratorHasCurrentEvent, it) && n < 8)
	{
		CHECK(MusicEventIteratorGetEventInfo(it, &times[n], &types[n], &data[n], &sizes[n]) == noErr);
		n++;
		CHECK(MusicEventIteratorNextEvent(it) == noErr);
	}
	CHECK(n == 5);
	// Time order; events with the same time are in the order they were added (observed on macOS).
	CHECK(times[0] == 0.0 && times[1] == 0.5 && times[2] == 1.0 && times[3] == 1.0 && times[4] == 2.0);
	CHECK(types[0] == kMusicEventType_MIDINoteMessage && types[1] == kMusicEventType_MIDINoteMessage);
	CHECK(sizes[0] == sizeof(MIDINoteMessage) && sizes[1] == sizeof(MIDINoteMessage));
	CHECK(((const MIDINoteMessage*) data[0])->note == 60);
	CHECK(((const MIDINoteMessage*) data[0])->releaseVelocity == 5);
	CHECK(((const MIDINoteMessage*) data[1])->note == 66);
	CHECK(((const MIDINoteMessage*) data[2])->channel == 2);
	CHECK(((const MIDINoteMessage*) data[3])->channel == 3);
	CHECK(((const MIDINoteMessage*) data[4])->duration == 2.0f);

	// Beyond the last event: nothing current, something before.
	CHECK(!has(MusicEventIteratorHasCurrentEvent, it));
	CHECK(!has(MusicEventIteratorHasNextEvent, it));
	CHECK(has(MusicEventIteratorHasPreviousEvent, it));

	// Output parameters that are not wanted may be NULL.
	CHECK(MusicEventIteratorSeek(it, 0.0) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(it, NULL, NULL, NULL, NULL) == noErr);
	MusicTimeStamp t = -1;
	CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, NULL, NULL) == noErr && t == 0.0);

	// Seek: the first event at the time, or else the first one after it.
	CHECK(MusicEventIteratorSeek(it, 1.0) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, &data[0], NULL) == noErr && t == 1.0);
	CHECK(((const MIDINoteMessage*) data[0])->channel == 2);
	CHECK(MusicEventIteratorSeek(it, 0.75) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, NULL, NULL) == noErr && t == 1.0);
	CHECK(MusicEventIteratorSeek(it, 0.25) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(it, &t, &types[0], &data[0], NULL) == noErr && t == 0.5);
	CHECK(((const MIDINoteMessage*) data[0])->note == 66);
	CHECK(MusicEventIteratorSeek(it, 2.5) == noErr);
	CHECK(!has(MusicEventIteratorHasCurrentEvent, it));
	// QUIRK test: a time that is not a number is accepted and goes to the first event (observed on macOS
	// 26.7.3 and 10.14.4)
	CHECK(MusicEventIteratorSeek(it, NAN) == noErr);
	CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, NULL, NULL) == noErr && t == 0.0);

	// Backwards from the end of the track, as the documentation shows.
	CHECK(MusicEventIteratorSeek(it, kMusicTimeStamp_EndOfTrack) == noErr);
	CHECK(!has(MusicEventIteratorHasCurrentEvent, it));
	double back[8];
	int m = 0;
	while (has(MusicEventIteratorHasPreviousEvent, it) && m < 8)
	{
		CHECK(MusicEventIteratorPreviousEvent(it) == noErr);
		CHECK(MusicEventIteratorGetEventInfo(it, &back[m], NULL, NULL, NULL) == noErr);
		m++;
	}
	CHECK(m == 5);
	CHECK(back[0] == 2.0 && back[1] == 1.0 && back[2] == 1.0 && back[3] == 0.5 && back[4] == 0.0);

	// At the first event the iterator stays and the call fails.
	CHECK(!has(MusicEventIteratorHasPreviousEvent, it));
	CHECK(MusicEventIteratorPreviousEvent(it) == kAudioToolboxErr_StartOfTrack);
	CHECK(MusicEventIteratorGetEventInfo(it, &t, NULL, NULL, NULL) == noErr && t == 0.0);

	// HasNextEvent is about an event after the current one.
	CHECK(has(MusicEventIteratorHasNextEvent, it));
	CHECK(MusicEventIteratorSeek(it, 2.0) == noErr);
	CHECK(has(MusicEventIteratorHasCurrentEvent, it));
	CHECK(!has(MusicEventIteratorHasNextEvent, it));
	CHECK(MusicEventIteratorNextEvent(it) == noErr);
	CHECK(!has(MusicEventIteratorHasCurrentEvent, it));
	CHECK(DisposeMusicEventIterator(it) == noErr);

	// The tempo track takes tempo events and, like any track, other events.
	MusicTrack tempo = NULL;
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 0.0, 140.0) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(tempo, 0.0, &n1a) == noErr);
	CHECK(NewMusicEventIterator(tempo, &it) == noErr);
	CHECK(has(MusicEventIteratorHasCurrentEvent, it));
	CHECK(MusicEventIteratorGetEventInfo(it, &t, &types[0], &data[0], &sizes[0]) == noErr);
	CHECK(t == 0.0 && types[0] == kMusicEventType_ExtendedTempo && sizes[0] == sizeof(ExtendedTempoEvent));
	CHECK(((const ExtendedTempoEvent*) data[0])->bpm == 140.0);
	CHECK(DisposeMusicEventIterator(it) == noErr);

	// Time stamps below zero or not a number, and tempos that are zero, negative or not a number, are not
	// refused: macOS 26.7.3 and 10.14.4 accept them (a track of its own takes the first two: there they are not
	// kept in the order of the others)
	MusicTrack odd = NULL;
	CHECK(MusicSequenceNewTrack(seq, &odd) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(odd, -1.0, &n1a) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(odd, NAN, &n1a) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 1.0, 0.0) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 2.0, -120.0) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(track, 1.0, NULL) == kAudio_ParamError);
	CHECK(MusicTrackNewMIDINoteEvent(NULL, 1.0, &n1a) == kAudio_ParamError);
	CHECK(NewMusicEventIterator(NULL, &it) == kAudio_ParamError);
	CHECK(NewMusicEventIterator(track, NULL) == kAudio_ParamError);
	CHECK(DisposeMusicEventIterator(NULL) == kAudio_ParamError);

	// Refused events were not added
	CHECK(NewMusicEventIterator(track, &it) == noErr);
	CHECK(MusicEventIteratorSeek(it, kMusicTimeStamp_EndOfTrack) == noErr);
	int count = 0;
	while (has(MusicEventIteratorHasPreviousEvent, it) && count < 20)
	{
		CHECK(MusicEventIteratorPreviousEvent(it) == noErr);
		count++;
	}
	CHECK(count == 5);
	CHECK(DisposeMusicEventIterator(it) == noErr);

	CHECK(DisposeMusicSequence(seq) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
