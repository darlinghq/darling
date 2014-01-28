#ifndef NSX11EVENT_H
#define NSX11EVENT_H
#include "NSEvent.h"
#include <xcb/xcb.h>
#include <Foundation/NSDate.h>

@interface NSX11Event : NSEvent
{
	@public
	xcb_generic_event_t m_event;
	NSTimeInterval m_ts;
}

-(CGEventRef) CGEvent;
- (NSTimeInterval)timestamp;
- (NSEventType)type;
- (NSInteger)windowNumber;
- (NSWindow*) window;
- (NSPoint)locationInWindow;
- (NSInteger)buttonNumber;
- (unsigned short)keyCode;

- (NSString *)characters;
- (NSString *)charactersIgnoringModifiers;
@end

#endif

