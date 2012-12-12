#ifndef MACH_COMPAT_H
#define MACH_COMPAT_H

extern "C" void (*mach_init_routine)();
extern "C" void* (*_cthread_init_routine)();
extern "C" void (*_cthread_exit_routine) (int);

#endif

