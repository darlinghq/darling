/*
This file is part of Darling.

Copyright (C) 2026 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MLDR_CPU_CAPS_X86_H_
#define _MLDR_CPU_CAPS_X86_H_

#include <stdbool.h>
#include <stdint.h>

// The CPUID and XCR0 values the commpage capability bits are derived from.
struct x86_cpu_features {
	uint32_t leaf1_ecx;  // CPUID.01H:ECX
	uint32_t leaf1_edx;  // CPUID.01H:EDX
	uint32_t leaf7_ebx;  // CPUID.(EAX=07H,ECX=0):EBX
	uint64_t xcr0;       // XCR0, or 0 if the OS has not enabled XGETBV
	bool long_mode;      // CPUID.80000001H:EDX.LM
};

// Reads the features of the CPU we are running on.
void x86_cpu_features_read(struct x86_cpu_features* features);

// Translates CPU features into the commpage's kHas* capability bits.
uint64_t x86_cpu_caps_decode(const struct x86_cpu_features* features);

#endif
