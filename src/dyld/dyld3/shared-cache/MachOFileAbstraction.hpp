/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#ifndef __MACH_O_FILE_ABSTRACTION__
#define __MACH_O_FILE_ABSTRACTION__

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>
#include <mach/machine.h>

// suport older versions of mach-o/loader.h



#define DYLD_CACHE_ADJ_V2_FORMAT				0x7F

#define DYLD_CACHE_ADJ_V2_POINTER_32			0x01
#define DYLD_CACHE_ADJ_V2_POINTER_64			0x02
#define DYLD_CACHE_ADJ_V2_DELTA_32			    0x03
#define DYLD_CACHE_ADJ_V2_DELTA_64			    0x04
#define DYLD_CACHE_ADJ_V2_ARM64_ADRP			0x05
#define DYLD_CACHE_ADJ_V2_ARM64_OFF12			0x06
#define DYLD_CACHE_ADJ_V2_ARM64_BR26			0x07
#define DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT			0x08
#define DYLD_CACHE_ADJ_V2_ARM_BR24				0x09
#define DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT		0x0A
#define DYLD_CACHE_ADJ_V2_THUMB_BR22			0x0B
#define DYLD_CACHE_ADJ_V2_IMAGE_OFF_32			0x0C
#define DYLD_CACHE_ADJ_V2_THREADED_POINTER_64   0x0D

#include "FileAbstraction.hpp"
//#include "Architectures.hpp"

// utility to pair together a cpu-type and cpu-sub-type
struct ArchPair
{
	uint32_t	arch;
	uint32_t	subtype;
	
	ArchPair(uint32_t cputype, uint32_t cpusubtype) : arch(cputype), subtype(cpusubtype) {}
	
	bool operator<(const ArchPair& other) const { 
		if ( this->arch != other.arch )
			return (this->arch < other.arch);
		return (this->subtype < other.subtype);
	}

	bool operator==(const ArchPair& other) const { 
        return this->arch == other.arch  &&  this->subtype == other.subtype;
	}
};


//
// This abstraction layer makes every mach-o file look like a 64-bit mach-o file with native endianness
//

