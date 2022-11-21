#ifndef CRASHREPORTERCLIENT_H_
#define CRASHREPORTERCLIENT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CRASHREPORTER_ANNOTATIONS_SECTION "__crash_info"
#define CRASHREPORTER_ANNOTATIONS_VERSION 5
#define CRASH_REPORTER_CLIENT_HIDDEN __attribute__((visibility("hidden")))

inline void CRSetCrashLogMessage(const char* text) {}
inline void CRSetCrashLogMessage2(const char* path) {}
inline const char* CRGetCrashLogMessage(void) { return 0; }

struct crashreporter_annotations_t {
    uint64_t version;
    uint64_t message;
    uint64_t signature_string;
    uint64_t backtrace;
    uint64_t message2;
    uint64_t thread;
    uint64_t dialog_mode;
    uint64_t abort_cause;
};

CRASH_REPORTER_CLIENT_HIDDEN
extern struct crashreporter_annotations_t gCRAnnotations;

#ifdef __cplusplus
}
#endif

#endif
