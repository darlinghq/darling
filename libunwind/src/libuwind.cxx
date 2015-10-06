/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
 

#include <mach/mach_types.h>
#include <mach/machine.h>
#include <new>

#include "libunwind.h"
#include "libunwind_priv.h"

#include "UnwindCursor.hpp"



using namespace libunwind;


// setup debug logging hooks
INITIALIZE_DEBUG_PRINT_API
INITIALIZE_DEBUG_PRINT_UNWINDING


#if __ppc__ || __i386__ ||  __x86_64__

// internal object to represent this processes address space
static LocalAddressSpace sThisAddressSpace;

///
/// record the registers and stack position of the caller 
///
extern int unw_getcontext(unw_context_t*);
// note: unw_getcontext() implemented in assembly


///
/// create a cursor of a thread in this process given 'context' recorded by unw_getcontext()
///
EXPORT int unw_init_local(unw_cursor_t* cursor, unw_context_t* context)
{
	DEBUG_PRINT_API("unw_init_local(cursor=%p, context=%p)\n", cursor, context);
	// use "placement new" to allocate UnwindCursor in the cursor buffer
#if __i386__
	new ((void*)cursor) UnwindCursor<LocalAddressSpace,Registers_x86>(context, sThisAddressSpace);
#elif __x86_64__
	new ((void*)cursor) UnwindCursor<LocalAddressSpace,Registers_x86_64>(context, sThisAddressSpace);
#elif __ppc__
	new ((void*)cursor) UnwindCursor<LocalAddressSpace,Registers_ppc>(context, sThisAddressSpace);
#endif
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	co->setInfoBasedOnIPRegister();

	return UNW_ESUCCESS;
}

#if UNW_REMOTE

EXPORT unw_addr_space_t unw_local_addr_space = (unw_addr_space_t)&sThisAddressSpace;

///
/// create a cursor into a thread in another process
///
EXPORT int unw_init_remote_thread(unw_cursor_t* cursor, unw_addr_space_t as, thread_t thread)
{
	// special case: unw_init_remote(xx, unw_local_addr_space, xx)
	if ( as == (unw_addr_space_t)&sThisAddressSpace )
		return unw_init_local(cursor, NULL); //FIXME

	// use "placement new" to allocate UnwindCursor in the cursor buffer
	switch ( as->cpuType ) {
		case CPU_TYPE_I386:
			new ((void*)cursor) UnwindCursor<OtherAddressSpace<Pointer32<LittleEndian> >, 
											Registers_x86>(((unw_addr_space_i386*)as)->oas, thread);
			break;
		case CPU_TYPE_X86_64:
			new ((void*)cursor) UnwindCursor<OtherAddressSpace<Pointer64<LittleEndian> >, 
											Registers_x86_64>(((unw_addr_space_x86_64*)as)->oas, thread);
			break;
		case CPU_TYPE_POWERPC:
			new ((void*)cursor) UnwindCursor<OtherAddressSpace<Pointer32<BigEndian> >, 
											Registers_ppc>(((unw_addr_space_ppc*)as)->oas, thread);
			break;
		default:
			return UNW_EUNSPEC;
	}
	return UNW_ESUCCESS;
}

static bool rosetta(task_t task)
{
	return false; // FIXME
}

static bool is64bit(task_t task)
{
	return false; // FIXME
}

///
/// create an address_space object for use in examining another task
///
EXPORT unw_addr_space_t unw_create_addr_space_for_task(task_t task)
{
#if __i386__
	if ( rosetta(task) ) {
		unw_addr_space_ppc* as = new unw_addr_space_ppc(task);
		as->taskPort = task;
		as->cpuType = CPU_TYPE_POWERPC;
		//as->oas
	}
	else if ( is64bit(task) ) {
		unw_addr_space_x86_64* as = new unw_addr_space_x86_64(task);
		as->taskPort = task;
		as->cpuType = CPU_TYPE_X86_64;
		//as->oas
	}
	else  {
		unw_addr_space_i386* as = new unw_addr_space_i386(task);
		as->taskPort = task;
		as->cpuType = CPU_TYPE_I386;
		//as->oas
	}
#else
	// FIXME
#endif
}


