#ifndef BSDTHREAD_REGISTER_H
#define BSDTHREAD_REGISTER_H
typedef void (*bsdthread_entry_t)(void* /* self */, int /* thread_port */, void* /* stack */, unsigned long /* stacksize */, unsigned int /* flags */);

extern int pthread_obj_size;
extern bsdthread_entry_t pthread_entry_point;

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		void* dummy, void* targetconc, unsigned long long dpq_offset);

#endif

