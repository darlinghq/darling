#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <CoreServices/MacTypes.h>

class CarbonComponent;
struct ComponentDescription;

typedef CarbonComponent* ComponentInstance;
typedef void* Component;

extern "C" {

Component FindNextComponent(Component prev, ComponentDescription* desc);
long CountComponents(ComponentDescription* desc);

OSErr OpenAComponent(Component comp, ComponentInstance* out);
ComponentInstance OpenComponent(Component comp);
OSErr CloseComponent(ComponentInstance inst);

}

struct ComponentDescription
{
	OSType componentType, componentSubType, componentManufacturer;
	UInt32 componentFlags, componentFlagsMask;
};

enum
{
	kAppleManufacturer = 'appl'
};

#endif

