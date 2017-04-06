/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef __MACHO_REBASER__
#define __MACHO_REBASER__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/reloc.h>
#include <mach-o/x86_64/reloc.h>
#include <mach-o/arm/reloc.h>
#include <vector>
#include <set>

#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"
#include "MachOLayout.hpp"
#include "MachOTrie.hpp"



class AbstractRebaser
{
public:
	virtual cpu_type_t							getArchitecture() const = 0;
	virtual uint64_t							getBaseAddress() const = 0;
	virtual uint64_t							getVMSize() const = 0;
	virtual bool								rebase(std::vector<void*>&) = 0;
};


template <typename A>
class Rebaser : public AbstractRebaser
{
public:
												Rebaser(const MachOLayoutAbstraction&);
	virtual										~Rebaser() {}

	virtual cpu_type_t							getArchitecture() const;
	virtual uint64_t							getBaseAddress() const;
	virtual uint64_t							getVMSize() const;
	virtual bool								rebase(std::vector<void*>&);

protected:
	typedef typename A::P					P;
	typedef typename A::P::E				E;
	typedef typename A::P::uint_t			pint_t;
		
	pint_t*										mappedAddressForNewAddress(pint_t vmaddress);
	pint_t										getSlideForNewAddress(pint_t newAddress);

private:
	void										adjustLoadCommands();
	void										adjustSymbolTable();
	void										adjustDATA();
	void										adjustCode();
	void										applyRebaseInfo(std::vector<void*>& pointersInData);
	void										adjustReferencesUsingInfoV2(std::vector<void*>& pointersInData);
	void										adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
																uint64_t imageStartAddress, uint64_t imageEndAddress, std::vector<void*>& pointersInData);
	bool										adjustExportInfo();
	void										doRebase(int segIndex, uint64_t segOffset, uint8_t type, std::vector<void*>& pointersInData);
	pint_t										getSlideForVMAddress(pint_t vmaddress);
	pint_t										maskedVMAddress(pint_t vmaddress);
	pint_t*										mappedAddressForVMAddress(pint_t vmaddress);
	const uint8_t*								doCodeUpdateForEachULEB128Address(const uint8_t* p, uint8_t kind, uint64_t orgBaseAddress, int64_t codeToDataDelta, int64_t codeToImportDelta);
	void										doCodeUpdate(uint8_t kind, uint64_t address, int64_t codeToDataDelta, int64_t codeToImportDelta);
	void										doLocalRelocation(const macho_relocation_info<P>* reloc);
	bool										unequalSlides() const;

protected:	
	const macho_header<P>*						fHeader; 
	uint8_t*									fLinkEditBase;				// add file offset to this to get linkedit content
	const MachOLayoutAbstraction&				fLayout;
private:
	const macho_symtab_command<P>*				fSymbolTable;
	const macho_dysymtab_command<P>*			fDynamicSymbolTable;
	const macho_dyld_info_command<P>*			fDyldInfo;
	const macho_linkedit_data_command<P>*		fSplitSegInfo;
	bool										fSplittingSegments;
	bool										fHasSplitSegInfoV2;
	std::vector<uint64_t>						fSectionOffsetsInSegment;
};


template <typename A>
Rebaser<A>::Rebaser(const MachOLayoutAbstraction& layout)
 : 	fLayout(layout), fLinkEditBase(0), fSymbolTable(NULL), fDynamicSymbolTable(NULL), 
    fDyldInfo(NULL), fSplitSegInfo(NULL), fSplittingSegments(false), fHasSplitSegInfoV2(false)
{
	fHeader = (const macho_header<P>*)fLayout.getSegments()[0].mappedAddress();
	switch ( fHeader->filetype() ) {
		case MH_DYLIB:
		case MH_BUNDLE:
			break;
		default:
			throw "file is not a dylib or bundle";
	}
	
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( strcmp(seg.name(), "__LINKEDIT") == 0 ) {
			fLinkEditBase = (uint8_t*)seg.mappedAddress() - seg.fileOffset();
			break;
		}
	}
	if ( fLinkEditBase == NULL )	
		throw "no __LINKEDIT segment";
		
	// get symbol table info
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)fHeader + sizeof(macho_header<P>));
	const uint32_t cmd_count = fHeader->ncmds();
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch (cmd->cmd()) {
			case LC_SYMTAB:
				fSymbolTable = (macho_symtab_command<P>*)cmd;
				break;
			case LC_DYSYMTAB:
				fDynamicSymbolTable = (macho_dysymtab_command<P>*)cmd;
				break;
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				fDyldInfo = (macho_dyld_info_command<P>*)cmd;
				break;
			case LC_SEGMENT_SPLIT_INFO:
				fSplitSegInfo = (macho_linkedit_data_command<P>*)cmd;
				break;
			case macho_segment_command<P>::CMD: {
				// update segment/section file offsets
				macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
				macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
				macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
				for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
					fSectionOffsetsInSegment.push_back(sect->addr() - segCmd->vmaddr());
				}
			}
		}
		cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	}	

	if ( fDyldInfo == NULL )
		throw "no LC_DYLD_INFO load command";
	
	fSplittingSegments = layout.hasSplitSegInfo() && this->unequalSlides();

	if ( fSplitSegInfo != NULL ) {
		const uint8_t* infoStart = &fLinkEditBase[fSplitSegInfo->dataoff()];
		fHasSplitSegInfoV2 = ( *infoStart == DYLD_CACHE_ADJ_V2_FORMAT );
	}
}

