// Copyright (C) 2026 Lubos Dolezel
//
// mach_vm_region() with the VM_REGION_TOP_INFO flavor: reports the region at
// (or above) the address with its share mode and resident page counts, which
// memory accounting code uses to count the resident pages of shared memory.
// - untouched anonymous memory has no memory object yet (SM_EMPTY, 0 pages);
// - touched private memory is SM_PRIVATE with the touched pages resident;
// - memory mapped from a memory entry is SM_SHARED, its touched pages counted
//   as shared resident pages;
// - an address above every region gives KERN_INVALID_ADDRESS, and a count
//   that is too small KERN_INVALID_ARGUMENT.
// The other flavors (VM_REGION_BASIC_INFO_64, mach_vm_region_recurse) must
// also stop at the end of the user address space: a walk over all regions
// ends below it, with KERN_INVALID_ADDRESS.
// Builds for x86_64 and i386 (-target i386-apple-macos10.13) and runs on macOS.
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int failures;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); failures++; } } while (0)

#define PAGES 16
#define TOUCHED 5

static kern_return_t top_info(mach_vm_address_t *address, mach_vm_size_t *size, vm_region_top_info_data_t *info)
{
    mach_msg_type_number_t count = VM_REGION_TOP_INFO_COUNT;
    mach_port_t object = MACH_PORT_NULL;
    memset(info, 0xa5, sizeof *info);
    kern_return_t kr = mach_vm_region(mach_task_self(), address, size, VM_REGION_TOP_INFO, (vm_region_info_t)info, &count, &object);
    if (kr == KERN_SUCCESS)
        CHECK(count == VM_REGION_TOP_INFO_COUNT);
    return kr;
}

// allocates PAGES pages between two inaccessible guard pages, so that the region can't be merged with its neighbours
static mach_vm_address_t allocate_isolated(mach_vm_size_t page)
{
    mach_vm_address_t base = 0;
    CHECK(mach_vm_allocate(mach_task_self(), &base, (PAGES + 2) * page, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);
    CHECK(mach_vm_protect(mach_task_self(), base, page, FALSE, VM_PROT_NONE) == KERN_SUCCESS);
    CHECK(mach_vm_protect(mach_task_self(), base + (PAGES + 1) * page, page, FALSE, VM_PROT_NONE) == KERN_SUCCESS);
    return base + page;
}

int main(void)
{
    const mach_vm_size_t page = getpagesize();
    vm_region_top_info_data_t info;
    mach_vm_address_t address;
    mach_vm_size_t size;

    // untouched private memory
    mach_vm_address_t fresh = allocate_isolated(page);
    address = fresh;
    CHECK(top_info(&address, &size, &info) == KERN_SUCCESS);
    CHECK(address == fresh && size == PAGES * page);
    printf("untouched: mode %d private %u shared %u\n", info.share_mode, info.private_pages_resident, info.shared_pages_resident);
    CHECK(info.share_mode == SM_EMPTY);
    CHECK(info.private_pages_resident == 0 && info.shared_pages_resident == 0);

    // touched private memory
    mach_vm_address_t touched = allocate_isolated(page);
    for (int i = 0; i < TOUCHED; i++)
        ((volatile char *)touched)[i * page] = 1;
    address = touched;
    CHECK(top_info(&address, &size, &info) == KERN_SUCCESS);
    CHECK(address == touched && size == PAGES * page);
    printf("touched: mode %d private %u shared %u\n", info.share_mode, info.private_pages_resident, info.shared_pages_resident);
    CHECK(info.share_mode == SM_PRIVATE);
    CHECK(info.private_pages_resident >= TOUCHED && info.shared_pages_resident == 0);

    // memory mapped from a memory entry
    memory_object_size_t entry_size = PAGES * page;
    mach_port_t entry = MACH_PORT_NULL;
    CHECK(mach_make_memory_entry_64(mach_task_self(), &entry_size, 0, MAP_MEM_NAMED_CREATE | VM_PROT_READ | VM_PROT_WRITE, &entry, MACH_PORT_NULL) == KERN_SUCCESS);
    mach_vm_address_t shared = 0;
    CHECK(mach_vm_map(mach_task_self(), &shared, PAGES * page, 0, VM_FLAGS_ANYWHERE, entry, 0, FALSE, VM_PROT_READ | VM_PROT_WRITE, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_NONE) == KERN_SUCCESS);
    for (int i = 0; i < TOUCHED; i++)
        ((volatile char *)shared)[i * page] = 1;
    address = shared;
    CHECK(top_info(&address, &size, &info) == KERN_SUCCESS);
    CHECK(address <= shared && address + size >= shared + PAGES * page);
    printf("memory entry: mode %d private %u shared %u\n", info.share_mode, info.private_pages_resident, info.shared_pages_resident);
    CHECK(info.share_mode == SM_SHARED);
    CHECK(info.shared_pages_resident >= TOUCHED && info.shared_pages_resident <= PAGES);
    CHECK(info.private_pages_resident == 0);

    // above every region
    const mach_vm_address_t high = (sizeof(void *) == 8) ? 0x7ffffffff000ULL : 0xfffff000ULL;
    address = high;
    CHECK(top_info(&address, &size, &info) == KERN_INVALID_ADDRESS);

    vm_region_basic_info_data_64_t basic;
    mach_msg_type_number_t basic_count = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t basic_object = MACH_PORT_NULL;
    address = high;
    kern_return_t kr = mach_vm_region(mach_task_self(), &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&basic, &basic_count, &basic_object);
    printf("basic info above every region: 0x%x\n", kr);
    CHECK(kr == KERN_INVALID_ADDRESS);

    vm_region_submap_short_info_data_64_t submap;
    mach_msg_type_number_t submap_count = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
    natural_t depth = 0;
    address = high;
    kr = mach_vm_region_recurse(mach_task_self(), &address, &size, &depth, (vm_region_recurse_info_t)&submap, &submap_count);
    printf("recurse above every region: 0x%x\n", kr);
    CHECK(kr == KERN_INVALID_ADDRESS);

    // a walk over every region ends below the end of the user address space
    // (64-bit: the lower half of the address space; 32-bit: below 4 GiB)
    const mach_vm_address_t user_end = (sizeof(void *) == 8) ? 0x800000000000ULL : 0x100000000ULL;
    mach_vm_address_t last_end = 0;
    int regions = 0;
    address = 0;
    for (;;) {
        basic_count = VM_REGION_BASIC_INFO_COUNT_64;
        kr = mach_vm_region(mach_task_self(), &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)&basic, &basic_count, &basic_object);
        if (kr != KERN_SUCCESS)
            break;
        regions++;
        last_end = address + size;
        address = last_end;
        if (regions > 100000 || last_end == 0)
            break;
    }
    printf("walk: %d regions, last ends at 0x%llx, then 0x%x\n", regions, (unsigned long long)last_end, kr);
    CHECK(regions > 0 && regions <= 100000);
    CHECK(last_end <= user_end);
    CHECK(kr == KERN_INVALID_ADDRESS);

    // too small a count
    mach_msg_type_number_t count = VM_REGION_TOP_INFO_COUNT - 1;
    mach_port_t object = MACH_PORT_NULL;
    address = touched;
    CHECK(mach_vm_region(mach_task_self(), &address, &size, VM_REGION_TOP_INFO, (vm_region_info_t)&info, &count, &object) == KERN_INVALID_ARGUMENT);

    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
