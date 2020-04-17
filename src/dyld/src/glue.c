/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2010 Apple Inc. All rights reserved.
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

#define _FORTIFY_SOURCE 0

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <TargetConditionals.h>
#include <libkern/OSAtomic.h>
#include <errno.h>
#include <pthread.h>
#include <corecrypto/ccdigest.h>
#include <corecrypto/ccsha1.h>
#include <corecrypto/ccsha2.h>

#if TARGET_OS_SIMULATOR
	#include "dyldSyscallInterface.h"
    #include <mach-o/dyld_images.h>
	#include <mach-o/loader.h>
	#include <mach-o/nlist.h>
	#include <mach/kern_return.h>
	#if __LP64__
		#define LC_SEGMENT_COMMAND			LC_SEGMENT_64
		typedef struct segment_command_64	macho_segment_command;
		typedef struct mach_header_64		macho_header;
		typedef struct nlist_64				macho_nlist;
	#else
		#define LC_SEGMENT_COMMAND			LC_SEGMENT
		typedef struct segment_command		macho_segment_command;
		typedef struct mach_header			macho_header;
		typedef struct nlist				macho_nlist;
	#endif

    #define DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE    (32*1024)
    #define DYLD_PROCESS_INFO_NOTIFY_LOAD_ID            0x1000
    #define DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID          0x2000
    #define DYLD_PROCESS_INFO_NOTIFY_MAIN_ID            0x3000

    struct dyld_process_info_image_entry {
        uuid_t                        uuid;
        uint64_t                    loadAddress;
        uint32_t                    pathStringOffset;
        uint32_t                    pathLength;
    };

    struct dyld_process_info_notify_header {
        mach_msg_header_t            header;
        uint32_t                    version;
        uint32_t                    imageCount;
        uint32_t                    imagesOffset;
        uint32_t                    stringsOffset;
        uint64_t                    timestamp;
    };
#endif

// from _simple.h in libc
typedef struct _SIMPLE*	_SIMPLE_STRING;
extern void				_simple_vdprintf(int __fd, const char *__fmt, va_list __ap);
extern void				_simple_dprintf(int __fd, const char *__fmt, ...);
extern _SIMPLE_STRING	_simple_salloc(void);
extern int				_simple_vsprintf(_SIMPLE_STRING __b, const char *__fmt, va_list __ap);
extern void				_simple_sfree(_SIMPLE_STRING __b);
extern char *			_simple_string(_SIMPLE_STRING __b);

// dyld::log(const char* format, ...)
extern void _ZN4dyld3logEPKcz(const char*, ...);

// dyld::halt(const char* msg);
extern void _ZN4dyld4haltEPKc(const char* msg) __attribute__((noreturn));

extern void dyld_fatal_error(const char* errString) __attribute__((noreturn));


// abort called by C++ unwinding code
void abort()
{
	_ZN4dyld4haltEPKc("dyld calling abort()\n");
}

// std::terminate called by C++ unwinding code
void _ZSt9terminatev()
{
	_ZN4dyld4haltEPKc("dyld std::terminate()\n");
}

// std::unexpected called by C++ unwinding code
void _ZSt10unexpectedv()
{
	_ZN4dyld4haltEPKc("dyld std::unexpected()\n");
}

// __cxxabiv1::__terminate(void (*)()) called to terminate process
void _ZN10__cxxabiv111__terminateEPFvvE()
{
	_ZN4dyld4haltEPKc("dyld std::__terminate()\n");
}

// __cxxabiv1::__unexpected(void (*)()) called to terminate process
void _ZN10__cxxabiv112__unexpectedEPFvvE()
{
	_ZN4dyld4haltEPKc("dyld std::__unexpected()\n");
}

// std::__terminate() called by C++ unwinding code
void _ZSt11__terminatePFvvE(void (*func)(void))
{
	_ZN4dyld4haltEPKc("dyld std::__terminate()\n");
}

// std::__unexpected() called by C++ unwinding code
void _ZSt12__unexpectedPFvvE(void (*func)(void))
{
	_ZN4dyld4haltEPKc("dyld std::__unexpected()\n");
}

// terminate_handler get_terminate()
void* _ZSt13get_terminatev()
{
    return NULL;
}

// unexpected_handler get_unexpected()
void* _ZSt14get_unexpectedv()
{
    return NULL;
}

// new_handler get_new_handler()
void* _ZSt15get_new_handlerv()
{
    return NULL;
}



// __cxxabiv1::__terminate_handler
void* _ZN10__cxxabiv119__terminate_handlerE  = &_ZSt9terminatev;

// __cxxabiv1::__unexpected_handler
void* _ZN10__cxxabiv120__unexpected_handlerE = &_ZSt10unexpectedv;


int	myfprintf(FILE* file, const char* format, ...) __asm("_fprintf");

// called by libuwind code before aborting
size_t fwrite(const void* ptr, size_t size, size_t nitme, FILE* stream)
{
	return myfprintf(stream, "%s", (char*)ptr);
}

// called by libuwind code before aborting
int	fprintf(FILE* file, const char* format, ...)
{
	va_list	list;
	va_start(list, format);
	_simple_vdprintf(STDERR_FILENO, format, list);
	va_end(list);
	return 0;
}

// called by LIBC_ABORT
void abort_report_np(const char* format, ...)
{
	va_list list;
	const char *str;
	_SIMPLE_STRING s = _simple_salloc();
	if ( s != NULL ) {
		va_start(list, format);
		_simple_vsprintf(s, format, list);
		va_end(list);
		str = _simple_string(s);
	}
	else {
		// _simple_salloc failed, but at least format may have useful info by itself
		str = format;
	}
	_ZN4dyld4haltEPKc(str);
	// _ZN4dyld4haltEPKc doesn't return, so we can't call _simple_sfree
}

