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

#define KERN_OSTYPE              1
#define KERN_OSRELEASE           2
#define KERN_OSREV               3
#define KERN_VERSION             4
#define KERN_MAXVNODES           5
#define KERN_MAXPROC             6
#define KERN_MAXFILES            7
#define KERN_ARGMAX              8
#define KERN_SECURELVL           9
#define KERN_HOSTNAME           10
#define KERN_HOSTID             11
#define KERN_CLOCKRATE          12
#define KERN_VNODE              13
#define KERN_PROC               14
#define KERN_FILE               15
#define KERN_PROF               16
#define KERN_POSIX1             17
#define KERN_NGROUPS            18
#define KERN_JOB_CONTROL        19
#define KERN_SAVED_IDS          20
#define KERN_BOOTTIME           21
#define KERN_NISDOMAINNAME      22
#define KERN_DOMAINNAME         KERN_NISDOMAINNAME
#define KERN_MAXPARTITIONS      23
#define KERN_KDEBUG             24
#define KERN_UPDATEINTERVAL     25
#define KERN_OSRELDATE          26
#define KERN_NTP_PLL            27
#define KERN_BOOTFILE           28
#define KERN_MAXFILESPERPROC    29
#define KERN_MAXPROCPERUID      30
#define KERN_DUMPDEV            31
#define KERN_IPC                32
#define KERN_DUMMY              33
#define KERN_PS_STRINGS         34
#define KERN_USRSTACK32         35
#define KERN_LOGSIGEXIT         36
#define KERN_SYMFILE            37
#define KERN_PROCARGS           38
#define KERN_NETBOOT            40
#define KERN_SYSV               42
#define KERN_AFFINITY           43
#define KERN_TRANSLATE          44
#define KERN_CLASSIC            KERN_TRANSLATE
#define KERN_EXEC               45
#define KERN_CLASSICHANDLER     KERN_EXEC
#define KERN_AIOMAX             46
#define KERN_AIOPROCMAX         47
#define KERN_AIOTHREADS         48
#define KERN_PROCARGS2          49
#define KERN_COREFILE           50
#define KERN_COREDUMP           51
#define KERN_SUGID_COREDUMP     52
#define KERN_PROCDELAYTERM      53
#define KERN_SHREG_PRIVATIZABLE 54
#define KERN_LOW_PRI_WINDOW     56
#define KERN_LOW_PRI_DELAY      57
#define KERN_POSIX              58
#define KERN_USRSTACK64         59
#define KERN_NX_PROTECTION      60
#define KERN_TFP                61
#define KERN_PROCNAME           62
#define KERN_THALTSTACK         63
#define KERN_SPECULATIVE_READS  64
#define KERN_OSVERSION          65
#define KERN_SAFEBOOT           66
#define KERN_LCTX               67
#define KERN_RAGEVNODE          68
#define KERN_TTY                69
#define KERN_CHECKOPENEVT       70
#define KERN_THREADNAME         71
#define KERN_MAXID              72

#endif

