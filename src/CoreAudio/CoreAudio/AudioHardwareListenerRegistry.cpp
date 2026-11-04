// src/CoreAudio/CoreAudio/AudioHardwareListenerRegistry.cpp
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

#include "AudioHardwareListenerRegistry.h"
#include <algorithm>

void AudioHardwareListenerRegistry::add(uint32_t selector, void* proc, void* context)
{
	m_listeners[selector].push_back({proc, context});
}

void AudioHardwareListenerRegistry::remove(uint32_t selector, void* proc, void* context)
{
	auto it = m_listeners.find(selector);
	if (it == m_listeners.end())
		return;

	auto& entries = it->second;
	entries.erase(std::remove_if(entries.begin(), entries.end(), [&](const Entry& e) {
		return e.proc == proc && e.context == context;
	}), entries.end());
}

void AudioHardwareListenerRegistry::fire(uint32_t selector, const std::function<void(void*, void*)>& cb) const
{
	auto it = m_listeners.find(selector);
	if (it == m_listeners.end())
		return;

	for (const Entry& e : it->second)
		cb(e.proc, e.context);
}
