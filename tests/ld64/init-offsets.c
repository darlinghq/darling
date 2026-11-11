// Copyright (C) 2026 Lubos Dolezel
// Object for init-offsets.sh: two initializers, which -init_offsets turns
// into 32-bit image offsets in __TEXT,__init_offsets.
volatile int init_state;
__attribute__((constructor)) static void init1(void) { init_state += 1; }
__attribute__((constructor)) static void init2(void) { init_state += 2; }
