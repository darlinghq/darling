/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

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

#ifndef AUGRAPHNODE_H
#define AUGRAPHNODE_H
#include <AudioToolbox/AUGraph.h>
#include <vector>
#include <map>
#include <mutex>

struct AUGraphNodeEntry
{
	AUNode id;
	AudioComponentDescription description;
	AudioComponent component = nullptr;   // resolved in AUGraphOpen
	AudioUnit unit = nullptr;             // instantiated in AUGraphOpen
};

struct AUGraphData
{
	std::map<AUNode, AUGraphNodeEntry> nodes;
	std::recursive_mutex mutex;
	std::vector<AUNodeConnection> connections;
	std::vector<AUNodeRenderCallback> callbacks;
	AUNode nextNodeId = 1;
	bool isOpen = false;
	bool isInitialized = false;
	bool isRunning = false;
};

#endif /* AUGRAPHNODE_H */
