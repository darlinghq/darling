/*
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Walking the address space with mach_vm_region(): a walk over every region of a process that has
 * thousands of mappings must cost time proportional to the number of regions, and the residency
 * counts that VM_REGION_TOP_INFO reports must be exact and current.
 * - Every cell of a large arena is a few pages followed by an inaccessible page (so cells never
 *   merge); cell i has its first (i % 5) pages touched.
 *   Walking up, walking down and visiting the cells in a scattered order must all report the same
 *   resident counts, and an untouched cell is SM_EMPTY.
 * - A page touched just before the region is queried is counted, also when the region before it
 *   was queried a moment earlier (an ascending walk must not report stale counts), and when the
 *   same region is queried again after touching more of it.
 * - A MAP_SHARED mapping is SM_SHARED and its touched pages count as shared resident pages.
 * - Walks with the other flavors (VM_REGION_BASIC_INFO_64, mach_vm_region_recurse) still find
 *   every cell.
 * Each walk prints its time; a walk of the arena that takes longer than a generous bound fails.
 * Builds for x86_64 and i386 and runs on macOS.
 */
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define CELLS 2000
#define CELL_PAGES 4
// a cell is its data pages followed by one inaccessible page
#define CELL_STRIDE (CELL_PAGES + 1)
// far above the fraction of a second a walk takes, far below the tens of seconds a quadratic one costs
#define WALK_LIMIT_SECONDS 10.0

static int failures;
#define CHECK(cond) do { if (!(cond)) { if (failures++ < 30) printf("FAIL line %d: %s\n", __LINE__, #cond); } } while (0)

static mach_vm_size_t page;
static mach_vm_address_t arena;

