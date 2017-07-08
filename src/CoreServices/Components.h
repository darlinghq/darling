#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <MacTypes.h>

#ifdef __cplusplus
class CarbonComponent;
#else
struct __CarbonComponent;
typedef struct __CarbonComponent CarbonComponent;
#endif
struct ComponentDescription;
typedef struct ComponentDescription ComponentDescription;

typedef CarbonComponent* ComponentInstance;
typedef void* Component;

#ifdef __cplusplus
extern "C" {
#endif

Component FindNextComponent(Component prev, ComponentDescription* desc);
long CountComponents(ComponentDescription* desc);

OSErr OpenAComponent(Component comp, ComponentInstance* out);
ComponentInstance OpenComponent(Component comp);
OSErr CloseComponent(ComponentInstance inst);

#ifdef __cplusplus
}
#endif

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