//
// mach-o load command
//
template <typename P>
class macho_load_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(command.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(command.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(command.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(command.cmdsize, value); }

	typedef typename P::E		E;
private:
	load_command	command;
};


//
// mach-o segment load command
//
template <typename P> struct macho_segment_content {};
template <> struct macho_segment_content<Pointer32<BigEndian> >    { segment_command	fields; enum { CMD = LC_SEGMENT		}; };
template <> struct macho_segment_content<Pointer64<BigEndian> >	   { segment_command_64	fields; enum { CMD = LC_SEGMENT_64	}; };
template <> struct macho_segment_content<Pointer32<LittleEndian> > { segment_command	fields; enum { CMD = LC_SEGMENT		}; };
template <> struct macho_segment_content<Pointer64<LittleEndian> > { segment_command_64	fields; enum { CMD = LC_SEGMENT_64	}; };

template <typename P>
class macho_segment_command {
public:
	uint32_t		cmd() const						INLINE { return E::get32(segment.fields.cmd); }
	void			set_cmd(uint32_t value)			INLINE { E::set32(segment.fields.cmd, value); }

	uint32_t		cmdsize() const					INLINE { return E::get32(segment.fields.cmdsize); }
	void			set_cmdsize(uint32_t value)		INLINE { E::set32(segment.fields.cmdsize, value); }

	const char*		segname() const					INLINE { return segment.fields.segname; }
	void			set_segname(const char* value)	INLINE { strncpy(segment.fields.segname, value, 16); }
	
	uint64_t		vmaddr() const					INLINE { return P::getP(segment.fields.vmaddr); }
	void			set_vmaddr(uint64_t value)		INLINE { P::setP(segment.fields.vmaddr, value); }

	uint64_t		vmsize() const					INLINE { return P::getP(segment.fields.vmsize); }
	void			set_vmsize(uint64_t value)		INLINE { P::setP(segment.fields.vmsize, value); }

	uint64_t		fileoff() const					INLINE { return P::getP(segment.fields.fileoff); }
	void			set_fileoff(uint64_t value)		INLINE { P::setP(segment.fields.fileoff, value); }

	uint64_t		filesize() const				INLINE { return P::getP(segment.fields.filesize); }
	void			set_filesize(uint64_t value)	INLINE { P::setP(segment.fields.filesize, value); }

	uint32_t		maxprot() const					INLINE { return E::get32(segment.fields.maxprot); }
	void			set_maxprot(uint32_t value)		INLINE { E::set32((uint32_t&)segment.fields.maxprot, value); }

	uint32_t		initprot() const				INLINE { return E::get32(segment.fields.initprot); }
	void			set_initprot(uint32_t value)	INLINE { E::set32((uint32_t&)segment.fields.initprot, value); }

	uint32_t		nsects() const					INLINE { return E::get32(segment.fields.nsects); }
	void			set_nsects(uint32_t value)		INLINE { E::set32(segment.fields.nsects, value); }

	uint32_t		flags() const					INLINE { return E::get32(segment.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(segment.fields.flags, value); }

	enum {
		CMD = macho_segment_content<P>::CMD
	};

	typedef typename P::E		E;
private:
	macho_segment_content<P>	segment;
};


//
// mach-o section 
//
template <typename P> struct macho_section_content {};
template <> struct macho_section_content<Pointer32<BigEndian> >    { section	fields; };
template <> struct macho_section_content<Pointer64<BigEndian> >	   { section_64	fields; };
template <> struct macho_section_content<Pointer32<LittleEndian> > { section	fields; };
template <> struct macho_section_content<Pointer64<LittleEndian> > { section_64	fields; };

template <typename P>
class macho_section {
public:
	const char*		sectname() const				INLINE { return section.fields.sectname; }
	void			set_sectname(const char* value)	INLINE { strncpy(section.fields.sectname, value, 16); }
	
	const char*		segname() const					INLINE { return section.fields.segname; }
	void			set_segname(const char* value)	INLINE { strncpy(section.fields.segname, value, 16); }
	
	uint64_t		addr() const					INLINE { return P::getP(section.fields.addr); }
	void			set_addr(uint64_t value)		INLINE { P::setP(section.fields.addr, value); }

	uint64_t		size() const					INLINE { return P::getP(section.fields.size); }
	void			set_size(uint64_t value)		INLINE { P::setP(section.fields.size, value); }

	uint32_t		offset() const					INLINE { return E::get32(section.fields.offset); }
	void			set_offset(uint32_t value)		INLINE { E::set32(section.fields.offset, value); }

	uint32_t		align() const					INLINE { return E::get32(section.fields.align); }
	void			set_align(uint32_t value)		INLINE { E::set32(section.fields.align, value); }

	uint32_t		reloff() const					INLINE { return E::get32(section.fields.reloff); }
	void			set_reloff(uint32_t value)		INLINE { E::set32(section.fields.reloff, value); }

	uint32_t		nreloc() const					INLINE { return E::get32(section.fields.nreloc); }
	void			set_nreloc(uint32_t value)		INLINE { E::set32(section.fields.nreloc, value); }

	uint32_t		flags() const					INLINE { return E::get32(section.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(section.fields.flags, value); }

	uint32_t		reserved1() const				INLINE { return E::get32(section.fields.reserved1); }
	void			set_reserved1(uint32_t value)	INLINE { E::set32(section.fields.reserved1, value); }

	uint32_t		reserved2() const				INLINE { return E::get32(section.fields.reserved2); }
	void			set_reserved2(uint32_t value)	INLINE { E::set32(section.fields.reserved2, value); }

	typedef typename P::E		E;
private:
	macho_section_content<P>	section;
};


//
// mach-o dylib load command
//
template <typename P>
class macho_dylib_command {
public:
	uint32_t		cmd() const									INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)						INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const								INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)					INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		name_offset() const							INLINE { return E::get32(fields.dylib.name.offset); }
	void			set_name_offset(uint32_t value)				INLINE { E::set32(fields.dylib.name.offset, value);  }
	
	uint32_t		timestamp() const							INLINE { return E::get32(fields.dylib.timestamp); }
	void			set_timestamp(uint32_t value)				INLINE { E::set32(fields.dylib.timestamp, value); }

	uint32_t		current_version() const						INLINE { return E::get32(fields.dylib.current_version); }
	void			set_current_version(uint32_t value)			INLINE { E::set32(fields.dylib.current_version, value); }

	uint32_t		compatibility_version() const				INLINE { return E::get32(fields.dylib.compatibility_version); }
	void			set_compatibility_version(uint32_t value)	INLINE { E::set32(fields.dylib.compatibility_version, value); }

	const char*		name() const								INLINE { return (const char*)&fields + name_offset(); }
	void			set_name_offset()							INLINE { set_name_offset(sizeof(fields)); }
	
	typedef typename P::E		E;
private:
	dylib_command	fields;
};


//
// mach-o dylinker load command
//
template <typename P>
class macho_dylinker_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		name_offset() const					INLINE { return E::get32(fields.name.offset); }
	void			set_name_offset(uint32_t value)		INLINE { E::set32(fields.name.offset, value);  }
	
	const char*		name() const						INLINE { return (const char*)&fields + name_offset(); }
	void			set_name_offset()					INLINE { set_name_offset(sizeof(fields)); }
	
	typedef typename P::E		E;
private:
	dylinker_command	fields;
};


