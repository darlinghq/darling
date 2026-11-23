// Copyright (C) 2026 Lubos Dolezel
//
// MusicPlayerGetHostTimeForBeats and MusicPlayerGetBeatsForHostTime (MusicPlayer.h) translate between
// the beats of a playing player and host time through the tempo map of its sequence, from the beat
// and host time at which the player started. They return an error for a player that is not playing and
// for a beat or a host time before the start. The result codes, which the
// documentation does not name, and what happens to a beat that cannot be converted are what macOS 26.7.3
// does, observed with these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/HostTime.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

// MusicPlayerStart needs somewhere to send the events: on a Mac with no audio output (a session with no sound) the
// default graph cannot start and Start fails with kAudioToolboxErr_InvalidPlayerState. Where CoreMIDI can make a
// virtual destination (found at run time; Darling's CoreMIDI cannot, and the player needs none there) a track with
// that destination makes the sequence startable. If the player still cannot start the test says so and stops.
typedef OSStatus (*ClientCreateFunction)(CFStringRef, void*, void*, UInt32*);
typedef OSStatus (*DestinationCreateFunction)(UInt32, CFStringRef, void (*)(const void*, void*, void*), void*, UInt32*);

static void ignoreMIDI(const void* packets, void* refCon, void* connRefCon) { (void) packets; (void) refCon; (void) connRefCon; }

static UInt32 g_destination = 0;

static OSStatus newSequence(MusicSequence* outSequence)
{
	OSStatus status = NewMusicSequence(outSequence);
	if (status != noErr)
		return status;
	if (g_destination == 0)
	{
		ClientCreateFunction clientCreate = (ClientCreateFunction) dlsym(RTLD_DEFAULT, "MIDIClientCreate");
		DestinationCreateFunction destinationCreate = (DestinationCreateFunction) dlsym(RTLD_DEFAULT, "MIDIDestinationCreate");
		UInt32 client = 0, destination = 0;
		if (clientCreate && destinationCreate && clientCreate(CFSTR("test"), NULL, NULL, &client) == noErr
			&& destinationCreate(client, CFSTR("test"), ignoreMIDI, NULL, &destination) == noErr)
			g_destination = destination;
	}
	if (g_destination != 0)
	{
		MusicTrack track = NULL;
		if (MusicSequenceNewTrack(*outSequence, &track) == noErr)
			MusicTrackSetDestMIDIEndpoint(track, g_destination);
	}
	return noErr;
}

// Whether a player can start a sequence made by newSequence on this machine
static int canStart(void)
{
	MusicSequence seq = NULL;
	MusicPlayer player = NULL;
	if (newSequence(&seq) != noErr || NewMusicPlayer(&player) != noErr || MusicPlayerSetSequence(player, seq) != noErr)
		return 0;
	OSStatus status = MusicPlayerStart(player);
	MusicPlayerStop(player);
	MusicPlayerSetSequence(player, NULL);
	DisposeMusicPlayer(player);
	DisposeMusicSequence(seq);
	return status == noErr;
}

