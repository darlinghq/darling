#ifndef _MOBILE_KEY_BAG_H_
#define _MOBILE_KEY_BAG_H_

#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __MKBKeyBagHandle* MKBKeyBagHandleRef;

int MKBKeyBagCreateWithData(CFDataRef keybagBlob, MKBKeyBagHandleRef* newHandle);
int MKBKeyBagGetAKSHandle(MKBKeyBagHandleRef keybag, int32_t* handle);
int MKBKeyBagUnlock(MKBKeyBagHandleRef keybag, CFDataRef passcode);

#ifdef __cplusplus
}
#endif

#endif // _MOBILE_KEY_BAG_H_