// libc uses assert()
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-noreturn"
void __assert_rtn(const char* func, const char* file, int line, const char* failedexpr)
{
    if (func == NULL) {
		_ZN4dyld3logEPKcz("Assertion failed: (%s), file %s, line %d.\n", failedexpr, file, line);
        abort_report_np("Assertion failed: (%s), file %s, line %d.\n", failedexpr, file, line);
    } else {
		_ZN4dyld3logEPKcz("Assertion failed: (%s), function %s, file %s, line %d.\n", failedexpr, func, file, line);
        abort_report_np("Assertion failed: (%s), function %s, file %s, line %d.\n", failedexpr, func, file, line);
    }
}
#pragma clang diagnostic pop

int sprintf(char * restrict str, const char * restrict format, ...)
{
    va_list list;
    _SIMPLE_STRING s = _simple_salloc();
    va_start(list, format);
    _simple_vsprintf(s, format, list);
    va_end(list);
    strcpy(str, _simple_string(s));
    _simple_sfree(s);
    return 0;
}

// real cthread_set_errno_self() has error handling that pulls in
// pthread_exit() which pulls in fprintf()
extern int* __error(void);
void cthread_set_errno_self(int err)
{
	int* ep = __error();
     *ep = err;
}

/*
 * We have our own localtime() to avoid needing the notify API which is used
 * by the code in libc.a for localtime() which is used by arc4random().
 */
struct tm* localtime(const time_t* t)
{
	return (struct tm*)NULL;
}

// malloc calls exit(-1) in case of errors...
void exit(int x)
{
	_ZN4dyld4haltEPKc("exit()");
}

// static initializers make calls to __cxa_atexit
void __cxa_atexit()
{
	// do nothing, dyld never terminates
}

//
// The stack protector routines in lib.c bring in too much stuff, so
// make our own custom ones.
//
long __stack_chk_guard = 0;


void __guard_setup(const char* apple[])
{
	for (const char** p = apple; *p != NULL; ++p) {
		if ( strncmp(*p, "stack_guard=", 12) == 0 ) {
			// kernel has provide a random value for us
			for (const char* s = *p + 12; *s != '\0'; ++s) {
				char c = *s;
				long value = 0;
				if ( (c >= 'a') && (c <= 'f') )
					value = c - 'a' + 10;
				else if ( (c >= 'A') && (c <= 'F') )
					value = c - 'A' + 10;
				else if ( (c >= '0') && (c <= '9') )
					value = c - '0';
				__stack_chk_guard <<= 4;
				__stack_chk_guard |= value;
			}
			if ( __stack_chk_guard != 0 )
				return;
		}
	}
#if !TARGET_OS_SIMULATOR
#if __LP64__
	__stack_chk_guard = ((long)arc4random() << 32) | arc4random();
#else
	__stack_chk_guard = arc4random();
#endif
#endif
}

extern void _ZN4dyld4haltEPKc(const char*);
void __stack_chk_fail()
{
	_ZN4dyld4haltEPKc("stack buffer overrun");
}


// std::_throw_bad_alloc()
void _ZSt17__throw_bad_allocv()
{
	_ZN4dyld4haltEPKc("__throw_bad_alloc()");
}

// std::_throw_length_error(const char* x)
void _ZSt20__throw_length_errorPKc()
{
	_ZN4dyld4haltEPKc("_throw_length_error()");
}

// The aligned version of new isn't in libc++abi-static.a but might be called
// by __libcpp_allocate unless it is optimized perfectly
extern void* _Znwm(unsigned long size);
void* _ZnwmSt11align_val_t(unsigned long size, size_t align) {
    return _Znwm(size);
}

// The aligned version of new isn't in libc++abi-static.a but might be called
// by __libcpp_deallocate unless it is optimized perfectly
extern void _ZdlPv(void* ptr);
void _ZdlPvSt11align_val_t(void* ptr, size_t align) {
    _ZdlPv(ptr);
}

// the libc.a version of this drags in ASL
void __chk_fail()
{
	_ZN4dyld4haltEPKc("__chk_fail()");
}


// referenced by libc.a(pthread.o) but unneeded in dyld
void set_malloc_singlethreaded() {}
int PR_5243343_flag = 0;


// used by some pthread routines
char* mach_error_string(mach_error_t err)
{
	return (char *)"unknown error code";
}
char* mach_error_type(mach_error_t err)
{
	return (char *)"(unknown/unknown)";
}

// _pthread_reap_thread calls fprintf(stderr).
// We map fprint to _simple_vdprintf and ignore FILE* stream, so ok for it to be NULL
FILE* __stderrp = NULL;
FILE* __stdoutp = NULL;

// work with c++abi.a
void (*__cxa_terminate_handler)(void) = _ZSt9terminatev;
void (*__cxa_unexpected_handler)(void) = _ZSt10unexpectedv;

void abort_message(const char* format, ...)
{
	va_list	list;
	va_start(list, format);
	_simple_vdprintf(STDERR_FILENO, format, list);
	va_end(list);
}

void __cxa_bad_typeid()
{
	_ZN4dyld4haltEPKc("__cxa_bad_typeid()");
}

// to work with libc++
void _ZNKSt3__120__vector_base_commonILb1EE20__throw_length_errorEv()
{
	_ZN4dyld4haltEPKc("std::vector<>::_throw_length_error()");
}

// libc.a sometimes missing memset
#undef memset
void* memset(void* b, int c, size_t len)
{
	uint8_t* p = (uint8_t*)b;
	for(size_t i=len; i > 0; --i)
		*p++ = c;
	return b;
}


