#ifndef DASESSION_H
#define DASESSION_H
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DASession * DASessionRef;

typedef void* dispatch_queue_t; // hack

CFTypeID DASessionGetTypeID(void);
DASessionRef DASessionCreate(CFAllocatorRef allocator);
void DASessionScheduleWithRunLoop(DASessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode);
void DASessionUnscheduleFromRunLoop(DASessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode);
void DASessionSetDispatchQueue(DASessionRef session, dispatch_queue_t queue);

typedef struct _DAApprovalSession * DAApprovalSessionRef;

CFTypeID DAApprovalSessionGetTypeID(void);
DAApprovalSessionRef DAApprovalSessionCreate(CFAllocatorRef allocator);
void DAApprovalSessionScheduleWithRunLoop(DAApprovalSessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode);
void DAApprovalSessionUnscheduleFromRunLoop(DAApprovalSessionRef session, CFRunLoopRef runLoop, CFStringRef runLoopMode);

#ifdef __cplusplus
}
#endif

#endif /* DASESSION_H */

