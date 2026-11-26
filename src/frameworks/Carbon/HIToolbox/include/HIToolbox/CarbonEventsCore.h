#ifndef _Carbon_Carbon_Events_Core_H_
#define _Carbon_Carbon_Events_Core_H_

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFRunLoop.h>
#include <CoreServices/MacTypes.h>
#include <HIToolbox/Menus.h>
#include <HIToolbox/HIGeometry.h>
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
	kEventClassMouse = 'mous',
	kEventClassKeyboard = 'keyb',
	kEventClassCommand = 'cmds',
	kEventClassMenu = 'menu',
	kEventClassAppleEvent = 'eppc',
};

// Kinds of the mouse class
enum {
	kEventMouseDown = 1,
	kEventMouseUp = 2,
	kEventMouseMoved = 5,
	kEventMouseDragged = 6,
	kEventMouseWheelMoved = 10,
};

// Kinds of the keyboard class
enum {
	kEventRawKeyDown = 1,
	kEventRawKeyRepeat = 2,
	kEventRawKeyUp = 3,
	kEventRawKeyModifiersChanged = 4,
};

enum {
	kEventCommandProcess = 1,
	kEventCommandUpdateStatus = 2,
};

// Attributes of an HICommand
enum {
	kHICommandFromMenu = 1 << 0,
	kHICommandFromControl = 1 << 1,
	kHICommandFromWindow = 1 << 2,
};

// Kinds of the menu class
enum {
	kEventMenuBeginTracking = 1,
	kEventMenuEndTracking = 2,
	kEventMenuChangeTrackingMode = 3,
	kEventMenuOpening = 4,
	kEventMenuClosed = 5,
	kEventMenuTargetItem = 6,
	kEventMenuMatchKey = 7,
	kEventMenuEnableItems = 8,
	kEventMenuPopulate = 9,
	kEventMenuDispose = 1001,
};

// The application class: events about the front process, which are sent to the application target
// of every process that uses Carbon events.
enum {
	kEventClassApplication = 'appl',
};

enum {
	kEventAppActivated = 1,
	kEventAppDeactivated = 2,
	kEventAppFrontSwitched = 7,
};

// kEventParamProcessID of kEventAppFrontSwitched: the ProcessSerialNumber of the new front process
enum {
	kEventParamProcessID = 'psn ',
	typeProcessSerialNumber = 'psn ',
};

// Parameters of the menu events
enum {
	kEventParamMenuContext = 'mctx',
	kEventParamMenuItemIndex = 'item',
	kEventParamMenuCommand = 'mcmd',
	kEventParamEnableMenuForKeyEvent = 'fork',
	kEventParamMenuEventOptions = 'meop',
	kEventParamMenuFirstOpen = '1sto',
	kEventParamCurrentMenuTrackingMode = 'cmtm',
	kEventParamNewMenuTrackingMode = 'nmtm',
	typeMenuRef = 'menu',
	typeMenuItemIndex = 'midx',
	typeMenuCommand = 'mcmd',
	typeMenuTrackingMode = 'mtmd',
	typeMenuEventOptions = 'meop',
};

// The character a raw key event produced (Mac Roman), set by whoever posts the event
enum {
	kEventParamKeyMacCharCodes = 'kchr',
};

enum {
	kEventParamMouseLocation = 'mloc',
	kEventParamMouseButton = 'mbtn',
	kEventParamClickCount = 'ccnt',
	kEventParamKeyModifiers = 'kmod',
	kEventParamKeyCode = 'kcod',
	kEventParamKeyUnicodes = 'kuni',
	kEventParamKeyboardType = 'kbdt',
};

enum {
	typeHIPoint = 'hipt',
	typeMouseButton = 'mbtn',
	typeHICommand = 'hcmd',
};

// EventMouseButton
enum {
	kEventMouseButtonPrimary = 1,
	kEventMouseButtonSecondary = 2,
	kEventMouseButtonTertiary = 3,
};
typedef UInt16 EventMouseButton;

