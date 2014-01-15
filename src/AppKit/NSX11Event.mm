#include "NSX11Event.h"
#include <X11/Xutil.h>
#include <Foundation/NSException.h>

static void throwInconsistency()
{
	[NSException raise: NSInternalInconsistencyException
				format: @"Invalid NSEvent type for this call"];
}

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

// TODO: check event type and throw inconsistency exceptions
- (NSInteger)buttonNumber
{
	return m_event.xbutton.button;
}

- (unsigned short)keyCode
{
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	return m_event.xkey.keycode;
}

- (NSString *)characters
{
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	char text[10];
	XLookupString(&m_event.xkey, text, sizeof(text), nullptr, nullptr);
	return [NSString stringWithUTF8String: text];
}

- (NSString *)charactersIgnoringModifiers
{
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	char text[10];
	XKeyEvent ev = m_event.xkey;
	
	ev.state = 0x10;
	
	XLookupString(&ev, text, sizeof(text), nullptr, nullptr);
	return [NSString stringWithUTF8String: text];
}

@end


