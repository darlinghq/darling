/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
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
 
//
//	C++ interface to lower levels of libuwind 
//

#ifndef __ADDRESSSPACE_HPP__
#define __ADDRESSSPACE_HPP__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <mach-o/loader.h>
#include <mach-o/getsect.h>
#include <mach-o/dyld_priv.h>
#include <Availability.h>

#include "FileAbstraction.hpp"
#include "libunwind.h"
#include "InternalMacros.h"
#include "dwarf2.h"


#if __i386__ && defined(__IPHONE_OS_VERSION_MIN_REQUIRED)
// For iOS simulator to link, we need a __dyld section
// We need one to access private _dyld_func_lookup function.

struct __DATA__dyld { long lazy; int (*lookup)(const char*, void**); };

static volatile struct __DATA__dyld  myDyldSection __attribute__ ((section ("__DATA,__dyld"))) = { 0, NULL };


static int my_dyld_func_lookup(const char* dyld_func_name, void **address)
{
	return (*myDyldSection.lookup)(dyld_func_name, address);
}

bool _dyld_find_unwind_sections(void* addr, dyld_unwind_sections* info)
{
    static void* (*p)(void*, dyld_unwind_sections*) = NULL;

	if(p == NULL)
	    my_dyld_func_lookup("__dyld_find_unwind_sections", (void**)&p);
	return p(addr, info);
}
#endif


namespace libunwind {

///
/// LocalAddressSpace is used as a template parameter to UnwindCursor when unwinding a thread
/// in the same process.  It compiles away and making local unwinds very fast.
///
class LocalAddressSpace
{
public:
	
	#if __LP64__
		typedef uint64_t	pint_t;
		typedef  int64_t	sint_t;
	#else
		typedef uint32_t	pint_t;
		typedef  int32_t	sint_t;
	#endif
		uint8_t			get8(pint_t addr)	{ return *((uint8_t*)addr); }
		uint16_t		get16(pint_t addr)	{ return *((uint16_t*)addr); }
		uint32_t		get32(pint_t addr)	{ return *((uint32_t*)addr); }
		uint64_t		get64(pint_t addr)	{ return *((uint64_t*)addr); }
		double			getDouble(pint_t addr)	{ return *((double*)addr); }
		v128			getVector(pint_t addr)	{ return *((v128*)addr); }
		uintptr_t		getP(pint_t addr);
	static uint64_t		getULEB128(pint_t& addr, pint_t end);
	static int64_t		getSLEB128(pint_t& addr, pint_t end);
	
