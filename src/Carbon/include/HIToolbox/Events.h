#ifndef _Carbon_Events_H_
#define _Carbon_Events_H_

#ifdef __cplusplus
extern "C" {
#endif

void GetKeys(KeyMap theKeys);
void FlushEvents(EventMask a, EventMask b);

OSStatus SetEventMask(EventMask a);
OSStatus SetEventParameter(EventRef a, EventParamName b, EventParamType c, UInt32 d, const void * e);

#ifdef __cplusplus
}
#endif

#endif
