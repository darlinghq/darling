#ifndef _MLDR_LOADER_H_
#define _MLDR_LOADER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

struct load_results {
	unsigned long mh;
	unsigned long entry_point;
	unsigned long stack_size;
	unsigned long dyld_all_image_location;
	unsigned long dyld_all_image_size;
	uint8_t uuid[16];

	unsigned long vm_addr_max;
	bool _32on64;
	unsigned long base;
	uint32_t bprefs[4];
	char* root_path;
	unsigned long stack_top;
	char* socket_path;
	int kernfd;

	size_t argc;
	size_t envc;
	char** argv;
	char** envp;
};

#endif // _MLDR_LOADER_H_
