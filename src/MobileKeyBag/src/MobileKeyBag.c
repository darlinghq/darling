#include <MobileKeyBag/MobileKeyBag.h>
#include <stdio.h>

int MKBKeyBagCreateWithData(CFDataRef keybagBlob, MKBKeyBagHandleRef* newHandle) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return -1;
};

int MKBKeyBagGetAKSHandle(MKBKeyBagHandleRef keybag, int32_t* handle) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return -1;
};

int MKBKeyBagUnlock(MKBKeyBagHandleRef keybag, CFDataRef passcode) {
	printf("STUB %s\n", __PRETTY_FUNCTION__);
	return -1;
};
