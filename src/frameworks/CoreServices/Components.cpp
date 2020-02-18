#include "darling-config.h"
#include "Components.h"
#include "ComponentsInternal.h"
#include "darling-config.h"
#include <CoreServices/MacErrors.h>

#define TRACE1(x)
#define TRACE2(x,y)

Component FindNextComponent(Component prev, ComponentDescription* desc)
{
	TRACE2(prev, desc);
	return nullptr;
}

long CountComponents(ComponentDescription* desc)
{
	return 0;
}

OSErr OpenAComponent(Component comp, ComponentInstance* out)
{
	TRACE1(comp);

	if (!out)
		return paramErr;

	*out = nullptr;

	return unimpErr;
}

ComponentInstance OpenComponent(Component comp)
{
	// TRACE1(comp);

	ComponentInstance inst;
	OpenAComponent(comp, &inst);
	return inst;
}

OSErr CloseComponent(ComponentInstance inst)
{
	// TRACE1(inst);

	delete inst;
	return noErr;
}

