/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

// Music players (MusicPlayer.h): the state of a player, which sequence it has, its position in
// beats, whether it is playing and its rate.
//
// Nothing is played: no event of the sequence is sent to an audio unit or a MIDI endpoint and no
// user callback is called. A started player has a position that advances, as documented: it
// increases, scaled by the play rate through the tempo map, until the player is stopped, whether
// or not the sequence has run out of events. The clock is the host clock of CoreAudio.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <CoreAudio/HostTime.h>
#include <cmath>
#include <new>
#include "MusicSequenceImpl.h"

namespace
{

// The position of the player, in beats, now.
MusicTimeStamp currentTime(const OpaqueMusicPlayer* player)
{
	if (!player->playing)
		return player->time;

	const UInt64 now = AudioGetCurrentHostTime();
	const Float64 elapsed = now > player->anchorHostTime ? (now - player->anchorHostTime) / AudioGetHostClockFrequency() : 0;
	const Float64 seconds = musicimpl::secondsForBeats(player->sequence, player->time) + elapsed * player->rate;
	return musicimpl::beatsForSeconds(player->sequence, seconds);
}

// Runs f, which may allocate, and reports a failed allocation as a result code.
template <typename F>
OSStatus guarded(F f)
{
	try
	{
		return f();
	}
	catch (const std::bad_alloc&)
	{
		return kAudio_MemFullError;
	}
}

// Makes the current position the anchor, as a playing player must when its rate or position change.
void reanchor(OpaqueMusicPlayer* player, MusicTimeStamp time)
{
	player->time = time;
	player->anchorHostTime = AudioGetCurrentHostTime();
}

}

OSStatus NewMusicPlayer(MusicPlayer* outPlayer)
{
	if (!outPlayer)
		return kAudio_ParamError;

	OpaqueMusicPlayer* player = new (std::nothrow) OpaqueMusicPlayer;
	if (!player)
		return kAudio_MemFullError;

	*outPlayer = player;
	return noErr;
}

OSStatus DisposeMusicPlayer(MusicPlayer inPlayer)
{
	if (!inPlayer)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	if (inPlayer->sequence)
		inPlayer->sequence->playerCount--;
	delete inPlayer;
	return noErr;
}