// The identifiers of the commands the event loop handles itself
enum {
	kHICommandOK = 'ok  ',
	kHICommandCancel = 'not!',
	kHICommandQuit = 'quit',
	kHICommandUndo = 'undo',
	kHICommandRedo = 'redo',
	kHICommandCut = 'cut ',
	kHICommandCopy = 'copy',
	kHICommandPaste = 'past',
	kHICommandClear = 'clea',
	kHICommandSelectAll = 'sall',
	kHICommandHide = 'hide',
	kHICommandHideOthers = 'hido',
	kHICommandShowAll = 'shal',
	kHICommandPreferences = 'pref',
	kHICommandZoomWindow = 'zoom',
	kHICommandMinimizeWindow = 'mini',
	kHICommandMinimizeAll = 'mina',
	kHICommandArrangeInFront = 'frnt',
	kHICommandBringAllToFront = 'bfrt',
	kHICommandWindowListSeparator = 'wldv',
	kHICommandWindowListTerminator = 'wlst',
	kHICommandSelectWindow = 'swin',
	kHICommandRotateWindowsForward = 'rotw',
	kHICommandRotateWindowsBackward = 'rotb',
	kHICommandAbout = 'abou',
	kHICommandNew = 'new ',
	kHICommandOpen = 'open',
	kHICommandClose = 'clos',
	kHICommandSave = 'save',
	kHICommandSaveAs = 'svas',
	kHICommandRevert = 'rvrt',
	kHICommandPrint = 'prnt',
	kHICommandPageSetup = 'page',
};

enum {
	kEventAppleEvent = 1,
};

enum {
	kEventParamAEEventClass = 'eevc',
	kEventParamAEEventID = 'eeid',
};

