#ifndef DYLD_H
#define DYLD_H

extern "C" int g_argc asm("NXArgc");
extern "C" char** g_argv asm("NXArgv");

#endif

