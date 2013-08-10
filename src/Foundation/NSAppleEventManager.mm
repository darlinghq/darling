#include "NSAppleEventManager.h"
#include <util/debug.h>
#include <CoreServices/MacErrors.h>

@implementation NSAppleEventManager

+ (NSAppleEventManager *)sharedAppleEventManager
{
	static NSAppleEventManager* instance = [[NSAppleEventManager alloc] init];
	
	return instance;
}

- (NSAppleEventDescriptor *)appleEventForSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID
{
	STUB();
	return NULL;
}

- (NSAppleEventDescriptor *)currentAppleEvent
{
	STUB();
	return NULL;
}

- (NSAppleEventDescriptor *)currentReplyAppleEvent
{
	STUB();
	return NULL;
}

- (OSErr)dispatchRawAppleEvent:(const AppleEvent *)theAppleEvent
                  withRawReply:(AppleEvent *)theReply
                 handlerRefCon:(UInt32)handlerRefcon
{
	STUB();
	return unimpErr;
}

- (void)removeEventHandlerForEventClass:(AEEventClass)eventClass
                             andEventID:(AEEventID)eventID
{
	STUB();
}

- (NSAppleEventDescriptor *)replyAppleEventForSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID
{
	STUB();
	return NULL;
}

- (void)resumeWithSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID
{
	STUB();
}

- (void)setCurrentAppleEventAndReplyEventWithSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID
{
	STUB();
}

- (void)setEventHandler:(id)handler
            andSelector:(SEL)handleEventSelector
          forEventClass:(AEEventClass)eventClass
             andEventID:(AEEventID)eventID
{
	STUB();
}

- (NSAppleEventManagerSuspensionID)suspendCurrentAppleEvent
{
	STUB();
	return NULL;
}

@end