// <rdar://problem/10111032> wrap calls to stat() with check for EAGAIN
int _ZN4dyld7my_statEPKcP4stat(const char* path, struct stat* buf)
{
	int result;
	do {
		result = stat(path, buf);
    } while ((result == -1) && ((errno == EAGAIN) || (errno == EINTR)));

	return result;
}

// <rdar://problem/13805025> dyld should retry open() if it gets an EGAIN
int _ZN4dyld7my_openEPKcii(const char* path, int flag, int other)
{
	int result;
	do {
		result = open(path, flag, other);
	} while ((result == -1) && ((errno == EAGAIN) || (errno == EINTR)));

	return result;
}


//
// The dyld in the iOS simulator cannot do syscalls, so it calls back to
// host dyld.
//

#if TARGET_OS_SIMULATOR

int myopen(const char* path, int oflag, int extra) __asm("_open");
int myopen(const char* path, int oflag, int extra) {
	return gSyscallHelpers->open(path, oflag, extra);
}

int close(int fd) {
	return gSyscallHelpers->close(fd);
}

ssize_t pread(int fd, void* buf, size_t nbytes, off_t offset) {
	return gSyscallHelpers->pread(fd, buf , nbytes, offset);
}

ssize_t write(int fd, const void *buf, size_t nbytes) {
	return gSyscallHelpers->write(fd, buf , nbytes);
}

void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset) {
	return gSyscallHelpers->mmap(addr, len, prot, flags, fd, offset);
}

int munmap(void* addr, size_t len) {
	return gSyscallHelpers->munmap(addr, len);
}

int madvise(void* addr, size_t len, int advice) {
	return gSyscallHelpers->madvise(addr, len, advice);
}

int stat(const char* path, struct stat* buf) {
	return gSyscallHelpers->stat(path, buf);
}

int myfcntl(int fd, int cmd, void* result) __asm("_fcntl");
int myfcntl(int fd, int cmd, void* result) {
	return gSyscallHelpers->fcntl(fd, cmd, result);
}

int myioctl(int fd, unsigned long request, void* result) __asm("_ioctl");
int myioctl(int fd, unsigned long request, void* result) {
	return gSyscallHelpers->ioctl(fd, request, result);
}

int issetugid() {
	return gSyscallHelpers->issetugid();
}

char* getcwd(char* buf, size_t size) {
	return gSyscallHelpers->getcwd(buf, size);
}

char* realpath(const char* file_name, char* resolved_name) {
	return gSyscallHelpers->realpath(file_name, resolved_name);
}



kern_return_t vm_allocate(vm_map_t target_task, vm_address_t *address,
						  vm_size_t size, int flags) {
	return gSyscallHelpers->vm_allocate(target_task, address, size, flags);
}

kern_return_t vm_deallocate(vm_map_t target_task, vm_address_t address,
							vm_size_t size) {
	return gSyscallHelpers->vm_deallocate(target_task, address, size);
}

kern_return_t vm_protect(vm_map_t target_task, vm_address_t address,
							vm_size_t size, boolean_t max, vm_prot_t prot) {
	return gSyscallHelpers->vm_protect(target_task, address, size, max, prot);
}


void _ZN4dyld3logEPKcz(const char* format, ...) {
	va_list	list;
	va_start(list, format);
	gSyscallHelpers->vlog(format, list);
	va_end(list);
}

