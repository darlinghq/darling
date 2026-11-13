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

// Block based property listeners, built on the function pointer listeners of AudioObject.
//
// A registration keeps its own copy of the block and, if there is one, a reference to the queue.
// With a queue the block is dispatched asynchronously; without one it is called directly.

#include <CoreAudio/AudioHardware.h>
#include <dispatch/dispatch.h>
#include <Block.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>
#include <vector>

namespace
{

struct BlockRegistration
{
	AudioObjectID object;
	AudioObjectPropertyAddress address;
	dispatch_queue_t queue;
	AudioObjectPropertyListenerBlock block; // the copy this registration owns
	AudioObjectPropertyListenerBlock given; // the pointer the caller passed, only compared, never called
};

std::mutex g_registrationsLock;
std::vector<BlockRegistration*> g_registrations;

bool sameAddress(const AudioObjectPropertyAddress& a, const AudioObjectPropertyAddress& b)
{
	return a.mSelector == b.mSelector && a.mScope == b.mScope && a.mElement == b.mElement;
}

OSStatus deliverToBlock(AudioObjectID, UInt32 count, const AudioObjectPropertyAddress* addresses, void* clientData)
{
	BlockRegistration* registration = static_cast<BlockRegistration*>(clientData);
	AudioObjectPropertyListenerBlock block = registration->block;

	if (registration->queue == nullptr)
	{
		block(count, addresses);
		return noErr;
	}

	// The addresses are only valid during this call, but the block runs later.
	size_t size = count * sizeof(AudioObjectPropertyAddress);
	AudioObjectPropertyAddress* copy = static_cast<AudioObjectPropertyAddress*>(malloc(size ? size : 1));
	if (copy == nullptr)
		return kAudioHardwareUnspecifiedError;
	memcpy(copy, addresses, size);

	// The dispatched block retains the listener block, so removing the registration while a
	// delivery is queued is safe.
	dispatch_async(registration->queue, ^{
		block(count, copy);
		free(copy);
	});
	return noErr;
}

}

OSStatus AudioObjectAddPropertyListenerBlock(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener)
{
	if (inAddress == nullptr || inListener == nullptr)
		return kAudioHardwareIllegalOperationError;

	BlockRegistration* registration = new BlockRegistration{inObjectID, *inAddress, inDispatchQueue, Block_copy(inListener), inListener};
	if (inDispatchQueue != nullptr)
		dispatch_retain(inDispatchQueue);

	OSStatus status = AudioObjectAddPropertyListener(inObjectID, inAddress, deliverToBlock, registration);
	if (status != noErr)
	{
		if (inDispatchQueue != nullptr)
			dispatch_release(inDispatchQueue);
		Block_release(registration->block);
		delete registration;
		return status;
	}

	std::lock_guard<std::mutex> guard(g_registrationsLock);
	g_registrations.push_back(registration);
	return noErr;
}

OSStatus AudioObjectRemovePropertyListenerBlock(AudioObjectID inObjectID, const AudioObjectPropertyAddress* inAddress,
		dispatch_queue_t inDispatchQueue, AudioObjectPropertyListenerBlock inListener)
{
	if (inAddress == nullptr || inListener == nullptr)
		return kAudioHardwareIllegalOperationError;

	BlockRegistration* registration = nullptr;
	{
		std::lock_guard<std::mutex> guard(g_registrationsLock);
		for (auto it = g_registrations.begin(); it != g_registrations.end(); ++it)
		{
			// Block_copy of a heap block returns the same block, so the pointer the caller registered
			// (or the copy it got back) identifies the registration.
			if ((*it)->object == inObjectID && sameAddress((*it)->address, *inAddress)
				&& (*it)->queue == inDispatchQueue && ((*it)->given == inListener || (*it)->block == inListener))
			{
				registration = *it;
				g_registrations.erase(it);
				break;
			}
		}
	}
	// QUIRK: removing a block that is not registered reports success. Not documented (the header only
	// says the result indicates success or failure). Evidence: black-box observation on macOS 26.7.3
	// (tier 2), removing the same block a second time returns 0.
	// Test: tests/src/newer-sdk-symbols.m, "removing a listener that is no longer registered succeeds"
	if (registration == nullptr)
		return noErr;

	OSStatus status = AudioObjectRemovePropertyListener(inObjectID, inAddress, deliverToBlock, registration);
	if (registration->queue != nullptr)
		dispatch_release(registration->queue);
	Block_release(registration->block);
	delete registration;
	return status;
}
