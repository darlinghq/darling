// src/CoreAudio/CoreAudio/AudioHardwareListenerRegistry.h
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

#ifndef AUDIOHARDWARELISTENERREGISTRY_H
#define AUDIOHARDWARELISTENERREGISTRY_H
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

// Pure bookkeeping for one AudioHardwareImpl object's property listeners.
// Deliberately independent of AudioObjectPropertyListenerProc's real
// signature so it can be unit tested without CoreAudio headers; the real
// proc/context pointers are stored as void* and the caller (AudioHardwareImpl)
// does the real cast when firing.
class AudioHardwareListenerRegistry
{
public:
	// The wildcards of kAudioObjectPropertyScopeWildcard and kAudioObjectPropertyElementWildcard.
	static const uint32_t kScopeWildcard = 0x2a2a2a2a;
	static const uint32_t kElementWildcard = 0xffffffffu;

	// A listener on a selector alone listens on every scope and element.
	void add(uint32_t selector, void* proc, void* context);
	void remove(uint32_t selector, void* proc, void* context);
	void add(uint32_t selector, uint32_t scope, uint32_t element, void* proc, void* context);
	void remove(uint32_t selector, uint32_t scope, uint32_t element, void* proc, void* context);

	// Invokes `cb(proc, context)` for every listener registered on `selector`.
	void fire(uint32_t selector, const std::function<void(void*, void*)>& cb) const;
	// Invokes `cb(proc, context)` for every listener whose scope and element cover the given ones
	// (a listener's wildcard matches anything, as does a wildcard in the changed address).
	void fire(uint32_t selector, uint32_t scope, uint32_t element, const std::function<void(void*, void*)>& cb) const;

	// True when no listener is registered on any selector.
	bool empty() const;

private:
	struct Entry { void* proc; void* context; uint32_t scope; uint32_t element; };
	mutable std::mutex m_mutex;
	std::unordered_map<uint32_t, std::vector<Entry>> m_listeners;
};

#endif /* AUDIOHARDWARELISTENERREGISTRY_H */