//
// mach-o sub_framework load command
//
template <typename P>
class macho_sub_framework_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		umbrella_offset() const				INLINE { return E::get32(fields.umbrella.offset); }
	void			set_umbrella_offset(uint32_t value)	INLINE { E::set32(fields.umbrella.offset, value);  }
	
	const char*		umbrella() const					INLINE { return (const char*)&fields + umbrella_offset(); }
	void			set_umbrella_offset()				INLINE { set_umbrella_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_framework_command	fields;
};


//
// mach-o sub_client load command
//
template <typename P>
class macho_sub_client_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		client_offset() const				INLINE { return E::get32(fields.client.offset); }
	void			set_client_offset(uint32_t value)	INLINE { E::set32(fields.client.offset, value);  }
	
	const char*		client() const						INLINE { return (const char*)&fields + client_offset(); }
	void			set_client_offset()					INLINE { set_client_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_client_command	fields;
};


//
// mach-o sub_umbrella load command
//
template <typename P>
class macho_sub_umbrella_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		sub_umbrella_offset() const				INLINE { return E::get32(fields.sub_umbrella.offset); }
	void			set_sub_umbrella_offset(uint32_t value)	INLINE { E::set32(fields.sub_umbrella.offset, value);  }
	
	const char*		sub_umbrella() const					INLINE { return (const char*)&fields + sub_umbrella_offset(); }
	void			set_sub_umbrella_offset()				INLINE { set_sub_umbrella_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_umbrella_command	fields;
};


//
// mach-o sub_library load command
//
template <typename P>
class macho_sub_library_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		sub_library_offset() const				INLINE { return E::get32(fields.sub_library.offset); }
	void			set_sub_library_offset(uint32_t value)	INLINE { E::set32(fields.sub_library.offset, value);  }
	
	const char*		sub_library() const						INLINE { return (const char*)&fields + sub_library_offset(); }
	void			set_sub_library_offset()				INLINE { set_sub_library_offset(sizeof(fields)); }
		
	typedef typename P::E		E;
private:
	sub_library_command	fields;
};


//
// mach-o uuid load command
//
template <typename P>
class macho_uuid_command {
public:
	uint32_t		cmd() const								INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)					INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const							INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)				INLINE { E::set32(fields.cmdsize, value); }

	const uint8_t*	uuid() const							INLINE { return fields.uuid; }
	void			set_uuid(uint8_t value[16])				INLINE { memcpy(&fields.uuid, value, 16); }
			
	typedef typename P::E		E;
private:
	uuid_command	fields;
};


//
// mach-o routines load command
//
template <typename P> struct macho_routines_content {};
template <> struct macho_routines_content<Pointer32<BigEndian> >    { routines_command		fields; enum { CMD = LC_ROUTINES	}; };
template <> struct macho_routines_content<Pointer64<BigEndian> >	{ routines_command_64	fields; enum { CMD = LC_ROUTINES_64	}; };
template <> struct macho_routines_content<Pointer32<LittleEndian> > { routines_command		fields; enum { CMD = LC_ROUTINES	}; };
template <> struct macho_routines_content<Pointer64<LittleEndian> > { routines_command_64	fields; enum { CMD = LC_ROUTINES_64	}; };

