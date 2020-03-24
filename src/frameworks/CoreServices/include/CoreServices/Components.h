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

#ifndef COMPONENTS_H
#define COMPONENTS_H
#include <MacTypes.h>
#include <CoreServices/Resources.h>

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

typedef struct ResourceSpec
{
	OSType resType;
	SInt16 resID;
} ResourceSpec;

typedef struct ComponentPlatformInfo
{
	SInt32 componentFlags;
	ResourceSpec component;
	SInt16 platformType;
} ComponentPlatformInfo;

struct ComponentDescription
{
	OSType componentType, componentSubType, componentManufacturer;
	UInt32 componentFlags, componentFlagsMask;
};

typedef struct ExtComponentResource
{
	ComponentDescription cs;
	ResourceSpec component;
	ResourceSpec componentName;
	ResourceSpec componentInfo;
	ResourceSpec componentIcon;
	SInt32 componentVersion;
	SInt32 componentRegisterFlags;
	SInt16 componentIconFamily;
	SInt32 count;
	ComponentPlatformInfo platformArray[1];
} ExtComponentResource;

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

OSErr OpenAComponentResFile(Component aComponent, ResFileRefNum* resRef);

#ifdef __cplusplus
}
#endif

enum
{
	kAppleManufacturer = 'appl'
};

#endif

