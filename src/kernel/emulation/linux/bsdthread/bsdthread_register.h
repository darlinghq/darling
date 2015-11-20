#ifndef BSDTHREAD_REGISTER_H
#define BSDTHREAD_REGISTER_H
typedef void (*bsdthread_entry_t)(void* /* self */, int /* thread_port */, void* /* funptr */,
		void* /* funarg */, unsigned long /* stacksize */, unsigned int /* flags */);
typedef void (*bsdwqthread_entry_t)(void* /* self */, int /* thread_port */, void* /* stackaddr */,
		void* /* item */, int /* reuse */);

extern int pthread_obj_size;
extern bsdthread_entry_t pthread_entry_point;
extern bsdwqthread_entry_t wqueue_entry_point;

long sys_bsdthread_register(void* thread_start, void* wqthread, int pthsize,
		void* dummy, void* targetconc, unsigned long long dpq_offset);

#endif

