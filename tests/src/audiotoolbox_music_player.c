// Copyright (C) 2026 Lubos Dolezel
//
// MusicPlayer (MusicPlayer.h). A new player is stopped, at time 0, with a play rate of 1.0 and no
// sequence (MusicPlayerGetSequence then returns kAudioToolboxErr_NoSequence). A sequence cannot be
// set on a playing player nor disposed while a player has it. The time is in beats, not range
// checked, and can be set while playing. A started player's time keeps increasing, scaled by the
// rate through the tempo of the sequence, until it is stopped, and a stopped player stays where it
// got to. The rate must be greater than zero. Time checks use bounds any scheduler satisfies.
// Checks marked GUESS pin what the documentation leaves open (codes for a player with no sequence,
// a sequence in use and a bad rate or time, and starting a started player); not observed on macOS.
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

static int g_failures = 0;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %d: %s\n", __LINE__, #cond); g_failures++; } } while (0)

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

	// GUESS: with no sequence there is nothing to preroll or start.
	CHECK(MusicPlayerPreroll(player) == kAudioToolboxErr_NoSequence);
	CHECK(MusicPlayerStart(player) == kAudioToolboxErr_NoSequence);
	CHECK(!playing(player));
	CHECK(MusicPlayerStop(player) == noErr);

	// The sequence of a player; a sequence in use cannot be disposed. GUESS: the code for that is
	// kAudioToolboxErr_InvalidPlayerState.
	MusicSequence seq = NULL;
	MusicTrack tempo = NULL;
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicSequenceGetTempoTrack(seq, &tempo) == noErr);
	CHECK(MusicTrackNewExtendedTempoEvent(tempo, 0, 60) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq) == noErr);
	CHECK(MusicPlayerGetSequence(player, &got) == noErr && got == seq);
	CHECK(DisposeMusicSequence(seq) == kAudioToolboxErr_InvalidPlayerState);
	CHECK(MusicPlayerPreroll(player) == noErr);

	// Time while stopped; it is not range checked. GUESS: not a number is kAudio_ParamError.
	CHECK(MusicPlayerSetTime(player, 3.5) == noErr);
	CHECK(timeOf(player) == 3.5);
	CHECK(MusicPlayerSetTime(player, -2.0) == noErr);
	CHECK(timeOf(player) == -2.0);
	CHECK(MusicPlayerSetTime(player, NAN) == kAudio_ParamError);
	CHECK(timeOf(player) == -2.0);
	CHECK(MusicPlayerSetTime(player, 0) == noErr);

	// Rates: greater than zero. GUESS: kAudio_ParamError otherwise.
	CHECK(MusicPlayerSetPlayRateScalar(player, 0) == kAudio_ParamError);
	CHECK(MusicPlayerSetPlayRateScalar(player, -1) == kAudio_ParamError);
	CHECK(MusicPlayerGetPlayRateScalar(player, &rate) == noErr && rate == 1.0);
	CHECK(MusicPlayerSetPlayRateScalar(player, 8.0) == noErr);
	CHECK(MusicPlayerGetPlayRateScalar(player, &rate) == noErr && rate == 8.0);

	// Playing: at 60 beats per minute and a rate of 8, 0.25 seconds are at least 2 beats.
	CHECK(MusicPlayerStart(player) == noErr);
	CHECK(playing(player));
	// GUESS: starting a player that is playing is noErr.
	CHECK(MusicPlayerStart(player) == noErr);
	usleep(250000);
	double t1 = timeOf(player);
	CHECK(t1 >= 2.0 * 0.95);
	usleep(10000);
	CHECK(timeOf(player) >= t1);

	// A sequence cannot be set while the player plays. GUESS: kAudioToolboxErr_InvalidPlayerState.
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
	CHECK(stopped >= t2);
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
	// GUESS: setting a sequence leaves the time of the player as it is.
	MusicSequence seq2 = NULL;
	CHECK(NewMusicSequence(&seq2) == noErr);
	CHECK(MusicPlayerSetTime(player, 7.0) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq2) == noErr);
	CHECK(timeOf(player) == 7.0);
	CHECK(DisposeMusicSequence(seq) == noErr);
	CHECK(DisposeMusicSequence(seq2) == kAudioToolboxErr_InvalidPlayerState);
	CHECK(MusicPlayerSetSequence(player, NULL) == noErr);
	CHECK(MusicPlayerGetSequence(player, &got) == kAudioToolboxErr_NoSequence);
	CHECK(DisposeMusicSequence(seq2) == noErr);

	// Disposing a player releases its sequence, even while it plays.
	CHECK(NewMusicSequence(&seq) == noErr);
	CHECK(MusicPlayerSetSequence(player, seq) == noErr);
	CHECK(MusicPlayerStart(player) == noErr);
	CHECK(DisposeMusicPlayer(player) == noErr);
	CHECK(DisposeMusicSequence(seq) == noErr);

	CHECK(DisposeMusicPlayer(NULL) == kAudio_ParamError);

	if (g_failures == 0)
		printf("PASS\n");
	return g_failures ? 1 : 0;
}
