#ifndef MACERRORS_H
#define MACERRORS_H
#include <CoreFoundation/CFBase.h>
#include <MacTypes.h>

inline OSStatus makeOSStatus(int errNo) { return 100000 + errNo; }

#define memFullErr -108
#define internalComponentErr -2070
//#define noErr		0
#define unimpErr	-4
#define fnfErr		-43 // file not found
#define eofErr -39
#define paramErr	-50
#define handlerNotFoundErr -1856
#define kLocalesBufferTooSmallErr -30001
#define kUCOutputBufferTooSmall -25340

#define kMPTimeoutErr -29296

enum {
	invalidComponentID = -3000,
	validInstancesExist = -3001,
	componentNotCaptured = -3002,
	componentDontRegister = -3003,
	unresolvedComponentDLLErr = -3004,
	retryComponentRegistrationErr = -3005,
	badComponentInstance = (int)0x80008001,
	badComponentSelector = (int)0x80008002,
};

enum {
	resourceInMemory = -188,
	writingPastEnd = -189,
	inputOutOfBounds = -190,
	resNotFound = -191,
	resFNotFound = -193,
	addResFailed = -194,
	addRefFailed = -195,
	rmvResFailed = -196,
	rmvRefFailed = -197,
	resAttrErr = -198,
	mapReadErr = -199,
};

#endif
