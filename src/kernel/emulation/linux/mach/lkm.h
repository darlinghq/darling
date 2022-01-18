#ifndef _LKM_H
#define _LKM_H

#include <os/tsd.h>

// TSD slot 6 is reserved by Apple for Windows/WINE compatibility.
// Since WINE can never run under Darling, we can use that slot for our own purposes.
// Namely, we use it to store the the per-thread darlingserver socket.
#define __TSD_DSERVER_RPC_FD 6

void mach_driver_init(const char** applep);
int lkm_call(int call_nr, void* arg);
int lkm_call_raw(int call_nr, void* arg); // w/o errno translation
int mach_driver_get_fd(void);
int mach_driver_get_dyld_fd(void);

#endif

