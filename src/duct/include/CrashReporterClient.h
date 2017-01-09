#ifndef CRASHREPORTERCLIENT_H_
#define CRASHREPORTERCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

void CRSetCrashLogMessage(const char* text);
void CRSetCrashLogMessage2(const char* path);

#ifdef __cplusplus
}
#endif

#endif