template <> cpu_type_t Rebaser<x86>::getArchitecture()    const { return CPU_TYPE_I386; }
template <> cpu_type_t Rebaser<x86_64>::getArchitecture() const { return CPU_TYPE_X86_64; }
template <> cpu_type_t Rebaser<arm>::getArchitecture() const { return CPU_TYPE_ARM; }
template <> cpu_type_t Rebaser<arm64>::getArchitecture() const { return CPU_TYPE_ARM64; }

template <typename A>
bool Rebaser<A>::unequalSlides() const
{
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	uint64_t slide = segments[0].newAddress() - segments[0].address();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( (seg.newAddress() - seg.address()) != slide )
			return true;
	}
	return false;
}

template <typename A>
uint64_t Rebaser<A>::getBaseAddress() const
{
	return fLayout.getSegments()[0].address();
}

template <typename A>
uint64_t Rebaser<A>::getVMSize() const
{
	uint64_t highestVMAddress = 0;
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( seg.address() > highestVMAddress )
			highestVMAddress = seg.address();
	}
	return (((highestVMAddress - getBaseAddress()) + 4095) & (-4096));
}



template <typename A>
bool Rebaser<A>::rebase(std::vector<void*>& pointersInData)
{
	if ( fHasSplitSegInfoV2 )  {
		this->adjustReferencesUsingInfoV2(pointersInData);
	}
	else {
		//fprintf(stderr, "warning: dylib with old split-seg info: %s\n", fLayout.getFilePath());
		// update writable segments that have internal pointers
		this->applyRebaseInfo(pointersInData);

		// if splitting segments, update code-to-data references
		this->adjustCode();
	}

	// update load commands
	this->adjustLoadCommands();

	// update symbol table  
	this->adjustSymbolTable();

	// update export info
	return this->adjustExportInfo();
}

template <typename A>
void Rebaser<A>::adjustLoadCommands()
{
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)fHeader + sizeof(macho_header<P>));
	const uint32_t cmd_count = fHeader->ncmds();
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd() ) {
			case LC_ID_DYLIB:
				if ( (fHeader->flags() & MH_PREBOUND) != 0 ) {
					// clear timestamp so that any prebound clients are invalidated
					macho_dylib_command<P>* dylib  = (macho_dylib_command<P>*)cmd;
					dylib->set_timestamp(1);
				}
				break;
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				if ( (fHeader->flags() & MH_PREBOUND) != 0 ) {
					// clear expected timestamps so that this image will load with invalid prebinding 
					macho_dylib_command<P>* dylib  = (macho_dylib_command<P>*)cmd;
					dylib->set_timestamp(2);
				}
				break;
			case macho_routines_command<P>::CMD:
				// update -init command
				{
					struct macho_routines_command<P>* routines = (struct macho_routines_command<P>*)cmd;
					routines->set_init_address(routines->init_address() + this->getSlideForVMAddress(routines->init_address()));
				}
				break;
			case macho_segment_command<P>::CMD:
				// update segment commands
				{
					macho_segment_command<P>* seg = (macho_segment_command<P>*)cmd;
					pint_t slide = this->getSlideForVMAddress(seg->vmaddr());
					seg->set_vmaddr(seg->vmaddr() + slide);
					macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)seg + sizeof(macho_segment_command<P>));
					macho_section<P>* const sectionsEnd = &sectionsStart[seg->nsects()];
					for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
						sect->set_addr(sect->addr() + slide);
					}
				}
				break;
		}
		cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	}
}

template <>
uint64_t Rebaser<arm64>::maskedVMAddress(pint_t vmaddress)
{
	return (vmaddress & 0x0FFFFFFFFFFFFFFF);
}

template <typename A>
typename A::P::uint_t Rebaser<A>::maskedVMAddress(pint_t vmaddress)
{
	return vmaddress;
}


template <typename A>
typename A::P::uint_t Rebaser<A>::getSlideForVMAddress(pint_t vmaddress)
{
	pint_t vmaddr = this->maskedVMAddress(vmaddress);
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( (seg.address() <= vmaddr) && (seg.size() != 0) && ((vmaddr < (seg.address()+seg.size())) || (seg.address() == vmaddr)) ) {
			return seg.newAddress() - seg.address();
		}
	}
	throwf("vm address 0x%08llX not found", (uint64_t)vmaddr);
}


template <typename A>
typename A::P::uint_t* Rebaser<A>::mappedAddressForVMAddress(pint_t vmaddress)
{
	pint_t vmaddr = this->maskedVMAddress(vmaddress);
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( (seg.address() <= vmaddr) && (vmaddr < (seg.address()+seg.size())) ) {
			return (pint_t*)((vmaddr - seg.address()) + (uint8_t*)seg.mappedAddress());
		}
	}
	throwf("mappedAddressForVMAddress(0x%08llX) not found", (uint64_t)vmaddr);
}

template <typename A>
typename A::P::uint_t* Rebaser<A>::mappedAddressForNewAddress(pint_t vmaddress)
{
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( (seg.newAddress() <= vmaddress) && (vmaddress < (seg.newAddress()+seg.size())) ) {
			return (pint_t*)((vmaddress - seg.newAddress()) + (uint8_t*)seg.mappedAddress());
		}
	}
	throwf("mappedAddressForNewAddress(0x%08llX) not found", (uint64_t)vmaddress);
}

template <typename A>
typename A::P::uint_t Rebaser<A>::getSlideForNewAddress(pint_t newAddress)
{
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
		const MachOLayoutAbstraction::Segment& seg = *it;
		if ( (seg.newAddress() <= newAddress) && (newAddress < (seg.newAddress()+seg.size())) ) {
			return seg.newAddress() - seg.address();
		}
	}
	throwf("new address 0x%08llX not found", (uint64_t)newAddress);
}

