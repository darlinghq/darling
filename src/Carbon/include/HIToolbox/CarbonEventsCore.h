#ifndef _Carbon_Carbon_Events_Core_H_
#define _Carbon_Carbon_Events_Core_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int EventRef;
typedef void* EventHandlerCallRef;
typedef OSType EventParamType;

typedef double EventTime;
typedef uint32_t EventMask;
typedef void* EventTargetRef;
typedef OSType EventParamName;
typedef struct EventTypeSpec { uint32_t params[2]; } EventTypeSpec;
typedef EventTime EventTimeout;
typedef void* EventHandlerProcPtr;
typedef uint32_t EventAttributes;
typedef uint32_t EventQueueRef;
typedef void* EventHandlerUPP;
typedef int16_t EventPriority;
typedef void* EventHandlerRef;


typedef struct HICommand { uint8_t data[16]; } HICommand;
typedef struct KeyMap { char data[16]; } KeyMap;


OSStatus CallNextEventHandler(EventHandlerCallRef a, EventRef b);
OSStatus CreateEvent(CFAllocatorRef a, UInt32 b, UInt32 c, EventTime d, EventAttributes e, EventRef * f);
UInt32 GetEventClass(EventRef a);
UInt32 GetEventKind(EventRef a);
OSStatus GetEventParameter(EventRef a, EventParamName b, EventParamType c, EventParamType * d, UInt32 e, UInt32 * f, void * g);

OSStatus GetMainEventQueue();
OSStatus InstallEventHandler(EventTargetRef a, EventHandlerUPP b, UInt32 c, const EventTypeSpec * d, void * e, EventHandlerRef * f);


EventHandlerUPP NewEventHandlerUPP(EventHandlerProcPtr a); 
OSStatus PostEventToQueue(EventQueueRef a, EventRef b, EventPriority c);

OSStatus ReceiveNextEvent(UInt32 a, const EventTypeSpec * b, EventTimeout c, Boolean d, EventRef * e);

OSStatus ReleaseEvent(EventRef a);
OSStatus RemoveEventHandler(EventHandlerRef a);

OSStatus SendEventToEventTarget(EventRef a, EventTargetRef b);

#ifdef __cplusplus
}
#endif

#endif
