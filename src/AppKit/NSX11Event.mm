#include "NSX11Event.h"
#include <Foundation/NSException.h>

static void throwInconsistency()
{
	[NSException raise: NSInternalInconsistencyException
				format: @"Invalid NSEvent type for this call"];
}

@implementation NSX11Event

-(id) initWithXCBEvent: (xcb_generic_event_t*) event
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
	switch (m_event.response_type)
	{
		case XCB_KEY_PRESS:
			return NSKeyDown;
		case XCB_KEY_RELEASE:
			return NSKeyUp;
		case XCB_BUTTON_PRESS:
			switch (((xcb_button_press_event_t*)&m_event)->detail)
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
		case XCB_BUTTON_RELEASE:
			switch (((xcb_button_press_event_t*)&m_event)->detail)
			{
				case 1:
					return NSLeftMouseUp;
				case 3:
					return NSRightMouseUp;
				default:
					return NSOtherMouseUp;
			}
		case XCB_ENTER_NOTIFY:
			return NSMouseEntered;
		case XCB_LEAVE_NOTIFY:
			return NSMouseExited;
		case XCB_MOTION_NOTIFY:
			return NSMouseMoved; // TODO: NSLeftMouseDragged / NSRightMouseDragged
		default:
			return NSAppKitDefined;
	}
}

- (NSInteger)windowNumber
{
	return ((xcb_button_press_event_t*)&m_event)->root;
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
	return ((xcb_button_press_event_t*)&m_event)->detail;
}

- (unsigned short)keyCode
{
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	return ((xcb_key_press_event_t*) &m_event)->detail;
}

- (NSString *)characters
{
	/*
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	char text[10];
	XLookupString(&m_event.xkey, text, sizeof(text), nullptr, nullptr);
	return [NSString stringWithUTF8String: text];
	*/
	return nullptr; // TODO: not easily doable with xcb
}

- (NSString *)charactersIgnoringModifiers
{
	/*
	NSEventType type = [self type];
	
	if (type != NSKeyDown && type != NSKeyUp)
		throwInconsistency();
	
	char text[10];
	XKeyEvent ev = m_event.xkey;
	
	ev.state = 0x10;
	
	XLookupString(&ev, text, sizeof(text), nullptr, nullptr);
	return [NSString stringWithUTF8String: text];
	*/
	return nullptr; // TODO: not easily doable with xcb
}

@end


