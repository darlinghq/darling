#include "NSEvent.h"

@implementation NSEvent

-(id) initWithQEvent: (QEvent*) event
           ownership: (BOOL) owner
{
	self = [super init];
	m_event = event;
	m_owner = owner != NO;
	
	return self;
}

-(QEvent*) _qtEvent
{
	return m_event;
}

-(void) dealloc
{
	if (m_owner)
		delete m_event;
	[super dealloc];
}

@end
