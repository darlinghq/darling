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

#ifndef _CS_COMPONENT_MANAGER_H
#define _CS_COMPONENT_MANAGER_H
#include "Components.h"
#include <unordered_map>
#include <mutex>
#include <memory>
#include <vector>
#include <list>
#include <string>
#include <stdint.h>
#include <CoreServices/MacTypes.h>
#include <CoreFoundation/CFBundle.h>

class ComponentManager
{
private:
	ComponentManager();

	// Search in /System/Library/Components for components
	// Examine resource forks of the main binary, but also check for Resources/$BUNDLE_NAME.rsrc
	void discoverComponents();
	void discoverComponents(const char* dir);
	void analyzeComponent(CFBundleRef bundle);
	void analyzeComponent(CFBundleRef bundle, ResFileRefNum resFile);
public:
	static ComponentManager* instance();

	// Dynamic registration (in memory)
	Component registerComponent(const ComponentDescription* cd, ComponentRoutineUPP entryPoint, SInt16 flags,
		const char* name, const char* info, void* icon);
	
	// Bundle registration (automatic)
	Component registerComponent(const ComponentDescription* cd, SInt16 flags, const char* path, const char* entryPoint,
		const char* name, const char* info, void* icon);

	void setDefault(Component c, SInt16 flags);
	OSStatus unregisterComponent(Component c);

	std::vector<Component> findMatching(const ComponentDescription* cd);

	// Extract ComponentInstance value based on current ABI
	static ComponentInstance getComponentInstance(ComponentParameters* desc);

	// Instantiate and call kComponentOpenSelect
	OSStatus instantiate(Component c, ComponentInstance* out);
	// Call kComponentCloseSelect and free
	OSStatus dispose(ComponentInstance c);
	// Call component functions
	OSStatus dispatch(ComponentParameters* cp);

	Handle getStorage(ComponentInstance ci);
	void setStorage(ComponentInstance ci, Handle storage);
private:
	struct ComponentData
	{
		// Assigned identifier
		Component component;

		ComponentDescription cd;
		ComponentRoutineUPP entryPoint;
		std::string name, info;
		
		std::string bundlePath, entryPointName;

		uint32_t instances;
	};

	struct ComponentInstanceData
	{
		// instance of
		Component component;
		ComponentData* componentData;

		// private data (context) of this instance
		Handle storage;
	};
	std::unordered_map<ComponentInstance, ComponentInstanceData> m_componentInstances;
	uint32_t nextInstanceId = 0x2000;
	std::recursive_mutex m_componentInstancesMutex;

	std::list<ComponentData> m_components;
	// Contains pointers into m_components
	std::unordered_map<Component, ComponentData*> m_componentsMap;
	uint32_t nextComponentId = 0x1000;

	std::recursive_mutex m_componentsMutex;
};

#endif
