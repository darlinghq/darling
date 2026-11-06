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
#include <utility>
#include <vector>

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
	AUGraphClose(inGraph);
	delete inGraph;
	return noErr;
}

OSStatus AUGraphAddNode(AUGraph inGraph, const AudioComponentDescription* inDescription, AUNode* outNode)
{
	if (!inGraph || !inDescription || !outNode)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

	AUNode id = inGraph->nextNodeId++;
	AUGraphNodeEntry entry;
	entry.id = id;
	entry.description = *inDescription;

	// A node added to an open graph gets its unit right away.
	if (inGraph->isOpen)
	{
		entry.component = AudioComponentFindNext(nullptr, &entry.description);
		if (!entry.component)
			return kAUGraphErr_InvalidAudioUnit;

		OSStatus s = AudioComponentInstanceNew(entry.component, &entry.unit);
		if (s != noErr)
			return s;
	}

	inGraph->nodes.emplace(id, entry);
	*outNode = id;
	return noErr;
}

OSStatus AUGraphRemoveNode(AUGraph inGraph, AUNode inNode)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	auto it = inGraph->nodes.find(inNode);
	if (it == inGraph->nodes.end())
		return kAUGraphErr_NodeNotFound;
	if (it->second.unit)
		AudioComponentInstanceDispose(it->second.unit);
	inGraph->nodes.erase(it);

	inGraph->connections.erase(std::remove_if(inGraph->connections.begin(), inGraph->connections.end(),
		[&](const AUNodeConnection& c) { return c.sourceNode == inNode || c.destNode == inNode; }),
		inGraph->connections.end());
	inGraph->callbacks.erase(std::remove_if(inGraph->callbacks.begin(), inGraph->callbacks.end(),
		[&](const AUNodeRenderCallback& c) { return c.destNode == inNode; }),
		inGraph->callbacks.end());
	return noErr;
}

OSStatus AUGraphGetNodeCount(AUGraph inGraph, UInt32* outNumberOfNodes)
{
	if (!inGraph || !outNumberOfNodes)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outNumberOfNodes = static_cast<UInt32>(inGraph->nodes.size());
	return noErr;
}

OSStatus AUGraphGetIndNode(AUGraph inGraph, UInt32 inIndex, AUNode* outNode)
{
	if (!inGraph || !outNode || inIndex >= inGraph->nodes.size())
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	auto it = inGraph->nodes.begin();
	std::advance(it, inIndex);
	*outNode = it->first;
	return noErr;
}

OSStatus AUGraphOpen(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	if (inGraph->isOpen)
		return noErr;

	for (auto& kv : inGraph->nodes)
	{
		AUGraphNodeEntry& entry = kv.second;
		entry.component = AudioComponentFindNext(nullptr, &entry.description);

		OSStatus s = entry.component ? AudioComponentInstanceNew(entry.component, &entry.unit) : kAUGraphErr_InvalidAudioUnit;
		if (s != noErr)
		{
			for (auto& created : inGraph->nodes)
			{
				if (created.second.unit)
				{
					AudioComponentInstanceDispose(created.second.unit);
					created.second.unit = nullptr;
				}
			}
			return s;
		}
	}

	inGraph->isOpen = true;
	return noErr;
}

OSStatus AUGraphClose(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outIsOpen = inGraph->isOpen;
	return noErr;
}

// Re-applies every connection as a kAudioUnitProperty_MakeConnection and every
// input callback as a kAudioUnitProperty_SetRenderCallback on the destination
// node's AudioUnit -- this is what actually makes rendering pull from the right place; "Update" must be callable while running (per the
// public contract), so it only touches AudioUnit properties, never tears
// down nodes.
OSStatus AUGraphUpdate(AUGraph inGraph, Boolean* outIsUpdated)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

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

	for (const AUNodeRenderCallback& c : inGraph->callbacks)
	{
		auto dstIt = inGraph->nodes.find(c.destNode);
		if (dstIt == inGraph->nodes.end())
			continue;

		OSStatus s = AudioUnitSetProperty(dstIt->second.unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
			c.destInputNumber, &c.cback, sizeof(c.cback));
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

	auto connection = std::remove_if(inGraph->connections.begin(), inGraph->connections.end(), [&](const AUNodeConnection& c) {
		return c.destNode == inDestNode && c.destInputNumber == inDestInputNumber;
	});
	auto callback = std::remove_if(inGraph->callbacks.begin(), inGraph->callbacks.end(), [&](const AUNodeRenderCallback& c) {
		return c.destNode == inDestNode && c.destInputNumber == inDestInputNumber;
	});
	const bool hadConnection = connection != inGraph->connections.end();
	const bool hadCallback = callback != inGraph->callbacks.end();
	inGraph->connections.erase(connection, inGraph->connections.end());
	inGraph->callbacks.erase(callback, inGraph->callbacks.end());

	if (!inGraph->isOpen)
		return noErr;

	// The unit keeps its input until told otherwise.
	auto dstIt = inGraph->nodes.find(inDestNode);
	if (dstIt != inGraph->nodes.end() && dstIt->second.unit)
	{
		if (hadConnection)
		{
			AudioUnitConnection none = { nullptr, 0, inDestInputNumber };
			AudioUnitSetProperty(dstIt->second.unit, kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input,
				inDestInputNumber, &none, sizeof(none));
		}
		if (hadCallback)
		{
			AURenderCallbackStruct none = { nullptr, nullptr };
			AudioUnitSetProperty(dstIt->second.unit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
				inDestInputNumber, &none, sizeof(none));
		}
	}

	return AUGraphUpdate(inGraph, nullptr);
}