template <typename P>
class macho_routines_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(routines.fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(routines.fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(routines.fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(routines.fields.cmdsize, value); }

	uint64_t		init_address() const				INLINE { return P::getP(routines.fields.init_address); }
	void			set_init_address(uint64_t value)	INLINE { P::setP(routines.fields.init_address, value); }

	uint64_t		init_module() const					INLINE { return P::getP(routines.fields.init_module); }
	void			set_init_module(uint64_t value)		INLINE { P::setP(routines.fields.init_module, value); }

	uint64_t		reserved1() const					INLINE { return P::getP(routines.fields.reserved1); }
	void			set_reserved1(uint64_t value)		INLINE { P::setP(routines.fields.reserved1, value); }
	
	uint64_t		reserved2() const					INLINE { return P::getP(routines.fields.reserved2); }
	void			set_reserved2(uint64_t value)		INLINE { P::setP(routines.fields.reserved2, value); }
	
	uint64_t		reserved3() const					INLINE { return P::getP(routines.fields.reserved3); }
	void			set_reserved3(uint64_t value)		INLINE { P::setP(routines.fields.reserved3, value); }
	
	uint64_t		reserved4() const					INLINE { return P::getP(routines.fields.reserved4); }
	void			set_reserved4(uint64_t value)		INLINE { P::setP(routines.fields.reserved4, value); }
	
	uint64_t		reserved5() const					INLINE { return P::getP(routines.fields.reserved5); }
	void			set_reserved5(uint64_t value)		INLINE { P::setP(routines.fields.reserved5, value); }
	
	uint64_t		reserved6() const					INLINE { return P::getP(routines.fields.reserved6); }
	void			set_reserved6(uint64_t value)		INLINE { P::setP(routines.fields.reserved6, value); }
	
	typedef typename P::E		E;
	enum {
		CMD = macho_routines_content<P>::CMD
	};
private:
	macho_routines_content<P>	routines;
};


//
// mach-o symbol table load command
//
template <typename P>
class macho_symtab_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		symoff() const				INLINE { return E::get32(fields.symoff); }
	void			set_symoff(uint32_t value)	INLINE { E::set32(fields.symoff, value);  }
	
	uint32_t		nsyms() const				INLINE { return E::get32(fields.nsyms); }
	void			set_nsyms(uint32_t value)	INLINE { E::set32(fields.nsyms, value);  }
	
	uint32_t		stroff() const				INLINE { return E::get32(fields.stroff); }
	void			set_stroff(uint32_t value)	INLINE { E::set32(fields.stroff, value);  }
	
	uint32_t		strsize() const				INLINE { return E::get32(fields.strsize); }
	void			set_strsize(uint32_t value)	INLINE { E::set32(fields.strsize, value);  }
	
	
	typedef typename P::E		E;
private:
	symtab_command	fields;
};