template <typename A>
void Rebaser<A>::adjustSymbolTable()
{
	macho_nlist<P>* symbolTable = (macho_nlist<P>*)(&fLinkEditBase[fSymbolTable->symoff()]);

	// walk all exports and slide their n_value
	macho_nlist<P>* lastExport = &symbolTable[fDynamicSymbolTable->iextdefsym()+fDynamicSymbolTable->nextdefsym()];
	for (macho_nlist<P>* entry = &symbolTable[fDynamicSymbolTable->iextdefsym()]; entry < lastExport; ++entry) {
		if ( (entry->n_type() & N_TYPE) == N_SECT )
			entry->set_n_value(entry->n_value() + this->getSlideForVMAddress(entry->n_value()));
	}

	// walk all local symbols and slide their n_value (don't adjust any stabs)
	macho_nlist<P>*  lastLocal = &symbolTable[fDynamicSymbolTable->ilocalsym()+fDynamicSymbolTable->nlocalsym()];
	for (macho_nlist<P>* entry = &symbolTable[fDynamicSymbolTable->ilocalsym()]; entry < lastLocal; ++entry) {
		if ( (entry->n_sect() != NO_SECT) && ((entry->n_type() & N_STAB) == 0) )
			entry->set_n_value(entry->n_value() + this->getSlideForVMAddress(entry->n_value()));
	}
}

template <typename A>
bool Rebaser<A>::adjustExportInfo()
{
	// if no export info, nothing to adjust
	if ( fDyldInfo->export_size() == 0 )
		return true;

	// since export info addresses are offsets from mach_header, everything in __TEXT is fine
	// only __DATA addresses need to be updated
	const uint8_t* start = fLayout.getDyldInfoExports();
	const uint8_t* end = &start[fDyldInfo->export_size()];
	std::vector<mach_o::trie::Entry> originalExports;
	try {
		parseTrie(start, end, originalExports);
	}
	catch (const char* msg) {
		throwf("%s in %s", msg, fLayout.getFilePath());
	}
	
	std::vector<mach_o::trie::Entry> newExports;
	newExports.reserve(originalExports.size());
	pint_t baseAddress = this->getBaseAddress();
	pint_t baseAddressSlide = this->getSlideForVMAddress(baseAddress);
	for (std::vector<mach_o::trie::Entry>::iterator it=originalExports.begin(); it != originalExports.end(); ++it) {
		// remove symbols used by the static linker only
		if (	   (strncmp(it->name, "$ld$", 4) == 0) 
				|| (strncmp(it->name, ".objc_class_name",16) == 0) 
				|| (strncmp(it->name, ".objc_category_name",19) == 0) ) {
			//fprintf(stderr, "ignoring symbol %s\n", it->name);
			continue;
		}
		// adjust symbols in slid segments
		//uint32_t oldOffset = it->address;
		it->address += (this->getSlideForVMAddress(it->address + baseAddress) - baseAddressSlide);
		//fprintf(stderr, "orig=0x%08X, new=0x%08llX, sym=%s\n", oldOffset, it->address, it->name);
		newExports.push_back(*it);
	}
	
	// rebuild export trie
	std::vector<uint8_t> newExportTrieBytes;
	newExportTrieBytes.reserve(fDyldInfo->export_size());
	mach_o::trie::makeTrie(newExports, newExportTrieBytes);
	// align
	while ( (newExportTrieBytes.size() % sizeof(pint_t)) != 0 )
		newExportTrieBytes.push_back(0);
	
	uint32_t newExportsSize = newExportTrieBytes.size();
	if ( newExportsSize <= fDyldInfo->export_size() ) {
		// override existing trie in place
		uint8_t *realStart = &fLinkEditBase[fDyldInfo->export_off()];
		bzero(realStart, fDyldInfo->export_size());
		memcpy(realStart, &newExportTrieBytes[0], newExportsSize);
		fLayout.setDyldInfoExports(realStart);
		return true;
	}
	else {
		// allocate new buffer and set export_off in layout object to use new buffer instead
		uint8_t* sideTrie = new uint8_t[newExportsSize];
		memcpy(sideTrie, &newExportTrieBytes[0], newExportsSize);
		fLayout.setDyldInfoExports(sideTrie);
		((macho_dyld_info_command<P>*)fDyldInfo)->set_export_off(0); // invalidate old trie
		((macho_dyld_info_command<P>*)fDyldInfo)->set_export_size(newExportsSize);
		return false;
	}
}