#if __i386__
void _ZN4dyld4vlogEPKcPc(const char* format, va_list list) {
#else
void _ZN4dyld4vlogEPKcP13__va_list_tag(const char* format, va_list list) {
#endif
	gSyscallHelpers->vlog(format, list);
}



void _ZN4dyld4warnEPKcz(const char* format, ...) {
	va_list	list;
	va_start(list, format);
	gSyscallHelpers->vwarn(format, list);
	va_end(list);
}


int pthread_mutex_lock(pthread_mutex_t* m) {
	return gSyscallHelpers->pthread_mutex_lock(m);
}

int pthread_mutex_unlock(pthread_mutex_t* m) {
	return gSyscallHelpers->pthread_mutex_unlock(m);
}

mach_port_t mach_thread_self() {
	return gSyscallHelpers->mach_thread_self();
}

kern_return_t mach_port_deallocate(ipc_space_t task, mach_port_name_t name) {
	return gSyscallHelpers->mach_port_deallocate(task, name);
}

mach_port_name_t task_self_trap() {
	return gSyscallHelpers->task_self_trap();
}

kern_return_t mach_timebase_info(mach_timebase_info_t info) {
	return gSyscallHelpers->mach_timebase_info(info);
}

bool OSAtomicCompareAndSwapPtrBarrier(void* old, void* new, void * volatile *value) {
	return gSyscallHelpers->OSAtomicCompareAndSwapPtrBarrier(old, new, value);
}

void OSMemoryBarrier()  {
	return gSyscallHelpers->OSMemoryBarrier();
}

uint64_t mach_absolute_time(void) {
	return gSyscallHelpers->mach_absolute_time();
}

kern_return_t thread_switch(mach_port_name_t thread_name,
							int option, mach_msg_timeout_t option_time) {
	if ( gSyscallHelpers->version < 2 )
		return KERN_FAILURE;
	return gSyscallHelpers->thread_switch(thread_name, option, option_time);
}

DIR* opendir(const char* path) {
	if ( gSyscallHelpers->version < 3 )
		return NULL;
	return gSyscallHelpers->opendir(path);
}

int	readdir_r(DIR* dirp, struct dirent* entry, struct dirent **result) {
	if ( gSyscallHelpers->version < 3 )
		return EPERM;
	return gSyscallHelpers->readdir_r(dirp, entry, result);
}

// HACK: readdir() is not used in dyld_sim, but it is pulled in by libc.a, then dead stripped.
struct dirent* readdir(DIR *dirp) {
    _ZN4dyld4haltEPKc("dyld_sim readdir() not supported\n");
}

int closedir(DIR* dirp) {
	if ( gSyscallHelpers->version < 3 )
		return EPERM;
	return gSyscallHelpers->closedir(dirp);
}

void coresymbolication_load_notifier(void* connection, uint64_t timestamp, const char* path, const struct mach_header* mh)
{
	// if host dyld supports this notifier, call into host dyld
	if ( gSyscallHelpers->version >= 4 )
		return gSyscallHelpers->coresymbolication_load_notifier(connection, timestamp, path, mh);
}

void coresymbolication_unload_notifier(void* connection, uint64_t timestamp, const char* path, const struct mach_header* mh)
{
	// if host dyld supports this notifier, call into host dyld
	if ( gSyscallHelpers->version >= 4 )
		return gSyscallHelpers->coresymbolication_unload_notifier(connection, timestamp, path, mh);
}

int mprotect(void* addr, size_t len, int prot)
{
    return vm_protect(mach_task_self(), (vm_address_t)addr,  len, false, prot);
}


#define SUPPORT_HOST_10_11  1

#if SUPPORT_HOST_10_11
typedef int               (*FuncPtr_proc_regionfilename)(int pid, uint64_t address, void* buffer, uint32_t bufferSize);
typedef pid_t             (*FuncPtr_getpid)(void);
typedef bool              (*FuncPtr_mach_port_insert_right)(ipc_space_t task, mach_port_name_t name, mach_port_t poly, mach_msg_type_name_t polyPoly);
typedef kern_return_t     (*FuncPtr_mach_port_allocate)(ipc_space_t, mach_port_right_t, mach_port_name_t*);
typedef mach_msg_return_t (*FuncPtr_mach_msg)(mach_msg_header_t *, mach_msg_option_t , mach_msg_size_t , mach_msg_size_t , mach_port_name_t , mach_msg_timeout_t , mach_port_name_t);
typedef void              (*FuncPtr_mach_msg_destroy)(mach_msg_header_t *);
typedef kern_return_t     (*FuncPtr_mach_port_construct)(ipc_space_t task, mach_port_options_ptr_t options, mach_port_context_t context, mach_port_name_t *name);
typedef kern_return_t     (*FuncPtr_mach_port_destruct)(ipc_space_t task, mach_port_name_t name, mach_port_delta_t srdelta, mach_port_context_t guard);

static FuncPtr_proc_regionfilename		 proc_proc_regionfilename = NULL;
static FuncPtr_getpid                    proc_getpid = NULL;
static FuncPtr_mach_port_insert_right    proc_mach_port_insert_right = NULL;
static FuncPtr_mach_port_allocate        proc_mach_port_allocate = NULL;
static FuncPtr_mach_msg                  proc_mach_msg = NULL;
static FuncPtr_mach_msg_destroy          proc_mach_msg_destroy = NULL;
static FuncPtr_mach_port_construct       proc_mach_port_construct = NULL;
static FuncPtr_mach_port_destruct        proc_mach_port_destruct = NULL;

static mach_port_t* sNotifyReplyPorts = NULL;
static bool*        sZombieNotifiers = NULL;

// Look up sycalls in host dyld needed by coresymbolication_ routines in dyld_sim
static void findHostFunctions() {
	// Only look up symbols once
	if ( proc_mach_msg != NULL )
		return;

	struct dyld_all_image_infos* imageInfo = (struct dyld_all_image_infos*)(gSyscallHelpers->getProcessInfo());
	const struct mach_header* hostDyldMH = imageInfo->dyldImageLoadAddress;

	// find symbol table and slide of host dyld
	uintptr_t slide = 0;
	const macho_nlist* symbolTable = NULL;
	const char* symbolTableStrings = NULL;
	const struct dysymtab_command* dynSymbolTable = NULL;
	const uint32_t cmd_count = hostDyldMH->ncmds;
	const struct load_command* const cmds = (struct load_command*)(((char*)hostDyldMH)+sizeof(macho_header));
	const struct load_command* cmd = cmds;
	const uint8_t* linkEditBase = NULL;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd) {
			case LC_SEGMENT_COMMAND:
				{
					const macho_segment_command* seg = (macho_segment_command*)cmd;
					if ( (seg->fileoff == 0) && (seg->filesize != 0) )
						slide = (uintptr_t)hostDyldMH - seg->vmaddr;
					if ( strcmp(seg->segname, "__LINKEDIT") == 0 )
						linkEditBase = (uint8_t*)(seg->vmaddr - seg->fileoff + slide);
				}
				break;
			case LC_SYMTAB:
				{
					const struct symtab_command* symtab = (struct symtab_command*)cmd;
					if ( linkEditBase == NULL )
						return;
					symbolTableStrings = (const char*)&linkEditBase[symtab->stroff];
					symbolTable = (macho_nlist*)(&linkEditBase[symtab->symoff]);
				}
				break;
			case LC_DYSYMTAB:
				dynSymbolTable = (struct dysymtab_command*)cmd;
				break;
		}
		cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
	}
	if ( symbolTableStrings == NULL )
		return;
	if ( dynSymbolTable == NULL )
		return;

	// scan local symbols in host dyld looking for load/unload functions
	const macho_nlist* const localsStart = &symbolTable[dynSymbolTable->ilocalsym];
	const macho_nlist* const localsEnd= &localsStart[dynSymbolTable->nlocalsym];
	for (const macho_nlist* s = localsStart; s < localsEnd; ++s) {
 		if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
			const char* name = &symbolTableStrings[s->n_un.n_strx];
			if ( strcmp(name, "_proc_regionfilename") == 0 )
				proc_proc_regionfilename = (FuncPtr_proc_regionfilename)(s->n_value + slide);
			else if ( strcmp(name, "_getpid") == 0 )
				proc_getpid = (FuncPtr_getpid)(s->n_value + slide);
			else if ( strcmp(name, "mach_port_insert_right") == 0 )
				proc_mach_port_insert_right = (FuncPtr_mach_port_insert_right)(s->n_value + slide);
			else if ( strcmp(name, "_mach_port_allocate") == 0 )
				proc_mach_port_allocate = (FuncPtr_mach_port_allocate)(s->n_value + slide);
			else if ( strcmp(name, "_mach_msg") == 0 )
                proc_mach_msg = (FuncPtr_mach_msg)(s->n_value + slide);
            else if (strcmp(name, "__ZN4dyldL17sNotifyReplyPortsE"))
                sNotifyReplyPorts = (mach_port_t*)(s->n_value + slide);
            else if (strcmp(name, "__ZN4dyldL16sZombieNotifiersE"))
                sZombieNotifiers = (bool *)(s->n_value + slide);
		}
	}
}