//
// mach-o dynamic symbol table load command
//
template <typename P>
class macho_dysymtab_command {
public:
	uint32_t		cmd() const							INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)				INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const						INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)			INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		ilocalsym() const					INLINE { return E::get32(fields.ilocalsym); }
	void			set_ilocalsym(uint32_t value)		INLINE { E::set32(fields.ilocalsym, value);  }
	
	uint32_t		nlocalsym() const					INLINE { return E::get32(fields.nlocalsym); }
	void			set_nlocalsym(uint32_t value)		INLINE { E::set32(fields.nlocalsym, value);  }
	
	uint32_t		iextdefsym() const					INLINE { return E::get32(fields.iextdefsym); }
	void			set_iextdefsym(uint32_t value)		INLINE { E::set32(fields.iextdefsym, value);  }
	
	uint32_t		nextdefsym() const					INLINE { return E::get32(fields.nextdefsym); }
	void			set_nextdefsym(uint32_t value)		INLINE { E::set32(fields.nextdefsym, value);  }
	
	uint32_t		iundefsym() const					INLINE { return E::get32(fields.iundefsym); }
	void			set_iundefsym(uint32_t value)		INLINE { E::set32(fields.iundefsym, value);  }
	
	uint32_t		nundefsym() const					INLINE { return E::get32(fields.nundefsym); }
	void			set_nundefsym(uint32_t value)		INLINE { E::set32(fields.nundefsym, value);  }
	
	uint32_t		tocoff() const						INLINE { return E::get32(fields.tocoff); }
	void			set_tocoff(uint32_t value)			INLINE { E::set32(fields.tocoff, value);  }
	
	uint32_t		ntoc() const						INLINE { return E::get32(fields.ntoc); }
	void			set_ntoc(uint32_t value)			INLINE { E::set32(fields.ntoc, value);  }
	
	uint32_t		modtaboff() const					INLINE { return E::get32(fields.modtaboff); }
	void			set_modtaboff(uint32_t value)		INLINE { E::set32(fields.modtaboff, value);  }
	
	uint32_t		nmodtab() const						INLINE { return E::get32(fields.nmodtab); }
	void			set_nmodtab(uint32_t value)			INLINE { E::set32(fields.nmodtab, value);  }
	
	uint32_t		extrefsymoff() const				INLINE { return E::get32(fields.extrefsymoff); }
	void			set_extrefsymoff(uint32_t value)	INLINE { E::set32(fields.extrefsymoff, value);  }
	
	uint32_t		nextrefsyms() const					INLINE { return E::get32(fields.nextrefsyms); }
	void			set_nextrefsyms(uint32_t value)		INLINE { E::set32(fields.nextrefsyms, value);  }
	
	uint32_t		indirectsymoff() const				INLINE { return E::get32(fields.indirectsymoff); }
	void			set_indirectsymoff(uint32_t value)	INLINE { E::set32(fields.indirectsymoff, value);  }
	
	uint32_t		nindirectsyms() const				INLINE { return E::get32(fields.nindirectsyms); }
	void			set_nindirectsyms(uint32_t value)	INLINE { E::set32(fields.nindirectsyms, value);  }
	
	uint32_t		extreloff() const					INLINE { return E::get32(fields.extreloff); }
	void			set_extreloff(uint32_t value)		INLINE { E::set32(fields.extreloff, value);  }
	
	uint32_t		nextrel() const						INLINE { return E::get32(fields.nextrel); }
	void			set_nextrel(uint32_t value)			INLINE { E::set32(fields.nextrel, value);  }
	
	uint32_t		locreloff() const					INLINE { return E::get32(fields.locreloff); }
	void			set_locreloff(uint32_t value)		INLINE { E::set32(fields.locreloff, value);  }
	
	uint32_t		nlocrel() const						INLINE { return E::get32(fields.nlocrel); }
	void			set_nlocrel(uint32_t value)			INLINE { E::set32(fields.nlocrel, value);  }
	
	typedef typename P::E		E;
private:
	dysymtab_command	fields;
};


//
// mach-o two-level hints load command
//
template <typename P>
class macho_twolevel_hints_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		offset() const				INLINE { return E::get32(fields.offset); }
	void			set_offset(uint32_t value)	INLINE { E::set32(fields.offset, value);  }
	
	uint32_t		nhints() const				INLINE { return E::get32(fields.nhints); }
	void			set_nhints(uint32_t value)	INLINE { E::set32(fields.nhints, value);  }
	
	typedef typename P::E		E;
private:
	twolevel_hints_command	fields;
};


//
// mach-o threads load command
//
template <typename P>
class macho_thread_command {
public:
	uint32_t		cmd() const											INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)								INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const										INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)							INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		flavor() const										INLINE { return E::get32(fields_flavor); }
	void			set_flavor(uint32_t value)							INLINE { E::set32(fields_flavor, value);  }
	
	uint32_t		count() const										INLINE { return E::get32(fields_count); }
	void			set_count(uint32_t value)							INLINE { E::set32(fields_count, value);  }
	
	uint64_t		thread_register(uint32_t index) const				INLINE { return P::getP(thread_registers[index]); }
	void			set_thread_register(uint32_t index, uint64_t value)	INLINE { P::setP(thread_registers[index], value); }
	
	typedef typename P::E		E;
	typedef typename P::uint_t	pint_t;
private:
	struct thread_command	fields;
	uint32_t				fields_flavor;
	uint32_t				fields_count;
	pint_t					thread_registers[1];
};


