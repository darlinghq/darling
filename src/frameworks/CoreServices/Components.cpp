#include "Components.h"
#include <CoreServices/MacErrors.h>
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

