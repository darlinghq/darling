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

#ifndef MUSICSEQUENCEIMPL_H
#define MUSICSEQUENCEIMPL_H

#include <AudioToolbox/MusicPlayer.h>
#include <memory>
#include <mutex>
#include <vector>

// The data model behind MusicSequence, MusicTrack, MusicEventIterator and MusicPlayer. A
// sequence owns its tempo track and its other tracks; a track points back to the sequence it
// belongs to. Every entry point holds musicimpl::lock() for its whole duration.

struct OpaqueMusicSequence;

struct OpaqueMusicTrack
{
	OpaqueMusicSequence* sequence = nullptr;
};

struct OpaqueMusicSequence
{
	OpaqueMusicSequence();

	OpaqueMusicTrack tempoTrack;
	std::vector<std::unique_ptr<OpaqueMusicTrack>> tracks;
};

namespace musicimpl
{

inline std::mutex& lock()
{
	static std::mutex m;
	return m;
}

}

#endif
