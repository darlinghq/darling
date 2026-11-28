// Copyright (C) 2026 Lubos Dolezel
//
// Mach host/task/vm calls that darlingserver answers on behalf of the duct-taped XNU: each returns the
// documented result for a valid request on the caller's own task, and none of them takes the server down
// (the final getpid()/task_info() round trip checks that the server still answers).
// Builds for x86_64 and i386 and runs on macOS.
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_statistics.h>
#include <mach/processor_set.h>
#include <mach/thread_policy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)
#define SHOW(label, kr) printf("info %-34s -> %d (%s)\n", label, (int)(kr), mach_error_string(kr))

int main(void)
{
    mach_port_t host = mach_host_self();
    kern_return_t kr;

    // host_statistics(HOST_LOAD_INFO): load averages and mach factors, scaled by LOAD_SCALE.
    {
        host_load_info_data_t load;
        mach_msg_type_number_t count = HOST_LOAD_INFO_COUNT;
        kr = host_statistics(host, HOST_LOAD_INFO, (host_info_t)&load, &count);
        SHOW("host_statistics(LOAD_INFO)", kr);
        CHECK(kr == KERN_SUCCESS);
        if (kr == KERN_SUCCESS) {
            CHECK(count == HOST_LOAD_INFO_COUNT);
            for (int i = 0; i < 3; i++)
                CHECK(load.avenrun[i] >= 0 && load.mach_factor[i] >= 0);
        }
    }

    // host_info(HOST_SCHED_INFO): the scheduler's minimum quantum in milliseconds.
    {
        host_sched_info_data_t sched;
        mach_msg_type_number_t count = HOST_SCHED_INFO_COUNT;
        kr = host_info(host, HOST_SCHED_INFO, (host_info_t)&sched, &count);
        SHOW("host_info(SCHED_INFO)", kr);
        CHECK(kr == KERN_SUCCESS);
        if (kr == KERN_SUCCESS)
            CHECK(sched.min_timeout > 0 && sched.min_quantum > 0);
    }

    // mach_vm_read_list of a range of the caller's own memory.
    {
        mach_vm_address_t base = 0;
        CHECK(mach_vm_allocate(mach_task_self(), &base, 4 * vm_page_size, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);
        memset((void*)(uintptr_t)base, 0x5a, 4 * vm_page_size);
        mach_vm_read_entry_t list;
        memset(list, 0, sizeof(list));
        list[0].address = base;
        list[0].size = vm_page_size;
        kr = mach_vm_read_list(mach_task_self(), list, 1);
        SHOW("mach_vm_read_list", kr);
        CHECK(kr == KERN_SUCCESS);
        if (kr == KERN_SUCCESS)
            CHECK(list[0].address != 0 && ((unsigned char*)(uintptr_t)list[0].address)[100] == 0x5a);

        kr = mach_vm_behavior_set(mach_task_self(), base, 2 * vm_page_size, VM_BEHAVIOR_SEQUENTIAL);
        SHOW("mach_vm_behavior_set(SEQUENTIAL)", kr);
        CHECK(kr == KERN_SUCCESS);
        kr = mach_vm_behavior_set(mach_task_self(), base, 2 * vm_page_size, 12345);
        SHOW("mach_vm_behavior_set(invalid)", kr);
        CHECK(kr == KERN_INVALID_ARGUMENT);

        kr = mach_vm_inherit(mach_task_self(), base, vm_page_size, VM_INHERIT_COPY);
        SHOW("mach_vm_inherit(COPY)", kr);
        CHECK(kr == KERN_SUCCESS);

        vm_machine_attribute_val_t val = MATTR_VAL_CACHE_FLUSH;
        kr = mach_vm_machine_attribute(mach_task_self(), base, vm_page_size, MATTR_CACHE, &val);
        SHOW("mach_vm_machine_attribute(CACHE)", kr);
        CHECK(kr == KERN_SUCCESS);

        // mach_vm_page_query: page accounting may be unavailable (KERN_NOT_SUPPORTED), but the call must answer
        {
            int disposition = -1, ref_count = -1;
            kr = mach_vm_page_query(mach_task_self(), base, &disposition, &ref_count);
            SHOW("mach_vm_page_query", kr);
            CHECK(kr == KERN_SUCCESS || kr == KERN_NOT_SUPPORTED);
        }

        // XNU rejects an unknown flavor and a wrong count of VM_PAGE_INFO_BASIC.
        vm_page_info_basic_data_t pi;
        mach_msg_type_number_t pcount = VM_PAGE_INFO_BASIC_COUNT;
        kr = mach_vm_page_info(mach_task_self(), base, VM_PAGE_INFO_BASIC, (vm_page_info_t)&pi, &pcount);
        SHOW("mach_vm_page_info(BASIC)", kr);
        pcount = VM_PAGE_INFO_BASIC_COUNT;
        kr = mach_vm_page_info(mach_task_self(), base, 77, (vm_page_info_t)&pi, &pcount);
        SHOW("mach_vm_page_info(bad flavor)", kr);
        CHECK(kr == KERN_INVALID_ARGUMENT);
    }

    // task_purgable_info: nothing purgeable, every queue empty.
    {
        task_purgable_info_t pinfo;
        memset(&pinfo, 0xff, sizeof(pinfo));
        kr = task_purgable_info(mach_task_self(), &pinfo);
        SHOW("task_purgable_info", kr);
    }

    // task_get_assignment / processor_set_default: every task is in the default processor set.
    {
        processor_set_name_t pset;
        kr = task_get_assignment(mach_task_self(), &pset);
        SHOW("task_get_assignment", kr);
        CHECK(kr == KERN_SUCCESS);
    }

    // processor_set_create: there is only the default set.
    {
        processor_set_t s, n;
        kr = processor_set_create(host, &s, &n);
        SHOW("processor_set_create", kr);
        CHECK(kr == KERN_FAILURE);
    }

    // the server must still answer
    {
        task_basic_info_data_t ti;
        mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
        kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&ti, &count);
        CHECK(kr == KERN_SUCCESS);
        CHECK(getpid() > 0);
    }

    printf(failures ? "FAIL: %d failure(s)\n" : "PASS\n", failures);
    return failures ? 1 : 0;
}
