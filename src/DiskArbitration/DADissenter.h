#ifndef DADISSENTER_H
#define DADISSENTER_H
#include <CoreFoundation/CoreFoundation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int DAReturn;

typedef struct _DADissenter* DADissenterRef;

DADissenterRef DADissenterCreate(CFAllocatorRef allocator, DAReturn status, CFStringRef string);
DAReturn DADissenterGetStatus(DADissenterRef dissenter);
CFStringRef DADissenterGetStatusString(DADissenterRef dissenter);

#ifdef __cplusplus
}
#endif

#endif /* DADISSENTER_H */

