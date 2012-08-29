#ifndef MACERRORS_H
#define MACERRORS_H

typedef long OSStatus;

inline OSStatus makeOSStatus(int errNo) { return 100000 + errNo; }

#endif
