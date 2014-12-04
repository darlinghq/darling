/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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
/*
** This file contains interfaces to the symbols defined int the crt modules.
** 3 April 1995
** Matt Watson (mwatson@next.com)
**
*/

#if defined(__DYNAMIC__)
#include "mach-o/dyld.h" /* defines _dyld_lookup_and_bind() */
#define DECLARE_VAR(var, type)					\
        static type * var ## _pointer = 0
#define DECLARE_PROGNAME(var, type)				\
        static type * var ## _pointer = 0;			\
        static type _priv_ ## var = 0
#define USE_VAR(var) (var ## _pointer)
#else
#define DECLARE_VAR(var, type) extern type var
#define DECLARE_PROGNAME(var, type) DECLARE_VAR(var, type)
#define USE_VAR(var) (& var)
#endif

DECLARE_VAR(NXArgv, char **);
DECLARE_VAR(NXArgc, int);
DECLARE_VAR(environ, char **);
DECLARE_VAR(_mh_execute_header, struct mach_header);
DECLARE_PROGNAME(__progname, char *);

char ***_NSGetArgv(void) {
    return(USE_VAR(NXArgv));
}

int *_NSGetArgc(void) {
    return(USE_VAR(NXArgc));
}

char ***_NSGetEnviron(void) {
    return(USE_VAR(environ));
}

char **_NSGetProgname(void) {
    return(USE_VAR(__progname));
}

struct mach_header *_NSGetMachExecuteHeader(void) {
    return(USE_VAR(_mh_execute_header));
}

#if __DYNAMIC__
struct ProgramVars
{
    void*	mh;
    int*	NXArgcPtr;
    char***	NXArgvPtr;
    char***	environPtr;
    char**	__prognamePtr;
};


#define SUPPORT_PRE_GM_10_5_EXECUTABLES (__ppc__ || __i386__)


/*
 * dyld calls libSystem_initializer() and passes it a ProgramVars struct containing pointers to the
 * main executable's NXArg* global variables. libSystem_initializer() calls __libc_init() which calls
 * _program_vars_init() passing the ProgramVars parameter.
 */
void __attribute__((visibility("hidden")))
_program_vars_init(const struct ProgramVars* vars) {
#if SUPPORT_PRE_GM_10_5_EXECUTABLES
    // to support transitional 10.5 main executables that don't have extended __dyld section and instead call _NSSetProgramVars,  
    // don't overwrite values set by _NSSetProgramVars() 
    if ( NXArgv_pointer != NULL )
	return;
#endif
    NXArgv_pointer		= vars->NXArgvPtr;
    NXArgc_pointer		= vars->NXArgcPtr;
    environ_pointer		= vars->environPtr;
    __progname_pointer		= vars->__prognamePtr;
    _mh_execute_header_pointer	= vars->mh;
}

#if SUPPORT_PRE_GM_10_5_EXECUTABLES
/*
 * This is only called by main executables built with pre 10-5 GM crt1.10.5.o.  In those programs, 
 * there is no extended __dyld section, dyld cannot tell _program_vars_init() where the real program
 * variables are, so they get temp values and are set for real here.
 */
void _NSSetProgramVars(int* crt_argc, char*** crt_argv, char*** crt_environ, struct mach_header* crt_mh, char** crt_progname) {
    NXArgv_pointer		= crt_argv;
    NXArgc_pointer		= crt_argc;
    environ_pointer		= crt_environ;
    __progname_pointer		= crt_progname;
    _mh_execute_header_pointer	= crt_mh;
}
#endif 
#endif /* __DYNAMIC__ */

#if __ppc__
/*
 * Fix for Radar bug 2200596 --
 * EH symbol definitions for gcc 2.7.2.x implementation of
 * C++ exception handling.  The problem:  the EH implementation
 * has "throw" store stuff into these pointers, and then as the
 * stack is unwound, the code generated into each function (for
 * checking whether this function contains a relevant "catch"
 * clause and for calling destructors for local variables) looks
 * at these globals to find the type and value thrown.
 *
 * The problem was that the compiler generated the symbols as
 * "common" symbols, and common symbols cannot be placed in
 * dynamic shared libraries.  So we must put these guys as
 * "data" symbols into crt0.o or the System Framework (library),
 * and the compiler must generate code that defines the symbols
 * as external references instead of common.
 *
 * I changed the symbol names (added the "_272") to be utterly
 * paranoid about any possible future use of similar names by
 * any future versions of gcc.
 *                      -- D. Landauer, Jan. 1998
 */

void *__eh_pc_gcc_272 = (void *)0;
void *__eh_type_gcc_272 = (void *)0;
void *__eh_value_gcc_272 = (void *)0;

/* This is what egcs uses for its global data pointer */
void *__eh_global_dataptr = (void *)0;
#endif /* __ppc__ */