//
// mach-o misc data 
//
template <typename P>
class macho_linkedit_data_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		dataoff() const				INLINE { return E::get32(fields.dataoff); }
	void			set_dataoff(uint32_t value)	INLINE { E::set32(fields.dataoff, value);  }
	
	uint32_t		datasize() const			INLINE { return E::get32(fields.datasize); }
	void			set_datasize(uint32_t value)INLINE { E::set32(fields.datasize, value);  }
	
	
	typedef typename P::E		E;
private:
	linkedit_data_command	fields;
};


//
// mach-o symbol table entry 
//
template <typename P> struct macho_nlist_content {};
template <> struct macho_nlist_content<Pointer32<BigEndian> >    { struct nlist		fields; };
template <> struct macho_nlist_content<Pointer64<BigEndian> >	 { struct nlist_64	fields; };
template <> struct macho_nlist_content<Pointer32<LittleEndian> > { struct nlist		fields; };
template <> struct macho_nlist_content<Pointer64<LittleEndian> > { struct nlist_64	fields; };

template <typename P>
class macho_nlist {
public:
	uint32_t		n_strx() const					INLINE { return E::get32(entry.fields.n_un.n_strx); }
	void			set_n_strx(uint32_t value)		INLINE { E::set32((uint32_t&)entry.fields.n_un.n_strx, value); }

	uint8_t			n_type() const					INLINE { return entry.fields.n_type; }
	void			set_n_type(uint8_t value)		INLINE { entry.fields.n_type = value; }

	uint8_t			n_sect() const					INLINE { return entry.fields.n_sect; }
	void			set_n_sect(uint8_t value)		INLINE { entry.fields.n_sect = value; }

	uint16_t		n_desc() const					INLINE { return E::get16(entry.fields.n_desc); }
	void			set_n_desc(uint16_t value)		INLINE { E::set16((uint16_t&)entry.fields.n_desc, value); }

	uint64_t		n_value() const					INLINE { return P::getP(entry.fields.n_value); }
	void			set_n_value(uint64_t value)		INLINE { P::setP(entry.fields.n_value, value); }

	typedef typename P::E		E;
private:
	macho_nlist_content<P>	entry;
};



//
// mach-o relocation info
//
template <typename P>
class macho_relocation_info {
public:
	uint32_t		r_address() const				INLINE { return E::get32(address); }
	void			set_r_address(uint32_t value)	INLINE { E::set32(address, value); }

	uint32_t		r_symbolnum() const				INLINE { return E::getBits(other, 0, 24); }
	void			set_r_symbolnum(uint32_t value) INLINE { E::setBits(other, value, 0, 24); }

	bool			r_pcrel() const					INLINE { return E::getBits(other, 24, 1); }
	void			set_r_pcrel(bool value)			INLINE { E::setBits(other, value, 24, 1); }	
	
	uint8_t			r_length() const				INLINE { return E::getBits(other, 25, 2); }
	void			set_r_length(uint8_t value)		INLINE { E::setBits(other, value, 25, 2); }
	
	bool			r_extern() const				INLINE { return E::getBits(other, 27, 1); }
	void			set_r_extern(bool value)		INLINE { E::setBits(other, value, 27, 1); }
	
	uint8_t			r_type() const					INLINE { return E::getBits(other, 28, 4); }
	void			set_r_type(uint8_t value)		INLINE { E::setBits(other, value, 28, 4); }
		
	void			set_r_length()					INLINE { set_r_length((sizeof(typename P::uint_t)==8) ? 3 : 2); }

	typedef typename P::E		E;
private:
	uint32_t		address;
	uint32_t		other;
};


