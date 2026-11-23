// Copyright (C) 2026 Lubos Dolezel
//
// The sequence type and the conversion between beats and seconds (MusicPlayer.h). A new sequence
// is beat based with a default tempo of 120 beats per minute. The type can be set to beats at any
// time, and to seconds or samples only while the tempo track has no tempo events. Beats and seconds are
// converted through the tempo events, a tempo holding until the next one. MusicSequenceSetSMPTEResolution
// packs the 16 bit division field of a MIDI file for a SMPTE time base: 30 frames per second and 80 ticks is
// 0xE250. What the documentation leaves open is what macOS 26.7.3 and 10.14.4 do, observed with these checks:
// the type does not change the conversions (the tempo is beats per minute for every type, 120 before the
// first tempo event), a sequence that is not of type beats takes one tempo event, and the codes for refusals.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

static int near(double a, double b)
{
	return fabs(a - b) < 1e-9 * (1 + fabs(a));
}

static double seconds(MusicSequence seq, double beats)
{
	double s = -1;
	CHECK(MusicSequenceGetSecondsForBeats(seq, beats, &s) == noErr);
	return s;
}

static double beats(MusicSequence seq, double secs)
{
	double b = -1;
	CHECK(MusicSequenceGetBeatsForSeconds(seq, secs, &b) == noErr);
	return b;
}

