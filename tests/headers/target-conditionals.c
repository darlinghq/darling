/*
 * This file is part of Darling.
 *
 * Copyright (C) 2026 Lubos Dolezel
 *
 * Darling is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Darling is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Darling.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Preprocessor checks for TargetConditionals.h and Availability.h. Compiled
 * by target-conditionals.sh for macOS and iOS targets with EXPECT_IOS set
 * (or not), and only the preprocessor matters: the file must compile cleanly.
 */
#include <TargetConditionals.h>
#include <Availability.h>

#ifdef EXPECT_IOS
#  if !TARGET_OS_IPHONE || !TARGET_OS_IOS || TARGET_OS_OSX || !TARGET_OS_MAC
#    error "iOS target must keep the target OS macros the compiler predefines"
#  endif
#else
#  if TARGET_OS_IPHONE || TARGET_OS_IOS || !TARGET_OS_OSX || !TARGET_OS_MAC
#    error "macOS target must be TARGET_OS_OSX and not iOS"
#  endif
#endif

#if TARGET_OS_WIN32 || TARGET_OS_UNIX || TARGET_OS_SIMULATOR || TARGET_OS_WATCH || TARGET_OS_TV
#  error "unrelated target OS macros must be 0"
#endif
#if TARGET_IPHONE_SIMULATOR != 0 || TARGET_OS_NANO != 0
#  error "deprecated aliases must be 0"
#endif

#if !TARGET_RT_LITTLE_ENDIAN || TARGET_RT_BIG_ENDIAN || !TARGET_RT_MAC_MACHO
#  error "runtime macros"
#endif

/* Public Availability version values */
#if __MAC_10_15 != 101500 || __MAC_10_15_4 != 101504 || __MAC_11_0 != 110000 || __MAC_13_5 != 130500
#  error "__MAC_* values"
#endif
#if __IPHONE_13_0 != 130000 || __IPHONE_14_0 != 140000 || __IPHONE_16_6 != 160600
#  error "__IPHONE_* values"
#endif
#if __TVOS_13_1 != 130100 || __TVOS_16_6 != 160600
#  error "__TVOS_* values"
#endif
#if __WATCHOS_6_0 != 60000 || __WATCHOS_9_6 != 90600
#  error "__WATCHOS_* values"
#endif
#if __DRIVERKIT_19_0 != 190000 || __DRIVERKIT_22_0 != 220000
#  error "__DRIVERKIT_* values"
#endif

int target_conditionals_ok(void) { return 1; }
