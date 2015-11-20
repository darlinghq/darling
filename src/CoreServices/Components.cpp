#include "darling-config.h"
#include "Components.h"
#include "ComponentsInternal.h"
#include "darling-config.h"
#include <AudioUnit/AUComponent.h>
#include <AudioUnit/AudioUnitBase.h>
#include <CoreServices/MacErrors.h>
#include <util/debug.h>

Component FindNextComponent(Component prev, ComponentDescription* desc)
{
	TRACE2(prev, desc);
#ifdef FRAMEWORK_COREAUDIO
	if ((desc->componentType & 0xffff0000) == kComponentTypeAudioUnit)
	{
		return AudioComponentFindNext((AudioComponent) prev, (AudioComponentDescription*) desc);
	}
	else
#endif
		return nullptr;
}

long CountComponents(ComponentDescription* desc)
{
#ifdef FRAMEWORK_COREAUDIO
	if ((desc->componentType & 0xffff0000) == kComponentTypeAudioUnit)
	{
		return AudioComponentCount((AudioComponentDescription*) desc);
	}
	else
#endif
		return 0;
}

OSErr OpenAComponent(Component comp, ComponentInstance* out)
{
	TRACE1(comp);

	if (!out)
		return paramErr;

	*out = nullptr;

#ifdef FRAMEWORK_COREAUDIO
	if (GetComponentType(comp) == kComponentTypeAudioUnit)
	{
		AudioComponentInstance inst;
		OSErr err;

		err = AudioComponentInstanceNew((AudioComponent) comp, &inst);

		*out = inst;
		return err;
	}
	else
#endif
		return unimpErr;
}

ComponentInstance OpenComponent(Component comp)
{
	TRACE1(comp);

	ComponentInstance inst;
	OpenAComponent(comp, &inst);
	return inst;
}

OSErr CloseComponent(ComponentInstance inst)
{
	TRACE1(inst);

	delete inst;
	return noErr;
}

