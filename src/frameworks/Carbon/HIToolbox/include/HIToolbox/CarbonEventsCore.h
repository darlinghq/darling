#ifndef _Carbon_Carbon_Events_Core_H_
#define _Carbon_Carbon_Events_Core_H_

#include <CoreFoundation/CFBase.h>
#include <CoreServices/MacTypes.h>
#include <HIToolbox/Menus.h>
#include <CoreGraphics/CGEventTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
	eventAlreadyPostedErr = -9860,
	eventTargetBusyErr = -9861,
	eventClassInvalidErr = -9862,
	eventClassIncorrectErr = -9864,
	eventHandlerAlreadyInstalledErr = -9866,
	eventInternalErr = -9868,
	eventKindIncorrectErr = -9869,
	eventParameterNotFoundErr = -9870,
	eventNotHandledErr = -9874,
	eventLoopTimedOutErr = -9875,
	eventLoopQuitErr = -9876,
	eventNotInQueueErr = -9877,
	eventHotKeyExistsErr = -9878,
	eventHotKeyInvalidErr = -9879,
	eventPassToNextTargetErr = -9880,
};

enum {
	kEventClassKeyboard = 'keyb',
};

enum {
	kEventParamDirectObject = '----',
};

enum {
	kEventAttributeNone = 0,
	kEventAttributeUserEvent = 1 << 0,
	kEventAttributeMonitored = 1 << 3,
};

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
typedef UInt32 EventAttributes;
typedef struct OpaqueEventQueueRef* EventQueueRef;
typedef OSStatus (*EventHandlerProcPtr)(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData);
typedef EventHandlerProcPtr EventHandlerUPP;
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


OSStatus CallNextEventHandler(EventHandlerCallRef inCallRef, EventRef inEvent);
OSStatus CreateEvent(CFAllocatorRef inAllocator, OSType inClassID, UInt32 inKind, EventTime inEventTime, EventAttributes inAttributes, EventRef* outEvent);
OSStatus CreateEventWithCGEvent(CFAllocatorRef inAllocator, CGEventRef inEvent, EventAttributes inAttributes, EventRef* outEvent);
CGEventRef CopyEventCGEvent(EventRef inEvent);
EventRef RetainEvent(EventRef inEvent);
UInt32 GetEventRetainCount(EventRef inEvent);
void ReleaseEvent(EventRef inEvent);
OSType GetEventClass(EventRef inEvent);
UInt32 GetEventKind(EventRef inEvent);
EventTime GetEventTime(EventRef inEvent);
OSStatus GetEventParameter(EventRef inEvent, EventParamName inName, EventParamType inDesiredType, EventParamType* outActualType, ByteCount inBufferSize, ByteCount* outActualSize, void* outData);
OSStatus SetEventParameter(EventRef inEvent, EventParamName inName, EventParamType inType, ByteCount inSize, const void* inDataPtr);

EventQueueRef GetMainEventQueue();
OSStatus PostEventToQueue(EventQueueRef a, EventRef b, EventPriority c);
OSStatus ReceiveNextEvent(ItemCount a, const EventTypeSpec* b, EventTimeout c, Boolean d, EventRef* e);

// Event handlers are installed on the event targets of the process (GetApplicationEventTarget(),
// GetEventDispatcherTarget()); the newest handler of a target sees an event first.
OSStatus InstallEventHandler(EventTargetRef inTarget, EventHandlerUPP inHandler, ItemCount inNumTypes, const EventTypeSpec* inList, void* inUserData, EventHandlerRef* outRef);
OSStatus RemoveEventHandler(EventHandlerRef inHandlerRef);
OSStatus SendEventToEventTarget(EventRef inEvent, EventTargetRef inTarget);

EventHandlerUPP NewEventHandlerUPP(EventHandlerProcPtr userRoutine);
void DisposeEventHandlerUPP(EventHandlerUPP userUPP);
OSStatus InvokeEventHandlerUPP(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData, EventHandlerUPP userUPP);

UInt32 GetCurrentKeyModifiers(void);

// SPI: checks whether a key event is a system symbolic hot key.
Boolean _IsSymbolicHotKeyEvent(EventRef inEvent, UInt32* outHotKeyCode, Boolean* outEnabled);

#ifdef __cplusplus
}
#endif

#endif
