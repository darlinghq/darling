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

// Music sequences and their tracks (MusicPlayer.h): creating and disposing sequences, adding,
// removing and finding tracks.
//
// A new sequence has only its tempo track. The track count and the track accessors leave the tempo
// track out; it is reached through MusicSequenceGetTempoTrack.

#include <AudioToolbox/MusicPlayer.h>
#include <CarbonCore/MacErrors.h>
#include <algorithm>
#include <new>
#include "MusicSequenceImpl.h"

OpaqueMusicSequence::OpaqueMusicSequence()
{
	tempoTrack.sequence = this;
}

namespace
{

std::vector<std::unique_ptr<OpaqueMusicTrack>>::iterator findTrack(OpaqueMusicSequence* sequence, MusicTrack track)
{
	return std::find_if(sequence->tracks.begin(), sequence->tracks.end(),
		[track](const std::unique_ptr<OpaqueMusicTrack>& t) { return t.get() == track; });
}

}

OSStatus NewMusicSequence(MusicSequence* outSequence)
{
	if (!outSequence)
		return kAudio_ParamError;

	OpaqueMusicSequence* sequence = new (std::nothrow) OpaqueMusicSequence;
	if (!sequence)
		return kAudio_MemFullError;

	*outSequence = sequence;
	return noErr;
}

OSStatus DisposeMusicSequence(MusicSequence inSequence)
{
	if (!inSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// A sequence cannot be disposed while a player has it. GUESS: the code is
	// kAudioToolboxErr_InvalidPlayerState; the documentation says only that it cannot be done.
	if (inSequence->playerCount > 0)
		return kAudioToolboxErr_InvalidPlayerState;

	delete inSequence;
	return noErr;
}

OSStatus MusicSequenceNewTrack(MusicSequence inSequence, MusicTrack* outTrack)
{
	if (!inSequence || !outTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	std::unique_ptr<OpaqueMusicTrack> track(new (std::nothrow) OpaqueMusicTrack);
	if (!track)
		return kAudio_MemFullError;
	track->sequence = inSequence;

	// The new track goes after all existing ones.
	inSequence->tracks.push_back(std::move(track));
	*outTrack = inSequence->tracks.back().get();
	return noErr;
}

OSStatus MusicSequenceDisposeTrack(MusicSequence inSequence, MusicTrack inTrack)
{
	if (!inSequence || !inTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	auto it = findTrack(inSequence, inTrack);
	if (it == inSequence->tracks.end())
	{
		// GUESS: the documentation names no result for a track that is not in the sequence, or for the
		// tempo track, which cannot be removed. kAudioToolboxErr_TrackNotFound is the code the
		// header names for a missing track. The alternative was kAudio_ParamError.
		return kAudioToolboxErr_TrackNotFound;
	}

	inSequence->tracks.erase(it);
	return noErr;
}

OSStatus MusicSequenceGetTrackCount(MusicSequence inSequence, UInt32* outNumberOfTracks)
{
	if (!inSequence || !outNumberOfTracks)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outNumberOfTracks = static_cast<UInt32>(inSequence->tracks.size());
	return noErr;
}

OSStatus MusicSequenceGetIndTrack(MusicSequence inSequence, UInt32 inTrackIndex, MusicTrack* outTrack)
{
	if (!inSequence || !outTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	// An index outside 0 .. track count - 1 is kAudio_ParamError, as the header documents.
	if (inTrackIndex >= inSequence->tracks.size())
		return kAudio_ParamError;

	*outTrack = inSequence->tracks[inTrackIndex].get();
	return noErr;
}

OSStatus MusicSequenceGetTrackIndex(MusicSequence inSequence, MusicTrack inTrack, UInt32* outTrackIndex)
{
	if (!inSequence || !inTrack || !outTrackIndex)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());

	auto it = findTrack(inSequence, inTrack);
	if (it == inSequence->tracks.end())
	{
		// GUESS: "an error" is all the documentation says; see MusicSequenceDisposeTrack.
		return kAudioToolboxErr_TrackNotFound;
	}

	*outTrackIndex = static_cast<UInt32>(it - inSequence->tracks.begin());
	return noErr;
}

OSStatus MusicSequenceGetTempoTrack(MusicSequence inSequence, MusicTrack* outTrack)
{
	if (!inSequence || !outTrack)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outTrack = &inSequence->tempoTrack;
	return noErr;
}

OSStatus MusicTrackGetSequence(MusicTrack inTrack, MusicSequence* outSequence)
{
	if (!inTrack || !outSequence)
		return kAudio_ParamError;

	std::lock_guard<std::mutex> guard(musicimpl::lock());
	*outSequence = inTrack->sequence;
	return noErr;
}
