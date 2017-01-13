#ifndef EXT_MREMAP_H
#define EXT_MREMAP_H

int __linux_mremap(void* old_addr, unsigned long old_size, unsigned long new_size, int flags, void* new_address);

#endif

