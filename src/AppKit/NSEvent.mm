#include "NSEvent.h"
#include "NSX11Event.h"

@implementation NSEvent

+ (NSEvent *)eventWithCGEvent:(CGEventRef)cgEvent
{
	NSX11Event* ev = [[NSX11Event alloc] init];
	ev->m_event = *cgEvent;
	return [ev autorelease];
}

- (id)initWithCoder:(NSCoder *)decoder
{
	return [self init]; // TODO
}

- (void)encodeWithCoder:(NSCoder *)encoder
{
	// TODO
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain]; // TODO
}

- (NSGraphicsContext *)context
{
	return nullptr;
}

- (NSInteger)absoluteX
{
	return 0;
}

- (NSInteger)absoluteY
{
	return 0;
}

- (NSInteger)absoluteZ
{
	return 0;
}

- (NSUInteger)buttonMask
{
	return 0;
}

- (NSInteger)buttonNumber
{
	return 0;
}

- (NSUInteger)capabilityMask
{
	return 0;
}

- (CGEventRef)CGEvent
{
	return 0;
}

- (NSString *)characters
{
	return 0;
}

- (NSString *)charactersIgnoringModifiers
{
	return 0;
}

- (NSInteger)clickCount
{
	return 0;
}

- (NSInteger)data1
{
	return 0;
}

- (NSInteger)data2
{
	return 0;
}

- (CGFloat)deltaX
{
	return 0;
}

- (CGFloat)deltaY
{
	return 0;
}

- (CGFloat)deltaZ
{
	return 0;
}

- (NSUInteger)deviceID
{
	return 0;
}

- (NSInteger)eventNumber
{
	return 0;
}

- (const void *)eventRef
{
	return 0;
}

- (BOOL)hasPreciseScrollingDeltas
{
	return 0;
}

- (BOOL)isARepeat
{
	return 0;
}

- (BOOL)isDirectionInvertedFromDevice
{
	return 0;
}

- (BOOL)isEnteringProximity
{
	return 0;
}

- (unsigned short)keyCode
{
	return 0;
}

- (NSPoint)locationInWindow
{
	return NSPoint{0, 0};
}

- (CGFloat)magnification
{
	return 0;
}

- (NSUInteger)modifierFlags
{
	return 0;
}

- (NSEventPhase)momentumPhase
{
	return 0;
}

- (NSEventPhase)phase
{
	return 0;
}

- (NSUInteger)pointingDeviceID
{
	return 0;
}

- (NSUInteger)pointingDeviceSerialNumber
{
	return 0;
}

- (NSPointingDeviceType)pointingDeviceType
{
	return 0;
}

- (float)pressure
{
	return 0;
}

- (float)rotation
{
	return 0;
}

- (CGFloat)scrollingDeltaX
{
	return 0;
}

- (CGFloat)scrollingDeltaY
{
	return 0;
}

- (short)subtype
{
	return 0;
}

- (NSUInteger)systemTabletID
{
	return 0;
}

- (NSUInteger)tabletID
{
	return 0;
}

- (float)tangentialPressure
{
	return 0;
}

- (NSPoint)tilt
{
	return NSPoint{0, 0};
}

- (NSTimeInterval)timestamp
{
	return 0;
}

- (NSInteger)trackingNumber
{
	return 0;
}

- (NSEventType)type
{
	return 0;
}

- (unsigned long long)uniqueID
{
	return 0;
}

- (void *)userData
{
	return 0;
}

- (id)vendorDefined
{
	return 0;
}

- (NSUInteger)vendorID
{
	return 0;
}

- (NSUInteger)vendorPointingDeviceType
{
	return 0;
}

- (NSWindow *)window
{
	return 0;
}

- (NSInteger)windowNumber
{
	return 0;
}



@end
