#ifndef _LKM_H
#define _LKM_H

void mach_driver_init(void);
int lkm_call(int call_nr, void* arg);

__attribute__((visibility("hidden")))
	extern int driver_fd;

#endif