// Look up sycalls in host dyld needed by coresymbolication_ routines in dyld_sim
static bool findHostLibSystemFunctions() {
    // Only look up symbols once
    if (proc_mach_msg_destroy != NULL && proc_mach_port_construct != NULL && proc_mach_port_destruct != NULL)
        return true;

    const struct mach_header* hostLibSystemMH = NULL;
    struct dyld_all_image_infos* imageInfo = (struct dyld_all_image_infos*)(gSyscallHelpers->getProcessInfo());
    const struct dyld_image_info* infoArray = imageInfo->infoArray;
    if (infoArray == NULL)
        return false;
    uint32_t imageCount = imageInfo->infoArrayCount;
    for (uint32_t i = 0; i<imageCount; ++i) {
        if (strcmp("/usr/lib/system/libsystem_kernel.dylib", infoArray[i].imageFilePath) == 0) {
            //Found the kernel interface
            hostLibSystemMH = infoArray[i].imageLoadAddress;
            break;
        }
    }
    if (hostLibSystemMH == NULL)
        return false;

    // find symbol table and slide of host dyld
    uintptr_t slide = 0;
    const macho_nlist* symbolTable = NULL;
    const char* symbolTableStrings = NULL;
    const struct dysymtab_command* dynSymbolTable = NULL;
    const uint32_t cmd_count = hostLibSystemMH->ncmds;
    const struct load_command* const cmds = (struct load_command*)(((char*)hostLibSystemMH)+sizeof(macho_header));
    const struct load_command* cmd = cmds;
    const uint8_t* linkEditBase = NULL;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd) {
            case LC_SEGMENT_COMMAND:
            {
                const macho_segment_command* seg = (macho_segment_command*)cmd;
                if ( (seg->fileoff == 0) && (seg->filesize != 0) )
                    slide = (uintptr_t)hostLibSystemMH - seg->vmaddr;
                if ( strcmp(seg->segname, "__LINKEDIT") == 0 )
                    linkEditBase = (uint8_t*)(seg->vmaddr - seg->fileoff + slide);
            }
                break;
            case LC_SYMTAB:
            {
                const struct symtab_command* symtab = (struct symtab_command*)cmd;
                if ( linkEditBase == NULL )
                    return false;
                symbolTableStrings = (const char*)&linkEditBase[symtab->stroff];
                symbolTable = (macho_nlist*)(&linkEditBase[symtab->symoff]);
            }
                break;
            case LC_DYSYMTAB:
                dynSymbolTable = (struct dysymtab_command*)cmd;
                break;
        }
        cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
    }
    if ( symbolTableStrings == NULL )
        return false;;
    if ( dynSymbolTable == NULL )
        return false;;

    // scan local symbols in host dyld looking for load/unload functions
    const macho_nlist* const localsStart = &symbolTable[dynSymbolTable->iextdefsym];
    const macho_nlist* const localsEnd= &localsStart[dynSymbolTable->nextdefsym];
    for (const macho_nlist* s = localsStart; s < localsEnd; ++s) {
        if ( ((s->n_type & N_TYPE) == N_SECT) && ((s->n_type & N_STAB) == 0) ) {
            const char* name = &symbolTableStrings[s->n_un.n_strx];
            if ( strcmp(name, "_mach_msg_destroy") == 0 )
                proc_mach_msg_destroy = (FuncPtr_mach_msg_destroy)(s->n_value + slide);
            else if ( strcmp(name, "_mach_port_construct") == 0 )
                proc_mach_port_construct = (FuncPtr_mach_port_construct)(s->n_value + slide);
            else if ( strcmp(name, "_mach_port_destruct") == 0 )
                proc_mach_port_destruct = (FuncPtr_mach_port_destruct)(s->n_value + slide);
        }
    }
    return (proc_mach_msg_destroy != NULL && proc_mach_port_construct != NULL && proc_mach_port_destruct != NULL);
}
#endif


int proc_regionfilename(int pid, uint64_t address, void* buffer, uint32_t bufferSize)
{
	if ( gSyscallHelpers->version >= 5 )
		return gSyscallHelpers->proc_regionfilename(pid, address, buffer, bufferSize);
#if SUPPORT_HOST_10_11
	findHostFunctions();
	if ( proc_proc_regionfilename )
		return (*proc_proc_regionfilename)(pid, address, buffer, bufferSize);
	else
		return 0;
#else
	return 0;
#endif
}

pid_t getpid()
{
	if ( gSyscallHelpers->version >= 5 )
		return gSyscallHelpers->getpid();
#if SUPPORT_HOST_10_11
	findHostFunctions();
	return (*proc_getpid)();
#else
	return 0;
#endif
}

