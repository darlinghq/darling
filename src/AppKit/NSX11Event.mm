#include "NSX11Event.h"

@implementation NSX11Event

-(id) initWithXEvent: (XEvent*) event
{
	self = [super init];
	m_event = *event;
	m_ts = [[NSDate date] timeIntervalSince1970];
	return self;
}

-(CGEventRef) CGEvent
{
	return &m_event;
}

- (NSTimeInterval)timestamp
{
	return m_ts;
}

- (NSEventType)type
{
	switch (m_event.type)
	{
		case KeyPress:
			return NSKeyDown;
		case KeyRelease:
			return NSKeyUp;
		case ButtonPress:
			switch (m_event.xbutton.button)
			{
				case 1:
					return NSLeftMouseDown;
				case 3:
					return NSRightMouseDown;
				case 4: // scroll up
				case 5: // scroll down
					return NSScrollWheel;
				default:
					return NSOtherMouseDown;
			}
		case ButtonRelease:
			switch (m_event.xbutton.button)
			{
				case 1:
					return NSLeftMouseUp;
				case 3:
					return NSRightMouseUp;
				default:
					return NSOtherMouseUp;
			}
		case EnterNotify:
			return NSMouseEntered;
		case LeaveNotify:
			return NSMouseExited;
		case MotionNotify:
			return NSMouseMoved; // TODO: NSLeftMouseDragged / NSRightMouseDragged
		default:
			return NSAppKitDefined;
	}
}

- (NSInteger)windowNumber
{
	return m_event.xany.window;
}

- (NSWindow*) window
{
	return nullptr; // TODO: when we have NSWindow
}

- (NSPoint)locationInWindow
{
	//return NSPoint{m_event.x, m_event.y+1};
	return NSPoint{0, 0};
}

@end