template <typename A>
void Rebaser<A>::doCodeUpdate(uint8_t kind, uint64_t address, int64_t codeToDataDelta, int64_t codeToImportDelta)
{
	//fprintf(stderr, "doCodeUpdate(kind=%d, address=0x%0llX, dataDelta=0x%08llX, importDelta=0x%08llX, path=%s)\n", 
	//				kind, address, codeToDataDelta, codeToImportDelta, fLayout.getFilePath());
	uint32_t* p;
	uint32_t instruction;
	uint32_t value;
	uint64_t value64;
	switch (kind) {
		case 1:	// 32-bit pointer
			p = (uint32_t*)mappedAddressForVMAddress(address);
			value = A::P::E::get32(*p);
			value += codeToDataDelta;
			 A::P::E::set32(*p, value);
			break;
		case 2: // 64-bit pointer
			p = (uint32_t*)mappedAddressForVMAddress(address);
			value64 =  A::P::E::get64(*(uint64_t*)p);
			value64 += codeToDataDelta;
			 A::P::E::set64(*(uint64_t*)p, value64);
			break;
		case 4:	// only used for i386, a reference to something in the IMPORT segment
			p = (uint32_t*)mappedAddressForVMAddress(address);
			value = A::P::E::get32(*p);
			value += codeToImportDelta;
			 A::P::E::set32(*p, value);
			break;			
        case 5: // used by thumb2 movw
			p = (uint32_t*)mappedAddressForVMAddress(address);
			instruction = A::P::E::get32(*p);
			// codeToDataDelta is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
			value = (instruction & 0x0000000F) + (codeToDataDelta >> 12);
			instruction = (instruction & 0xFFFFFFF0) | (value & 0x0000000F);
			A::P::E::set32(*p, instruction);
			break;
        case 6: // used by ARM movw
			p = (uint32_t*)mappedAddressForVMAddress(address);
			instruction = A::P::E::get32(*p);
			// codeToDataDelta is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
			value = ((instruction & 0x000F0000) >> 16) + (codeToDataDelta >> 12);
			instruction = (instruction & 0xFFF0FFFF) | ((value <<16) & 0x000F0000);
			A::P::E::set32(*p, instruction);
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
			// used by thumb2 movt (low nibble of kind is high 4-bits of paired movw)
			{
				p = (uint32_t*)mappedAddressForVMAddress(address);
				instruction = A::P::E::get32(*p);
				// extract 16-bit value from instruction
				uint32_t i =    ((instruction & 0x00000400) >> 10);
				uint32_t imm4 =  (instruction & 0x0000000F);
				uint32_t imm3 = ((instruction & 0x70000000) >> 28);
				uint32_t imm8 = ((instruction & 0x00FF0000) >> 16);
				uint32_t imm16 = (imm4 << 12) | (i << 11) | (imm3 << 8) | imm8;
				// combine with codeToDataDelta and kind nibble
				uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
				uint32_t newTargetValue = targetValue + codeToDataDelta;
				// construct new bits slices
				uint32_t imm4_	= (newTargetValue & 0xF0000000) >> 28;
				uint32_t i_		= (newTargetValue & 0x08000000) >> 27;
				uint32_t imm3_	= (newTargetValue & 0x07000000) >> 24;
				uint32_t imm8_	= (newTargetValue & 0x00FF0000) >> 16;
				// update instruction to match codeToDataDelta 
				uint32_t newInstruction = (instruction & 0x8F00FBF0) | imm4_ | (i_ << 10) | (imm3_ << 28) | (imm8_ << 16);
				A::P::E::set32(*p, newInstruction);
			}
			break;
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2D:
		case 0x2E:
		case 0x2F:
			// used by arm movt (low nibble of kind is high 4-bits of paired movw)
			{
				p = (uint32_t*)mappedAddressForVMAddress(address);
				instruction = A::P::E::get32(*p);
				// extract 16-bit value from instruction
				uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
				uint32_t imm12 = (instruction & 0x00000FFF);
				uint32_t imm16 = (imm4 << 12) | imm12;
				// combine with codeToDataDelta and kind nibble
				uint32_t targetValue = (imm16 << 16) | ((kind & 0xF) << 12);
				uint32_t newTargetValue = targetValue + codeToDataDelta;
				// construct new bits slices
				uint32_t imm4_  = (newTargetValue & 0xF0000000) >> 28;
				uint32_t imm12_ = (newTargetValue & 0x0FFF0000) >> 16;
				// update instruction to match codeToDataDelta 
				uint32_t newInstruction = (instruction & 0xFFF0F000) | (imm4_ << 16) | imm12_;
				A::P::E::set32(*p, newInstruction);
			}
			break;
		case 3: // used for arm64 ADRP
			p = (uint32_t*)mappedAddressForVMAddress(address);
			instruction = A::P::E::get32(*p);
			if ( (instruction & 0x9F000000) == 0x90000000 ) {
				// codeToDataDelta is always a multiple of 4096, so only top 4 bits of lo16 will ever need adjusting
				value64 = ((instruction & 0x60000000) >> 17) | ((instruction & 0x00FFFFE0) << 9);
				value64 += codeToDataDelta;
				instruction = (instruction & 0x9F00001F) | ((value64 << 17) & 0x60000000) | ((value64 >> 9) & 0x00FFFFE0);
				A::P::E::set32(*p, instruction);
			}
			break;
		default:
			throwf("invalid kind=%d in split seg info", kind);
	}
}

template <typename A>
const uint8_t* Rebaser<A>::doCodeUpdateForEachULEB128Address(const uint8_t* p, uint8_t kind, uint64_t orgBaseAddress, int64_t codeToDataDelta, int64_t codeToImportDelta)
{
	uint64_t address = 0;
	uint64_t delta = 0;
	uint32_t shift = 0;
	bool more = true;
	do {
		uint8_t byte = *p++;
		delta |= ((byte & 0x7F) << shift);
		shift += 7;
		if ( byte < 0x80 ) {
			if ( delta != 0 ) {
				address += delta;
				doCodeUpdate(kind, address+orgBaseAddress, codeToDataDelta, codeToImportDelta);
				delta = 0;
				shift = 0;
			}
			else {
				more = false;
			}
		}
	} while (more);
	return p;
}

