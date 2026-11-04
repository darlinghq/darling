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

#include "AUGraphNode.h"
#include <CarbonCore/MacErrors.h>
#include <algorithm>

OSStatus NewAUGraph(AUGraph* outGraph)
{
	if (!outGraph)
		return paramErr;
	*outGraph = new AUGraphData();
	return noErr;
}

OSStatus DisposeAUGraph(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	for (auto& kv : inGraph->nodes)
		if (kv.second.unit)
			AudioComponentInstanceDispose(kv.second.unit);
	delete inGraph;
	return noErr;
}

OSStatus AUGraphAddNode(AUGraph inGraph, const AudioComponentDescription* inDescription, AUNode* outNode)
{
	if (!inGraph || !inDescription || !outNode)
		return paramErr;

	AUNode id = inGraph->nextNodeId++;
	AUGraphNodeEntry entry;
	entry.id = id;
	entry.description = *inDescription;
	inGraph->nodes.emplace(id, entry);
	*outNode = id;
	return noErr;
}

OSStatus AUGraphRemoveNode(AUGraph inGraph, AUNode inNode)
{
	if (!inGraph)
		return paramErr;
	auto it = inGraph->nodes.find(inNode);
	if (it == inGraph->nodes.end())
		return kAUGraphErr_NodeNotFound;
	if (it->second.unit)
		AudioComponentInstanceDispose(it->second.unit);
	inGraph->nodes.erase(it);

	inGraph->connections.erase(std::remove_if(inGraph->connections.begin(), inGraph->connections.end(),
		[&](const AUNodeConnection& c) { return c.sourceNode == inNode || c.destNode == inNode; }),
		inGraph->connections.end());
	return noErr;
}

OSStatus AUGraphGetNodeCount(AUGraph inGraph, UInt32* outNumberOfNodes)
{
	if (!inGraph || !outNumberOfNodes)
		return paramErr;
	*outNumberOfNodes = static_cast<UInt32>(inGraph->nodes.size());
	return noErr;
}

OSStatus AUGraphGetIndNode(AUGraph inGraph, UInt32 inIndex, AUNode* outNode)
{
	if (!inGraph || !outNode || inIndex >= inGraph->nodes.size())
		return paramErr;
	auto it = inGraph->nodes.begin();
	std::advance(it, inIndex);
	*outNode = it->first;
	return noErr;
}

OSStatus AUGraphOpen(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (inGraph->isOpen)
		return noErr;

	for (auto& kv : inGraph->nodes)
	{
		AUGraphNodeEntry& entry = kv.second;
		entry.component = AudioComponentFindNext(nullptr, &entry.description);
		if (!entry.component)
			return kAUGraphErr_InvalidAudioUnit;

		OSStatus s = AudioComponentInstanceNew(entry.component, &entry.unit);
		if (s != noErr)
			return s;
	}

	inGraph->isOpen = true;
	return noErr;
}

OSStatus AUGraphClose(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (inGraph->isRunning)
		AUGraphStop(inGraph);
	if (inGraph->isInitialized)
		AUGraphUninitialize(inGraph);

	for (auto& kv : inGraph->nodes)
	{
		if (kv.second.unit)
		{
			AudioComponentInstanceDispose(kv.second.unit);
			kv.second.unit = nullptr;
		}
	}
	inGraph->isOpen = false;
	return noErr;
}

OSStatus AUGraphNodeInfo(AUGraph inGraph, AUNode inNode, AudioComponentDescription* outDescription, AudioUnit* outAudioUnit)
{
	if (!inGraph)
		return paramErr;
	auto it = inGraph->nodes.find(inNode);
	if (it == inGraph->nodes.end())
		return kAUGraphErr_NodeNotFound;

	if (outDescription)
		*outDescription = it->second.description;
	if (outAudioUnit)
		*outAudioUnit = it->second.unit;
	return noErr;
}

OSStatus AUGraphIsOpen(AUGraph inGraph, Boolean* outIsOpen)
{
	if (!inGraph || !outIsOpen)
		return paramErr;
	*outIsOpen = inGraph->isOpen;
	return noErr;
}

// Re-applies every connection as a kAudioUnitProperty_MakeConnection on the
// destination node's AudioUnit -- this is what actually makes rendering pull
// from the right place; "Update" must be callable while running (per the
// public contract), so it only touches AudioUnit properties, never tears
// down nodes.
OSStatus AUGraphUpdate(AUGraph inGraph, Boolean* outIsUpdated)
{
	if (!inGraph)
		return paramErr;

	if (!inGraph->isOpen)
	{
		if (outIsUpdated)
			*outIsUpdated = false;
		return noErr;
	}

	for (const AUNodeConnection& c : inGraph->connections)
	{
		auto srcIt = inGraph->nodes.find(c.sourceNode);
		auto dstIt = inGraph->nodes.find(c.destNode);
		if (srcIt == inGraph->nodes.end() || dstIt == inGraph->nodes.end())
			continue;

		AudioUnitConnection conn = { srcIt->second.unit, c.sourceOutputNumber, c.destInputNumber };
		OSStatus s = AudioUnitSetProperty(dstIt->second.unit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input,
			c.destInputNumber, &conn, sizeof(conn));
		if (s != noErr)
			return s;
	}

	if (outIsUpdated)
		*outIsUpdated = true;
	return noErr;
}