OSStatus AUGraphSetNodeInputCallback(AUGraph inGraph, AUNode inDestNode, UInt32 inDestInputNumber, const AURenderCallbackStruct* inInputCallback)
{
	if (!inGraph || !inInputCallback)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	if (inGraph->nodes.find(inDestNode) == inGraph->nodes.end())
		return kAUGraphErr_NodeNotFound;

	// An input is fed by one source: a callback replaces a previous connection or callback.
	AUGraphDisconnectNodeInput(inGraph, inDestNode, inDestInputNumber);

	inGraph->callbacks.push_back({ inDestNode, inDestInputNumber, *inInputCallback });

	if (inGraph->isOpen)
		return AUGraphUpdate(inGraph, nullptr);
	return noErr;
}

OSStatus AUGraphClearConnections(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

	std::vector<std::pair<AUNode, UInt32>> inputs;
	for (const AUNodeConnection& c : inGraph->connections)
		inputs.emplace_back(c.destNode, c.destInputNumber);
	for (const AUNodeRenderCallback& c : inGraph->callbacks)
		inputs.emplace_back(c.destNode, c.destInputNumber);

	for (const auto& input : inputs)
		AUGraphDisconnectNodeInput(inGraph, input.first, input.second);
	return noErr;
}

static AUNodeInteraction connectionInteraction(const AUNodeConnection& connection)
{
	AUNodeInteraction interaction;
	interaction.nodeInteractionType = kAUNodeInteraction_Connection;
	interaction.nodeInteraction.connection = connection;
	return interaction;
}

static AUNodeInteraction callbackInteraction(const AUNodeRenderCallback& callback)
{
	AUNodeInteraction interaction;
	interaction.nodeInteractionType = kAUNodeInteraction_InputCallback;
	interaction.nodeInteraction.inputCallback = callback;
	return interaction;
}

OSStatus AUGraphGetNumberOfInteractions(AUGraph inGraph, UInt32* outNumInteractions)
{
	if (!inGraph || !outNumInteractions)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outNumInteractions = static_cast<UInt32>(inGraph->connections.size() + inGraph->callbacks.size());
	return noErr;
}

OSStatus AUGraphGetInteractionInfo(AUGraph inGraph, UInt32 inInteractionIndex, AUNodeInteraction* outInteraction)
{
	if (!inGraph || !outInteraction)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

	if (inInteractionIndex < inGraph->connections.size())
	{
		*outInteraction = connectionInteraction(inGraph->connections[inInteractionIndex]);
		return noErr;
	}

	inInteractionIndex -= static_cast<UInt32>(inGraph->connections.size());
	if (inInteractionIndex < inGraph->callbacks.size())
	{
		*outInteraction = callbackInteraction(inGraph->callbacks[inInteractionIndex]);
		return noErr;
	}
	return paramErr;
}

OSStatus AUGraphCountNodeInteractions(AUGraph inGraph, AUNode inNode, UInt32* outNumInteractions)
{
	if (!inGraph || !outNumInteractions)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outNumInteractions = static_cast<UInt32>(std::count_if(inGraph->connections.begin(), inGraph->connections.end(),
		[&](const AUNodeConnection& c) { return c.sourceNode == inNode || c.destNode == inNode; })
		+ std::count_if(inGraph->callbacks.begin(), inGraph->callbacks.end(),
		[&](const AUNodeRenderCallback& c) { return c.destNode == inNode; }));
	return noErr;
}

OSStatus AUGraphGetNodeInteractions(AUGraph inGraph, AUNode inNode, UInt32* ioNumInteractions, AUNodeInteraction* outInteractions)
{
	if (!inGraph || !ioNumInteractions || !outInteractions)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);

	UInt32 count = 0;
	for (const AUNodeConnection& c : inGraph->connections)
	{
		if ((c.sourceNode == inNode || c.destNode == inNode) && count < *ioNumInteractions)
			outInteractions[count++] = connectionInteraction(c);
	}
	for (const AUNodeRenderCallback& c : inGraph->callbacks)
	{
		if (c.destNode == inNode && count < *ioNumInteractions)
			outInteractions[count++] = callbackInteraction(c);
	}

	*ioNumInteractions = count;
	return noErr;
}

OSStatus AUGraphInitialize(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	if (!inGraph->isOpen)
		return kAUGraphErr_CannotDoInCurrentContext;
	if (inGraph->isInitialized)
		return noErr;

	// Connections and callbacks made before the graph was opened haven't reached the units yet.
	OSStatus status = AUGraphUpdate(inGraph, nullptr);
	if (status != noErr)
		return status;

	for (auto& kv : inGraph->nodes)
	{
		OSStatus s = AudioUnitInitialize(kv.second.unit);
		if (s != noErr)
		{
			for (auto& initialized : inGraph->nodes)
				AudioUnitUninitialize(initialized.second.unit);
			return s;
		}
	}
	inGraph->isInitialized = true;
	return noErr;
}

OSStatus AUGraphUninitialize(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outIsInitialized = inGraph->isInitialized;
	return noErr;
}

OSStatus AUGraphStart(AUGraph inGraph)
{
	if (!inGraph)
		return paramErr;

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
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

	std::lock_guard<std::recursive_mutex> lock(inGraph->mutex);
	*outIsRunning = inGraph->isRunning;
	return noErr;
}
