#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <MacTypes.h>

struct ComponentDescription;
typedef struct ComponentDescription ComponentDescription;

typedef struct __ComponentInstance* ComponentInstance;

typedef SInt32 ComponentResult;

typedef struct ComponentRecord
{
	long data[1];
} ComponentRecord;
typedef ComponentRecord* Component;

typedef struct ComponentParameters
{
	UInt8 flags;
	UInt8 paramSize;
	SInt16 what;
#if __LP64__
	UInt32 padding;
#endif
	// 64-bit platforms:
	// params[0] is ComponentInstance
	// params[1+] are arguments
	//
	// 32-bit platforms
	// params[last] is ComponentInstance
	// params[-last] are arguments in reverse order
	long params[1];
} ComponentParameters;

enum {
	registerComponentGlobal = 1,
	registerComponentNoDuplicates = 2,
	registerComponentAfterExisting = 4,
	registerComponentAliasesOnly = 8,
};

enum {
	kComponentOpenSelect = -1,
	kComponentCloseSelect = -2,
	kComponentCanDoSelect = -3,
	kComponentVersionSelect = -4,
	kComponentRegisterSelect = -5,
	kComponentTargetSelect = -6,
	kComponentUnregisterSelect = -7,
	kComponentGetMPWorkFunctionSelect = -8,
	kComponentExecuteWiredActionSelect = -9,
	kComponentGetPublicResourceSelect = -10,
};

enum {
	defaultComponentIdentical = 0,
	defaultComponentFlags = 1,
	defaultComponentAnyManufacturer = 2,
	defaultComponentAnySubType = 4,
	defaultComponentAnyFlagsAnyManufacturer = (defaultComponentFlags + defaultComponentAnyManufacturer),
	defaultComponentAnyFlagsAnyManufacturerAnySubType = (defaultComponentFlags + defaultComponentAnyManufacturer + defaultComponentAnySubType),
};

typedef ComponentResult (*ComponentRoutineProcPtr)(ComponentParameters* cp, Handle componentStorage);
typedef ComponentRoutineProcPtr ComponentRoutineUPP;

#ifdef __cplusplus
extern "C" {
#endif

Component FindNextComponent(Component prev, ComponentDescription* desc);
long CountComponents(ComponentDescription* desc);

OSErr OpenAComponent(Component comp, ComponentInstance* out);
ComponentInstance OpenComponent(Component comp);
OSErr CloseComponent(ComponentInstance inst);

Component RegisterComponent(ComponentDescription *cd, ComponentRoutineUPP componentEntryPoint, SInt16 global,
	Handle componentName, Handle componentInfo, Handle componentIcon);
OSErr SetDefaultComponent(Component c, SInt16 flags);

// ComponentResult CallComponentFunction(ComponentParameters *params, ComponentFunctionUPP func);
ComponentResult CallComponentDispatch(ComponentParameters *cp);

Handle GetComponentInstanceStorage(ComponentInstance aComponentInstance);
void SetComponentInstanceStorage(ComponentInstance aComponentInstance, Handle theStorage);

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

