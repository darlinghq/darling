#ifndef _GDB_H_
#define _GDB_H_
#include <mach-o/dyld_images.h>
#include <stdint.h>

void setup_gdb_notifications(uint64_t slide, uint64_t addr);

#endif