int main(void)
{
	if (!canStart())
	{
		printf("SKIP: MusicPlayerStart does not work here (no audio output and no MIDI destination)\n");
		return 0;
	}
	const double freq = AudioGetHostClockFrequency();
	MusicSequence seq = NULL;
	MusicPlayer player = NULL;
	CHECK(newSequence(&seq) == noErr);
	CHECK(NewMusicPlayer(&player) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq) == noErr);

	// Not playing: kAudioToolboxErr_InvalidPlayerState (observed on macOS 26.7.3).
	UInt64 host = 0;
	MusicTimeStamp beats = 0;
	CHECK(MusicPlayerGetHostTimeForBeats(player, 1.0, &host) == kAudioToolboxErr_InvalidPlayerState);
	CHECK(MusicPlayerGetBeatsForHostTime(player, AudioGetCurrentHostTime(), &beats) == kAudioToolboxErr_InvalidPlayerState);

	// Started at beat 1 with the default tempo of 120 beats per minute: a beat is half a second, so
	// beat 5 is 2 seconds after the start, which was between t0 and t1.
	CHECK(MusicPlayerSetTime(player, 1.0) == noErr);
	UInt64 t0 = AudioGetCurrentHostTime();
	CHECK(MusicPlayerStart(player) == noErr);
	UInt64 t1 = AudioGetCurrentHostTime();
	CHECK(MusicPlayerGetHostTimeForBeats(player, 5.0, &host) == noErr);
	CHECK(host >= t0 + (UInt64) (2.0 * freq) && host <= t1 + (UInt64) (2.0 * freq));
	CHECK(MusicPlayerGetHostTimeForBeats(player, 1.0, &host) == noErr);
	CHECK(host >= t0 && host <= t1);

	// And back.
	CHECK(MusicPlayerGetHostTimeForBeats(player, 7.0, &host) == noErr);
	CHECK(MusicPlayerGetBeatsForHostTime(player, host, &beats) == noErr);
	CHECK(fabs(beats - 7.0) < 1e-6);

	// QUIRK test: a beat that is not finite, or so far ahead that its host time does not fit in 64 bits, is not
	// refused: the host time is the start plus -2^63 (observed on macOS 26.7.3 x86_64)
	CHECK(MusicPlayerGetHostTimeForBeats(player, INFINITY, &host) == noErr);
	UInt64 farAway = host;
	CHECK(MusicPlayerGetHostTimeForBeats(player, NAN, &host) == noErr && host == farAway);
	CHECK(MusicPlayerGetHostTimeForBeats(player, 1e300, &host) == noErr && host == farAway);
	CHECK(MusicPlayerGetHostTimeForBeats(player, 1e15, &host) == noErr && host == farAway);
	CHECK(farAway >= t0 + 0x8000000000000000ull && farAway <= t1 + 0x8000000000000000ull);

	// A beat or a host time before the start: kAudio_ParamError (observed on macOS 26.7.3).
	CHECK(MusicPlayerGetHostTimeForBeats(player, 0.5, &host) == kAudio_ParamError);
	CHECK(MusicPlayerGetBeatsForHostTime(player, t0 - (UInt64) freq, &beats) == kAudio_ParamError);

	// At twice the rate, with a tempo event of 60 beats per minute from the start: 1 beat a second
	// at rate 2 is 2 beats a second, so 2 beats take 1 second.
	MusicTrack tempo = NULL;
	CHECK(MusicPlayerStop(player) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 0, 60) == noErr);
	CHECK(MusicPlayerSetPlayRateScalar(player, 2.0) == noErr);
	CHECK(MusicPlayerSetTime(player, 0) == noErr);
	CHECK(MusicPlayerStart(player) == noErr);
	UInt64 h2 = 0, h4 = 0;
	CHECK(MusicPlayerGetHostTimeForBeats(player, 2.0, &h2) == noErr);
	CHECK(MusicPlayerGetHostTimeForBeats(player, 4.0, &h4) == noErr);
	CHECK(fabs((double) (h4 - h2) / freq - 1.0) < 1e-6);
	CHECK(MusicPlayerGetBeatsForHostTime(player, h4, &beats) == noErr && fabs(beats - 4.0) < 1e-6);

	CHECK(MusicPlayerGetHostTimeForBeats(NULL, 1.0, &host) == kAudio_ParamError);
	CHECK(MusicPlayerGetHostTimeForBeats(player, 1.0, NULL) == kAudio_ParamError);
	CHECK(MusicPlayerGetBeatsForHostTime(player, host, NULL) == kAudio_ParamError);

	CHECK(MusicPlayerStop(player) == noErr);
	CHECK(DisposeMusicPlayer(player) == noErr);
	CHECK(DisposeMusicSequence(seq) == noErr);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