// Options of SendEventToEventTargetWithOptions()
enum {
	kEventTargetDontPropagate = 1 << 0,    // the event is not passed on to the targets behind the target
	kEventTargetSendToAllHandlers = 1 << 1 // every handler of a target gets the event, handled or not
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

enum {
	kEventPriorityLow = 0,
	kEventPriorityStandard = 1,
	kEventPriorityHigh = 2,
};

// Durations (in seconds) for the timeouts of the event loop functions
#define kEventDurationSecond ((EventTime) 1.0)
#define kEventDurationMillisecond (kEventDurationSecond / 1000)
#define kEventDurationMicrosecond (kEventDurationSecond / 1000000)
#define kEventDurationNanosecond (kEventDurationSecond / 1000000000)
#define kEventDurationMinute (kEventDurationSecond * 60)
#define kEventDurationHour (kEventDurationMinute * 60)
#define kEventDurationDay (kEventDurationHour * 24)
#define kEventDurationNoWait ((EventTime) 0.0)
#define kEventDurationForever ((EventTime) -1.0)

enum {
	kEventQueueOptionsNone = 0,
};

typedef struct OpaqueEventLoopRef* EventLoopRef;
typedef struct __EventLoopTimer* EventLoopTimerRef;
typedef EventTime EventTimerInterval;
typedef void (*EventLoopTimerProcPtr)(EventLoopTimerRef inTimer, void* inUserData);
typedef EventLoopTimerProcPtr EventLoopTimerUPP;
typedef Boolean (*EventComparatorProcPtr)(EventRef inEvent, void* inCompareData);
typedef EventComparatorProcPtr EventComparatorUPP;


typedef struct HICommand {
	UInt32 attributes;
	UInt32 commandID;
	struct {
		MenuRef menuRef;
		MenuItemIndex menuItemIndex;
	} menu;
} HICommand;

// The source of a command: the menu of a menu command, the control or the window of a command that
// came from one (which of the fields is valid is told by the kHICommandFrom... attributes)
typedef union HICommandExtended {
	HICommand legacy;
	struct {
		UInt32 attributes;
		UInt32 commandID;
		union {
			struct OpaqueControlRef* control;
			struct OpaqueWindowPtr* window;
			struct {
				MenuRef menuRef;
				MenuItemIndex menuItemIndex;
			} menu;
		} source;
	};
} HICommandExtended;

// The number of elements of an array of EventTypeSpec
#define GetEventTypeCount(t) (sizeof((t)) / sizeof(EventTypeSpec))
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

EventRef CopyEvent(EventRef inOther);
OSStatus SetEventTime(EventRef inEvent, EventTime inTime);

EventQueueRef GetMainEventQueue(void);
EventQueueRef GetCurrentEventQueue(void);
EventRef GetCurrentEvent(void);
EventTime GetCurrentEventTime(void);
UInt32 GetCurrentEventKeyModifiers(void);
UInt32 GetCurrentEventButtonState(void);
UInt32 GetCurrentButtonState(void);

// Event queues. They can be used from any thread.
OSStatus PostEventToQueue(EventQueueRef inQueue, EventRef inEvent, EventPriority inPriority);
OSStatus RemoveEventFromQueue(EventQueueRef inQueue, EventRef inEvent);
Boolean IsEventInQueue(EventQueueRef inQueue, EventRef inEvent);
ItemCount GetNumEventsInQueue(EventQueueRef inQueue);
OSStatus FlushEventQueue(EventQueueRef inQueue);
OSStatus FlushEventsMatchingListFromQueue(EventQueueRef inQueue, ItemCount inNumTypes, const EventTypeSpec* inList);
OSStatus FlushSpecificEventsFromQueue(EventQueueRef inQueue, EventComparatorUPP inComparator, void* inCompareData);
EventRef FindSpecificEventInQueue(EventQueueRef inQueue, EventComparatorUPP inComparator, void* inCompareData);
EventRef AcquireFirstMatchingEventInQueue(EventQueueRef inQueue, ItemCount inNumTypes, const EventTypeSpec* inList, OptionBits inOptions);
OSStatus SetMouseCoalescingEnabled(Boolean inNewState, Boolean* outOldState);
Boolean IsMouseCoalescingEnabled(void);
EventComparatorUPP NewEventComparatorUPP(EventComparatorProcPtr userRoutine);
void DisposeEventComparatorUPP(EventComparatorUPP userUPP);

// Event loops
OSStatus ReceiveNextEvent(ItemCount inNumTypes, const EventTypeSpec* inList, EventTimeout inTimeout, Boolean inPullEvent, EventRef* outEvent);
EventLoopRef GetCurrentEventLoop(void);
EventLoopRef GetMainEventLoop(void);
CFRunLoopRef GetCFRunLoopFromEventLoop(EventLoopRef inEventLoop);
OSStatus RunCurrentEventLoop(EventTimeout inTimeout);
OSStatus QuitEventLoop(EventLoopRef inEventLoop);

// Event loop timers. A fire delay and an interval are in seconds; an interval of zero fires
// once, and a next fire time of kEventDurationForever stops the timer until it is set again.
OSStatus InstallEventLoopTimer(EventLoopRef inEventLoop, EventTimerInterval inFireDelay, EventTimerInterval inInterval, EventLoopTimerUPP inTimerProc, void* inTimerData, EventLoopTimerRef* outTimer);
OSStatus RemoveEventLoopTimer(EventLoopTimerRef inTimer);
OSStatus SetEventLoopTimerNextFireTime(EventLoopTimerRef inTimer, EventTimerInterval inNextFire);
EventLoopTimerUPP NewEventLoopTimerUPP(EventLoopTimerProcPtr userRoutine);
void DisposeEventLoopTimerUPP(EventLoopTimerUPP userUPP);

// Event handlers are installed on the event targets of the process (GetApplicationEventTarget(),
// GetEventDispatcherTarget()); the newest handler of a target sees an event first.
OSStatus InstallEventHandler(EventTargetRef inTarget, EventHandlerUPP inHandler, ItemCount inNumTypes, const EventTypeSpec* inList, void* inUserData, EventHandlerRef* outRef);
OSStatus RemoveEventHandler(EventHandlerRef inHandlerRef);
OSStatus AddEventTypesToHandler(EventHandlerRef inHandlerRef, ItemCount inNumTypes, const EventTypeSpec* inList);
OSStatus RemoveEventTypesFromHandler(EventHandlerRef inHandlerRef, ItemCount inNumTypes, const EventTypeSpec* inList);
OSStatus SendEventToEventTargetWithOptions(EventRef inEvent, EventTargetRef inTarget, OptionBits inOptions);

// Processes the Apple event a Carbon event of class kEventClassAppleEvent stands for.
OSStatus AEProcessEvent(EventRef inEvent);
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
