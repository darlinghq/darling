#ifndef SYSCTL_INC_H
#define SYSCTL_INC_H

// Copy & pasted from sysctl.h, because including that file
// is problematic (include collisions).

#define CTL_UNSPEC      0
#define CTL_KERN        1
#define CTL_VM          2
#define CTL_VFS         3
#define CTL_NET         4
#define CTL_DEBUG       5
#define CTL_HW          6
#define CTL_MACHDEP     7
#define CTL_USER        8
#define CTL_MAXID       9


#define HW_MACHINE       1
#define HW_MODEL         2
#define HW_NCPU          3
#define HW_BYTEORDER     4
#define HW_PHYSMEM       5
#define HW_USERMEM       6
#define HW_PAGESIZE      7
#define HW_DISKNAMES     8
#define HW_DISKSTATS     9
#define HW_EPOCH        10
#define HW_FLOATINGPT   11
#define HW_MACHINE_ARCH 12
#define HW_VECTORUNIT   13
#define HW_BUS_FREQ     14
#define HW_CPU_FREQ     15
#define HW_CACHELINE    16
#define HW_L1ICACHESIZE 17
#define HW_L1DCACHESIZE 18
#define HW_L2SETTINGS   19
#define HW_L2CACHESIZE  20
#define HW_L3SETTINGS   21
#define HW_L3CACHESIZE  22
#define HW_TB_FREQ      23
#define HW_MEMSIZE      24
#define HW_AVAILCPU     25
#define HW_MAXID        26

#endif

