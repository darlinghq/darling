#ifndef BSDTHREAD_REGISTER_H
#define BSDTHREAD_REGISTER_H

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		void* dummy, void* targetconc, unsigned long long dpq_offset);

#endif

