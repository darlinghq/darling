#ifndef _Carbon_Carbon_Events_H_
#define _Carbon_Carbon_Events_H_

#ifdef __cplusplus
extern "C" {
#endif

EventTargetRef GetApplicationEventTarget(void);
OSStatus GetEventDispatcherTarget();

OSStatus ProcessHICommand(const HICommand * a);

void RunApplicationEventLoop(void);


#ifdef __cplusplus
}
#endif

#endif