static double now(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static mach_vm_address_t cell_address(int i)
{
    return arena + (mach_vm_address_t)i * CELL_STRIDE * page;
}

// the pages of each cell that have been touched: the first (i % 5) at first
static unsigned char touched_mask[CELLS];

static unsigned expected_touched(int i)
{
    return __builtin_popcount(touched_mask[i]);
}

static kern_return_t top_info(mach_vm_address_t *address, mach_vm_size_t *size, vm_region_top_info_data_t *info)
{
    mach_msg_type_number_t count = VM_REGION_TOP_INFO_COUNT;
    mach_port_t object = MACH_PORT_NULL;
    return mach_vm_region(mach_task_self(), address, size, VM_REGION_TOP_INFO, (vm_region_info_t)info, &count, &object);
}

// the number of resident pages that the region of cell i reports, or -1 if it is not found where expected
static int cell_resident(int i, int *share_mode)
{
    vm_region_top_info_data_t info;
    mach_vm_address_t address = cell_address(i);
    mach_vm_size_t size;
    if (top_info(&address, &size, &info) != KERN_SUCCESS || address != cell_address(i) || size != CELL_PAGES * page)
        return -1;
    if (share_mode)
        *share_mode = info.share_mode;
    return info.private_pages_resident + info.shared_pages_resident;
}

static void check_cell(int i, unsigned expected)
{
    int mode = -1;
    int resident = cell_resident(i, &mode);
    if (resident != (int)expected) {
        if (failures++ < 30)
            printf("FAIL cell %d: %d resident pages, expected %u\n", i, resident, expected);
        return;
    }
    if (expected == 0)
        CHECK(mode == SM_EMPTY);
    else
        CHECK(mode == SM_PRIVATE);
}

static void touch(int cell, int index)
{
    ((volatile char *)cell_address(cell))[index * page] = 1;
    touched_mask[cell] |= 1 << index;
}

static double walk_top_info(unsigned *cells_seen, unsigned *regions)
{
    double start = now();
    mach_vm_address_t address = 0;
    mach_vm_size_t size;
    vm_region_top_info_data_t info;
    *cells_seen = 0;
    *regions = 0;
    while (top_info(&address, &size, &info) == KERN_SUCCESS) {
        (*regions)++;
        if (address >= arena && address < cell_address(CELLS) && (address - arena) % (CELL_STRIDE * page) == 0 && size == CELL_PAGES * page) {
            int i = (int)((address - arena) / (CELL_STRIDE * page));
            if ((int)(info.private_pages_resident + info.shared_pages_resident) != (int)expected_touched(i)) {
                if (failures++ < 30)
                    printf("FAIL walk cell %d: %u+%u resident, expected %u\n", i, info.private_pages_resident, info.shared_pages_resident, expected_touched(i));
            }
            (*cells_seen)++;
        }
        address += size;
        if (address == 0)
            break;
    }
    return now() - start;
}

static double walk_flavor(vm_region_flavor_t flavor, mach_msg_type_number_t info_count, unsigned *cells_seen)
{
    double start = now();
    mach_vm_address_t address = arena;
    mach_vm_size_t size;
    vm_region_info_data_t info;
    *cells_seen = 0;
    for (;;) {
        mach_msg_type_number_t count = info_count;
        mach_port_t object = MACH_PORT_NULL;
        if (mach_vm_region(mach_task_self(), &address, &size, flavor, (vm_region_info_t)&info, &count, &object) != KERN_SUCCESS || address >= cell_address(CELLS))
            break;
        if ((address - arena) % (CELL_STRIDE * page) == 0 && size == CELL_PAGES * page)
            (*cells_seen)++;
        address += size;
    }
    return now() - start;
}

static double walk_recurse(unsigned *cells_seen)
{
    double start = now();
    mach_vm_address_t address = arena;
    mach_vm_size_t size;
    *cells_seen = 0;
    for (;;) {
        vm_region_submap_short_info_data_64_t info;
        mach_msg_type_number_t count = VM_REGION_SUBMAP_SHORT_INFO_COUNT_64;
        natural_t depth = 0;
        if (mach_vm_region_recurse(mach_task_self(), &address, &size, &depth, (vm_region_recurse_info_t)&info, &count) != KERN_SUCCESS || address >= cell_address(CELLS))
            break;
        if ((address - arena) % (CELL_STRIDE * page) == 0 && size == CELL_PAGES * page)
            (*cells_seen)++;
        address += size;
    }
    return now() - start;
}

int main(void)
{
    page = getpagesize();

    CHECK(mach_vm_allocate(mach_task_self(), &arena, (mach_vm_size_t)CELLS * CELL_STRIDE * page, VM_FLAGS_ANYWHERE) == KERN_SUCCESS);
    for (int i = 0; i < CELLS; i++) {
        CHECK(mach_vm_protect(mach_task_self(), cell_address(i) + CELL_PAGES * page, page, FALSE, VM_PROT_NONE) == KERN_SUCCESS);
        for (int p = 0; p < i % 5; p++)
            touch(i, p);
    }

    // a walk over the whole address space, with the resident counts of every cell checked
    unsigned cells_seen, regions;
    double seconds = walk_top_info(&cells_seen, &regions);
    printf("walk VM_REGION_TOP_INFO: %u regions, %u cells, %.3f s\n", regions, cells_seen, seconds);
    CHECK(cells_seen == CELLS);
    CHECK(seconds < WALK_LIMIT_SECONDS);

    // the same counts when walking down, and when visiting the cells in a scattered order
    double start = now();
    // (only a sample of the cells: only an ascending walk is expected to be quick)
    for (int i = CELLS - 1; i >= 0; i -= 10)
        check_cell(i, expected_touched(i));
    printf("descending: %.3f s\n", now() - start);
    start = now();
    for (int i = 0, n = 0; n < CELLS / 10; n++, i = (i + 997) % CELLS)
        check_cell(i, expected_touched(i));
    printf("scattered: %.3f s\n", now() - start);

    // each touch is counted by the very next query, also when the previous query was for the cell before it
    for (int i = 0; i + 1 < CELLS; i += 2) {
        check_cell(i, expected_touched(i));
        if (!(touched_mask[i + 1] & (1 << (CELL_PAGES - 1)))) {
            touch(i + 1, CELL_PAGES - 1);
            check_cell(i + 1, expected_touched(i + 1));
        }
    }

    // querying one cell repeatedly sees the pages touched in between
    for (int p = 0; p < CELL_PAGES; p++) {
        int cell = CELLS / 2;
        if (!(touched_mask[cell] & (1 << p)))
            touch(cell, p);
        check_cell(cell, expected_touched(cell));
    }

    walk_top_info(&cells_seen, &regions);
    CHECK(cells_seen == CELLS);

    seconds = walk_flavor(VM_REGION_BASIC_INFO_64, VM_REGION_BASIC_INFO_COUNT_64, &cells_seen);
    printf("walk VM_REGION_BASIC_INFO_64: %u cells, %.3f s\n", cells_seen, seconds);
    CHECK(cells_seen == CELLS);
    CHECK(seconds < WALK_LIMIT_SECONDS);
    seconds = walk_recurse(&cells_seen);
    printf("walk mach_vm_region_recurse: %u cells, %.3f s\n", cells_seen, seconds);
    CHECK(cells_seen == CELLS);
    CHECK(seconds < WALK_LIMIT_SECONDS);

    // a shared mapping: its touched pages are shared resident pages
    char *shared = mmap(NULL, 6 * page, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
    CHECK(shared != MAP_FAILED);
    CHECK(mach_vm_protect(mach_task_self(), (mach_vm_address_t)(uintptr_t)shared + 4 * page, 2 * page, FALSE, VM_PROT_NONE) == KERN_SUCCESS);
    shared[0] = 1;
    shared[page] = 1;
    shared[2 * page] = 1;
    vm_region_top_info_data_t info;
    mach_vm_address_t address = (mach_vm_address_t)(uintptr_t)shared;
    mach_vm_size_t size;
    CHECK(top_info(&address, &size, &info) == KERN_SUCCESS);
    CHECK(address == (mach_vm_address_t)(uintptr_t)shared && size == 4 * page);
    printf("shared mapping: mode %d private %u shared %u\n", info.share_mode, info.private_pages_resident, info.shared_pages_resident);
    CHECK(info.share_mode == SM_SHARED);
    CHECK(info.shared_pages_resident == 3 && info.private_pages_resident == 0);

    if (failures) {
        printf("FAIL (%d)\n", failures);
        return 1;
    }
    printf("PASS\n");
    return 0;
}