///
/// delete an address_space object
///
EXPORT void unw_destroy_addr_space(unw_addr_space_t asp)
{
	switch ( asp->cpuType ) {
#if __i386__ || __x86_64__
		case CPU_TYPE_I386:	
			{
				unw_addr_space_i386* as = (unw_addr_space_i386*)asp;
				delete as;
			}
			break;
		case CPU_TYPE_X86_64:
			{
				unw_addr_space_x86_64* as = (unw_addr_space_x86_64*)asp;
				delete as;
			}
			break;
#endif
		case CPU_TYPE_POWERPC:
			{
				unw_addr_space_ppc* as = (unw_addr_space_ppc*)asp;
				delete as;
			}
			break;
	}
}
#endif // UNW_REMOTE


///
/// get value of specified register at cursor position in stack frame
///
EXPORT int unw_get_reg(unw_cursor_t* cursor, unw_regnum_t regNum, unw_word_t* value)
{
	DEBUG_PRINT_API("unw_get_reg(cursor=%p, regNum=%d, &value=%p)\n", cursor, regNum, value);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	if ( co->validReg(regNum) ) {
		*value = co->getReg(regNum);
		return UNW_ESUCCESS;
	}
	return UNW_EBADREG;
}


///
/// set value of specified register at cursor position in stack frame
///
EXPORT int unw_set_reg(unw_cursor_t* cursor, unw_regnum_t regNum, unw_word_t value)
{
	DEBUG_PRINT_API("unw_set_reg(cursor=%p, regNum=%d, value=0x%llX)\n", cursor, regNum, value);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	if ( co->validReg(regNum) ) {
		co->setReg(regNum, value);
		// specical case altering IP to re-find info (being called by personality function)
		if ( regNum == UNW_REG_IP ) {
			unw_proc_info_t info;
			co->getInfo(&info);
			uint64_t orgArgSize = info.gp;
			uint64_t orgFuncStart = info.start_ip;
			co->setInfoBasedOnIPRegister(false);
			// and adjust REG_SP if there was a DW_CFA_GNU_args_size
			if ( (orgFuncStart == info.start_ip) && (orgArgSize != 0) )
				co->setReg(UNW_REG_SP, co->getReg(UNW_REG_SP) + orgArgSize);
		}
		return UNW_ESUCCESS;
	}
	return UNW_EBADREG;
}


///
/// get value of specified float register at cursor position in stack frame
///
EXPORT int unw_get_fpreg(unw_cursor_t* cursor, unw_regnum_t regNum, unw_fpreg_t* value)
{
	DEBUG_PRINT_API("unw_get_fpreg(cursor=%p, regNum=%d, &value=%p)\n", cursor, regNum, value);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	if ( co->validFloatReg(regNum) ) {
		*value = co->getFloatReg(regNum);
		return UNW_ESUCCESS;
	}
	return UNW_EBADREG;
}

///
/// set value of specified float register at cursor position in stack frame
///
EXPORT int unw_set_fpreg(unw_cursor_t* cursor, unw_regnum_t regNum, unw_fpreg_t value)
{
	DEBUG_PRINT_API("unw_set_fpreg(cursor=%p, regNum=%d, value=%g)\n", cursor, regNum, value);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	if ( co->validFloatReg(regNum) ) {
		co->setFloatReg(regNum, value);
		return UNW_ESUCCESS;
	}
	return UNW_EBADREG;
}


///
/// move cursor to next frame 
///
EXPORT int unw_step(unw_cursor_t* cursor)
{
	DEBUG_PRINT_API("unw_step(cursor=%p)\n", cursor);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	return co->step();
}


