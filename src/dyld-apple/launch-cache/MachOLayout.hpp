/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*- 
 *
 * Copyright (c) 2006-2011 Apple Inc. All rights reserved.
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

#ifndef __MACHO_LAYOUT__
#define __MACHO_LAYOUT__

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
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

#include <vector>
#include <set>
#include <unordered_map>

#include "MachOFileAbstraction.hpp"
#include "Architectures.hpp"


void throwf(const char* format, ...) __attribute__((format(printf, 1, 2)));

__attribute__((noreturn))
void throwf(const char* format, ...) 
{
	va_list	list;
	char*	p;
	va_start(list, format);
	vasprintf(&p, format, list);
	va_end(list);
	
	const char*	t = p;
	throw t;
}


class MachOLayoutAbstraction
{
public:
	struct Segment
	{
	public:
					Segment(uint64_t addr, uint64_t vmsize, uint64_t offset, uint64_t file_size, uint64_t sectionsSize,
							uint64_t sectionsAlignment, uint64_t align,
							uint32_t prot, uint32_t sectionCount, const char* segName) : fOrigAddress(addr), fOrigSize(vmsize),
							fOrigFileOffset(offset),  fOrigFileSize(file_size), fOrigPermissions(prot), 
							fSize(vmsize), fFileOffset(offset), fFileSize(file_size), fAlignment(align),
							fPermissions(prot), fSectionCount(sectionCount), fSectionsSize(sectionsSize),
							fSectionsAlignment(sectionsAlignment), fNewAddress(0), fMappedAddress(NULL) {
								strlcpy(fOrigName, segName, 16);
							}
							
		uint64_t	address() const		{ return fOrigAddress; }
		uint64_t	size() const		{ return fSize; }
		uint64_t	fileOffset() const	{ return fFileOffset; }
		uint64_t	fileSize() const	{ return fFileSize; }
		uint32_t	permissions() const { return fPermissions; }
		bool		readable() const	{ return fPermissions & VM_PROT_READ; }
		bool		writable() const	{ return fPermissions & VM_PROT_WRITE; }
		bool		executable() const	{ return fPermissions & VM_PROT_EXECUTE; }
		uint64_t	alignment() const	{ return fAlignment; }
		const char* name() const		{ return fOrigName; }
		uint64_t	newAddress() const	{ return fNewAddress; }
		uint32_t	sectionCount() const{ return fSectionCount; }
		uint64_t	sectionsSize() const{ return fSectionsSize; }
		uint64_t	sectionsAlignment() const { return fSectionsAlignment; }
		void*		mappedAddress() const			{ return fMappedAddress; }
		void		setNewAddress(uint64_t addr)	{ fNewAddress = addr; }
		void		setMappedAddress(void* addr)	{ fMappedAddress = addr; }
		void		setSize(uint64_t new_size)		{ fSize = new_size; }
		void		setFileOffset(uint64_t new_off)	{ fFileOffset = new_off; }
		void		setFileSize(uint64_t new_size)	{ fFileSize = new_size; }
		void		setWritable(bool w)				{ if (w) fPermissions |= VM_PROT_WRITE; else fPermissions &= ~VM_PROT_WRITE; }
		void		setSectionsAlignment(uint64_t v){ fSectionsAlignment = v; }
		void		reset()							{ fSize=fOrigSize; fFileOffset=fOrigFileOffset; fFileSize=fOrigFileSize; fPermissions=fOrigPermissions; }
	private:
		uint64_t		fOrigAddress;
		uint64_t		fOrigSize;
		uint64_t		fOrigFileOffset;
		uint64_t		fOrigFileSize;
		uint32_t		fOrigPermissions;
		char			fOrigName[16];
		uint64_t		fSize;
		uint64_t		fFileOffset;
		uint64_t		fFileSize;
		uint64_t		fAlignment;
		uint32_t		fPermissions;
		uint32_t		fSectionCount;
		uint64_t		fSectionsSize;
		uint64_t		fSectionsAlignment;
		uint64_t		fNewAddress;
		void*			fMappedAddress;
	};

