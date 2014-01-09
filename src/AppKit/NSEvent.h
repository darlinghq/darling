#ifndef NSEVENT_H_
#define NSEVENT_H_
#include <Foundation/NSObject.h>

#ifdef DARLING_BUILD
#	include <QEvent>
#endif

@interface NSEvent : NSObject
#ifdef DARLING_BUILD
{
	QEvent* m_event;
	bool m_owner;
}

-(QEvent*) _qtEvent;

-(id) initWithQEvent: (QEvent*) event
           ownership: (BOOL) owner;

#endif

-(void) dealloc;
@end

enum {
   NSLeftMouseDown      = 1,
   NSLeftMouseUp        = 2,
   NSRightMouseDown     = 3,
   NSRightMouseUp       = 4,
   NSMouseMoved         = 5,
   NSLeftMouseDragged   = 6,
   NSRightMouseDragged  = 7,
   NSMouseEntered       = 8,
   NSMouseExited        = 9,
   NSKeyDown            = 10,
   NSKeyUp              = 11,
   NSFlagsChanged       = 12,
   NSAppKitDefined      = 13,
   NSSystemDefined      = 14,
   NSApplicationDefined = 15,
   NSPeriodic           = 16,
   NSCursorUpdate       = 17,
   NSScrollWheel        = 22,
   NSTabletPoint        = 23,
   NSTabletProximity    = 24,
   NSOtherMouseDown     = 25,
   NSOtherMouseUp       = 26,
   NSOtherMouseDragged  = 27,
   NSEventTypeGesture   = 29,
   NSEventTypeMagnify   = 30,
   NSEventTypeSwipe     = 31,
   NSEventTypeRotate    = 18,
   NSEventTypeBeginGesture = 19,
   NSEventTypeEndGesture   = 20,
   NSEventTypeSmartMagnify = 32,
   NSEventTypeQuickLook   = 33
};
typedef NSUInteger NSEventType;

enum {
   NSLeftMouseDownMask      = 1 << NSLeftMouseDown,
   NSLeftMouseUpMask        = 1 << NSLeftMouseUp,
   NSRightMouseDownMask     = 1 << NSRightMouseDown,
   NSRightMouseUpMask       = 1 << NSRightMouseUp,
   NSMouseMovedMask         = 1 << NSMouseMoved,
   NSLeftMouseDraggedMask   = 1 << NSLeftMouseDragged,
   NSRightMouseDraggedMask  = 1 << NSRightMouseDragged,
   NSMouseEnteredMask       = 1 << NSMouseEntered,
   NSMouseExitedMask        = 1 << NSMouseExited,
   NSKeyDownMask            = 1 << NSKeyDown,
   NSKeyUpMask              = 1 << NSKeyUp,
   NSFlagsChangedMask       = 1 << NSFlagsChanged,
   NSAppKitDefinedMask      = 1 << NSAppKitDefined,
   NSSystemDefinedMask      = 1 << NSSystemDefined,
   NSApplicationDefinedMask = 1 << NSApplicationDefined,
   NSPeriodicMask           = 1 << NSPeriodic,
   NSCursorUpdateMask       = 1 << NSCursorUpdate,
   NSScrollWheelMask        = 1 << NSScrollWheel,
   NSTabletPointMask           = 1 << NSTabletPoint,
   NSTabletProximityMask       = 1 << NSTabletProximity,
   NSOtherMouseDownMask     = 1 << NSOtherMouseDown,
   NSOtherMouseUpMask       = 1 << NSOtherMouseUp,
   NSOtherMouseDraggedMask  = 1 << NSOtherMouseDragged,
   NSEventMaskGesture          = 1 << NSEventTypeGesture,
   NSEventMaskMagnify          = 1 << NSEventTypeMagnify,
   NSEventMaskSwipe            = 1U << NSEventTypeSwipe,
   NSEventMaskRotate           = 1 << NSEventTypeRotate,
   NSEventMaskBeginGesture     = 1 << NSEventTypeBeginGesture,
   NSEventMaskEndGesture       = 1 << NSEventTypeEndGesture,
   NSEventMaskSmartMagnify = 1ULL << NSEventTypeSmartMagnify,
   NSAnyEventMask           = 0xffffffffU
};
typedef unsigned long long NSEventMask;
inline NSUInteger NSEventMaskFromType(NSEventType type) { return (1 << type); };

#endif
