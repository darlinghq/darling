#include "compat.h"

void (*mach_init_routine)() = []() {};
void* (*_cthread_init_routine)() = []() -> void* { return nullptr; };
void (*_cthread_exit_routine) (int) = [](int) {};