		pint_t			getEncodedP(pint_t& addr, pint_t end, uint8_t encoding);
		bool			findFunctionName(pint_t addr, char* buf, size_t bufLen, unw_word_t* offset);
		bool			findUnwindSections(pint_t addr, pint_t& mh, pint_t& dwarfStart, pint_t& dwarfLen, pint_t& compactStart);
	
};

LocalAddressSpace sThisAddress;

inline uintptr_t LocalAddressSpace::getP(pint_t addr)
{
#if __LP64__
	return get64(addr);
#else
	return get32(addr);
#endif
}

/* Read a ULEB128 into a 64-bit word.   */
inline uint64_t
LocalAddressSpace::getULEB128(pint_t& addr, pint_t end)
{
	const uint8_t* p = (uint8_t*)addr;
	const uint8_t* pend = (uint8_t*)end;
	uint64_t result = 0;
	int bit = 0;
	do  {
		uint64_t b;

		if ( p == pend )
			ABORT("truncated uleb128 expression");

		b = *p & 0x7f;

		if (bit >= 64 || b << bit >> bit != b) {
			ABORT("malformed uleb128 expression");
		}
		else {
			result |= b << bit;
			bit += 7;
		}
	} while ( *p++ >= 0x80 );
	addr = (pint_t)p;
	return result;
}

/* Read a SLEB128 into a 64-bit word.  */
inline int64_t
LocalAddressSpace::getSLEB128(pint_t& addr, pint_t end)
{
	const uint8_t* p = (uint8_t*)addr;
	int64_t result = 0;
	int bit = 0;
	uint8_t byte;
	do {
		byte = *p++;
		result |= ((byte & 0x7f) << bit);
		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers
	if ( (byte & 0x40) != 0 )
		result |= (-1LL) << bit;
	addr = (pint_t)p;
	return result;
}

LocalAddressSpace::pint_t
LocalAddressSpace::getEncodedP(pint_t& addr, pint_t end, uint8_t encoding)
{
	pint_t startAddr = addr;
	const uint8_t* p = (uint8_t*)addr;
	pint_t result;
	
	// first get value
	switch (encoding & 0x0F) {
		case DW_EH_PE_ptr:
			result = getP(addr);
			p += sizeof(pint_t);
			addr = (pint_t)p;
			break;
		case DW_EH_PE_uleb128:
			result = getULEB128(addr, end);
			break;
		case DW_EH_PE_udata2:
			result = get16(addr);
			p += 2;
			addr = (pint_t)p;
			break;
		case DW_EH_PE_udata4:
			result = get32(addr);
			p += 4;
			addr = (pint_t)p;
			break;
		case DW_EH_PE_udata8:
			result = get64(addr);
			p += 8;
			addr = (pint_t)p;
			break;
		case DW_EH_PE_sleb128:
			result = getSLEB128(addr, end);
			break;
		case DW_EH_PE_sdata2:
			result = (int16_t)get16(addr);
			p += 2;
			addr = (pint_t)p;
			break;
		case DW_EH_PE_sdata4:
			result = (int32_t)get32(addr);
			p += 4;
			addr = (pint_t)p;
			break;
		case DW_EH_PE_sdata8:
			result = get64(addr);
			p += 8;
			addr = (pint_t)p;
			break;
		default:
			ABORT("unknown pointer encoding");
	}
	
	// then add relative offset
	switch ( encoding & 0x70 ) {
		case DW_EH_PE_absptr:
			// do nothing
			break;
		case DW_EH_PE_pcrel:
			result += startAddr;
			break;
		case DW_EH_PE_textrel:
			ABORT("DW_EH_PE_textrel pointer encoding not supported");
			break;
		case DW_EH_PE_datarel:
			ABORT("DW_EH_PE_datarel pointer encoding not supported");
			break;
		case DW_EH_PE_funcrel:
			ABORT("DW_EH_PE_funcrel pointer encoding not supported");
			break;
		case DW_EH_PE_aligned:
			ABORT("DW_EH_PE_aligned pointer encoding not supported");
			break;
		default:
			ABORT("unknown pointer encoding");
			break;
	}
	
	if ( encoding & DW_EH_PE_indirect )
		result = getP(result);
	
	return result;
}


inline bool LocalAddressSpace::findUnwindSections(pint_t addr, pint_t& mh, pint_t& dwarfStart, pint_t& dwarfLen, pint_t& compactStart)
{
	dyld_unwind_sections info;
	if ( _dyld_find_unwind_sections((void*)addr, &info) ) {
		mh				= (pint_t)info.mh;
		dwarfStart		= (pint_t)info.dwarf_section;
		dwarfLen		= (pint_t)info.dwarf_section_length;
		compactStart	= (pint_t)info.compact_unwind_section;
		return true;
	}
	return false;
}


inline bool	LocalAddressSpace::findFunctionName(pint_t addr, char* buf, size_t bufLen, unw_word_t* offset)
{
	dl_info dyldInfo;
	if ( dladdr((void*)addr, &dyldInfo) ) {
		if ( dyldInfo.dli_sname != NULL ) {
			strlcpy(buf, dyldInfo.dli_sname, bufLen);
			*offset = (addr - (pint_t)dyldInfo.dli_saddr);
			return true;
		}
	}
	return false;
}



#if UNW_REMOTE

///
/// OtherAddressSpace is used as a template parameter to UnwindCursor when unwinding a thread
/// in the another process.  The other process can be a different endianness and a different
/// pointer size and is handled by the P template parameter.  
///
template <typename P>
class OtherAddressSpace
{
public:
						OtherAddressSpace(task_t task) : fTask(task) {}

		typedef typename P::uint_t	pint_t;

		uint8_t			get8(pint_t addr);
		uint16_t		get16(pint_t addr);
		uint32_t		get32(pint_t addr);
		uint64_t		get64(pint_t addr);
		pint_t			getP(pint_t addr);
		uint64_t		getULEB128(pint_t& addr, pint_t end);
		int64_t			getSLEB128(pint_t& addr, pint_t end);
		pint_t			getEncodedP(pint_t& addr, pint_t end, uint8_t encoding);
		bool			findFunctionName(pint_t addr, char* buf, size_t bufLen, unw_word_t* offset);
		bool			findUnwindSections(pint_t addr, unwind_sections& info);
private:
		void*			localCopy(pint_t addr);


		task_t			fTask;
};


template <typename P>
uint8_t OtherAddressSpace<P>::get8(pint_t addr)
{
	return *((uint8_t*)localCopy(addr));
}

template <typename P>
uint16_t OtherAddressSpace<P>::get16(pint_t addr)
{
	return P::E::get16(*(uint16_t*)localCopy(addr));
}

template <typename P>
uint32_t OtherAddressSpace<P>::get32(pint_t addr)
{
	return P::E::get32(*(uint32_t*)localCopy(addr));
}

template <typename P>
uint64_t OtherAddressSpace<P>::get64(pint_t addr)
{
	return P::E::get64(*(uint64_t*)localCopy(addr));
}

template <typename P>
typename P::uint_t OtherAddressSpace<P>::getP(pint_t addr)
{
	return P::getP(*(uint64_t*)localCopy(addr));
}

template <typename P>
uint64_t OtherAddressSpace<P>::getULEB128(pint_t& addr, pint_t end)
{
	uintptr_t size = (end - addr);
	LocalAddressSpace::pint_t laddr = (LocalAddressSpace::pint_t)localCopy(addr);
	LocalAddressSpace::pint_t sladdr = laddr;
	uint64_t result = LocalAddressSpace::getULEB128(laddr, laddr+size);
	addr += (laddr-sladdr);
	return result;
}

template <typename P>
int64_t OtherAddressSpace<P>::getSLEB128(pint_t& addr, pint_t end)
{
	uintptr_t size = (end - addr);
	LocalAddressSpace::pint_t laddr = (LocalAddressSpace::pint_t)localCopy(addr);
	LocalAddressSpace::pint_t sladdr = laddr;
	uint64_t result = LocalAddressSpace::getSLEB128(laddr, laddr+size);
	addr += (laddr-sladdr);
	return result;
}

template <typename P>
void* OtherAddressSpace<P>::localCopy(pint_t addr)
{
	// FIX ME
}

template <typename P>
bool OtherAddressSpace<P>::findFunctionName(pint_t addr, char* buf, size_t bufLen, unw_word_t* offset)
{
	// FIX ME
}



///
/// unw_addr_space is the base class that abstract unw_addr_space_t type in libunwind.h points to.  
///
struct unw_addr_space
{
	cpu_type_t				cpuType;
	task_t					taskPort;
};


///
/// unw_addr_space_i386 is the concrete instance that a unw_addr_space_t points to when examining
/// a 32-bit intel process.
///
struct unw_addr_space_i386 : public unw_addr_space
{
													unw_addr_space_i386(task_t task) : oas(task) {}
	OtherAddressSpace<Pointer32<LittleEndian> >		oas;
};


///
/// unw_addr_space_x86_64 is the concrete instance that a unw_addr_space_t points to when examining
/// a 64-bit intel process.
///
struct unw_addr_space_x86_64 : public unw_addr_space
{
													unw_addr_space_x86_64(task_t task) : oas(task) {}
	OtherAddressSpace<Pointer64<LittleEndian> >		oas;
};


///
/// unw_addr_space_ppc is the concrete instance that a unw_addr_space_t points to when examining
/// a 32-bit PowerPC process.
///
struct unw_addr_space_ppc : public unw_addr_space
{
													unw_addr_space_ppc(task_t task) : oas(task) {}
	OtherAddressSpace<Pointer32<BigEndian> >		oas;
};


#endif // UNW_REMOTE


} // namespace libunwind 


#endif // __ADDRESSSPACE_HPP__