kern_return_t mach_port_insert_right(ipc_space_t task, mach_port_name_t name, mach_port_t poly, mach_msg_type_name_t polyPoly)
{
	if ( gSyscallHelpers->version >= 5 )
		return gSyscallHelpers->mach_port_insert_right(task, name, poly, polyPoly);
#if SUPPORT_HOST_10_11
	findHostFunctions();
	if ( proc_mach_port_insert_right )
		return (*proc_mach_port_insert_right)(task, name, poly, polyPoly);
	else
		return KERN_NOT_SUPPORTED;
#else
	return KERN_NOT_SUPPORTED;
#endif
}

kern_return_t mach_port_allocate(ipc_space_t task, mach_port_right_t right, mach_port_name_t* name)
{
	if ( gSyscallHelpers->version >= 5 )
		return gSyscallHelpers->mach_port_allocate(task, right, name);
#if SUPPORT_HOST_10_11
	findHostFunctions();
	return (*proc_mach_port_allocate)(task, right, name);
#else
	return KERN_NOT_SUPPORTED;
#endif
}

kern_return_t mach_msg(mach_msg_header_t* msg, mach_msg_option_t option, mach_msg_size_t send_size, mach_msg_size_t rcv_size, mach_port_name_t rcv_name, mach_msg_timeout_t timeout, mach_port_name_t notify)
{
	if ( gSyscallHelpers->version >= 5 )
		return gSyscallHelpers->mach_msg(msg, option, send_size, rcv_size, rcv_name, timeout, notify);
#if SUPPORT_HOST_10_11
	findHostFunctions();
	return (*proc_mach_msg)(msg, option, send_size, rcv_size, rcv_name, timeout, notify);
#else
	return KERN_NOT_SUPPORTED;
#endif
}

void mach_msg_destroy(mach_msg_header_t *msg) {
    if ( gSyscallHelpers->version >= 12 ) {
        gSyscallHelpers->mach_msg_destroy(msg);
        return;
    }
#if SUPPORT_HOST_10_11
    if (findHostLibSystemFunctions()) {
        (*proc_mach_msg_destroy)(msg);
    }
#endif
}

kern_return_t mach_port_construct(ipc_space_t task, mach_port_options_ptr_t options, mach_port_context_t context, mach_port_name_t *name) {
    if ( gSyscallHelpers->version >= 12 ) {
        return gSyscallHelpers->mach_port_construct(task, options, context, name);
    }
#if SUPPORT_HOST_10_11
    if (findHostLibSystemFunctions()) {
        return (*proc_mach_port_construct)(task, options, context, name);
    }
#endif
    return KERN_NOT_SUPPORTED;
}

kern_return_t mach_port_destruct(ipc_space_t task, mach_port_name_t name, mach_port_delta_t srdelta, mach_port_context_t guard) {
    if ( gSyscallHelpers->version >= 12 ) {
        return gSyscallHelpers->mach_port_destruct(task, name, srdelta, guard);
    }
#if SUPPORT_HOST_10_11
    if (findHostLibSystemFunctions()) {
        return (*proc_mach_port_destruct)(task, name, srdelta, guard);
    }
#endif
    return KERN_NOT_SUPPORTED;
}

void abort_with_payload(uint32_t reason_namespace, uint64_t reason_code, void* payload, uint32_t payload_size, const char* reason_string, uint64_t reason_flags)
{
	if ( gSyscallHelpers->version >= 6 )
		gSyscallHelpers->abort_with_payload(reason_namespace, reason_code, payload, payload_size, reason_string, reason_flags);
	dyld_fatal_error(reason_string);
}

kern_return_t   task_info(task_name_t target_task, task_flavor_t flavor, task_info_t task_info_out, mach_msg_type_number_t *task_info_outCnt) {
    if ( gSyscallHelpers->version >= 8 )
        return gSyscallHelpers->task_info(target_task, flavor, task_info_out, task_info_outCnt);
    return KERN_NOT_SUPPORTED;
}

kern_return_t   thread_info(thread_inspect_t target_act, thread_flavor_t flavor, thread_info_t thread_info_out, mach_msg_type_number_t *thread_info_outCnt) {
    if ( gSyscallHelpers->version >= 8 )
        return gSyscallHelpers->task_info(target_act, flavor, thread_info_out, thread_info_outCnt);
    return KERN_NOT_SUPPORTED;
}

bool kdebug_is_enabled(uint32_t code) {
    if ( gSyscallHelpers->version >= 8 )
        return gSyscallHelpers->kdebug_is_enabled(code);
    return false;
}

int kdebug_trace(uint32_t code, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
    if ( gSyscallHelpers->version >= 8 )
        return gSyscallHelpers->kdebug_trace(code, arg1, arg2, arg3, arg4);
    return 0;
}

uint64_t kdebug_trace_string(uint32_t debugid, uint64_t str_id, const char *str) {
    if ( gSyscallHelpers->version >= 9 )
        return gSyscallHelpers->kdebug_trace_string(debugid, str_id, str);
    return 0;
}

uint64_t amfi_check_dyld_policy_self(uint64_t inFlags, uint64_t* outFlags)
{
    if ( gSyscallHelpers->version >= 10 )
        return gSyscallHelpers->amfi_check_dyld_policy_self(inFlags, outFlags);
    *outFlags = 0x3F;  // on old kernel, simulator process get all flags
    return 0;
}