///
/// get unwind info at cursor position in stack frame 
///
EXPORT int unw_get_proc_info(unw_cursor_t* cursor, unw_proc_info_t* info)
{
	DEBUG_PRINT_API("unw_get_proc_info(cursor=%p, &info=%p)\n", cursor, info);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	co->getInfo(info);
	if ( info->end_ip == 0 )
		return UNW_ENOINFO;
	else
		return UNW_ESUCCESS;
}

///
/// resume execution at cursor position (aka longjump) 
///
EXPORT int unw_resume(unw_cursor_t* cursor)
{
	DEBUG_PRINT_API("unw_resume(cursor=%p)\n", cursor);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	co->jumpto();
	return UNW_EUNSPEC;
}


///
/// get name of function at cursor position in stack frame 
///
EXPORT int unw_get_proc_name(unw_cursor_t* cursor, char* buf, size_t bufLen, unw_word_t* offset)
{
	DEBUG_PRINT_API("unw_get_proc_name(cursor=%p, &buf=%p, bufLen=%ld)\n", cursor, buf, bufLen);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	if ( co->getFunctionName(buf, bufLen, offset) )
		return UNW_ESUCCESS;
	else
		return UNW_EUNSPEC;
}


///
/// checks if a register is a floating-point register 
///
EXPORT int unw_is_fpreg(unw_cursor_t* cursor, unw_regnum_t regNum)
{
	DEBUG_PRINT_API("unw_is_fpreg(cursor=%p, regNum=%d)\n", cursor, regNum);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	return co->validFloatReg(regNum);
}


///
/// checks if a register is a floating-point register 
///
EXPORT const char* unw_regname(unw_cursor_t* cursor, unw_regnum_t regNum)
{
	DEBUG_PRINT_API("unw_regname(cursor=%p, regNum=%d)\n", cursor, regNum);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	return co->getRegisterName(regNum);
}


///
/// checks if current frame is signal trampoline 
///
EXPORT int unw_is_signal_frame(unw_cursor_t* cursor)
{
	DEBUG_PRINT_API("unw_is_signal_frame(cursor=%p)\n", cursor);
	AbstractUnwindCursor* co = (AbstractUnwindCursor*)cursor;
	return co->isSignalFrame();
}

#if !FOR_DYLD
///
/// SPI: walks cached dwarf entries
///
EXPORT void unw_iterate_dwarf_unwind_cache(void (*func)(unw_word_t ip_start, unw_word_t ip_end, unw_word_t fde, unw_word_t mh))
{
	DEBUG_PRINT_API("unw_iterate_dwarf_unwind_cache(func=%p)\n", func);
	DwarfFDECache<LocalAddressSpace>::iterateCacheEntries(func);
}
#endif // !FOR_DYLD


#if !FOR_DYLD
//
// IPI: for __register_frame()
//
void _unw_add_dynamic_fde(unw_word_t fde)
{
	CFI_Parser<LocalAddressSpace>::FDE_Info fdeInfo;
	CFI_Parser<LocalAddressSpace>::CIE_Info cieInfo;
	const char* message = CFI_Parser<LocalAddressSpace>::decodeFDE(sThisAddressSpace, (LocalAddressSpace::pint_t)fde, & fdeInfo, &cieInfo);
	if ( message == NULL ) {
		// dynamically registered FDEs don't have a mach_header group they are in.  Use fde as mh_group
		unw_word_t mh_group = fdeInfo.fdeStart;
		DwarfFDECache<LocalAddressSpace>::add(mh_group, fdeInfo.pcStart, fdeInfo.pcEnd, fdeInfo.fdeStart);
	}
	else {
		DEBUG_MESSAGE("_unw_add_dynamic_fde: bad fde: %s", message);
	}
}

//
// IPI: for __deregister_frame()
//
void _unw_remove_dynamic_fde(unw_word_t fde)
{
	// fde is own mh_group
	DwarfFDECache<LocalAddressSpace>::removeAllIn(fde);
}
#endif


#endif // __ppc__ || __i386__ ||  __x86_64__




