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

#include <stdlib.h>
#include <TargetConditionals.h>

struct ProgramVars; /* forward reference */

extern void _simple_asl_init(const char *envp[], const struct ProgramVars *vars);
extern void __pfz_setup(const char *apple[]);


void __libplatform_init(void *future_use __unused, const char *envp[], const char *apple[], const struct ProgramVars *vars) {

    /* In the Simulator, we just provide _simple for dyld */
#if !TARGET_IPHONE_SIMULATOR
    __pfz_setup(apple);
#endif
    _simple_asl_init(envp, vars);

}
