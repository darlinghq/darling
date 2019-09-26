#import <opendirectory/odutils.h>

#warning "TODO: Figure out real values"
const int eODCustomCallConfigureGetAuthRef = 0;
const int eODCustomCallConfigureDestroyAuthRef = 1;

const int eODCustomCallSearchSetCustomNodeList = 2;
const int eODCustomCallSearchSetPolicyAutomatic = 3;
const int eODCustomCallSearchSetPolicyLocalOnly = 4;
const int eODCustomCallSearchSetPolicyCustom = 5;

const uuid_t kODNullUUID = {0};

void *schema_get_value_at_index(CFArrayRef value, int index) {
    puts("STUB: schema_get_value_at_index called");
    return NULL;
}

void schema_deconstruct_result(CFDictionaryRef dict, CFStringRef *function_name, CFArrayRef *response) {
    puts("STUB: schema_deconstruct_result");
}

CFDictionaryRef schema_construct_requestv(CFStringRef function_name, CFIndex total_params, va_list ap) {
    puts("STUB: schema_construct_requestv");
    return NULL;
}

CFDictionaryRef schema_construct_request(CFStringRef function_name, CFIndex total_params, ...) {
    va_list ap;
    CFDictionaryRef result;

    va_start(ap, total_params);
    result = schema_construct_requestv(function_name, total_params, ap);
    va_end(ap);

    return result;
}
