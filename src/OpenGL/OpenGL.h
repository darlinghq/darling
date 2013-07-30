#ifndef OPENGL_H
#define OPENGL_H
#include "CGLContext.h"
#include "CGLTypes.h"

extern "C" {

CGLError CGLLockContext(CGLContextObj context);
CGLError CGLUnlockContext(CGLContextObj context);

}

#endif

