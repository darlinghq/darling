#ifndef NSEVENT_H_
#define NSEVENT_H_
#include <Foundation/NSObject.h>
#include <Foundation/NSGeometry.h>
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGEventRef.h>

@class NSGraphicsContext;
@class NSWindow;
typedef NSUInteger NSEventPhase;
typedef NSUInteger NSEventType;
typedef NSUInteger NSPointingDeviceType;
typedef unsigned long long NSEventMask;

@interface NSEvent : NSObject <NSCopying, NSCoding>
// TODO: static stuff
+ (NSEvent *)eventWithCGEvent:(CGEventRef)cgEvent;

- (id)initWithCoder:(NSCoder *)decoder;
- (void)encodeWithCoder:(NSCoder *)encoder;

- (id)copyWithZone:(NSZone *)zone;
- (NSGraphicsContext *)context;
- (NSInteger)absoluteX;
- (NSInteger)absoluteY;
- (NSInteger)absoluteZ;
- (NSUInteger)buttonMask;
- (NSInteger)buttonNumber;
- (NSUInteger)capabilityMask;
- (CGEventRef)CGEvent;
- (NSString *)characters;
- (NSString *)charactersIgnoringModifiers;
- (NSInteger)clickCount;
- (NSInteger)data1;
- (NSInteger)data2;
- (CGFloat)deltaX;
- (CGFloat)deltaY;
- (CGFloat)deltaZ;
- (NSUInteger)deviceID;
- (NSInteger)eventNumber;
- (const void *)eventRef;
- (BOOL)hasPreciseScrollingDeltas;
- (BOOL)isARepeat;
- (BOOL)isDirectionInvertedFromDevice;
- (BOOL)isEnteringProximity;
- (unsigned short)keyCode;
- (NSPoint)locationInWindow;
- (CGFloat)magnification;
- (NSUInteger)modifierFlags;
- (NSEventPhase)momentumPhase;
- (NSEventPhase)phase;
- (NSUInteger)pointingDeviceID;
- (NSUInteger)pointingDeviceSerialNumber;
- (NSPointingDeviceType)pointingDeviceType;
- (float)pressure;
- (float)rotation;
- (CGFloat)scrollingDeltaX;
- (CGFloat)scrollingDeltaY;
- (short)subtype;
- (NSUInteger)systemTabletID;
- (NSUInteger)tabletID;
- (float)tangentialPressure;
- (NSPoint)tilt;
- (NSTimeInterval)timestamp;
- (NSInteger)trackingNumber;
- (NSEventType)type;
- (unsigned long long)uniqueID;
- (void *)userData;
- (id)vendorDefined;
- (NSUInteger)vendorID;
- (NSUInteger)vendorPointingDeviceType;
- (NSWindow *)window;
- (NSInteger)windowNumber;

@end

enum {
   NSEventPhaseNone        = 0,
   NSEventPhaseBegan       = 0x1 << 0,
   NSEventPhaseStationary  = 0x1 << 1,
   NSEventPhaseChanged     = 0x1 << 2,
   NSEventPhaseEnded       = 0x1 << 3,
   NSEventPhaseCancelled   = 0x1 << 4,
   NSEventPhaseMayBegin    = 0x1 << 5
};

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
inline NSUInteger NSEventMaskFromType(NSEventType type) { return (1 << type); };

#endif
