// Copyright (C) 2026 Lubos Dolezel
//
// MusicPlayer (MusicPlayer.h). A new player is stopped, at time 0, with a play rate of 1.0 and no
// sequence (MusicPlayerGetSequence then returns kAudioToolboxErr_NoSequence). A sequence cannot be
// set on a playing player nor disposed while a player has it. The time is in beats, not range
// checked, and can be set while playing. A started player's time keeps increasing, scaled by the
// rate through the tempo of the sequence, until it is stopped, and a stopped player stays where it
// got to. The rate must be greater than zero. Time checks use bounds any scheduler satisfies.
// What the documentation leaves open (codes for a player with no sequence, a sequence in use and a bad rate or
// time, starting a started player, the time after setting a sequence) is what macOS 26.7.3 does, observed with
// these checks.
#include <AudioToolbox/AudioToolbox.h>
#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

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

static double timeOf(MusicPlayer player)
{
	MusicTimeStamp t = -12345;
	CHECK(MusicPlayerGetTime(player, &t) == noErr);
	return t;
}

static int playing(MusicPlayer player)
{
	Boolean b = 9;
	CHECK(MusicPlayerIsPlaying(player, &b) == noErr);
	CHECK(b == 0 || b == 1);
	return b;
}

int main(void)
{
	if (!canStart())
	{
		printf("SKIP: MusicPlayerStart does not work here (no audio output and no MIDI destination)\n");
		return 0;
	}
	MusicPlayer player = NULL;
	CHECK(NewMusicPlayer(NULL) == kAudio_ParamError);
	CHECK(NewMusicPlayer(&player) == noErr && player != NULL);

	// A new player.
	MusicSequence got = (MusicSequence) 1;
	CHECK(MusicPlayerGetSequence(player, &got) == kAudioToolboxErr_NoSequence);
	CHECK(!playing(player));
	CHECK(timeOf(player) == 0);
	Float64 rate = 0;
	CHECK(MusicPlayerGetPlayRateScalar(player, &rate) == noErr && rate == 1.0);

	// With no sequence there is nothing to preroll, start or stop (observed on macOS 26.7.3 and 10.14.4)
	CHECK(MusicPlayerPreroll(player) == kAudioToolboxErr_NoSequence);
	CHECK(MusicPlayerStart(player) == kAudioToolboxErr_NoSequence);
	CHECK(!playing(player));
	CHECK(MusicPlayerStop(player) == kAudioToolboxErr_NoSequence);

	// The sequence of a player; a sequence in use cannot be disposed (kAudioToolboxErr_CannotDoInCurrentContext,
	// observed on macOS 26.7.3).
	MusicSequence seq = NULL;
	MusicTrack tempo = NULL;
	CHECK(newSequence(&seq) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 0, 60) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq) == noErr);
	CHECK(MusicPlayerGetSequence(player, &got) == noErr && got == seq);
	CHECK(DisposeMusicSequence(seq) == kAudioToolboxErr_CannotDoInCurrentContext);
	CHECK(MusicPlayerPreroll(player) == noErr);

	// Time while stopped; it is not range checked: a time below zero or not a number is accepted.
	CHECK(MusicPlayerSetTime(player, 3.5) == noErr);
	CHECK(timeOf(player) == 3.5);
	CHECK(MusicPlayerSetTime(player, -2.0) == noErr);
	CHECK(timeOf(player) == -2.0);
	CHECK(MusicPlayerSetTime(player, NAN) == noErr);
	CHECK(isnan(timeOf(player)));
	CHECK(MusicPlayerSetTime(player, 0) == noErr);

	// Rates: greater than zero; kAudio_ParamError otherwise (not a number too).
	CHECK(MusicPlayerSetPlayRateScalar(player, NAN) == kAudio_ParamError);
	CHECK(MusicPlayerSetPlayRateScalar(player, 0) == kAudio_ParamError);
	CHECK(MusicPlayerSetPlayRateScalar(player, -1) == kAudio_ParamError);
	CHECK(MusicPlayerGetPlayRateScalar(player, &rate) == noErr && rate == 1.0);
	CHECK(MusicPlayerSetPlayRateScalar(player, 8.0) == noErr);
	CHECK(MusicPlayerGetPlayRateScalar(player, &rate) == noErr && rate == 8.0);

	// Playing: at 60 beats per minute and a rate of 8, 0.25 seconds are at least 2 beats.
	CHECK(MusicPlayerStart(player) == noErr);
	CHECK(playing(player));
	// Starting a player that is playing is noErr.
	CHECK(MusicPlayerStart(player) == noErr);
	usleep(250000);
	double t1 = timeOf(player);
	CHECK(t1 >= 2.0 * 0.95);
	usleep(10000);
	CHECK(timeOf(player) >= t1);

	// A sequence cannot be set while the player plays: kAudioToolboxErr_InvalidPlayerState.
	CHECK(MusicPlayerSetSequence(player, NULL) == kAudioToolboxErr_InvalidPlayerState);
	CHECK(MusicPlayerGetSequence(player, &got) == noErr && got == seq);

	// Setting the time of a playing player: it continues from there.
	CHECK(MusicPlayerSetTime(player, 100.0) == noErr);
	double t2 = timeOf(player);
	CHECK(t2 >= 100.0);
	CHECK(playing(player));

	// Stopping keeps the position; a stopped player does not advance.
	CHECK(MusicPlayerStop(player) == noErr);
	CHECK(!playing(player));
	double stopped = timeOf(player);
	CHECK(stopped >= 100.0);
	usleep(50000);
	CHECK(timeOf(player) == stopped);
	CHECK(MusicPlayerStop(player) == noErr);

	// Changing the rate of a playing player continues from its position at the new rate: 0.05 seconds
	// at 1 beat a second, then 0.02 seconds at 1000 times that.
	CHECK(MusicPlayerSetTime(player, 0) == noErr);
	CHECK(MusicPlayerSetPlayRateScalar(player, 1.0) == noErr);
	CHECK(MusicPlayerStart(player) == noErr);
	usleep(50000);
	CHECK(MusicPlayerSetPlayRateScalar(player, 1000.0) == noErr);
	usleep(20000);
	CHECK(timeOf(player) >= 0.05 * 0.95 + 20.0 * 0.95);
	CHECK(MusicPlayerStop(player) == noErr);

	// The player does not own the sequence; it can be replaced or removed, and then the sequence can go.
	// QUIRK test: setting a sequence puts the time of the player back to 0 (observed on macOS 26.7.3).
	MusicSequence seq2 = NULL;
	CHECK(newSequence(&seq2) == noErr);
	CHECK(MusicPlayerSetTime(player, 7.0) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq2) == noErr);
	CHECK(timeOf(player) == 0.0);
	CHECK(DisposeMusicSequence(seq) == noErr);
	CHECK(DisposeMusicSequence(seq2) == kAudioToolboxErr_CannotDoInCurrentContext);
	CHECK(MusicPlayerSetSequence(player, NULL) == noErr);
	CHECK(MusicPlayerGetSequence(player, &got) == kAudioToolboxErr_NoSequence);
	CHECK(DisposeMusicSequence(seq2) == noErr);

	// Disposing a player releases its sequence, even while it plays.
	CHECK(newSequence(&seq) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq) == noErr);
	CHECK(MusicPlayerStart(player) == noErr);
	CHECK(DisposeMusicPlayer(player) == noErr);
	CHECK(DisposeMusicSequence(seq) == noErr);

	CHECK(DisposeMusicPlayer(NULL) == kAudio_ParamError);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
