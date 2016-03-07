#ifndef CF_CFLOGUTILITIES_H_
#define CF_CFLOGUTILITIES_H_

enum {
	kCFLogLevelEmergency = 0,
	kCFLogLevelAlert,
	kCFLogLevelCritical,
	kCFLogLevelError,
	kCFLogLevelWarning,
	kCFLogLevelNotice,
	kCFLogLevelInfo,
	kCFLogLevelDebug
};

void CFLog(int32_t level, CFStringRef format, ...);

#endif

