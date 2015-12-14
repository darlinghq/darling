#include "dyld_priv.h"
#include <link.h>
#include <map>
#include <vector>
#include <stdexcept>
#include "LoadableObject.h"
#include "MachOMgr.h"
#include "MachOObject.h"
#include "dyld_public.h"

static std::map<dyld_image_state_change_handler, dyld_image_states> m_handlers;

static int walk_cb(struct dl_phdr_info *info, size_t size, void *data);

using namespace Darling;

void
dyld_register_image_state_change_handler(enum dyld_image_states state, bool batch, dyld_image_state_change_handler handler)
{
	if (!batch)
	{
		m_handlers[handler] = state;
	}
	else
	{
		std::vector<dyld_image_info> infos;

		// ELF
		dl_iterate_phdr(walk_cb, &infos);

		// Mach-O
		MachOMgr::instance()->iterateObjects([&](MachOObject* obj) {
			dyld_image_info info;

			if (obj->state() < state)
				return;

			info.imageLoadAddress = (struct mach_header*) obj->baseAddress();
			info.imageFilePath = obj->name();
			info.imageFileModDate = 0;

			infos.push_back(info);
		});

		handler(state, infos.size(), &infos[0]);
	}
}

static int walk_cb(struct dl_phdr_info *info, size_t size, void *data)
{
	std::vector<dyld_image_info>* infos;
	dyld_image_info dinfo;

	if (!info->dlpi_addr)
		return 0;

	infos = (std::vector<dyld_image_info>*) data;
	dinfo.imageLoadAddress = (struct mach_header*) info->dlpi_addr;
	dinfo.imageFilePath = info->dlpi_name;
	dinfo.imageFileModDate = 0;

	infos->push_back(dinfo);

	return 0;
}

void _dyld_run_handler_for_state(LoadableObject* obj)
{
	const dyld_image_states state = obj->state();
	dyld_image_info info;

	info.imageLoadAddress = (struct mach_header*) obj->baseAddress();
	info.imageFilePath = obj->name();
	info.imageFileModDate = 0;

	for (auto it = m_handlers.begin(); it != m_handlers.end(); it++)
	{
		if (it->second == state)
		{
			const char* error;
			error = it->first(state, 1, &info);

			if (error != nullptr)
				throw std::runtime_error(error);
		}
	}

	if (state == dyld_image_state_terminated)
		_dyld_free_eh_data(obj);
}