//
// mach-o scattered relocation info
// The bit fields are always in big-endian order (see mach-o/reloc.h)
//
template <typename P>
class macho_scattered_relocation_info {
public:
	bool			r_scattered() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 0, 1); }
	void			set_r_scattered(bool x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 0, 1);  E::set32(other, temp); }

	bool			r_pcrel() const				INLINE { return BigEndian::getBitsRaw(E::get32(other), 1, 1); }
	void			set_r_pcrel(bool x)			INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 1, 1);  E::set32(other, temp); }

	uint8_t			r_length() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 2, 2); }
	void			set_r_length(uint8_t x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 2, 2);  E::set32(other, temp); }

	uint8_t			r_type() const				INLINE { return BigEndian::getBitsRaw(E::get32(other), 4, 4); }
	void			set_r_type(uint8_t x)		INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 4, 4);  E::set32(other, temp); }

	uint32_t		r_address() const			INLINE { return BigEndian::getBitsRaw(E::get32(other), 8, 24); }
	void			set_r_address(uint32_t x)	INLINE { uint32_t temp = E::get32(other); BigEndian::setBitsRaw(temp, x, 8, 24);  E::set32(other, temp); }

	uint32_t		r_value() const				INLINE { return E::get32(value); }
	void			set_r_value(uint32_t x)		INLINE { E::set32(value, x); }

	uint32_t		r_other() const				INLINE { return other; }
	
	typedef typename P::E		E;
private:
	uint32_t		other;
	uint32_t		value;
};


//
// mach-o file header
//
template <typename P> struct macho_header_content {};
template <> struct macho_header_content<Pointer32<BigEndian> >    { mach_header		fields; };
template <> struct macho_header_content<Pointer64<BigEndian> >	  { mach_header_64	fields; };
template <> struct macho_header_content<Pointer32<LittleEndian> > { mach_header		fields; };
template <> struct macho_header_content<Pointer64<LittleEndian> > { mach_header_64	fields; };

template <typename P>
class macho_header {
public:
	uint32_t		magic() const					INLINE { return E::get32(header.fields.magic); }
	void			set_magic(uint32_t value)		INLINE { E::set32(header.fields.magic, value); }

	uint32_t		cputype() const					INLINE { return E::get32(header.fields.cputype); }
	void			set_cputype(uint32_t value)		INLINE { E::set32((uint32_t&)header.fields.cputype, value); }

	uint32_t		cpusubtype() const				INLINE { return E::get32(header.fields.cpusubtype); }
	void			set_cpusubtype(uint32_t value)	INLINE { E::set32((uint32_t&)header.fields.cpusubtype, value); }

	uint32_t		filetype() const				INLINE { return E::get32(header.fields.filetype); }
	void			set_filetype(uint32_t value)	INLINE { E::set32(header.fields.filetype, value); }

	uint32_t		ncmds() const					INLINE { return E::get32(header.fields.ncmds); }
	void			set_ncmds(uint32_t value)		INLINE { E::set32(header.fields.ncmds, value); }

	uint32_t		sizeofcmds() const				INLINE { return E::get32(header.fields.sizeofcmds); }
	void			set_sizeofcmds(uint32_t value)	INLINE { E::set32(header.fields.sizeofcmds, value); }

	uint32_t		flags() const					INLINE { return E::get32(header.fields.flags); }
	void			set_flags(uint32_t value)		INLINE { E::set32(header.fields.flags, value); }

	uint32_t		reserved() const				INLINE { return E::get32(header.fields.reserved); }
	void			set_reserved(uint32_t value)	INLINE { E::set32(header.fields.reserved, value); }

    const macho_segment_command<P>* getSegment(const char *segname) const
    {
        const macho_load_command<P>* cmds = (macho_load_command<P>*)((uint8_t*)this + sizeof(macho_header<P>));
        uint32_t cmd_count = this->ncmds();
        const macho_load_command<P>* cmd = cmds;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
                const macho_segment_command<P>* segcmd = (macho_segment_command<P>*)cmd;
                if (0 == strncmp(segname, segcmd->segname(), 16)) {
                    return segcmd;
              }
            }
            cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
        }
        return NULL;
    }

    const macho_section<P>* getSection(const char *segname, const char *sectname) const
    {
       const  macho_segment_command<P>* segcmd = getSegment(segname);
        if (!segcmd) return NULL;

        const macho_section<P>* sectcmd = (macho_section<P>*)(segcmd+1);
        uint32_t section_count = segcmd->nsects();
        for (uint32_t j = 0; j < section_count; ++j) {
            if (0 == ::strncmp(sectcmd[j].sectname(), sectname, 16)) {
                return sectcmd+j;
            }
        }

		if (strcmp(segname, "__DATA") == 0)
			return getSection("__DATA_CONST", sectname);
        return NULL;
    }

    const macho_load_command<P>* getLoadCommand(int query) const
    {
        const macho_load_command<P>* cmds = (macho_load_command<P>*)((uint8_t*)this + sizeof(macho_header<P>));
        uint32_t cmd_count = this->ncmds();
        const macho_load_command<P>* cmd = cmds;
        for (uint32_t i = 0; i < cmd_count; ++i) {
            if ( cmd->cmd() == query ) {
                return cmd;
            }
            cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
        }
        return NULL;
    }

	typedef typename P::E		E;
