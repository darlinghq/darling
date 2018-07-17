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

#ifndef _PMSAMPLE_H
#define _PMSAMPLE_H

#include <stdint.h>
#include <pmenergy.h>
#include <mach/mach.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// These constants are made up!!!
#define PM_SAMPLE_ALL 0xffffffff
#define PM_SAMPLE_NAME	1
#define PM_SAMPLE_INTERVAL	2
#define PM_SAMPLE_CPU	4
#define PM_SAMPLE_DISK	8

int pm_sample_task_and_pid(task_t task, pid_t pid, pm_task_energy_data_t* pme, uint64_t mach_time, unsigned int flags);

#ifdef __cplusplus
}
#endif

#endif