template <typename A>
void Rebaser<A>::adjustCode()
{
	if ( fSplittingSegments ) {
		// get uleb128 compressed runs of code addresses to update
		const uint8_t* infoStart = &fLinkEditBase[fSplitSegInfo->dataoff()];
		const uint8_t* infoEnd = &infoStart[fSplitSegInfo->datasize()];;
		// calculate how much we need to slide writable segments
		const uint64_t orgBaseAddress = this->getBaseAddress();
		int64_t codeToDataDelta = 0;
		int64_t codeToImportDelta = 0;
		const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
		const MachOLayoutAbstraction::Segment& codeSeg = segments[0];
		for(std::vector<MachOLayoutAbstraction::Segment>::const_iterator it = segments.begin(); it != segments.end(); ++it) {
			const MachOLayoutAbstraction::Segment& dataSeg = *it;
			if ( dataSeg.writable() ) {
				if ( (strcmp(dataSeg.name(), "__DATA") != 0) && (strcmp(dataSeg.name(), "__OBJC") != 0) )
					throwf("only one rw segment named '__DATA' can be used in dylibs placed in the dyld shared cache (%s)", fLayout.getFilePath());
				codeToDataDelta = (dataSeg.newAddress() - codeSeg.newAddress()) - (dataSeg.address() - codeSeg.address());
			}
		}
		// decompress and call doCodeUpdate() on each address
		for(const uint8_t* p = infoStart; (*p != 0) && (p < infoEnd);) {
			uint8_t kind = *p++;
			p = this->doCodeUpdateForEachULEB128Address(p, kind, orgBaseAddress, codeToDataDelta, codeToImportDelta);
		}
	}
}

template <typename A>
void Rebaser<A>::doRebase(int segIndex, uint64_t segOffset, uint8_t type, std::vector<void*>& pointersInData)
{
	const std::vector<MachOLayoutAbstraction::Segment>& segments = fLayout.getSegments();
	if ( segIndex > segments.size() )
		throw "bad segment index in rebase info";
	const MachOLayoutAbstraction::Segment& seg = segments[segIndex];
	uint8_t*  mappedAddr = (uint8_t*)seg.mappedAddress() + segOffset;
	pint_t*   mappedAddrP = (pint_t*)mappedAddr;
	uint32_t* mappedAddr32 = (uint32_t*)mappedAddr;
	pint_t valueP;
	pint_t valuePnew;
	uint32_t value32;
	int32_t svalue32;
	int32_t svalue32new;
	switch ( type ) {
		case REBASE_TYPE_POINTER:
			valueP= P::getP(*mappedAddrP);
			try {
				P::setP(*mappedAddrP, valueP + this->getSlideForVMAddress(valueP));
			}
			catch (const char* msg) {
				throwf("at offset=0x%08llX in seg=%s, pointer cannot be rebased because it does not point to __TEXT or __DATA. %s\n", 
						segOffset, seg.name(), msg);
			}
			break;
		
		case REBASE_TYPE_TEXT_ABSOLUTE32:
			value32 = E::get32(*mappedAddr32);
			E::set32(*mappedAddr32, value32 + this->getSlideForVMAddress(value32));
			break;
			
		case REBASE_TYPE_TEXT_PCREL32:
			svalue32 = E::get32(*mappedAddr32);
			valueP = seg.address() + segOffset + 4 + svalue32;
			valuePnew = valueP + this->getSlideForVMAddress(valueP);
			svalue32new = seg.address() + segOffset + 4 - valuePnew;
			E::set32(*mappedAddr32, svalue32new);
			break;
		
		default:
			throw "bad rebase type";
	}
	pointersInData.push_back(mappedAddr);
}


template <typename A>
void Rebaser<A>::applyRebaseInfo(std::vector<void*>& pointersInData)
{
	const uint8_t* p = &fLinkEditBase[fDyldInfo->rebase_off()];
	const uint8_t* end = &p[fDyldInfo->rebase_size()];
	
	uint8_t type = 0;
	int segIndex;
	uint64_t segOffset = 0;
	uint32_t count;
	uint32_t skip;
	bool done = false;
	while ( !done && (p < end) ) {
		uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
		uint8_t opcode = *p & REBASE_OPCODE_MASK;
		++p;
		switch (opcode) {
			case REBASE_OPCODE_DONE:
				done = true;
				break;
			case REBASE_OPCODE_SET_TYPE_IMM:
				type = immediate;
				break;
			case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
				segIndex = immediate;
				segOffset = read_uleb128(p, end);
				break;
			case REBASE_OPCODE_ADD_ADDR_ULEB:
				segOffset += read_uleb128(p, end);
				break;
			case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
				segOffset += immediate*sizeof(pint_t);
				break;
			case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
				for (int i=0; i < immediate; ++i) {
					doRebase(segIndex, segOffset, type, pointersInData);
					segOffset += sizeof(pint_t);
				}
				break;
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
				count = read_uleb128(p, end);
				for (uint32_t i=0; i < count; ++i) {
					doRebase(segIndex, segOffset, type, pointersInData);
					segOffset += sizeof(pint_t);
				}
				break;
			case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
				doRebase(segIndex, segOffset, type, pointersInData);
				segOffset += read_uleb128(p, end) + sizeof(pint_t);
				break;
			case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
				count = read_uleb128(p, end);
				skip = read_uleb128(p, end);
				for (uint32_t i=0; i < count; ++i) {
					doRebase(segIndex, segOffset, type, pointersInData);
					segOffset += skip + sizeof(pint_t);
				}
				break;
			default:
				throwf("bad rebase opcode %d", *p);
		}
	}	
}

