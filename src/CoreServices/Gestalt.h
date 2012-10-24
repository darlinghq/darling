#ifndef GESTALT_H
#define GESTALT_H

#define gestaltSystemVersion 'sysv'
#define gestaltSystemVersionMajor 'sys1'
#define gestaltSystemVersionMinor 'sys2'
#define gestaltSystemVersionPatch 'sys3'

#define gestaltSysArchitecture 'sysa'
#define gestaltPowerPC 2
#define gestaltIntel 10

extern "C" OSStatus Gestalt(uint32_t type, int* value);

#endif

