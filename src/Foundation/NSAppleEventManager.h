#ifndef NSAPPLEEVENTMANAGER_H
#define NSAPPLEEVENTMANAGER_H
#include "NSAppleEventDescriptor.h"

typedef const struct __NSAppleEventManagerSuspension *NSAppleEventManagerSuspensionID;
struct AppleEvent;

@interface NSAppleEventManager : NSObject

+ (NSAppleEventManager *)sharedAppleEventManager;

- (NSAppleEventDescriptor *)appleEventForSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID;
- (NSAppleEventDescriptor *)currentAppleEvent;
- (NSAppleEventDescriptor *)currentReplyAppleEvent;

- (OSErr)dispatchRawAppleEvent:(const AppleEvent *)theAppleEvent
                  withRawReply:(AppleEvent *)theReply
                 handlerRefCon:(UInt32)handlerRefcon;
- (void)removeEventHandlerForEventClass:(AEEventClass)eventClass
                             andEventID:(AEEventID)eventID;
- (NSAppleEventDescriptor *)replyAppleEventForSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID;
- (void)resumeWithSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID;
- (void)setCurrentAppleEventAndReplyEventWithSuspensionID:(NSAppleEventManagerSuspensionID)suspensionID;
- (void)setEventHandler:(id)handler
            andSelector:(SEL)handleEventSelector
          forEventClass:(AEEventClass)eventClass
             andEventID:(AEEventID)eventID;
- (NSAppleEventManagerSuspensionID)suspendCurrentAppleEvent;

@end

#endif
