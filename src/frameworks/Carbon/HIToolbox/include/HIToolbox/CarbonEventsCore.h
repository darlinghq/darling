#ifndef _Carbon_Carbon_Events_Core_H_
#define _Carbon_Carbon_Events_Core_H_

#include <CoreFoundation/CFBase.h>
#include <CoreServices/MacTypes.h>
#include <HIToolbox/Menus.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueEventRef* EventRef;
typedef void* EventHandlerCallRef;
typedef OSType EventParamType;

typedef double EventTime;
typedef UInt16 EventMask;
typedef UInt16 EventKind;
typedef void* EventTargetRef;
typedef OSType EventParamName;
typedef struct EventTypeSpec { OSType eventClass; UInt32 eventKind; } EventTypeSpec;
typedef EventTime EventTimeout;
typedef void* EventHandlerProcPtr;
typedef UInt32 EventAttributes;
typedef struct OpaqueEventQueueRef* EventQueueRef;
typedef void* EventHandlerUPP;
typedef SInt16 EventPriority;
typedef void* EventHandlerRef;


typedef struct HICommand {
	UInt32 attributes;
	UInt32 commandID;
	struct {
		MenuRef menuRef;
		MenuItemIndex menuItemIndex;
	} menu;
} HICommand;
typedef UInt32 KeyMap[4];


OSStatus CallNextEventHandler(EventHandlerCallRef a, EventRef b);
OSStatus CreateEvent(CFAllocatorRef a, OSType b, UInt32 c, EventTime d, EventAttributes e, EventRef* f);
OSType GetEventClass(EventRef a);
UInt32 GetEventKind(EventRef a);
OSStatus GetEventParameter(EventRef a, EventParamName b, EventParamType c, EventParamType* d, ByteCount e, ByteCount* f, void* g);

EventQueueRef GetMainEventQueue();
OSStatus InstallEventHandler(EventTargetRef a, EventHandlerUPP b, ItemCount c, const EventTypeSpec* d, void* e, EventHandlerRef* f);


EventHandlerUPP NewEventHandlerUPP(EventHandlerProcPtr a);
OSStatus PostEventToQueue(EventQueueRef a, EventRef b, EventPriority c);

OSStatus ReceiveNextEvent(ItemCount a, const EventTypeSpec* b, EventTimeout c, Boolean d, EventRef* e);

void ReleaseEvent(EventRef a);
OSStatus RemoveEventHandler(EventHandlerRef a);

OSStatus SendEventToEventTarget(EventRef a, EventTargetRef b);

#ifdef __cplusplus
}
#endif

#endif