	struct Library
	{
		const char*	name;
		uint32_t	currentVersion;
		uint32_t	compatibilityVersion;
		bool		weakImport;
	};
	
	
	virtual ArchPair							getArchPair() const = 0;
	virtual const char*							getFilePath() const = 0;
	virtual uint64_t							getOffsetInUniversalFile() const	= 0;
	virtual uint32_t							getFileType() const	= 0;
	virtual uint32_t							getFlags() const = 0;
	virtual	Library								getID() const = 0;
	virtual bool								isDylib() const = 0;
	virtual bool								isSplitSeg() const = 0;
	virtual bool								hasSplitSegInfo() const = 0;
	virtual bool								hasSplitSegInfoV2() const = 0;
	virtual bool								inSharableLocation() const = 0;
	virtual bool								hasDynamicLookupLinkage() const = 0;
	virtual bool								hasMainExecutableLookupLinkage() const = 0;
	virtual bool								isTwoLevelNamespace() const	= 0;
	virtual bool								hasDyldInfo() const	= 0;
	virtual bool								hasMultipleReadWriteSegments() const = 0;
	virtual	uint32_t							getNameFileOffset() const = 0;
	virtual time_t								getLastModTime() const = 0;
	virtual ino_t								getInode() const = 0;
	virtual std::vector<Segment>&				getSegments() = 0;
	virtual const std::vector<Segment>&			getSegments() const = 0;
	virtual const Segment*						getSegment(const char* name) const = 0;
	virtual const std::vector<Library>&			getLibraries() const = 0;
	virtual uint64_t							getBaseAddress() const = 0;
	virtual uint64_t							getVMSize() const = 0;
	virtual uint64_t							getBaseExecutableAddress() const = 0;
	virtual uint64_t							getBaseWritableAddress() const = 0;
	virtual uint64_t							getBaseReadOnlyAddress() const = 0;
	virtual uint64_t							getExecutableVMSize() const = 0;
	virtual uint64_t							getWritableVMSize() const = 0;
	virtual uint64_t							getReadOnlyVMSize() const = 0;
	// need getDyldInfoExports because export info uses ULEB encoding and size could grow
	virtual const uint8_t*						getDyldInfoExports() const = 0;
	virtual void								setDyldInfoExports(const uint8_t* newExports) const = 0;
	virtual void								uuid(uuid_t u) const = 0;
};




template <typename A>
class MachOLayout : public MachOLayoutAbstraction
{
public:
												MachOLayout(const void* machHeader, uint64_t offset, const char* path,
																	ino_t inode, time_t modTime, uid_t uid);
	virtual										~MachOLayout() {}

	virtual ArchPair							getArchPair() const		{ return fArchPair; }
	virtual const char*							getFilePath() const		{ return fPath; }
	virtual uint64_t							getOffsetInUniversalFile() const { return fOffset; }
	virtual uint32_t							getFileType() const		{ return fFileType; }
	virtual uint32_t							getFlags() const		{ return fFlags; }
	virtual	Library								getID() const			{ return fDylibID; }
	virtual bool								isDylib() const			{ return fIsDylib; }
	virtual bool								isSplitSeg() const;
	virtual bool								hasSplitSegInfo() const	{ return fSplitSegInfo != NULL; }
	virtual bool								hasSplitSegInfoV2() const{ return fHasSplitSegInfoV2; }
	virtual bool								inSharableLocation() const { return fShareableLocation; }
	virtual bool								hasDynamicLookupLinkage() const { return fDynamicLookupLinkage; }
	virtual bool								hasMainExecutableLookupLinkage() const { return fMainExecutableLookupLinkage; }
	virtual bool								isTwoLevelNamespace() const	{ return (fFlags & MH_TWOLEVEL); }
	virtual bool								hasDyldInfo() const		{ return fHasDyldInfo; }
	virtual bool								hasMultipleReadWriteSegments() const { return fHasTooManyWritableSegments; }
	virtual	uint32_t							getNameFileOffset() const{ return fNameFileOffset; }
	virtual time_t								getLastModTime() const	{ return fMTime; }
	virtual ino_t								getInode() const		{ return fInode; }
	virtual std::vector<Segment>&				getSegments()			{ return fSegments; }
	virtual const std::vector<Segment>&			getSegments() const		{ return fSegments; }
	virtual const Segment*						getSegment(const char* name) const;
	virtual const std::vector<Library>&			getLibraries() const	{ return fLibraries; }
	virtual uint64_t							getBaseAddress() const	{ return fLowSegment->address(); }
	virtual uint64_t							getVMSize() const		{ return fVMSize; }
	virtual uint64_t							getBaseExecutableAddress() const { return fLowExecutableSegment->address(); }
	virtual uint64_t							getBaseWritableAddress() const	{ return fLowWritableSegment->address(); }
	virtual uint64_t							getBaseReadOnlyAddress() const	{ return fLowReadOnlySegment->address(); }
	virtual uint64_t							getExecutableVMSize() const		{ return fVMExecutableSize; }
	virtual uint64_t							getWritableVMSize() const		{ return fVMWritablSize; }
	virtual uint64_t							getReadOnlyVMSize() const		{ return fVMReadOnlySize; }
	virtual const uint8_t*						getDyldInfoExports() const		{ return fDyldInfoExports; }
	virtual void								setDyldInfoExports(const uint8_t* newExports) const { fDyldInfoExports = newExports; }
	virtual void								uuid(uuid_t u) const { memcpy(u, fUUID, 16); }
	
private:
	typedef typename A::P					P;
	typedef typename A::P::E				E;
	typedef typename A::P::uint_t			pint_t;
	
