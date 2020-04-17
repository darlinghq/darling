#ifndef _CS_DEBUGGING_H
#define _CS_DEBUGGING_H
#include <CoreServices/MacTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void DebugAssert(OSType componentSignature, UInt32 options, const char *assertionString,
	const char *exceptionLabelString, const char *errorString, const char *fileName,
	long lineNumber, void *value);

typedef void(*DebugAssertOutputHandlerUPP)(OSType componentSignature, UInt32 options, const char *assertionString,
	const char *exceptionLabelString, const char *errorString, const char *fileName,
	long lineNumber, ConstStr255Param outputMsg);

extern void InstallDebugAssertOutputHandler(DebugAssertOutputHandlerUPP handler);

extern UInt32 TaskLevel(void);

extern void DebugStr(ConstStr255Param msg);

#ifdef __cplusplus
}
#endif

#endif
