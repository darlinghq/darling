#ifndef NSX11EVENT_H
#define NSX11EVENT_H
#include "NSEvent.h"
#include <X11/Xlib.h>
#include <Foundation/NSDate.h>

@interface NSX11Event : NSEvent
{
	@public
	XEvent m_event;
	NSTimeInterval m_ts;
}

-(CGEventRef) CGEvent;
- (NSTimeInterval)timestamp;
- (NSEventType)type;
- (NSInteger)windowNumber;
- (NSWindow*) window;
- (NSPoint)locationInWindow;
@end

#endif