	uint64_t									segmentSize(const macho_segment_command<typename A::P>* segCmd) const;
	uint64_t									segmentFileSize(const macho_segment_command<typename A::P>* segCmd) const;
	uint64_t									segmentAlignment(const macho_segment_command<typename A::P>* segCmd) const;
	uint64_t									sectionsSize(const macho_segment_command<typename A::P>* segCmd) const;
	uint64_t									sectionsAlignment(const macho_segment_command<typename A::P>* segCmd) const;

	bool										validReadWriteSeg(const Segment& seg) const;
	
	static cpu_type_t							arch();

	const char*									fPath;
	uint64_t									fOffset;
	uint32_t									fFileType;
	ArchPair									fArchPair;
	uint32_t									fFlags;
	std::vector<Segment>						fSegments;
	std::vector<Library>						fLibraries;
	const Segment*								fLowSegment;
	const Segment*								fLowExecutableSegment;
	const Segment*								fLowWritableSegment;
	const Segment*								fLowReadOnlySegment;
	Library										fDylibID;
	uint32_t									fNameFileOffset;
	time_t										fMTime;
	ino_t										fInode;
	uint64_t									fVMSize;
	uint64_t									fVMExecutableSize;
	uint64_t									fVMWritablSize;
	uint64_t									fVMReadOnlySize;
	const macho_linkedit_data_command<P>*		fSplitSegInfo;
	bool										fHasSplitSegInfoV2;
	bool										fShareableLocation;
	bool										fDynamicLookupLinkage;
	bool										fMainExecutableLookupLinkage;
	bool										fIsDylib;
	bool										fHasDyldInfo;
	bool										fHasTooManyWritableSegments;
	mutable const uint8_t*						fDyldInfoExports;
	uuid_t										fUUID;
};



class UniversalMachOLayout
{
public:
												UniversalMachOLayout(const char* path, const std::set<ArchPair>* onlyArchs=NULL);
												~UniversalMachOLayout() {}

	static const UniversalMachOLayout&			find(const char* path, const std::set<ArchPair>* onlyArchs=NULL);
	const MachOLayoutAbstraction*				getSlice(ArchPair ap) const;
	const std::vector<MachOLayoutAbstraction*>&	allLayouts() const { return fLayouts; }

private:
	class CStringHash {
	public:
		size_t operator()(const char* __s) const {
			size_t __h = 0;
			for ( ; *__s; ++__s)
				__h = 5 * __h + *__s;
			return __h;
		};
	};
	struct CStringEquals {
		bool operator()(const char* left, const char* right) const { return (strcmp(left, right) == 0); }
	};
	typedef std::unordered_map<const char*, const UniversalMachOLayout*, CStringHash, CStringEquals> PathToNode;

	static bool					requestedSlice(const std::set<ArchPair>* onlyArchs, cpu_type_t cpuType, cpu_subtype_t cpuSubType);

