#ifndef _SYS_MEMDEV_H_
#define _SYS_MEMDEV_H_

#include <sys/appleapiopts.h>

#ifdef KERNEL_PRIVATE

#ifdef __APPLE_API_PRIVATE

void mdevinit(vm_offset_t base, unsigned int size);

#endif /* __APPLE_API_PRIVATE */

#endif /* KERNEL_PRIVATE */

#endif  /* _SYS_MEMDEV_H_*/
