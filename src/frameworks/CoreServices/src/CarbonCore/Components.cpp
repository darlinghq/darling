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

#include <CoreServices/Components.h>
#include <CoreServices/MacErrors.h>
#include <CoreServices/MacMemory.h>
#include "ComponentManager.h"

Component FindNextComponent(Component prev, ComponentDescription* desc)
{
	std::vector<Component> components = ComponentManager::instance()->findMatching(desc);
	if (components.empty())
		return nullptr;

	if (prev == nullptr)
		return components[0];

	for (size_t i = 0; i < components.size()-1; i++)
	{
		if (components[i] == prev)
		{
			return components[i+1];
		}
	}

	return nullptr;
}

OSErr OpenADefaultComponent(OSType componentType, OSType componentSubType, ComponentInstance *ci)
{
	ComponentDescription desc = {
		.componentFlags = 0,
		.componentFlagsMask = 0,
		.componentManufacturer = 0,
		.componentType = componentType,
		.componentSubType = componentSubType,
	};
	Component c = FindNextComponent(nullptr, &desc);

	if (!c)
	{
		*ci = nullptr;
		return invalidComponentID;
	}

	return OpenAComponent(c, ci);
}

long CountComponentInstances(Component aComponent)
{
	return ComponentManager::instance()->componentInstances(aComponent);
}

long CountComponents(ComponentDescription* desc)
{
	return ComponentManager::instance()->findMatching(desc).size();
}

OSErr OpenAComponent(Component comp, ComponentInstance* out)
{
	return ComponentManager::instance()->instantiate(comp, out);
}

ComponentInstance OpenComponent(Component comp)
{
	ComponentInstance rv;
	if (ComponentManager::instance()->instantiate(comp, &rv) == noErr)
		return rv;
	return nullptr;
}

OSErr CloseComponent(ComponentInstance inst)
{
	return ComponentManager::instance()->dispose(inst);
}

Component RegisterComponent(ComponentDescription *cd, ComponentRoutineUPP componentEntryPoint, SInt16 global,
	Handle componentName, Handle componentInfo, Handle componentIcon)
{
	return ComponentManager::instance()->registerComponent(cd, componentEntryPoint, global,
		(const char*) componentName, (const char*) componentInfo, (void*) componentIcon);
}

OSErr SetDefaultComponent(Component c, SInt16 flags)
{
	ComponentManager::instance()->setDefault(c, flags);
	return noErr;
}

ComponentResult CallComponentDispatch(ComponentParameters *cp)
{
	return ComponentManager::instance()->dispatch(cp);
}

Handle GetComponentInstanceStorage(ComponentInstance aComponentInstance)
{
	return ComponentManager::instance()->getStorage(aComponentInstance);
}

void SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage)
{
	ComponentManager::instance()->setStorage(aComponentInstance, theStorage);
}

OSErr OpenAComponentResFile(Component aComponent, ResFileRefNum* resRef)
{
	return ComponentManager::instance()->resFileForComponent(aComponent, resRef);
}

static void stringToHandle(const std::string& str, Handle h)
{
	SetHandleSize(h, str.length() + 1);
	Ptr p = *h;

	p[0] = str.length();
	memcpy(p+1, str.c_str(), str.length());
}

OSErr GetComponentInfo(Component aComponent, ComponentDescription *cd,
	Handle componentName, Handle componentInfo, Handle componentIcon)
{
	ComponentManager::ComponentData cmd;
	OSStatus status;

	status = ComponentManager::instance()->componentData(aComponent, &cmd);
	if (status != noErr)
		return status;
	
	if (cd)
		*cd = cmd.cd;
	if (componentName)
		stringToHandle(cmd.name, componentName);
	if (componentInfo)
		stringToHandle(cmd.info, componentInfo);

	if (componentIcon)
		EmptyHandle(componentIcon);

	return noErr;
}

ComponentResult CallComponentCanDo(ComponentInstance ci, SInt16 ftnNumber)
{
	ComponentParameters* params = (ComponentParameters*) alloca(sizeof(ComponentParameters) + sizeof(long));

	params->flags = 0;
	params->what = kComponentCanDoSelect;
	params->paramSize = sizeof(long) * 2;

#if __LP64__
	params->params[0] = long(ci);
	params->params[1] = ftnNumber;
#else

	params->params[1] = long(ci);
#ifdef __ppc__
	params->params[0] = long(ftnNumber) << 16;
#else
	params->params[0] = ftnNumber;
#endif

#endif
	return ComponentManager::instance()->dispatch(params);
}

OSErr CloseComponentResFile(ResFileRefNum refnum)
{
	CloseResFile(refnum);
	return ResError();
}

Component GetComponentIDFromComponentInstance(ComponentInstance inst)
{
	return ComponentManager::instance()->componentID(inst);
}
