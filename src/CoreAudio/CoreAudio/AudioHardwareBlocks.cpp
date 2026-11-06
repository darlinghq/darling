/*
This file is part of Darling.

Copyright (C) 2026 Darling Team

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

#include <CoreAudio/AudioHardware.h>
#include <CarbonCore/MacErrors.h>
#include <dispatch/dispatch.h>
#include <Block.h>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <vector>

// Block-based listeners are layered on the function-pointer API: each block
// registration owns a trampoline context that forwards notifications to the
// caller's dispatch queue.
namespace
{
	struct BlockListener
	{
		AudioObjectID object;
		AudioObjectPropertyAddress address;
		dispatch_queue_t queue;
		AudioObjectPropertyListenerBlock block;
		AudioObjectPropertyListenerBlock key; // the block as originally passed in
	};

	std::mutex g_blockListenersLock;
	std::vector<BlockListener*> g_blockListeners;

	bool addressesEqual(const AudioObjectPropertyAddress& a, const AudioObjectPropertyAddress& b)
	{
		return a.mSelector == b.mSelector && a.mScope == b.mScope && a.mElement == b.mElement;
	}

	OSStatus blockListenerTrampoline(AudioObjectID, UInt32 inNumberAddresses,
			const AudioObjectPropertyAddress inAddresses[], void* inClientData)
	{
		BlockListener* listener = static_cast<BlockListener*>(inClientData);
		AudioObjectPropertyAddress* addresses = static_cast<AudioObjectPropertyAddress*>(
				malloc(sizeof(AudioObjectPropertyAddress) * (inNumberAddresses ? inNumberAddresses : 1)));
		if (!addresses)
			return noErr;
		memcpy(addresses, inAddresses, sizeof(AudioObjectPropertyAddress) * inNumberAddresses);

		AudioObjectPropertyListenerBlock block = listener->block;
		dispatch_async(listener->queue, ^{
			block(inNumberAddresses, addresses);
			free(addresses);
		});
		return noErr;
	}
}

OSStatus AudioObjectAddPropertyListenerBlock(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener)
{
	if (!inAddress || !inListener)
		return kAudioHardwareIllegalOperationError;

	BlockListener* listener = new BlockListener;
	listener->object = inObjectID;
	listener->address = *inAddress;
	listener->queue = inDispatchQueue ? inDispatchQueue : dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
	dispatch_retain(listener->queue);
	listener->key = inListener;
	listener->block = Block_copy(inListener);

	OSStatus status = AudioObjectAddPropertyListener(inObjectID, inAddress, blockListenerTrampoline, listener);
	if (status != noErr)
	{
		Block_release(listener->block);
		dispatch_release(listener->queue);
		delete listener;
		return status;
	}

	std::lock_guard<std::mutex> guard(g_blockListenersLock);
	g_blockListeners.push_back(listener);
	return noErr;
}

OSStatus AudioObjectRemovePropertyListenerBlock(AudioObjectID inObjectID,
		const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener)
{
	if (!inAddress || !inListener)
		return kAudioHardwareIllegalOperationError;

	dispatch_queue_t queue = inDispatchQueue ? inDispatchQueue : dispatch_get_global_queue(QOS_CLASS_DEFAULT, 0);
	BlockListener* found = nullptr;

	{
		std::lock_guard<std::mutex> guard(g_blockListenersLock);
		for (auto it = g_blockListeners.begin(); it != g_blockListeners.end(); ++it)
		{
			BlockListener* l = *it;
			if (l->object == inObjectID && addressesEqual(l->address, *inAddress)
					&& l->queue == queue && (l->key == inListener || l->block == inListener))
			{
				found = l;
				g_blockListeners.erase(it);
				break;
			}
		}
	}

	if (!found)
		return kAudioHardwareBadObjectError;

	OSStatus status = AudioObjectRemovePropertyListener(inObjectID, inAddress, blockListenerTrampoline, found);
	Block_release(found->block);
	dispatch_release(found->queue);
	delete found;
	return status;
}
