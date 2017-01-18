#include "gdb.h"
#include <string.h>
#include <stdio.h>

__attribute__ ((noinline))
static void gdb_notifier(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[]);
static void dyld_notification_wrapper(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[]);

static struct dyld_all_image_infos* orig_dyld_all_image_infos;

// This is the symbol GDB looks for
struct dyld_all_image_infos _dyld_all_image_infos = {
	.version = 15,
	.infoArrayCount = 0,
	.infoArray = NULL,
	.notification = &gdb_notifier,
};

void gdb_notifier(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[])
{
	// GDB sets a breakpoint in this function
}

struct jump
{
	uint16_t mov;
	void* addr;
	uint16_t jump;
} __attribute__ ((packed));

void setup_gdb_notifications(uint64_t slide, uint64_t addr)
{
	orig_dyld_all_image_infos = (struct dyld_all_image_infos*)(addr + slide);

	// dyld will later rebase the address in notification,
	// but at this point we must add slide manually.
	struct jump* jump = (struct jump*)(((uint64_t)orig_dyld_all_image_infos->notification) + slide);

	// Rewrite instructions in the notification function to redirect the call to us.
#ifdef __x86_64__
	jump->mov = 0xb948; // movabs imm,%rcx
	jump->addr = (void*) &dyld_notification_wrapper; // immediate for preceding movabs
	jump->jump = 0xe1ff; // jmpq *%ecx
#else
#	error TODO: Unsupported platform
#endif
}

void dyld_notification_wrapper(enum dyld_image_mode mode, uint32_t infoCount, const struct dyld_image_info info[])
{
	// Copy over all data from dyld's copy of the structure to the one GDB can find (ours)
	memcpy(&_dyld_all_image_infos, orig_dyld_all_image_infos, sizeof(_dyld_all_image_infos));
	_dyld_all_image_infos.notification = &gdb_notifier;

	printf("Got notification from dyld\n");
	gdb_notifier(mode, infoCount, info);
}

