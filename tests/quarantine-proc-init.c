// Copyright (C) 2026 Lubos Dolezel
//
// Guest test: _qtn_proc_init resets a process quarantine state to that of a
// fresh download, QTN_FLAG_DOWNLOAD (1), whatever flags it had (observed on
// macOS 26.7.3, x86_64, with the same calls).
#include <stdint.h>
#include <stdio.h>

typedef struct _qtn_proc *qtn_proc_t;
extern qtn_proc_t _qtn_proc_alloc(void);
extern void _qtn_proc_init(qtn_proc_t);
extern uint32_t _qtn_proc_get_flags(qtn_proc_t);
extern int _qtn_proc_set_flags(qtn_proc_t, uint32_t);
extern void _qtn_proc_free(qtn_proc_t);

int main(void)
{
    int failures = 0;
    const uint32_t before[] = { 0, 0x1, 0x2, 0x4, 0x40, 0x41 };
    for (unsigned i = 0; i < sizeof before / sizeof before[0]; i++) {
        qtn_proc_t proc = _qtn_proc_alloc();
        _qtn_proc_set_flags(proc, before[i]);
        _qtn_proc_init(proc);
        uint32_t after = _qtn_proc_get_flags(proc);
        printf("%s: flags 0x%x, then init: 0x%x\n", after == 0x1 ? "PASS" : "FAIL", before[i], after);
        failures += after != 0x1;
        _qtn_proc_free(proc);
    }
    return failures != 0;
}