private:
	macho_header_content<P>	header;
};



//
// compressed dyld info load command
//
template <typename P>
class macho_dyld_info_command {
public:
	uint32_t		cmd() const					INLINE { return E::get32(fields.cmd); }
	void			set_cmd(uint32_t value)		INLINE { E::set32(fields.cmd, value); }

	uint32_t		cmdsize() const				INLINE { return E::get32(fields.cmdsize); }
	void			set_cmdsize(uint32_t value)	INLINE { E::set32(fields.cmdsize, value); }

	uint32_t		rebase_off() const				INLINE { return E::get32(fields.rebase_off); }
	void			set_rebase_off(uint32_t value)	INLINE { E::set32(fields.rebase_off, value);  }
	
	uint32_t		rebase_size() const				INLINE { return E::get32(fields.rebase_size); }
	void			set_rebase_size(uint32_t value)	INLINE { E::set32(fields.rebase_size, value);  }
	
	uint32_t		bind_off() const				INLINE { return E::get32(fields.bind_off); }
	void			set_bind_off(uint32_t value)	INLINE { E::set32(fields.bind_off, value);  }
	
	uint32_t		bind_size() const				INLINE { return E::get32(fields.bind_size); }
	void			set_bind_size(uint32_t value)	INLINE { E::set32(fields.bind_size, value);  }
	
	uint32_t		weak_bind_off() const				INLINE { return E::get32(fields.weak_bind_off); }
	void			set_weak_bind_off(uint32_t value)	INLINE { E::set32(fields.weak_bind_off, value);  }
	
	uint32_t		weak_bind_size() const				INLINE { return E::get32(fields.weak_bind_size); }
	void			set_weak_bind_size(uint32_t value)	INLINE { E::set32(fields.weak_bind_size, value);  }
	
	uint32_t		lazy_bind_off() const				INLINE { return E::get32(fields.lazy_bind_off); }
	void			set_lazy_bind_off(uint32_t value)	INLINE { E::set32(fields.lazy_bind_off, value);  }
	
	uint32_t		lazy_bind_size() const				INLINE { return E::get32(fields.lazy_bind_size); }
	void			set_lazy_bind_size(uint32_t value)	INLINE { E::set32(fields.lazy_bind_size, value);  }
	
	uint32_t		export_off() const				INLINE { return E::get32(fields.export_off); }
	void			set_export_off(uint32_t value)	INLINE { E::set32(fields.export_off, value);  }
	
	uint32_t		export_size() const				INLINE { return E::get32(fields.export_size); }
	void			set_export_size(uint32_t value)	INLINE { E::set32(fields.export_size, value);  }
	
	
	typedef typename P::E		E;
private:
	dyld_info_command	fields;
};

#ifndef NO_ULEB 
inline uint64_t read_uleb128(const uint8_t*& p, const uint8_t* end) {
	uint64_t result = 0;
	int		 bit = 0;
	do {
		if (p == end)
			throw "malformed uleb128 extends beyond trie";

		uint64_t slice = *p & 0x7f;

		if (bit >= 64 || slice << bit >> bit != slice)
			throw "uleb128 too big for 64-bits";
		else {
			result |= (slice << bit);
			bit += 7;
		}
	} 
	while (*p++ & 0x80);
	return result;
}
	

inline int64_t read_sleb128(const uint8_t*& p, const uint8_t* end)
{
	int64_t result = 0;
	int bit = 0;
	uint8_t byte;
	do {
		if (p == end)
			throw "malformed sleb128";
		byte = *p++;
		result |= (((int64_t)(byte & 0x7f)) << bit);
		bit += 7;
	} while (byte & 0x80);
	// sign extend negative numbers
	if ( (byte & 0x40) != 0 )
		result |= (~0ULL) << bit;
	return result;
}

#endif


#endif	// __MACH_O_FILE_ABSTRACTION__


