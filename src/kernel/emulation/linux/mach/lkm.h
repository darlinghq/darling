#ifndef _LKM_H
#define _LKM_H

void mach_driver_init(const char** applep);
int lkm_call(int call_nr, void* arg);
int lkm_call_raw(int call_nr, void* arg); // w/o errno translation
int mach_driver_get_fd(void);

#endif

