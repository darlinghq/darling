/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#include <sys/errno.h>
#include <sys/sfi.h>
#include <sys/types.h>
#include <unistd.h>

int system_set_sfi_window(uint64_t sfi_window_usec)
{
	return __sfi_ctl(SFI_CTL_OPERATION_SFI_SET_WINDOW, 0, sfi_window_usec, NULL);
}

int system_get_sfi_window(uint64_t *sfi_window_usec)
{
	return __sfi_ctl(SFI_CTL_OPERATION_SFI_GET_WINDOW, 0, 0, sfi_window_usec);
}

int sfi_set_class_offtime(sfi_class_id_t class_id, uint64_t offtime_usec)
{
	return __sfi_ctl(SFI_CTL_OPERATION_SET_CLASS_OFFTIME, class_id, offtime_usec, NULL);
}

int sfi_get_class_offtime(sfi_class_id_t class_id, uint64_t *offtime_usec)
{
	return __sfi_ctl(SFI_CTL_OPERATION_GET_CLASS_OFFTIME, class_id, 0, offtime_usec);
}

int sfi_process_set_flags(pid_t pid, uint32_t flags)
{
	return __sfi_pidctl(SFI_PIDCTL_OPERATION_PID_SET_FLAGS, pid, flags, NULL);
}

int sfi_process_get_flags(pid_t pid, uint32_t *flags)
{
	return __sfi_pidctl(SFI_PIDCTL_OPERATION_PID_GET_FLAGS, pid, 0, flags);
}