void _ZN4dyld24notifyMonitoringDyldMainEv() {
    if ( gSyscallHelpers->version >= 11 ) {
        gSyscallHelpers->notifyMonitoringDyldMain();
        return;
    }
#if SUPPORT_HOST_10_11
    findHostFunctions();
    struct dyld_all_image_infos* imageInfo = (struct dyld_all_image_infos*)(gSyscallHelpers->getProcessInfo());
    for (int slot=0; slot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++slot) {
        if ( (imageInfo->notifyPorts[slot] != 0 ) && !sZombieNotifiers[slot] ) {
            if ( sNotifyReplyPorts[slot] == 0 ) {
                if ( !mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &sNotifyReplyPorts[slot]) )
                    mach_port_insert_right(mach_task_self(), sNotifyReplyPorts[slot], sNotifyReplyPorts[slot], MACH_MSG_TYPE_MAKE_SEND);
                //dyld::log("allocated reply port %d\n", sNotifyReplyPorts[slot]);
            }
            //dyld::log("found port to send to\n");
            uint8_t messageBuffer[sizeof(mach_msg_header_t) + MAX_TRAILER_SIZE];
            mach_msg_header_t* h = (mach_msg_header_t*)messageBuffer;
            h->msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,MACH_MSG_TYPE_MAKE_SEND); // MACH_MSG_TYPE_MAKE_SEND_ONCE
            h->msgh_id          = DYLD_PROCESS_INFO_NOTIFY_MAIN_ID;
            h->msgh_local_port    = sNotifyReplyPorts[slot];
            h->msgh_remote_port    = imageInfo->notifyPorts[slot];
            h->msgh_reserved    = 0;
            h->msgh_size        = (mach_msg_size_t)sizeof(messageBuffer);
            //dyld::log("sending to port[%d]=%d, size=%d, reply port=%d, id=0x%X\n", slot, dyld::gProcessInfo->notifyPorts[slot], h->msgh_size, sNotifyReplyPorts[slot], h->msgh_id);
            kern_return_t sendResult = mach_msg(h, MACH_SEND_MSG | MACH_RCV_MSG | MACH_RCV_TIMEOUT, h->msgh_size, h->msgh_size, sNotifyReplyPorts[slot], 5000, MACH_PORT_NULL);
            //dyld::log("send result = 0x%X, msg_id=%d, msg_size=%d\n", sendResult, h->msgh_id, h->msgh_size);
            if ( sendResult == MACH_SEND_INVALID_DEST ) {
                // sender is not responding, detatch
                //dyld::log("process requesting notification gone. deallocation send port %d and receive port %d\n", dyld::gProcessInfo->notifyPorts[slot], sNotifyReplyPorts[slot]);
                mach_port_deallocate(mach_task_self(), imageInfo->notifyPorts[slot]);
                mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[slot]);
                imageInfo->notifyPorts[slot] = 0;
                sNotifyReplyPorts[slot] = 0;
            }
            else if ( sendResult == MACH_RCV_TIMED_OUT ) {
                // client took too long, ignore him from now on
                sZombieNotifiers[slot] = true;
                mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[slot]);
                sNotifyReplyPorts[slot] = 0;
            }
        }
    }
#endif
}

#if SUPPORT_HOST_10_11
static void notifyMonitoringDyld(bool unloading, unsigned portSlot, unsigned imageCount,  const struct mach_header* loadAddresses[], const char* imagePaths[])
{
    if ( sZombieNotifiers[portSlot] )
        return;
    struct dyld_all_image_infos* imageInfo = (struct dyld_all_image_infos*)(gSyscallHelpers->getProcessInfo());
    unsigned entriesSize = imageCount*sizeof(struct dyld_process_info_image_entry);
    unsigned pathsSize = 0;
    for (unsigned j=0; j < imageCount; ++j) {
        pathsSize += (strlen(imagePaths[j]) + 1);
    }
    unsigned totalSize = (sizeof(struct dyld_process_info_notify_header) + MAX_TRAILER_SIZE + entriesSize + pathsSize + 127) & -128;   // align
    if ( totalSize > DYLD_PROCESS_INFO_NOTIFY_MAX_BUFFER_SIZE ) {
        // Putting all image paths into one message would make buffer too big.
        // Instead split into two messages.  Recurse as needed until paths fit in buffer.
        unsigned imageHalfCount = imageCount/2;
        notifyMonitoringDyld(unloading, portSlot, imageHalfCount, loadAddresses, imagePaths);
        notifyMonitoringDyld(unloading, portSlot, imageCount - imageHalfCount, &loadAddresses[imageHalfCount], &imagePaths[imageHalfCount]);
        return;
    }
    uint8_t    buffer[totalSize];
    struct dyld_process_info_notify_header* header = (struct dyld_process_info_notify_header*)buffer;
    header->version            = 1;
    header->imageCount        = imageCount;
    header->imagesOffset    = sizeof(struct dyld_process_info_notify_header);
    header->stringsOffset    = sizeof(struct dyld_process_info_notify_header) + entriesSize;
    header->timestamp        = imageInfo->infoArrayChangeTimestamp;
    struct dyld_process_info_image_entry* entries = (struct dyld_process_info_image_entry*)&buffer[header->imagesOffset];
    char* const pathPoolStart = (char*)&buffer[header->stringsOffset];
    char* pathPool = pathPoolStart;
    for (unsigned j=0; j < imageCount; ++j) {
        strcpy(pathPool, imagePaths[j]);
        uint32_t len = (uint32_t)strlen(pathPool);
        bzero(entries->uuid, 16);
        const macho_header* mh = (const macho_header*)loadAddresses[j];
        const uint32_t cmd_count = mh->ncmds;
        const struct load_command* const cmds = (struct load_command*)((char*)mh + sizeof(macho_header));
        const struct load_command* cmd = cmds;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            if (cmd->cmd == LC_UUID) {
                    struct uuid_command* uc = (struct uuid_command*)cmd;
                    memcpy(&entries->uuid[0], uc->uuid, 16);
                    break;
            }
            cmd = (const struct load_command*)(((char*)cmd)+cmd->cmdsize);
        }
        entries->loadAddress = (uint64_t)loadAddresses[j];
        entries->pathStringOffset = (uint32_t)(pathPool - pathPoolStart);
        entries->pathLength  = len;
        pathPool += (len +1);
        ++entries;
    }

    if ( sNotifyReplyPorts[portSlot] == 0 ) {
        if ( !mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &sNotifyReplyPorts[portSlot]) )
            mach_port_insert_right(mach_task_self(), sNotifyReplyPorts[portSlot], sNotifyReplyPorts[portSlot], MACH_MSG_TYPE_MAKE_SEND);
        //dyld::log("allocated reply port %d\n", sNotifyReplyPorts[portSlot]);
    }
    //dyld::log("found port to send to\n");
    mach_msg_header_t* h = (mach_msg_header_t*)buffer;
    h->msgh_bits        = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND,MACH_MSG_TYPE_MAKE_SEND); // MACH_MSG_TYPE_MAKE_SEND_ONCE
    h->msgh_id            = unloading ? DYLD_PROCESS_INFO_NOTIFY_UNLOAD_ID : DYLD_PROCESS_INFO_NOTIFY_LOAD_ID;
    h->msgh_local_port    = sNotifyReplyPorts[portSlot];
    h->msgh_remote_port = imageInfo->notifyPorts[portSlot];
    h->msgh_reserved    = 0;
    h->msgh_size        = (mach_msg_size_t)sizeof(buffer);
    //dyld::log("sending to port[%d]=%d, size=%d, reply port=%d, id=0x%X\n", portSlot, dyld::gProcessInfo->notifyPorts[portSlot], h->msgh_size, sNotifyReplyPorts[portSlot], h->msgh_id);
    kern_return_t sendResult = mach_msg(h, MACH_SEND_MSG | MACH_RCV_MSG | MACH_RCV_TIMEOUT, h->msgh_size, h->msgh_size, sNotifyReplyPorts[portSlot], 5000, MACH_PORT_NULL);
    //dyld::log("send result = 0x%X, msg_id=%d, msg_size=%d\n", sendResult, h->msgh_id, h->msgh_size);
    if ( sendResult == MACH_SEND_INVALID_DEST ) {
        // sender is not responding, detatch
        //dyld::log("process requesting notification gone. deallocation send port %d and receive port %d\n", dyld::gProcessInfo->notifyPorts[portSlot], sNotifyReplyPorts[portSlot]);
        mach_port_deallocate(mach_task_self(), imageInfo->notifyPorts[portSlot]);
        mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[portSlot]);
        imageInfo->notifyPorts[portSlot] = 0;
        sNotifyReplyPorts[portSlot] = 0;
    }
    else if ( sendResult == MACH_RCV_TIMED_OUT ) {
        // client took too long, ignore him from now on
        sZombieNotifiers[portSlot] = true;
        mach_port_deallocate(mach_task_self(), sNotifyReplyPorts[portSlot]);
        sNotifyReplyPorts[portSlot] = 0;
    }
}
#endif

