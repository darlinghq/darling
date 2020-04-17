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

#ifndef _AT_AUDIO_COMPONENT_MANAGER_H
#define _AT_AUDIO_COMPONENT_MANAGER_H
#include <AudioToolbox/AudioComponent.h>
#include <CoreFoundation/CFBundle.h>
#include <mutex>
#include <unordered_map>
#include <stdint.h>
#include <string>
#include <memory>
#include <vector>

class __attribute__((visibility("hidden"))) AudioComponentManager
{
private:
	AudioComponentManager();
	void discoverComponents();
	void discoverComponents(const char* dir);
	void analyzeComponent(CFBundleRef bundle);

	static CFBundleRef bundleFromPath(const char* path);

	struct RegisteredComponent
	{
		AudioComponent id;
		AudioComponentDescription desc;
		std::string name;
		uint32_t version;
		AudioComponentFactoryFunction factory;
		AudioComponentFlags flags = 0;

		CFBundleRef loadedBundle = nullptr;
		std::string bundlePath, entryPointName;

		int instances = 0;
	};
	
	RegisteredComponent* getById(AudioComponent component);
public:
	static AudioComponentManager* instance();
	// Returns true if it looks like an instance managed by this class.
	// False means it is probably managed by the Carbon Component Manager.
	static bool isOurInstance(AudioComponentInstance instance);
	static bool isOurInstance(AudioComponent component);

	AudioComponent registerComponent(const AudioComponentDescription* desc, const char* name,
		uint32_t version, AudioComponentFactoryFunction factory);
	AudioComponent registerComponent(const AudioComponentDescription* desc, const char* name,
		uint32_t version, const char* bundlePath, const char* entryPointName, AudioComponentFlags flags);

	OSStatus instantiate(AudioComponent c, AudioComponentInstance* out);
	OSStatus dispose(AudioComponentInstance instance);

	AudioComponentPlugInInterface* instanceInterface(AudioComponentInstance instance);

	std::vector<AudioComponent> findMatching(const AudioComponentDescription* cd);

	OSStatus getDescription(AudioComponent c, AudioComponentDescription* out);
	OSStatus getName(AudioComponent c, const char** name);
	OSStatus getVersion(AudioComponent c, uint32_t* version);

	AudioComponent getComponent(AudioComponentInstance instance);
private:

	std::unordered_map<AudioComponent, RegisteredComponent> m_components;
	uint32_t m_nextComponentId = 0x4000 | 0x80000000;
	std::recursive_mutex m_componentsMutex;

	struct ComponentInstanceData
	{
		AudioComponent component;
		AudioComponentPlugInInterface* object;
	};

	std::unordered_map<AudioComponentInstance, ComponentInstanceData> m_componentInstances;
	uint32_t m_nextComponentInstanceId = 0x1000 | 0x80000000;
	std::recursive_mutex m_componentInstancesMutex;
};

#endif
