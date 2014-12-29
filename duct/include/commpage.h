#ifndef COMMPAGE_H
#define COMMPAGE_H
#include <stdint.h>

extern uint8_t __commpage[];

#define _COMM_PAGE_BASE_ADDRESS (__commpage)
#define _COMM_PAGE_AREA_LENGTH        ( 1 * 4096 )                            /* reserved length of entire comm area */
//#define _COMM_PAGE_BASE_ADDRESS       ( 0xffff0000 )                          /* base address of allocated memory */
#define _COMM_PAGE_START_ADDRESS      ( _COMM_PAGE_BASE_ADDRESS )   /* address traditional commpage code starts on */
#define _COMM_PAGE_AREA_USED          ( 1 * 4096 )                            /* this is the amt actually allocated */
#define _COMM_PAGE_SIGS_OFFSET        0x8000                                      /* offset to routine signatures */


#define _COMM_PAGE_SIGNATURE            (_COMM_PAGE_START_ADDRESS+0x000)        /* first few bytes are a signature */
#define _COMM_PAGE_VERSION              (_COMM_PAGE_START_ADDRESS+0x01E)        /* 16-bit version# */
#define _COMM_PAGE_THIS_VERSION         12                                      /* version of the commarea format */
  
#define _COMM_PAGE_CPU_CAPABILITIES     (_COMM_PAGE_START_ADDRESS+0x020)        /* uint32_t _cpu_capabilities */
#define _COMM_PAGE_NCPUS                (_COMM_PAGE_START_ADDRESS+0x022)        /* uint8_t number of configured CPUs (hw.logicalcpu at boot time) */
#define _COMM_PAGE_UNUSED0                      (_COMM_PAGE_START_ADDRESS+0x024)        /* 2 unused bytes, reserved for future expansion of cpu_capabilities */
#define _COMM_PAGE_CACHE_LINESIZE       (_COMM_PAGE_START_ADDRESS+0x026)        /* uint16_t cache line size */

#define _COMM_PAGE_SCHED_GEN            (_COMM_PAGE_START_ADDRESS+0x028)        /* uint32_t scheduler generation number (count of pre-emptions) */
#define _COMM_PAGE_MEMORY_PRESSURE      (_COMM_PAGE_START_ADDRESS+0x02c)        /* uint32_t copy of vm_memory_pressure */
#define _COMM_PAGE_SPIN_COUNT           (_COMM_PAGE_START_ADDRESS+0x030)        /* uint32_t max spin count for mutex's */

#define _COMM_PAGE_ACTIVE_CPUS          (_COMM_PAGE_START_ADDRESS+0x034)        /* uint8_t number of active CPUs (hw.activecpu) */
#define _COMM_PAGE_PHYSICAL_CPUS        (_COMM_PAGE_START_ADDRESS+0x035)        /* uint8_t number of physical CPUs (hw.physicalcpu_max) */
#define _COMM_PAGE_LOGICAL_CPUS (_COMM_PAGE_START_ADDRESS+0x036)        /* uint8_t number of logical CPUs (hw.logicalcpu_max) */
#define _COMM_PAGE_UNUSED1              (_COMM_PAGE_START_ADDRESS+0x037)        /* 1 unused bytes */
#define _COMM_PAGE_MEMORY_SIZE          (_COMM_PAGE_START_ADDRESS+0x038)        /* uint64_t max memory size */

#define _COMM_PAGE_CPUFAMILY            (_COMM_PAGE_START_ADDRESS+0x040)        /* uint32_t hw.cpufamily, x86*/
#define _COMM_PAGE_UNUSED2              (_COMM_PAGE_START_ADDRESS+0x044)        /* [0x44,0x50) unused */

#define _COMM_PAGE_TIME_DATA_START      (_COMM_PAGE_START_ADDRESS+0x050)        /* base of offsets below (_NT_SCALE etc) */
#define _COMM_PAGE_NT_TSC_BASE          (_COMM_PAGE_START_ADDRESS+0x050)        /* used by nanotime() */
#define _COMM_PAGE_NT_SCALE             (_COMM_PAGE_START_ADDRESS+0x058)        /* used by nanotime() */
#define _COMM_PAGE_NT_SHIFT             (_COMM_PAGE_START_ADDRESS+0x05c)        /* used by nanotime() */
#define _COMM_PAGE_NT_NS_BASE           (_COMM_PAGE_START_ADDRESS+0x060)        /* used by nanotime() */
#define _COMM_PAGE_NT_GENERATION        (_COMM_PAGE_START_ADDRESS+0x068)        /* used by nanotime() */
#define _COMM_PAGE_GTOD_GENERATION      (_COMM_PAGE_START_ADDRESS+0x06c)        /* used by gettimeofday() */
#define _COMM_PAGE_GTOD_NS_BASE         (_COMM_PAGE_START_ADDRESS+0x070)        /* used by gettimeofday() */
#define _COMM_PAGE_GTOD_SEC_BASE        (_COMM_PAGE_START_ADDRESS+0x078)        /* used by gettimeofday() */

#define _COMM_PAGE_END                  (_COMM_PAGE_START_ADDRESS+0xfff)        /* end of common page */

#define _COMM_PAGE_TEXT_START		(_COMM_PAGE_START_ADDRESS+0x1000)

#endif