OSStatus MusicPlayerSetSequence(MusicPlayer inPlayer, MusicSequence inSequence)
{
	if (!inPlayer)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A sequence cannot be set while the player is playing: kAudioToolboxErr_InvalidPlayerState (observed on
	// macOS 26.7.3). A NULL sequence removes the sequence (the header allows it).
	if (inPlayer->playing)
		return kAudioToolboxErr_InvalidPlayerState;

	if (inPlayer->sequence)
		inPlayer->sequence->playerCount--;
	inPlayer->sequence = inSequence;
	if (inSequence)
		inSequence->playerCount++;
	// QUIRK: setting a sequence, or none, puts the time back to zero. Evidence: black-box observation on
	// macOS 26.7.3, tier 2. Test: tests/src/audiotoolbox_music_player.c, "setting a sequence".
	inPlayer->time = 0;
	return noErr;
}

OSStatus MusicPlayerGetSequence(MusicPlayer inPlayer, MusicSequence* outSequence)
{
	if (!inPlayer || !outSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outSequence = inPlayer->sequence;
	if (!inPlayer->sequence)
		return kAudioToolboxErr_NoSequence;
	return noErr;
}

OSStatus MusicPlayerSetTime(MusicPlayer inPlayer, MusicTimeStamp inTime)
{
	// The time is not range checked: a time below zero or not a number is accepted (observed on macOS 26.7.3).
	if (!inPlayer)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A playing player continues from the new time.
	reanchor(inPlayer, inTime);
	return noErr;
}

OSStatus MusicPlayerGetTime(MusicPlayer inPlayer, MusicTimeStamp* outTime)
{
	if (!inPlayer || !outTime)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	return guarded([&] { *outTime = currentTime(inPlayer); return noErr; });
}

OSStatus MusicPlayerPreroll(MusicPlayer inPlayer)
{
	if (!inPlayer)
		return kAudio_ParamError;

	// There is nothing to prepare, as nothing is rendered. A player with no sequence is
	// kAudioToolboxErr_NoSequence (observed on macOS 26.7.3 and 10.14.4).
	std::lock_guard<std::mutex> guard(musicimpl::lock());
	if (!inPlayer->sequence)
		return kAudioToolboxErr_NoSequence;
	return noErr;
}

OSStatus MusicPlayerStart(MusicPlayer inPlayer)
{
	if (!inPlayer)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A player with no sequence is kAudioToolboxErr_NoSequence (observed on macOS 26.7.3 and 10.14.4); starting a
	// player that is already playing is noErr and changes nothing (observed on 26.7.3). macOS also fails when
	// the destinations of the sequence cannot be started (no audio output), which cannot happen here.
	if (!inPlayer->sequence)
		return kAudioToolboxErr_NoSequence;
	if (inPlayer->playing)
		return noErr;

	inPlayer->playing = true;
	reanchor(inPlayer, inPlayer->time);
	return noErr;
}

OSStatus MusicPlayerStop(MusicPlayer inPlayer)
{
	if (!inPlayer)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	// A player with no sequence is kAudioToolboxErr_NoSequence here too (observed on macOS 26.7.3 and 10.14.4).
	if (!inPlayer->sequence)
		return kAudioToolboxErr_NoSequence;
	if (!inPlayer->playing)
		return noErr;

	// The player stays where it got to.
	return guarded([&] { inPlayer->time = currentTime(inPlayer); inPlayer->playing = false; return noErr; });
}

OSStatus MusicPlayerIsPlaying(MusicPlayer inPlayer, Boolean* outIsPlaying)
{
	if (!inPlayer || !outIsPlaying)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outIsPlaying = inPlayer->playing;
	return noErr;
}

OSStatus MusicPlayerSetPlayRateScalar(MusicPlayer inPlayer, Float64 inScaleRate)
{
	// The rate must be greater than zero (not zero, not negative, not a number): kAudio_ParamError otherwise
	// (observed on macOS 26.7.3); an infinite rate is accepted there.
	if (!inPlayer || !(inScaleRate > 0))
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A playing player keeps its position and goes on at the new rate.
	return guarded([&] { reanchor(inPlayer, currentTime(inPlayer)); inPlayer->rate = inScaleRate; return noErr; });
}

OSStatus MusicPlayerGetPlayRateScalar(MusicPlayer inPlayer, Float64* outScaleRate)
{
	if (!inPlayer || !outScaleRate)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outScaleRate = inPlayer->rate;
	return noErr;
}

OSStatus MusicPlayerGetHostTimeForBeats(MusicPlayer inPlayer, MusicTimeStamp inBeats, UInt64* outHostTime)
{
	if (!inPlayer || !outHostTime)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// Only a playing player has a host time for a beat, and none for a beat before the beat it started
	// at: kAudioToolboxErr_InvalidPlayerState for a player that is not playing and kAudio_ParamError for a beat
	// before the start (observed on macOS 26.7.3, which moves the start when the time or the rate of a playing
	// player is set).
	if (!inPlayer->playing)
		return kAudioToolboxErr_InvalidPlayerState;
	if (inBeats < inPlayer->time)
		return kAudio_ParamError;

	return guarded([&]() -> OSStatus {
		const Float64 seconds = (musicimpl::secondsForBeats(inPlayer->sequence, inBeats)
			- musicimpl::secondsForBeats(inPlayer->sequence, inPlayer->time)) / inPlayer->rate;
		// QUIRK: a beat whose host time is not a number or does not fit in 64 bits (infinity, NaN, 1e15 beats) is
		// not refused: the number of ticks since the start is then -2^63 as an x86 conversion gives it, and the
		// host time is the start plus that. Evidence: black-box observation on macOS 26.7.3 x86_64, tier 2.
		// Test: tests/src/audiotoolbox_music_player_host_time.c, "beats that do not fit".
		const Float64 ticks = seconds * AudioGetHostClockFrequency();
		const UInt64 offset = (ticks >= -9223372036854775808.0 && ticks < 9223372036854775808.0)
			? static_cast<UInt64>(static_cast<SInt64>(ticks)) : 0x8000000000000000ull;
		*outHostTime = inPlayer->anchorHostTime + offset;
		return noErr;
	});
}

OSStatus MusicPlayerGetBeatsForHostTime(MusicPlayer inPlayer, UInt64 inHostTime, MusicTimeStamp* outBeats)
{
	if (!inPlayer || !outBeats)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// As for the host time of a beat, with kAudio_ParamError for a host time before the start.
	if (!inPlayer->playing)
		return kAudioToolboxErr_InvalidPlayerState;
	if (inHostTime < inPlayer->anchorHostTime)
		return kAudio_ParamError;

	return guarded([&] {
		const Float64 elapsed = (inHostTime - inPlayer->anchorHostTime) / AudioGetHostClockFrequency();
		*outBeats = musicimpl::beatsForSeconds(inPlayer->sequence,
			musicimpl::secondsForBeats(inPlayer->sequence, inPlayer->time) + elapsed * inPlayer->rate);
		return noErr;
	});
}