	static PathToNode							fgLayoutCache;
	const char*									fPath;
	std::vector<MachOLayoutAbstraction*>		fLayouts;
};

UniversalMachOLayout::PathToNode UniversalMachOLayout::fgLayoutCache;




const MachOLayoutAbstraction* UniversalMachOLayout::getSlice(ArchPair ap) const
{
	// use matching cputype and cpusubtype
	for(std::vector<MachOLayoutAbstraction*>::const_iterator it=fLayouts.begin(); it != fLayouts.end(); ++it) {
		const MachOLayoutAbstraction* layout = *it;
		if ( layout->getArchPair().arch == ap.arch ) {
            switch ( ap.arch ) {
                case CPU_TYPE_ARM:
				case CPU_TYPE_X86_64:
                   if ( (layout->getArchPair().subtype & ~CPU_SUBTYPE_MASK) == (ap.subtype & ~CPU_SUBTYPE_MASK) )
                        return layout;
                    break;
                 default:
                    return layout;
            }
        }
	}
	// if requesting x86_64h and it did not exist, try x86_64 as a fallback
	if ((ap.arch == CPU_TYPE_X86_64) && (ap.subtype == CPU_SUBTYPE_X86_64_H)) {
		ap.subtype = CPU_SUBTYPE_X86_64_ALL;
		return this->getSlice(ap);
	}
	return NULL;
}


const UniversalMachOLayout& UniversalMachOLayout::find(const char* path, const std::set<ArchPair>* onlyArchs)
{
	// look in cache
	PathToNode::iterator pos = fgLayoutCache.find(path);
	if ( pos != fgLayoutCache.end() )
		return *pos->second;
		
	// create UniversalMachOLayout
	const UniversalMachOLayout* result = new UniversalMachOLayout(path, onlyArchs);
	
	// add it to cache
	fgLayoutCache[result->fPath] = result;
	
	return *result;
}


bool UniversalMachOLayout::requestedSlice(const std::set<ArchPair>* onlyArchs, cpu_type_t cpuType, cpu_subtype_t cpuSubType)
{
	if ( onlyArchs == NULL )
		return true;
	// must match cputype and cpusubtype
	for (std::set<ArchPair>::const_iterator it = onlyArchs->begin(); it != onlyArchs->end(); ++it) {
		ArchPair anArch = *it;
		if ( cpuType == anArch.arch ) {
            switch ( cpuType ) {
                case CPU_TYPE_ARM:
                    if ( cpuSubType == anArch.subtype ) 
                        return true;
                    break;
                default:
                    return true;
            }
        }
	}
	return false;
}