OSStatus AUGraphConnectNodeInput(AUGraph inGraph, AUNode inSourceNode, UInt32 inSourceOutputNumber, AUNode inDestNode, UInt32 inDestInputNumber)
{
	if (!inGraph)
		return paramErr;
	if (inGraph->nodes.find(inSourceNode) == inGraph->nodes.end() || inGraph->nodes.find(inDestNode) == inGraph->nodes.end())
		return kAUGraphErr_NodeNotFound;

	// Replace any existing connection into the same destination input, matching
	// real AUGraph semantics (one source per destination input at a time).
	AUGraphDisconnectNodeInput(inGraph, inDestNode, inDestInputNumber);

	inGraph->connections.push_back({ inSourceNode, inSourceOutputNumber, inDestNode, inDestInputNumber });

	if (inGraph->isOpen)
		return AUGraphUpdate(inGraph, nullptr);
	return noErr;
}

OSStatus AUGraphDisconnectNodeInput(AUGraph inGraph, AUNode inDestNode, UInt32 inDestInputNumber)
{
	if (!inGraph)
		return paramErr;

	auto it = std::remove_if(inGraph->connections.begin(), inGraph->connections.end(), [&](const AUNodeConnection& c) {
		return c.destNode == inDestNode && c.destInputNumber == inDestInputNumber;
	});
	inGraph->connections.erase(it, inGraph->connections.end());

	if (inGraph->isOpen)
		return AUGraphUpdate(inGraph, nullptr);
	return noErr;
}

OSStatus AUGraphClearConnections(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	inGraph->connections.clear();
	return noErr;
}

OSStatus AUGraphCountNodeInteractions(AUGraph inGraph, AUNode inNode, UInt32* outNumInteractions)
{
	if (!inGraph || !outNumInteractions)
		return paramErr;
	*outNumInteractions = static_cast<UInt32>(std::count_if(inGraph->connections.begin(), inGraph->connections.end(),
		[&](const AUNodeConnection& c) { return c.sourceNode == inNode || c.destNode == inNode; }));
	return noErr;
}

OSStatus AUGraphInitialize(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (!inGraph->isOpen)
		return kAUGraphErr_CannotDoInCurrentContext;
	if (inGraph->isInitialized)
		return noErr;

	for (auto& kv : inGraph->nodes)
	{
		OSStatus s = AudioUnitInitialize(kv.second.unit);
		if (s != noErr)
			return s;
	}
	inGraph->isInitialized = true;
	return noErr;
}

OSStatus AUGraphUninitialize(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (inGraph->isRunning)
		AUGraphStop(inGraph);

	for (auto& kv : inGraph->nodes)
		if (kv.second.unit)
			AudioUnitUninitialize(kv.second.unit);
	inGraph->isInitialized = false;
	return noErr;
}

OSStatus AUGraphIsInitialized(AUGraph inGraph, Boolean* outIsInitialized)
{
	if (!inGraph || !outIsInitialized)
		return paramErr;
	*outIsInitialized = inGraph->isInitialized;
	return noErr;
}

OSStatus AUGraphStart(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (!inGraph->isInitialized)
		return kAUGraphErr_CannotDoInCurrentContext;
	if (inGraph->isRunning)
		return noErr;

	// Only output-capable nodes (kAudioUnitType_Output) are started directly;
	// everything upstream is pulled via the render chain AUGraphUpdate() wired up.
	for (auto& kv : inGraph->nodes)
	{
		if (kv.second.description.componentType == kAudioUnitType_Output)
		{
			OSStatus s = AudioOutputUnitStart(kv.second.unit);
			if (s != noErr)
				return s;
		}
	}
	inGraph->isRunning = true;
	return noErr;
}

OSStatus AUGraphStop(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;
	if (!inGraph->isRunning)
		return noErr;

	for (auto& kv : inGraph->nodes)
		if (kv.second.description.componentType == kAudioUnitType_Output && kv.second.unit)
			AudioOutputUnitStop(kv.second.unit);
	inGraph->isRunning = false;
	return noErr;
}

OSStatus AUGraphIsRunning(AUGraph inGraph, Boolean* outIsRunning)
{
	if (!inGraph || !outIsRunning)
		return paramErr;
	*outIsRunning = inGraph->isRunning;
	return noErr;
}