int main(void)
{
	MusicSequence seq = NULL;
	MusicTrack tempo = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);

	MusicSequenceType type = 0;
	CHECK(MusicSequenceGetSequenceType(seq, &type) == noErr && type == kMusicSequenceType_Beats);

	// The default tempo is 120 beats per minute: two beats a second.
	CHECK(near(seconds(seq, 0), 0));
	CHECK(near(seconds(seq, 4), 2));
	CHECK(near(beats(seq, 3), 6));

	// Tempo events: 120 from 0, 60 from beat 4, 240 from beat 6.
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 0, 120) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 4, 60) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 6, 240) == noErr);
	CHECK(near(seconds(seq, 4), 2));
	CHECK(near(seconds(seq, 5), 3));
	CHECK(near(seconds(seq, 6), 4));
	CHECK(near(seconds(seq, 10), 5));
	CHECK(near(beats(seq, 2), 4));
	CHECK(near(beats(seq, 3.5), 5.5));
	CHECK(near(beats(seq, 4), 6));
	CHECK(near(beats(seq, 5), 10));
	for (double b = 0; b < 12; b += 0.37)
		CHECK(near(beats(seq, seconds(seq, b)), b));

	// Events in the tempo track that are not tempo events do not matter.
	MIDINoteMessage note = { 0, 60, 100, 0, 1 };
	CHECK(MusicTrackNewMIDINoteEvent(tempo, 1, &note) == noErr);
	CHECK(near(seconds(seq, 4), 2));

	// A tempo event at the same time as another replaces it; a tempo track whose first event is
	// later than zero (all of this as observed on macOS 26.7.3 and 10.14.4).
	MusicSequence seq2 = NULL;
	MusicTrack tempo2 = NULL;
	CHECK(NewMusicSequence(&seq2) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq2, &tempo2) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo2, 2, 60) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo2, 2, 30) == noErr);
	// Before the first tempo event the tempo is the default 120: beats 0..2 take one second.
	CHECK(near(seconds(seq2, 2), 1));
	// Of two tempo events at the same time the later one holds: 30 bpm is 2 seconds a beat.
	CHECK(near(seconds(seq2, 3), 3));
	CHECK(DisposeMusicSequence(seq2) == noErr);

	// Out of range input and NULL arguments: a time below zero is kAudio_ParamError and leaves the output alone; one
	// that is not a number or is infinite converts to such a result
	double out = 7;
	CHECK(MusicSequenceGetSecondsForBeats(seq, -1, &out) == kAudio_ParamError);
	CHECK(MusicSequenceGetBeatsForSeconds(seq, -1, &out) == kAudio_ParamError);
	CHECK(out == 7);
	// (a sequence of its own, without tempo events: macOS 26.7.3 crashes on a time that is not a number when
	// there are some)
	MusicSequence plainSeq = NULL;
	CHECK(NewMusicSequence(&plainSeq) == noErr);
	CHECK(MusicSequenceGetSecondsForBeats(plainSeq, NAN, &out) == noErr && isnan(out));
	CHECK(MusicSequenceGetBeatsForSeconds(plainSeq, NAN, &out) == noErr && isnan(out));
	CHECK(MusicSequenceGetSecondsForBeats(plainSeq, INFINITY, &out) == noErr && isinf(out));
	CHECK(DisposeMusicSequence(plainSeq) == noErr);
	out = 7;
	CHECK(MusicSequenceGetSecondsForBeats(seq, 1, NULL) == kAudio_ParamError);
	CHECK(MusicSequenceGetSecondsForBeats(NULL, 1, &out) == kAudio_ParamError);
	CHECK(MusicSequenceGetSequenceType(seq, NULL) == kAudio_ParamError);

	// Seconds and samples only without tempo events (kAudioToolboxErr_InvalidSequenceType otherwise); beats at any
	// time. A value that is no sequence type is accepted and kept (QUIRK test, observed on macOS).
	CHECK(MusicSequenceSetSequenceType(seq, kMusicSequenceType_Seconds) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(MusicSequenceSetSequenceType(seq, kMusicSequenceType_Samples) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(MusicSequenceSetSequenceType(seq, 'xxxx') == noErr);
	CHECK(MusicSequenceGetSequenceType(seq, &type) == noErr && type == 'xxxx');
	CHECK(MusicSequenceSetSequenceType(seq, kMusicSequenceType_Beats) == noErr);
	CHECK(MusicSequenceGetSequenceType(seq, &type) == noErr && type == kMusicSequenceType_Beats);
	CHECK(MusicSequenceSetSequenceType(NULL, kMusicSequenceType_Beats) == kAudio_ParamError);

	// Seconds: the conversions read the tempo events as beats per minute whatever the type (a beat is a second at
	// tempo 60, and 0.5 s at the default 120 when there is no tempo event). Other events in the tempo track are
	// not tempo events and do not prevent the change.
	MusicSequence secs = NULL;
	MusicTrack secsTempo = NULL;
	CHECK(NewMusicSequence(&secs) == noErr);
	CHECK(MusicSequenceGetTempoTrack(secs, &secsTempo) == noErr);
	CHECK(MusicTrackNewMIDINoteEvent(secsTempo, 0, &note) == noErr);
	CHECK(MusicSequenceSetSequenceType(secs, kMusicSequenceType_Seconds) == noErr);
	CHECK(MusicSequenceGetSequenceType(secs, &type) == noErr && type == kMusicSequenceType_Seconds);
	CHECK(near(seconds(secs, 2.5), 1.25));
	CHECK(MusicTrackNewExtendedTempoEvent(secsTempo, 0, 60) == noErr);
	CHECK(near(seconds(secs, 2.5), 2.5));
	// QUIRK test: a sequence of another type than beats takes one tempo event only
	CHECK(MusicTrackNewExtendedTempoEvent(secsTempo, 4, 120) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(near(beats(secs, 7), 7));
	// With a tempo event the type can no longer change to samples, but it can go back to beats.
	CHECK(MusicSequenceSetSequenceType(secs, kMusicSequenceType_Samples) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(MusicSequenceGetSequenceType(secs, &type) == noErr && type == kMusicSequenceType_Seconds);
	CHECK(MusicSequenceSetSequenceType(secs, kMusicSequenceType_Beats) == noErr);
	CHECK(near(seconds(secs, 2), 2));
	CHECK(DisposeMusicSequence(secs) == noErr);

	// Samples: the same, the tempo is beats per minute (a tempo of 48000 is 1.25 ms a beat) and one tempo event is
	// all there can be
	MusicSequence samples = NULL;
	MusicTrack samplesTempo = NULL;
	CHECK(NewMusicSequence(&samples) == noErr);
	CHECK(MusicSequenceGetTempoTrack(samples, &samplesTempo) == noErr);
	CHECK(MusicSequenceSetSequenceType(samples, kMusicSequenceType_Samples) == noErr);
	CHECK(near(seconds(samples, 44100), 22050));
	CHECK(MusicTrackNewExtendedTempoEvent(samplesTempo, 0, 48000) == noErr);
	CHECK(near(seconds(samples, 48000), 60));
	CHECK(near(beats(samples, 0.5), 400));
	CHECK(MusicTrackNewExtendedTempoEvent(samplesTempo, 48000, 24000) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(MusicSequenceSetSequenceType(samples, kMusicSequenceType_Seconds) == kAudioToolboxErr_InvalidSequenceType);
	CHECK(DisposeMusicSequence(samples) == noErr);

	// SMPTE resolutions for MIDI files.
	CHECK(MusicSequenceSetSMPTEResolution(30, 80) == (SInt16) 0xE250);
	CHECK(MusicSequenceSetSMPTEResolution(-30, 80) == (SInt16) 0xE250);
	CHECK(MusicSequenceSetSMPTEResolution(25, 40) == (SInt16) 0xE728);
	CHECK(MusicSequenceSetSMPTEResolution(24, 4) == (SInt16) 0xE804);
	SignedByte fps = 0;
	Byte ticks = 0;
	MusicSequenceGetSMPTEResolution((SInt16) 0xE250, &fps, &ticks);
	CHECK(fps == -30 && ticks == 80);
	MusicSequenceGetSMPTEResolution((SInt16) 0xE728, &fps, &ticks);
	CHECK(fps == -25 && ticks == 40);
	MusicSequenceGetSMPTEResolution(MusicSequenceSetSMPTEResolution(29, 100), &fps, &ticks);
	CHECK(fps == -29 && ticks == 100);

	CHECK(DisposeMusicSequence(seq) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
