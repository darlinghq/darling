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
		default:
			return 0;
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


