/*
 * Darling - pmenergy & pmsample stub
 * Copyright (c) 2018 Lubos Dolezel, All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

#ifndef _PMENERGY_H
#define _PMENERGY_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct pm_task_energy_data
{
	struct
	{
		uint64_t total_user, total_system, task_interrupt_wakeups, task_platform_idle_wakeups;
	} sti;

	uint64_t diskio_bytesread, diskio_byteswritten, pageins;
};

typedef struct pm_task_energy_data pm_task_energy_data_t;

uint64_t pm_energy_impact(const pm_task_energy_data_t* pme);

#ifdef __cplusplus
}
#endif

#endif

