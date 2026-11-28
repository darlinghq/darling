// Copyright (C) 2026 Lubos Dolezel
//
// mach_memory_entry_ownership(): the owner of a named memory entry changes, and the ledger tag is one of the
// documented tags (VM_LEDGER_TAG_*) or the "unchanged" value -1, which changes the owner and keeps the tag.
// An undefined tag is refused with KERN_INVALID_ARGUMENT.
// Builds for x86_64 and i386 and runs on macOS.
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/memory_entry.h>
#include <mach/vm_statistics.h>
#include <stdio.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)
#define SHOW(label, kr) printf("info %-34s -> %d (%s)\n", label, (int)(kr), mach_error_string(kr))

int main(void)
{
    mach_vm_address_t base = 0;
    mach_vm_size_t size = 4 * vm_page_size;
    CHECK(mach_vm_allocate(mach_task_self(), &base, size, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);

    memory_object_size_t entry_size = size;
    mach_port_t entry = MACH_PORT_NULL;
    kern_return_t kr = mach_make_memory_entry_64(mach_task_self(), &entry_size, base,
        VM_PROT_READ | VM_PROT_WRITE | MAP_MEM_NAMED_CREATE, &entry, MACH_PORT_NULL);
    SHOW("mach_make_memory_entry_64", kr);
    CHECK(kr == KERN_SUCCESS && entry != MACH_PORT_NULL);
    if (kr != KERN_SUCCESS) {
        printf("FAIL: no named entry\n");
        return 1;
    }

    kr = mach_memory_entry_ownership(entry, mach_task_self(), VM_LEDGER_TAG_DEFAULT, 0);
    SHOW("ownership(self, TAG_DEFAULT)", kr);
    CHECK(kr == KERN_SUCCESS);

    kr = mach_memory_entry_ownership(entry, mach_task_self(), -1, 0);
    SHOW("ownership(self, UNCHANGED = -1)", kr);
    CHECK(kr == KERN_SUCCESS);

    kr = mach_memory_entry_ownership(entry, mach_task_self(), VM_LEDGER_TAG_MAX + 1000, 0);
    SHOW("ownership(self, undefined tag)", kr);
    CHECK(kr == KERN_INVALID_ARGUMENT);

    printf(failures ? "FAIL: %d failure(s)\n" : "PASS\n", failures);
    return failures ? 1 : 0;
}