void _ZN4dyld20notifyMonitoringDyldEbjPPK11mach_headerPPKc(bool unloading, unsigned imageCount, const struct mach_header* loadAddresses[], const char* imagePaths[]) {
    if ( gSyscallHelpers->version >= 11 ) {
        gSyscallHelpers->notifyMonitoringDyld(unloading, imageCount, loadAddresses, imagePaths);
        return;
    }
#if SUPPORT_HOST_10_11
    findHostFunctions();
    for (int slot=0; slot < DYLD_MAX_PROCESS_INFO_NOTIFY_COUNT; ++slot) {
        notifyMonitoringDyld(unloading, slot, imageCount, loadAddresses, imagePaths);
    }
#endif
}

int* __error(void) {
	return gSyscallHelpers->errnoAddress();
}

void mach_init() {
	mach_task_self_ = task_self_trap();
	//_task_reply_port = _mach_reply_port();
}

mach_port_t mach_task_self_ = MACH_PORT_NULL;

extern int myerrno_fallback  __asm("_errno");
int myerrno_fallback = 0;


vm_size_t vm_kernel_page_mask = 0xFFF;
vm_size_t vm_page_size = 0x1000;

#endif  // TARGET_OS_SIMULATOR


#if ! TARGET_OS_SIMULATOR
	#include <mach-o/dyld_process_info.h>

	void _dyld_debugger_notification(enum dyld_notify_mode mode, unsigned long count, uint64_t machHeaders[])
	{
		// Do nothing.  This exists for the debugger to set a break point on to see what images have been loaded or unloaded.
	}
#endif


void* _NSConcreteStackBlock[32];
void* _NSConcreteGlobalBlock[32];

void _Block_object_assign()
{
	_ZN4dyld4haltEPKc("_Block_object_assign()");
}

void _Block_object_dispose(const void* object, int flags)
{
	// only support stack blocks in dyld: BLOCK_FIELD_IS_BYREF=8
	if ( flags != 8 )
		_ZN4dyld4haltEPKc("_Block_object_dispose()");
}



#if !TARGET_OS_SIMULATOR
errno_t memset_s(void* s, rsize_t smax, int c, rsize_t n)
{
    errno_t err = 0;
    if (s == NULL)
        return EINVAL;
    if (n > smax) {
        err = EOVERFLOW;
        n = smax;
    }
    memset(s, c, n);
    return err;
}
#endif
    
void uuid_unparse_upper(const uuid_t uu, uuid_string_t out)
{
    sprintf(out,
             "%02X%02X%02X%02X-"
             "%02X%02X-"
             "%02X%02X-"
             "%02X%02X-"
             "%02X%02X%02X%02X%02X%02X",
             uu[0], uu[1], uu[2], uu[3],
             uu[4], uu[5],
             uu[6], uu[7],
             uu[8], uu[9],
             uu[10], uu[11], uu[12], uu[13], uu[14], uu[15]);
}