UniversalMachOLayout::UniversalMachOLayout(const char* path, const std::set<ArchPair>* onlyArchs)
 : fPath(strdup(path))
{
	// map in whole file
	int fd = ::open(path, O_RDONLY, 0);
	if ( fd == -1 ) {
		int err = errno;
		if  ( err == ENOENT )
			throwf("file not found");
		else
			throwf("can't open file, errno=%d", err);
	}
	struct stat stat_buf;
	if ( fstat(fd, &stat_buf) == -1)
		throwf("can't stat open file %s, errno=%d", path, errno);
	if ( stat_buf.st_size < 20 )
		throwf("file too small %s", path);
	uint8_t* p = (uint8_t*)::mmap(NULL, stat_buf.st_size, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
	if ( p == (uint8_t*)(-1) )
		throwf("can't map file %s, errno=%d", path, errno);
	::close(fd);

	try {
		// if fat file, process each architecture
		const fat_header* fh = (fat_header*)p;
		const mach_header* mh = (mach_header*)p;
		if ( fh->magic == OSSwapBigToHostInt32(FAT_MAGIC) ) {
			// Fat header is always big-endian
			const struct fat_arch* slices = (struct fat_arch*)(p + sizeof(struct fat_header));
			const uint32_t sliceCount = OSSwapBigToHostInt32(fh->nfat_arch);
			for (uint32_t i=0; i < sliceCount; ++i) {
				if ( requestedSlice(onlyArchs, OSSwapBigToHostInt32(slices[i].cputype), OSSwapBigToHostInt32(slices[i].cpusubtype)) ) {
					uint32_t fileOffset = OSSwapBigToHostInt32(slices[i].offset);
					if ( fileOffset > stat_buf.st_size ) {
						throwf("malformed universal file, slice %u for architecture 0x%08X is beyond end of file: %s", 
								i, OSSwapBigToHostInt32(slices[i].cputype), path);
					}
					if ( (fileOffset+OSSwapBigToHostInt32(slices[i].size)) > stat_buf.st_size ) {
						throwf("malformed universal file, slice %u for architecture 0x%08X is beyond end of file: %s", 
								i, OSSwapBigToHostInt32(slices[i].cputype), path);
					}
					try {
						switch ( OSSwapBigToHostInt32(slices[i].cputype) ) {
							case CPU_TYPE_I386:
								fLayouts.push_back(new MachOLayout<x86>(&p[fileOffset], fileOffset, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
								break;
							case CPU_TYPE_X86_64:
								fLayouts.push_back(new MachOLayout<x86_64>(&p[fileOffset], fileOffset, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
								break;
							case CPU_TYPE_ARM:
								fLayouts.push_back(new MachOLayout<arm>(&p[fileOffset], fileOffset, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
								break;
							case CPU_TYPE_ARM64:
								fLayouts.push_back(new MachOLayout<arm64>(&p[fileOffset], fileOffset, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
								break;
							default:
								throw "unknown slice in fat file";
						}
					}
					catch (const char* msg) {
						fprintf(stderr, "warning: %s for %s\n", msg, path);
					}
				}
			}
		}
		else {
			try {
				if ( (OSSwapLittleToHostInt32(mh->magic) == MH_MAGIC) && (OSSwapLittleToHostInt32(mh->cputype) == CPU_TYPE_I386)) {
					if ( requestedSlice(onlyArchs, OSSwapLittleToHostInt32(mh->cputype), OSSwapLittleToHostInt32(mh->cpusubtype)) ) 
						fLayouts.push_back(new MachOLayout<x86>(mh, 0, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
				}
				else if ( (OSSwapLittleToHostInt32(mh->magic) == MH_MAGIC_64) && (OSSwapLittleToHostInt32(mh->cputype) == CPU_TYPE_X86_64)) {
					if ( requestedSlice(onlyArchs, OSSwapLittleToHostInt32(mh->cputype), OSSwapLittleToHostInt32(mh->cpusubtype)) ) 
						fLayouts.push_back(new MachOLayout<x86_64>(mh, 0, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
				}
				else if ( (OSSwapLittleToHostInt32(mh->magic) == MH_MAGIC) && (OSSwapLittleToHostInt32(mh->cputype) == CPU_TYPE_ARM)) {
					if ( requestedSlice(onlyArchs, OSSwapLittleToHostInt32(mh->cputype), OSSwapLittleToHostInt32(mh->cpusubtype)) ) 
						fLayouts.push_back(new MachOLayout<arm>(mh, 0, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
				}
				else if ( (OSSwapLittleToHostInt32(mh->magic) == MH_MAGIC_64) && (OSSwapLittleToHostInt32(mh->cputype) == CPU_TYPE_ARM64)) {
					if ( requestedSlice(onlyArchs, OSSwapLittleToHostInt32(mh->cputype), OSSwapLittleToHostInt32(mh->cpusubtype)) ) 
						fLayouts.push_back(new MachOLayout<arm64>(mh, 0, fPath, stat_buf.st_ino, stat_buf.st_mtime, stat_buf.st_uid));
				}
				else {
					throw "unknown file format";
				}
			}
			catch (const char* msg) {
				fprintf(stderr, "warning: %s for %s\n", msg, path);
			}
		}
	}
	catch (...) {
		::munmap(p, stat_buf.st_size);
		throw;
	}
}


template <typename A>
uint64_t MachOLayout<A>::segmentSize(const macho_segment_command<typename A::P>* segCmd) const
{
	// <rdar://problem/13089366> segments may have 16KB alignment padding at end, if so we can remove that in cache
	if ( segCmd->nsects() > 0 ) {
		const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
		const macho_section<P>* const lastSection = &sectionsStart[segCmd->nsects()-1];
		uint64_t endSectAddr = lastSection->addr() + lastSection->size();
		uint64_t endSectAddrPage = (endSectAddr + 4095) & (-4096);
		if ( endSectAddrPage < (segCmd->vmaddr() + segCmd->vmsize()) ) {
			uint64_t size =  endSectAddrPage - segCmd->vmaddr();
			//if ( size != segCmd->vmsize() )
			//	fprintf(stderr, "trim %s size=0x%08llX instead of 0x%08llX for %s\n", 
			//		segCmd->segname(), size, segCmd->vmsize(), getFilePath());
			return size;
		}
	}
	return segCmd->vmsize();
}

template <typename A>
uint64_t MachOLayout<A>::segmentAlignment(const macho_segment_command<typename A::P>* segCmd) const
{
	int p2align = 12;
	if ( segCmd->nsects() > 0 ) {
		const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
		const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()-1];
		for (const macho_section<P>*  sect=sectionsStart; sect < sectionsEnd; ++sect) {
			if ( sect->align() > p2align )
				p2align = sect->align();
		}
	}
	return (1 << p2align);
}

template <typename A>
uint64_t MachOLayout<A>::segmentFileSize(const macho_segment_command<typename A::P>* segCmd) const
{
	// <rdar://problem/13089366> segments may have 16KB alignment padding at end, if so we can remove that in cache
	if ( segCmd->nsects() > 0 ) {
		uint64_t endOffset = segCmd->fileoff();
		const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
		const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
		for (const macho_section<P>* sect=sectionsStart; sect < sectionsEnd; ++sect) {
			if ( sect->offset() != 0 )
				endOffset = sect->offset() + sect->size();
		}
		uint64_t size = (endOffset - segCmd->fileoff() + 4095) & (-4096);
		//if ( size != segCmd->filesize() )
		//	fprintf(stderr, "trim %s filesize=0x%08llX instead of 0x%08llX for %s\n", 
		//		segCmd->segname(), size, segCmd->filesize(), getFilePath());
		return size;
	}
	return segCmd->filesize();
}

template <typename A>
uint64_t MachOLayout<A>::sectionsSize(const macho_segment_command<typename A::P>* segCmd) const
{
	if ( segCmd->nsects() > 0 ) {
		const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
		const macho_section<P>* const lastSection = &sectionsStart[segCmd->nsects()-1];
		uint64_t endSectAddr = lastSection->addr() + lastSection->size();
		if ( endSectAddr < (segCmd->vmaddr() + segCmd->vmsize()) ) {
			uint64_t size =  endSectAddr - segCmd->vmaddr();
			return size;
		}
	}
	return segCmd->vmsize();
}

template <typename A>
uint64_t MachOLayout<A>::sectionsAlignment(const macho_segment_command<typename A::P>* segCmd) const
{
	int p2align = 4;
	if ( hasSplitSegInfoV2() && (segCmd->nsects() > 0) ) {
		const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
		const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()-1];
		for (const macho_section<P>*  sect=sectionsStart; sect < sectionsEnd; ++sect) {
			if ( sect->align() > p2align )
				p2align = sect->align();
		}
	}
	return (1 << p2align);
}



template <typename A>
MachOLayout<A>::MachOLayout(const void* machHeader, uint64_t offset, const char* path, ino_t inode, time_t modTime, uid_t uid)
 : fPath(path), fOffset(offset), fArchPair(0,0), fMTime(modTime), fInode(inode), fSplitSegInfo(NULL), fHasSplitSegInfoV2(false),
   fShareableLocation(false), fDynamicLookupLinkage(false), fMainExecutableLookupLinkage(false), fIsDylib(false), 
	fHasDyldInfo(false), fHasTooManyWritableSegments(false), fDyldInfoExports(NULL)
{
	fDylibID.name = NULL;
	fDylibID.currentVersion = 0;
	fDylibID.compatibilityVersion = 0;
	bzero(fUUID, sizeof(fUUID));
	
	const macho_header<P>* mh = (const macho_header<P>*)machHeader;
	if ( mh->cputype() != arch() )
		throw "Layout object is wrong architecture";
	switch ( mh->filetype() ) {
		case MH_DYLIB:
			fIsDylib = true;
			break;
		case MH_BUNDLE:
		case MH_EXECUTE:
		case MH_DYLIB_STUB:
		case MH_DYLINKER:
			break;
		default:
			throw "file is not a mach-o final linked image";
	}
	fFlags = mh->flags();
	fFileType = mh->filetype();
	fArchPair.arch = mh->cputype();
	fArchPair.subtype = mh->cpusubtype();

	const macho_dyld_info_command<P>* dyldInfo = NULL;
	const macho_symtab_command<P>* symbolTableCmd = NULL;
	const macho_dysymtab_command<P>* dynamicSymbolTableCmd = NULL;
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
	const uint32_t cmd_count = mh->ncmds();
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		switch ( cmd->cmd() ) {
			case LC_ID_DYLIB:	
				{
					macho_dylib_command<P>* dylib  = (macho_dylib_command<P>*)cmd;
					fDylibID.name = strdup(dylib->name());
					fDylibID.currentVersion = dylib->current_version();
					fDylibID.compatibilityVersion = dylib->compatibility_version();
					fNameFileOffset = dylib->name() - (char*)machHeader;
					fShareableLocation = ( (strncmp(fDylibID.name, "/usr/lib/", 9) == 0) || (strncmp(fDylibID.name, "/System/Library/", 16) == 0) );
				}
				break;
			case LC_LOAD_DYLIB:
			case LC_LOAD_WEAK_DYLIB:
			case LC_REEXPORT_DYLIB:
			case LC_LOAD_UPWARD_DYLIB:
				{
					macho_dylib_command<P>* dylib = (macho_dylib_command<P>*)cmd;
					Library lib;
					lib.name = strdup(dylib->name());
					lib.currentVersion = dylib->current_version();
					lib.compatibilityVersion = dylib->compatibility_version();
					lib.weakImport = ( cmd->cmd() == LC_LOAD_WEAK_DYLIB );
					fLibraries.push_back(lib);
				}
				break;
			case LC_SEGMENT_SPLIT_INFO:
				fSplitSegInfo = (macho_linkedit_data_command<P>*)cmd;
				break;
			case macho_segment_command<P>::CMD:
				{
					const macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
					fSegments.push_back(Segment(segCmd->vmaddr(), segmentSize(segCmd), segCmd->fileoff(), 
								segmentFileSize(segCmd), sectionsSize(segCmd), sectionsAlignment(segCmd),
								segmentAlignment(segCmd), segCmd->initprot(),
								segCmd->nsects(), segCmd->segname()));
				}
				break;
			case LC_SYMTAB:
				symbolTableCmd = (macho_symtab_command<P>*)cmd;
				break;
			case LC_DYSYMTAB:
				dynamicSymbolTableCmd = (macho_dysymtab_command<P>*)cmd;
				break;
			case LC_DYLD_INFO:
			case LC_DYLD_INFO_ONLY:
				fHasDyldInfo = true;
				dyldInfo = (struct macho_dyld_info_command<P>*)cmd;
				break;
			case LC_UUID:
				{
					const macho_uuid_command<P>* uc = (macho_uuid_command<P>*)cmd;
					memcpy(&fUUID, uc->uuid(), 16);
				}
				break;
		}
		cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	}

	fLowSegment = NULL;
	fLowExecutableSegment = NULL;
	fLowWritableSegment = NULL;
	fLowReadOnlySegment = NULL;
	fVMExecutableSize = 0;
	fVMWritablSize = 0;
	fVMReadOnlySize = 0;
	fVMSize = 0;
	const Segment* highSegment = NULL;
	for(std::vector<Segment>::const_iterator it = fSegments.begin(); it != fSegments.end(); ++it) {
		const Segment& seg = *it;
		if ( (fLowSegment == NULL) || (seg.address() < fLowSegment->address()) )
			fLowSegment = &seg;
		if ( (highSegment == NULL) || (seg.address() > highSegment->address()) )
			highSegment = &seg;
		if ( seg.executable() ) {
			if ( (fLowExecutableSegment == NULL) || (seg.address() < fLowExecutableSegment->address()) )
				fLowExecutableSegment = &seg;
			fVMExecutableSize += seg.size();
		}
		else if ( seg.writable()) {
			if ( (fLowWritableSegment == NULL) || (seg.address() < fLowWritableSegment->address()) )
				fLowWritableSegment = &seg;
			fVMWritablSize += seg.size();
			if ( !validReadWriteSeg(seg) ) {
				fHasTooManyWritableSegments = true;
			}
		}
		else {
			if ( (fLowReadOnlySegment == NULL) || (seg.address() < fLowReadOnlySegment->address()) )
				fLowReadOnlySegment = &seg;
			fVMReadOnlySize += seg.size();
		}		
	}
	if ( (highSegment != NULL) && (fLowSegment != NULL) )
		fVMSize = (highSegment->address() + highSegment->size() - fLowSegment->address() + 4095) & (-4096);			

	// scan undefines looking, for magic ordinals
	if ( (symbolTableCmd != NULL) && (dynamicSymbolTableCmd != NULL) ) {
		const macho_nlist<P>* symbolTable = (macho_nlist<P>*)((uint8_t*)machHeader + symbolTableCmd->symoff());
		const uint32_t startUndefs = dynamicSymbolTableCmd->iundefsym();
		const uint32_t endUndefs = startUndefs + dynamicSymbolTableCmd->nundefsym();
		for (uint32_t i=startUndefs; i < endUndefs; ++i) {
			uint8_t ordinal = GET_LIBRARY_ORDINAL(symbolTable[i].n_desc());
			if ( ordinal == DYNAMIC_LOOKUP_ORDINAL )
				fDynamicLookupLinkage = true;
			else if ( ordinal == EXECUTABLE_ORDINAL )
				fMainExecutableLookupLinkage = true;
		}
	}
	
	if ( dyldInfo != NULL ) {
		if ( dyldInfo->export_off() != 0 ) {
			fDyldInfoExports = (uint8_t*)machHeader + dyldInfo->export_off();
		}
	}

	if ( fSplitSegInfo != NULL ) {
		const uint8_t* infoStart = (uint8_t*)machHeader + fSplitSegInfo->dataoff();
		fHasSplitSegInfoV2 = ( *infoStart == DYLD_CACHE_ADJ_V2_FORMAT );
		if ( !fHasSplitSegInfoV2 ) {
			// split seg version not known when segments created
			// v1 does not support packing, so simulate that by forcing alignment
			for (Segment& seg : fSegments) {
				seg.setSectionsAlignment(4096);
			}
		}
	}

}

template <> cpu_type_t MachOLayout<x86>::arch()     { return CPU_TYPE_I386; }
template <> cpu_type_t MachOLayout<x86_64>::arch()  { return CPU_TYPE_X86_64; }
template <> cpu_type_t MachOLayout<arm>::arch()		{ return CPU_TYPE_ARM; }
template <> cpu_type_t MachOLayout<arm64>::arch()	{ return CPU_TYPE_ARM64; }

template <>
bool MachOLayout<x86>::validReadWriteSeg(const Segment& seg) const
{
	return (strcmp(seg.name(), "__DATA") == 0) || (strcmp(seg.name(), "__OBJC") == 0);
}

template <typename A>
bool MachOLayout<A>::validReadWriteSeg(const Segment& seg) const
{
	return (strcmp(seg.name(), "__DATA") == 0);
}


template <>
bool MachOLayout<x86>::isSplitSeg() const
{
	return ( (this->getFlags() & MH_SPLIT_SEGS) != 0 );
}

template <>
bool MachOLayout<arm>::isSplitSeg() const
{
	return ( (this->getFlags() & MH_SPLIT_SEGS) != 0 );
}

template <typename A>
bool MachOLayout<A>::isSplitSeg() const
{
	return false;
}

template <typename A>
const MachOLayoutAbstraction::Segment* MachOLayout<A>::getSegment(const char* name) const
{
	for(std::vector<Segment>::const_iterator it = fSegments.begin(); it != fSegments.end(); ++it) {
		const Segment& seg = *it;
		if ( strcmp(seg.name(), name) == 0 )
			return &seg;
	}
	return NULL;
}



#endif // __MACHO_LAYOUT__



