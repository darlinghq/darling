#ifndef __MSGTRACER_CLIENT_H__
#define __MSGTRACER_CLIENT_H__

#include <asl.h>

/**
 * how it seems to be used:
 *   msgtracer_log_with_keys(
 *     "message.id.here", // usually the bundle id of the caller plus some arbitrary message name
 *     ASL_LEVEL_<something>, // one of the asl log levels in `asl.h`
 *     // the rest of the arguments are just pairs of arguments
 *     // the pairs go a little something like this:
 *     kMsgTracerKey<some-key-name>, // one of the keys in `msgtracer_keys.h`
 *     <some-c-string-value> // value for the provided key
 *     ... // as many of those pairs as you want
 *     NULL // and finally you terminate it with `NULL` so the function knows where the arguments end
 *   )
 */
extern void msgtracer_log_with_keys(char* logger_id, int log_level, ...);

#endif
