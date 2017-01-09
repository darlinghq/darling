#ifndef CRASHREPORTERCLIENT_H_
#define CRASHREPORTERCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

inline void CRSetCrashLogMessage(const char* text) {}
inline void CRSetCrashLogMessage2(const char* path) {}
inline const char* CRGetCrashLogMessage(void) { return 0; }

#ifdef __cplusplus
}
#endif

#endif
