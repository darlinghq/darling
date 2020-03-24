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

#include "ComponentManager.h"
#include <CoreServices/MacErrors.h>
#include <CoreServices/Resources.h>

// Some clues about how this work can be found here:
// https://vintageapple.org/develop/pdf/develop-12_9212_December_1992.pdf

ComponentManager* ComponentManager::instance()
{
	static ComponentManager inst;
	return &inst;
}

ComponentManager::ComponentManager()
{
	discoverComponents();
}

void ComponentManager::discoverComponents()
{
	// TODO
}

Component ComponentManager::registerComponent(const ComponentDescription* cd, ComponentRoutineUPP entryPointIn, SInt16 flags,
	const char* name, const char* info, void* icon)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	if (flags & registerComponentNoDuplicates)
	{
		if (!findMatching(cd).empty())
			return nullptr;
	}

	ComponentData data;
	data.cd = *cd;
	data.entryPoint = entryPointIn;
	data.instances = 0;

	if (name)
		data.name = name;
	if (info)
		data.info = info;

	ComponentData* dataPtr;
	if (flags & registerComponentAfterExisting)
	{
		m_components.push_back(data);
		dataPtr = &m_components.back();
	}
	else
	{
		m_components.push_front(data);
		dataPtr = &m_components.front();
	}
	
	Component c = Component(uintptr_t(nextComponentId++));
	dataPtr->component = c;

	m_componentsMap.insert(std::make_pair(c, dataPtr));

	return c;
}

void ComponentManager::setDefault(Component c, SInt16 flags)
{
	// FIXME: This is wrong, because we don't honor flags
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);
	// Just move the element to the front

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return;

	for (auto it = m_components.begin(); it != m_components.end(); it++)
	{
		ComponentData* d = &(*it);
		if (d == itMap->second)
		{
			// TODO: Check whether element address is unchanged after this!
			m_components.splice(m_components.begin(), m_components, it);
			break;
		}
	}
}

OSStatus ComponentManager::unregisterComponent(Component c)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	if (itMap->second->instances != 0)
		return validInstancesExist;

	for (auto it = m_components.begin(); it != m_components.end(); it++)
	{
		ComponentData* d = &(*it);
		if (d == itMap->second)
		{
			m_components.erase(it);
			break;
		}
	}

	m_componentsMap.erase(itMap);
	return noErr;
}

std::vector<Component> ComponentManager::findMatching(const ComponentDescription* cd)
{
	std::vector<Component> rv;

	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	for (const ComponentData& d : m_components)
	{
		if (!cd->componentManufacturer || cd->componentManufacturer == d.cd.componentManufacturer)
		{
			if (!cd->componentSubType || cd->componentSubType == d.cd.componentSubType)
			{
				if (!cd->componentType || cd->componentType == d.cd.componentType)
				{
					rv.push_back(d.component);
				}
			}
		}
	}

	return rv;
}

ComponentInstance ComponentManager::getComponentInstance(ComponentParameters* desc)
{
#if __LP64__
	return (ComponentInstance) desc->params[0];
#else
	return (ComponentInstance) desc->params[(desc->paramSize / sizeof(long)) - 1];
#endif
}

OSStatus ComponentManager::instantiate(Component c, ComponentInstance* out)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	*out = nullptr;

	auto itMap = m_componentsMap.find(c);
	if (itMap == m_componentsMap.end())
		return invalidComponentID;

	ComponentInstanceData cid;

	cid.component = c;
	cid.componentData = itMap->second;
	cid.componentData->instances++;
	cid.storage = nullptr;

	l.unlock();

	std::unique_lock<std::recursive_mutex> l2(m_componentInstancesMutex);
	ComponentInstance inst = ComponentInstance(uintptr_t(nextInstanceId++));

	m_componentInstances.insert(std::make_pair(inst, cid));
	l2.unlock();

	ComponentParameters cp;
	cp.flags = 0;
	cp.paramSize = sizeof(void*);
	cp.what = kComponentOpenSelect;
	cp.params[0] = uintptr_t(inst);

	OSStatus status = dispatch(&cp);
	if (status != noErr)
	{
		// "Note that whenever an open request fails, the Component Manager always issues the close request."
		dispose(inst);
		return status;
	}

	*out = inst;
	return noErr;
}

OSStatus ComponentManager::dispose(ComponentInstance c)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto it = m_componentInstances.find(c);
	if (it == m_componentInstances.end())
		return badComponentInstance;

	ComponentParameters cp;
	cp.flags = 0;
	cp.paramSize = sizeof(void*);
	cp.what = kComponentCloseSelect;
	cp.params[0] = uintptr_t(c);

	OSStatus status = dispatch(&cp);

	m_componentInstances.erase(it);
	return status;
}

OSStatus ComponentManager::dispatch(ComponentParameters* cp)
{
	std::unique_lock<std::recursive_mutex> l(m_componentInstancesMutex);

	auto it = m_componentInstances.find(getComponentInstance(cp));
	if (it == m_componentInstances.end())
		return badComponentInstance;

	return it->second.componentData->entryPoint(cp, it->second.storage);
}

Handle ComponentManager::getStorage(ComponentInstance ci)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto it = m_componentInstances.find(ci);
	if (it == m_componentInstances.end())
		return nullptr;

	return it->second.storage;
}

void ComponentManager::setStorage(ComponentInstance ci, Handle storage)
{
	std::unique_lock<std::recursive_mutex> l(m_componentsMutex);

	auto it = m_componentInstances.find(ci);
	if (it == m_componentInstances.end())
		return;

	it->second.storage = storage;
}
