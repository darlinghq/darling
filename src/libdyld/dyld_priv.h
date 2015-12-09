#ifndef DYLD_PRIV_H
#define DYLD_PRIV_H
#include <stdint.h>

namespace Darling {
class LoadableObject;
}

extern "C" {

struct dyld_image_info
{
	const struct mach_header* imageLoadAddress;
	const char* imageFilePath;
	uintptr_t imageFileModDate;
};

enum dyld_image_states
{
	dyld_image_state_mapped					= 10,
	dyld_image_state_dependents_mapped		= 20,
	dyld_image_state_rebased				= 30, 
	dyld_image_state_bound					= 40,
	dyld_image_state_dependents_initialized	= 45,
	dyld_image_state_initialized			= 50,
	dyld_image_state_terminated				= 60
};

typedef const char* (*dyld_image_state_change_handler)(enum dyld_image_states state, uint32_t infoCount, const struct dyld_image_info info[]);

extern void
dyld_register_image_state_change_handler(enum dyld_image_states state, bool batch, dyld_image_state_change_handler handler);

void _dyld_run_handler_for_state(Darling::LoadableObject* obj);

}

#endif

