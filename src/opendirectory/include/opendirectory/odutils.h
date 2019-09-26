#ifndef ODUTILS_H
#define ODUTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <uuid/uuid.h>
#include <CoreFoundation/CoreFoundation.h>

extern const uuid_t kODNullUUID;

extern const int eODCustomCallConfigureGetAuthRef;
extern const int eODCustomCallConfigureDestroyAuthRef;

extern const int eODCustomCallSearchSetCustomNodeList;
extern const int eODCustomCallSearchSetPolicyAutomatic;
extern const int eODCustomCallSearchSetPolicyLocalOnly;
extern const int eODCustomCallSearchSetPolicyCustom;

void *schema_get_value_at_index(CFArrayRef value, int index);
void schema_deconstruct_result(CFDictionaryRef dict, CFStringRef *function_name, CFArrayRef *response);
CFDictionaryRef schema_construct_requestv(CFStringRef function_name, CFIndex total_params, va_list ap);
CFDictionaryRef schema_construct_request(CFStringRef function_name, CFIndex total_params, ...);

#endif