template <>
void Rebaser<arm64>::adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
									uint64_t imageStartAddress, uint64_t imageEndAddress, std::vector<void*>& pointersInData)
{
	uint64_t value64;
	uint64_t* mappedAddr64;
	uint32_t value32;
	uint32_t* mappedAddr32;
	uint32_t instruction;
	int64_t offsetAdjust;
	switch ( kind ) {
		case DYLD_CACHE_ADJ_V2_DELTA_32:
			mappedAddr32 = (uint32_t*)mappedAddr;
			value32 = arm64::P::E::get32(*mappedAddr32);
			E::set32(*mappedAddr32, value32 + adjust);
			break;
		case DYLD_CACHE_ADJ_V2_POINTER_64:
			mappedAddr64 = (uint64_t*)mappedAddr;
			if ( toNewAddress != (E::get64(*mappedAddr64) + targetSlide) )
				throwf("bad DYLD_CACHE_ADJ_V2_POINTER_64 value not as expected at address 0x%llX\n", fromNewAddress);
			E::set64(*mappedAddr64, toNewAddress);
			pointersInData.push_back(mappedAddr);
			break;
		case DYLD_CACHE_ADJ_V2_DELTA_64:
			mappedAddr64 = (uint64_t*)mappedAddr;
			value64 = arm64::P::E::get64(*mappedAddr64);
			E::set64(*mappedAddr64, value64 + adjust);
			break;
		case DYLD_CACHE_ADJ_V2_IMAGE_OFF_32:
			mappedAddr32 = (uint32_t*)mappedAddr;
			value64 = toNewAddress - imageStartAddress;
			E::set32(*mappedAddr32, (uint32_t)value64);
			break;
		case DYLD_CACHE_ADJ_V2_ARM64_ADRP:
			mappedAddr32 = (uint32_t*)mappedAddr;
			instruction = arm64::P::E::get32(*mappedAddr32);
			if ( (instruction & 0x9F000000) == 0x90000000 ) {
				//value64 = ((instruction & 0x60000000) >> 17) | ((instruction & 0x00FFFFE0) << 9);
				uint32_t newPage21 = ((toNewAddress & ~0xFFF) - (fromNewAddress & ~0xFFF)) >> 12;
				instruction = (instruction & 0x9F00001F) | ((newPage21 << 29) & 0x60000000) | ((newPage21 << 3) & 0x00FFFFE0);
				arm64::P::E::set32(*mappedAddr32, instruction);
			}
			else {
				// ADRP instructions are sometimes optimized to other instructions (e.g. ADR) after the split-seg-info is generated
			}
			break;
		case DYLD_CACHE_ADJ_V2_ARM64_OFF12:
			mappedAddr32 = (uint32_t*)mappedAddr;
			instruction = arm64::P::E::get32(*mappedAddr32);
			offsetAdjust = (adjust & 0xFFF);
			if ( offsetAdjust == 0 )
				break;
			if ( (instruction & 0x3B000000) == 0x39000000 ) {
				// LDR/STR imm12
				if ( offsetAdjust != 0 ) {
					uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
					uint32_t newAddend;
					switch ( instruction & 0xC0000000 ) {
						case 0x00000000:
							if ( (instruction & 0x04800000) == 0x04800000 ) {
								if ( offsetAdjust & 0xF )
									throwf("can't adjust off12 scale=16 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
								if ( encodedAddend*16 >= 4096 )
									throwf("off12 scale=16 instruction points outside its page at mapped address=%p", mappedAddr);
								newAddend = (encodedAddend + offsetAdjust/16) % 256;
							}
							else {
								// scale=1
								newAddend = encodedAddend + offsetAdjust;
							}
							break;
						case 0x40000000:
							if ( offsetAdjust & 1 )
								throwf("can't adjust off12 scale=2 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
							if ( encodedAddend*2 >= 4096 )
								throwf("off12 scale=2 instruction points outside its page at mapped address=%p", mappedAddr);
							newAddend = (encodedAddend + offsetAdjust/2) % 2048;
							break;
						case 0x80000000:
							if ( offsetAdjust & 3 )
								throwf("can't adjust off12 scale=4 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
							if ( encodedAddend*4 >= 4096 )
								throwf("off12 scale=4 instruction points outside its page at mapped address=%p", mappedAddr);
							newAddend = (encodedAddend + offsetAdjust/4) % 1024;
							break;
						case 0xC0000000:
							if ( offsetAdjust & 7 )
								throwf("can't adjust off12 scale=8 instruction by %lld bytes at mapped address=%p", offsetAdjust, mappedAddr);
							if ( encodedAddend*8 >= 4096 )
								throwf("off12 scale=8 instruction points outside its page at mapped address=%p", mappedAddr);
							newAddend = (encodedAddend + offsetAdjust/8) % 512;
							break;
					}
					uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
					arm64::P::E::set32(*mappedAddr32, newInstruction);
				}
			}
			else if ( (instruction & 0xFFC00000) == 0x91000000 ) {
				// ADD imm12
				if ( instruction & 0x00C00000 )
					throwf("ADD off12 uses shift at mapped address=%p", mappedAddr);
				uint32_t encodedAddend = ((instruction & 0x003FFC00) >> 10);
				uint32_t newAddend = (encodedAddend + offsetAdjust) & 0xFFF;
				uint32_t newInstruction = (instruction & 0xFFC003FF) | (newAddend << 10);
				arm64::P::E::set32(*mappedAddr32, newInstruction);
			}
			else if ( instruction != 0xD503201F ) {
				// ignore imm12 instructions optimized into a NOP, but warn about others
				fprintf(stderr, "unknown off12 instruction 0x%08X at 0x%0llX in %s\n", instruction, fromNewAddress, fLayout.getFilePath());
			}
			break;
		case DYLD_CACHE_ADJ_V2_ARM64_BR26:
			// nothing to do with calls to stubs
			break;
		default:
			throwf("unknown split seg kind=%d", kind);
	}
}

static bool isThumbMovw(uint32_t instruction)
{
	return ( (instruction & 0x8000FBF0) == 0x0000F240 );
}

static bool isThumbMovt(uint32_t instruction)
{
	return ( (instruction & 0x8000FBF0) == 0x0000F2C0 );
}

static uint16_t getThumbWord(uint32_t instruction)
{
	uint32_t i = ((instruction & 0x00000400) >> 10);
	uint32_t imm4 = (instruction & 0x0000000F);
	uint32_t imm3 = ((instruction & 0x70000000) >> 28);
	uint32_t imm8 = ((instruction & 0x00FF0000) >> 16);
	return ((imm4 << 12) | (i << 11) | (imm3 << 8) | imm8);
}

static uint32_t setThumbWord(uint32_t instruction, uint16_t word) {
	uint32_t imm4 = (word & 0xF000) >> 12;
	uint32_t i =    (word & 0x0800) >> 11;
	uint32_t imm3 = (word & 0x0700) >> 8;
	uint32_t imm8 =  word & 0x00FF;
	return (instruction & 0x8F00FBF0) | imm4 | (i << 10) | (imm3 << 28) | (imm8 << 16);
}

static bool isArmMovw(uint32_t instruction)
{
	return (instruction & 0x0FF00000) == 0x03000000;
}

static bool isArmMovt(uint32_t instruction)
{
	return (instruction & 0x0FF00000) == 0x03400000;
}

static uint16_t getArmWord(uint32_t instruction)
{
	uint32_t imm4 = ((instruction & 0x000F0000) >> 16);
	uint32_t imm12 = (instruction & 0x00000FFF);
	return (imm4 << 12) | imm12;
}

static uint32_t setArmWord(uint32_t instruction, uint16_t word) {
	uint32_t imm4 = (word & 0xF000) >> 12;
	uint32_t imm12 = word & 0x0FFF;
	return (instruction & 0xFFF0F000) | (imm4 << 16) | imm12;
}


template <>
void Rebaser<arm>::adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
								  uint64_t imageStartAddress, uint64_t imageEndAddress, std::vector<void*>& pointersInData)
{
	uint32_t value32;
	uint32_t* mappedAddr32 = (uint32_t*)mappedAddr;
	static uint32_t* lastMappedAddr32 = NULL;
	static uint32_t lastKind = 0;
	static uint32_t lastToNewAddress = 0;
	switch ( kind ) {
		case DYLD_CACHE_ADJ_V2_DELTA_32:
			value32 = arm64::P::E::get32(*mappedAddr32);
			E::set32(*mappedAddr32, value32 + adjust);
			break;
		case DYLD_CACHE_ADJ_V2_POINTER_32:
			if ( toNewAddress != (E::get32(*mappedAddr32) + targetSlide) )
				throwf("bad DYLD_CACHE_ADJ_V2_POINTER_32 value not as expected at address 0x%llX\n", fromNewAddress);
			E::set32(*mappedAddr32, toNewAddress);
			pointersInData.push_back( mappedAddr);
			break;
		case DYLD_CACHE_ADJ_V2_IMAGE_OFF_32:
            if ( adjust == 0 )
                break;
			mappedAddr32 = (uint32_t*)mappedAddr;
			value32 = (uint32_t)(toNewAddress - imageStartAddress);
			E::set32(*mappedAddr32, value32);
			break;
		case DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT:
			// to update a movw/movt pair we need to extract the 32-bit they will make,
			// add the adjust and write back the new movw/movt pair.
			if ( lastKind == kind ) {
				if ( lastToNewAddress == toNewAddress ) {
					uint32_t instruction1 = E::get32(*lastMappedAddr32);
					uint32_t instruction2 = E::get32(*mappedAddr32);
					if ( isThumbMovw(instruction1) && isThumbMovt(instruction2) ) {
						uint16_t high = getThumbWord(instruction2);
						uint16_t low  = getThumbWord(instruction1);
						uint32_t full = high << 16 | low;
						full += adjust;
						instruction1 = setThumbWord(instruction1, full & 0xFFFF);
						instruction2 = setThumbWord(instruction2, full >> 16);
					}
					else if ( isThumbMovt(instruction1) && isThumbMovw(instruction2) ) {
						uint16_t high = getThumbWord(instruction1);
						uint16_t low  = getThumbWord(instruction2);
						uint32_t full = high << 16 | low;
						full += adjust;
						instruction2 = setThumbWord(instruction2, full & 0xFFFF);
						instruction1 = setThumbWord(instruction1, full >> 16);
					}
					else {
						throw "two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but not";
					}
					E::set32(*lastMappedAddr32, instruction1);
					E::set32(*mappedAddr32, instruction2);
					kind = 0;
				}
				else {
					throw "two DYLD_CACHE_ADJ_V2_THUMB_MOVW_MOVT in a row but target different addresses";
				}
			}
			break;
		case DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT:
			// to update a movw/movt pair we need to extract the 32-bit they will make,
			// add the adjust and write back the new movw/movt pair.
			if ( lastKind == kind ) {
				if ( lastToNewAddress == toNewAddress ) {
					uint32_t instruction1 = E::get32(*lastMappedAddr32);
					uint32_t instruction2 = E::get32(*mappedAddr32);
					if ( isArmMovw(instruction1) && isArmMovt(instruction2) ) {
						uint16_t high = getArmWord(instruction2);
						uint16_t low  = getArmWord(instruction1);
						uint32_t full = high << 16 | low;
						full += adjust;
						instruction1 = setArmWord(instruction1, full & 0xFFFF);
						instruction2 = setArmWord(instruction2, full >> 16);
					}
					else if ( isArmMovt(instruction1) && isArmMovw(instruction2) ) {
						uint16_t high = getArmWord(instruction1);
						uint16_t low  = getArmWord(instruction2);
						uint32_t full = high << 16 | low;
						full += adjust;
						instruction2 = setArmWord(instruction2, full & 0xFFFF);
						instruction1 = setArmWord(instruction1, full >> 16);
					}
					else {
						throw "two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but not";
					}
					E::set32(*lastMappedAddr32, instruction1);
					E::set32(*mappedAddr32, instruction2);
					kind = 0;
				}
				else {
					throw "two DYLD_CACHE_ADJ_V2_ARM_MOVW_MOVT in a row but target different addresses";
				}
			}
			break;
		case DYLD_CACHE_ADJ_V2_ARM_BR24:
		case DYLD_CACHE_ADJ_V2_THUMB_BR22:
			// nothing to do with calls to stubs
			break;
		default:
			throwf("v2 split seg info kind (%d) not supported yet", kind);
	}
	lastKind = kind;
	lastToNewAddress = toNewAddress;
	lastMappedAddr32 = mappedAddr32;
}


template <typename A>
void Rebaser<A>::adjustReference(uint32_t kind, uint8_t* mappedAddr, uint64_t fromNewAddress, uint64_t toNewAddress, int64_t adjust, int64_t targetSlide,
								uint64_t imageStartAddress, uint64_t imageEndAddress, std::vector<void*>& pointersInData)
{
	throw "v2 split seg info not supported yet";
}


template <typename A>
void Rebaser<A>::adjustReferencesUsingInfoV2(std::vector<void*>& pointersInData)
{
	static const bool log = false;

	const uint8_t* infoStart = &fLinkEditBase[fSplitSegInfo->dataoff()];
	const uint8_t* infoEnd = &infoStart[fSplitSegInfo->datasize()];;
	if ( *infoStart++ != DYLD_CACHE_ADJ_V2_FORMAT )
		throw "malformed split seg info";

	// build section arrays of slide and mapped address for each section
	std::vector<uint64_t> sectionSlides;
	std::vector<uint64_t> sectionNewAddress;
	std::vector<uint8_t*> sectionMappedAddress;
	sectionSlides.reserve(16);
	sectionNewAddress.reserve(16);
	sectionMappedAddress.reserve(16);
	// section index 0 refers to mach_header
	const MachOLayoutAbstraction::Segment& textSeg = fLayout.getSegments().front();
	sectionMappedAddress.push_back((uint8_t*)textSeg.mappedAddress());
	sectionSlides.push_back(textSeg.newAddress() - textSeg.address());
	sectionNewAddress.push_back(textSeg.newAddress());
	// section 1 and later refer to real sections
	unsigned sectionIndex = 0;
	for (const MachOLayoutAbstraction::Segment& seg : fLayout.getSegments()) {
		uint64_t segSlide = seg.newAddress() - seg.address();
		for (uint32_t i=0; i < seg.sectionCount(); ++i) {
			if (log) fprintf(stderr, "seg=%s, sectIndex=%d, mapped at=%p, offsetInSeg=0x%08llX\n", seg.name(), sectionIndex, seg.mappedAddress(), fSectionOffsetsInSegment[sectionIndex]);
			sectionMappedAddress.push_back((uint8_t*)seg.mappedAddress() + fSectionOffsetsInSegment[sectionIndex]);
			sectionSlides.push_back(segSlide);
			sectionNewAddress.push_back(seg.newAddress() + fSectionOffsetsInSegment[sectionIndex]);
			++sectionIndex;
		}
	}

	// Whole		 :== <count> FromToSection+
	// FromToSection :== <from-sect-index> <to-sect-index> <count> ToOffset+
	// ToOffset		 :== <to-sect-offset-delta> <count> FromOffset+
	// FromOffset	 :== <kind> <count> <from-sect-offset-delta>
	const uint8_t* p = infoStart;
	uint64_t sectionCount = read_uleb128(p, infoEnd);
	for (uint64_t i=0; i < sectionCount; ++i) {
		uint64_t fromSectionIndex = read_uleb128(p, infoEnd);
		uint64_t toSectionIndex = read_uleb128(p, infoEnd);
		uint64_t toOffsetCount = read_uleb128(p, infoEnd);
		uint64_t fromSectionSlide = sectionSlides[fromSectionIndex];
		uint64_t fromSectionNewAddress = sectionNewAddress[fromSectionIndex];
		uint8_t* fromSectionMappedAddress = sectionMappedAddress[fromSectionIndex];
		uint64_t toSectionSlide = sectionSlides[toSectionIndex];
		uint64_t toSectionNewAddress = sectionNewAddress[toSectionIndex];
		if (log) printf("from sect=%lld (mapped=%p), to sect=%lld (new addr=0x%llX):\n", fromSectionIndex, fromSectionMappedAddress, toSectionIndex, toSectionNewAddress);
		uint64_t toSectionOffset = 0;
		for (uint64_t j=0; j < toOffsetCount; ++j) {
			uint64_t toSectionDelta = read_uleb128(p, infoEnd);
			uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
			toSectionOffset += toSectionDelta;
			for (uint64_t k=0; k < fromOffsetCount; ++k) {
				uint64_t kind = read_uleb128(p, infoEnd);
				uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
				uint64_t fromSectionOffset = 0;
				for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
					uint64_t delta = read_uleb128(p, infoEnd);
					fromSectionOffset += delta;
					int64_t deltaAdjust = toSectionSlide - fromSectionSlide;
					if (log) printf("  kind=%lld, from offset=0x%0llX, to offset=0x%0llX, adjust=0x%llX, targetSlide=0x%llX\n", kind, fromSectionOffset, toSectionOffset, deltaAdjust, toSectionSlide);
					uint8_t*  fromMappedAddr = fromSectionMappedAddress + fromSectionOffset;
					uint64_t toNewAddress = toSectionNewAddress + toSectionOffset;
					uint64_t fromNewAddress = fromSectionNewAddress + fromSectionOffset;
					adjustReference(kind, fromMappedAddr, fromNewAddress, toNewAddress, deltaAdjust, toSectionSlide, sectionNewAddress.front(), sectionNewAddress.back(), pointersInData);
				}
			}
		}
	}


}

#endif // __MACHO_REBASER__




