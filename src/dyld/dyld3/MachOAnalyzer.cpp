/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#include <sys/types.h>
#include <mach/mach.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach-o/reloc.h>
#include <mach-o/nlist.h>
#include <TargetConditionals.h>

#include "MachOAnalyzer.h"
#include "CodeSigningTypes.h"
#include "Array.h"


namespace dyld3 {


const MachOAnalyzer* MachOAnalyzer::validMainExecutable(Diagnostics& diag, const mach_header* mh, const char* path, uint64_t sliceLength,
                                                        const GradedArchs& archs, Platform platform)
{
    const MachOAnalyzer* result = (const MachOAnalyzer*)mh;
    if ( !result->validMachOForArchAndPlatform(diag, (size_t)sliceLength, path, archs, platform) )
        return nullptr;
    if ( !result->isDynamicExecutable() )
        return nullptr;

    return result;
}


closure::LoadedFileInfo MachOAnalyzer::load(Diagnostics& diag, const closure::FileSystem& fileSystem,
                                            const char* path, const GradedArchs& archs, Platform platform, char realerPath[MAXPATHLEN])
{
    // FIXME: This should probably be an assert, but if we happen to have a diagnostic here then something is wrong
    // above us and we should quickly return instead of doing unnecessary work.
    if (diag.hasError())
        return closure::LoadedFileInfo();

    closure::LoadedFileInfo info;
    if (!fileSystem.loadFile(path, info, realerPath, ^(const char *format, ...) {
        va_list list;
        va_start(list, format);
        diag.error(format, list);
        va_end(list);
    })) {
        return closure::LoadedFileInfo();
    }

    // If we now have an error, but succeeded, then we must have tried multiple paths, one of which errored, but
    // then succeeded on a later path.  So clear the error.
    if (diag.hasError())
        diag.clearError();

    // if fat, remap just slice needed
    bool fatButMissingSlice;
    const FatFile*       fh = (FatFile*)info.fileContent;
    uint64_t sliceOffset = info.sliceOffset;
    uint64_t sliceLen = info.sliceLen;
    if ( fh->isFatFileWithSlice(diag, info.fileContentLen, archs, sliceOffset, sliceLen, fatButMissingSlice) ) {
        // unmap anything before slice
        fileSystem.unloadPartialFile(info, sliceOffset, sliceLen);
        // Update the info to keep track of the new slice offset.
        info.sliceOffset = sliceOffset;
        info.sliceLen = sliceLen;
    }
    else if ( diag.hasError() ) {
        // We must have generated an error in the fat file parsing so use that error
        fileSystem.unloadFile(info);
        return closure::LoadedFileInfo();
    }
    else if ( fatButMissingSlice ) {
        diag.error("missing compatible arch in %s", path);
        fileSystem.unloadFile(info);
        return closure::LoadedFileInfo();
    }

    const MachOAnalyzer* mh = (MachOAnalyzer*)info.fileContent;

    // validate is mach-o of requested arch and platform
    if ( !mh->validMachOForArchAndPlatform(diag, (size_t)info.sliceLen, path, archs, platform) ) {
        fileSystem.unloadFile(info);
        return closure::LoadedFileInfo();
    }

    // if has zero-fill expansion, re-map
    mh = mh->remapIfZeroFill(diag, fileSystem, info);

    // on error, remove mappings and return nullptr
    if ( diag.hasError() ) {
        fileSystem.unloadFile(info);
        return closure::LoadedFileInfo();
    }

    // now that LINKEDIT is at expected offset, finish validation
    mh->validLinkedit(diag, path);

    // on error, remove mappings and return nullptr
    if ( diag.hasError() ) {
        fileSystem.unloadFile(info);
        return closure::LoadedFileInfo();
    }

    return info;
}

#if DEBUG
// only used in debug builds of cache builder to verify segment moves are valid
void MachOAnalyzer::validateDyldCacheDylib(Diagnostics& diag, const char* path) const
{
    validLinkedit(diag, path);
    validSegments(diag, path, 0xffffffff);
}
#endif

uint64_t MachOAnalyzer::mappedSize() const
{
    uint64_t vmSpace;
    bool     hasZeroFill;
    analyzeSegmentsLayout(vmSpace, hasZeroFill);
    return vmSpace;
}

bool MachOAnalyzer::validMachOForArchAndPlatform(Diagnostics& diag, size_t sliceLength, const char* path, const GradedArchs& archs, Platform platform) const
{
    // must start with mach-o magic value
    if ( (this->magic != MH_MAGIC) && (this->magic != MH_MAGIC_64) ) {
        diag.error("could not use '%s' because it is not a mach-o file: 0x%08X 0x%08X", path, this->magic, this->cputype);
        return false;
    }

    if ( !archs.grade(this->cputype, this->cpusubtype) ) {
        diag.error("could not use '%s' because it is not a compatible arch", path);
        return false;
    }

    // must be a filetype dyld can load
    switch ( this->filetype ) {
        case MH_EXECUTE:
        case MH_DYLIB:
        case MH_BUNDLE:
        case MH_DYLINKER:
           break;
        default:
            diag.error("could not use '%s' because it is not a dylib, bundle, or executable, filetype=0x%08X", path, this->filetype);
           return false;
    }

    // validate load commands structure
    if ( !this->validLoadCommands(diag, path, sliceLength) ) {
        return false;
    }

    // filter out static executables
    if ( (this->filetype == MH_EXECUTE) && !isDynamicExecutable() ) {
        diag.error("could not use '%s' because it is a static executable", path);
        return false;
    }

    // must match requested platform (do this after load commands are validated)
    if ( !this->supportsPlatform(platform) ) {
        diag.error("could not use '%s' because it was built for a different platform", path);
        return false;
    }

    // validate dylib loads
    if ( !validEmbeddedPaths(diag, platform, path) )
        return false;

    // validate segments
    if ( !validSegments(diag, path, sliceLength) )
        return false;

    // validate entry
    if ( this->filetype == MH_EXECUTE ) {
        if ( !validMain(diag, path) )
            return false;
    }

    // further validations done in validLinkedit()

    return true;
}

bool MachOAnalyzer::validLinkedit(Diagnostics& diag, const char* path) const
{
    // validate LINKEDIT layout
    if ( !validLinkeditLayout(diag, path) )
        return false;

    if ( hasChainedFixups() ) {
        if ( !validChainedFixupsInfo(diag, path) )
            return false;
    }
    else {
        // validate rebasing info
        if ( !validRebaseInfo(diag, path) )
            return false;

       // validate binding info
        if ( !validBindInfo(diag, path) )
            return false;
    }

    return true;
}

bool MachOAnalyzer::validLoadCommands(Diagnostics& diag, const char* path, size_t fileLen) const
{
    // check load command don't exceed file length
    if ( this->sizeofcmds + machHeaderSize() > fileLen ) {
        diag.error("in '%s' load commands exceed length of file", path);
        return false;
    }

    // walk all load commands and sanity check them
    Diagnostics walkDiag;
    forEachLoadCommand(walkDiag, ^(const load_command* cmd, bool& stop) {});
    if ( walkDiag.hasError() ) {
#if BUILDING_CACHE_BUILDER
        diag.error("in '%s' %s", path, walkDiag.errorMessage().c_str());
#else
        diag.error("in '%s' %s", path, walkDiag.errorMessage());
#endif
        return false;
    }

    // check load commands fit in TEXT segment
    __block bool foundTEXT    = false;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 ) {
            foundTEXT = true;
            if ( this->sizeofcmds + machHeaderSize() > info.fileSize ) {
                diag.error("in '%s' load commands exceed length of __TEXT segment", path);
            }
            if ( info.fileOffset != 0 ) {
                diag.error("in '%s' __TEXT segment not start of mach-o", path);
            }
            stop = true;
        }
    });
    if ( !diag.noError() && !foundTEXT ) {
        diag.error("in '%s' __TEXT segment not found", path);
        return false;
    }

    return true;
}

const MachOAnalyzer* MachOAnalyzer::remapIfZeroFill(Diagnostics& diag, const closure::FileSystem& fileSystem, closure::LoadedFileInfo& info) const
{
    uint64_t vmSpaceRequired;
    bool     hasZeroFill;
    analyzeSegmentsLayout(vmSpaceRequired, hasZeroFill);

    if ( hasZeroFill ) {
        vm_address_t newMappedAddr;
        if ( ::vm_allocate(mach_task_self(), &newMappedAddr, (size_t)vmSpaceRequired, VM_FLAGS_ANYWHERE) != 0 ) {
            diag.error("vm_allocate failure");
            return nullptr;
        }
        // re-map each segment read-only, with runtime layout
        uint64_t textSegVmAddr = preferredLoadAddress();
        forEachSegment(^(const SegmentInfo& segmentInfo, bool& stop) {
            if ( segmentInfo.fileSize != 0 ) {
                kern_return_t r = vm_copy(mach_task_self(), (vm_address_t)((long)info.fileContent+segmentInfo.fileOffset), (vm_size_t)segmentInfo.fileSize, (vm_address_t)(newMappedAddr+segmentInfo.vmAddr-textSegVmAddr));
                if ( r != KERN_SUCCESS ) {
                    diag.error("vm_copy() failure");
                    stop = true;
                }
            }
        });
        if ( diag.noError() ) {
            // remove original mapping and return new mapping
            fileSystem.unloadFile(info);

            // make the new mapping read-only
            ::vm_protect(mach_task_self(), newMappedAddr, (vm_size_t)vmSpaceRequired, false, VM_PROT_READ);

            // Set vm_deallocate as the unload method.
            info.unload = [](const closure::LoadedFileInfo& info) {
                ::vm_deallocate(mach_task_self(), (vm_address_t)info.fileContent, (size_t)info.fileContentLen);
            };

            // And update the file content to the new location
            info.fileContent = (const void*)newMappedAddr;
            info.fileContentLen = vmSpaceRequired;
            return (const MachOAnalyzer*)info.fileContent;
        }
        else {
            // new mapping failed, return old mapping with an error in diag
            ::vm_deallocate(mach_task_self(), newMappedAddr, (size_t)vmSpaceRequired);
            return nullptr;
        }
    }

    return this;
}

void MachOAnalyzer::analyzeSegmentsLayout(uint64_t& vmSpace, bool& hasZeroFill) const
{
    __block bool     writeExpansion = false;
    __block uint64_t lowestVmAddr   = 0xFFFFFFFFFFFFFFFFULL;
    __block uint64_t highestVmAddr  = 0;
    __block uint64_t sumVmSizes     = 0;
    forEachSegment(^(const SegmentInfo& segmentInfo, bool& stop) {
        if ( strcmp(segmentInfo.segName, "__PAGEZERO") == 0 )
            return;
        if ( segmentInfo.writable() && (segmentInfo.fileSize !=  segmentInfo.vmSize) )
            writeExpansion = true; // zerofill at end of __DATA
        if ( segmentInfo.vmAddr < lowestVmAddr )
            lowestVmAddr = segmentInfo.vmAddr;
        if ( segmentInfo.vmAddr+segmentInfo.vmSize > highestVmAddr )
            highestVmAddr = segmentInfo.vmAddr+segmentInfo.vmSize;
        sumVmSizes += segmentInfo.vmSize;
    });
    uint64_t totalVmSpace = (highestVmAddr - lowestVmAddr);
    // LINKEDIT vmSize is not required to be a multiple of page size.  Round up if that is the case
    const uint64_t pageSize = uses16KPages() ? 0x4000 : 0x1000;
    totalVmSpace = (totalVmSpace + (pageSize - 1)) & ~(pageSize - 1);
    bool hasHole = (totalVmSpace != sumVmSizes); // segments not contiguous

    vmSpace     = totalVmSpace;
    hasZeroFill = writeExpansion || hasHole;
}

bool MachOAnalyzer::enforceFormat(Malformed kind) const
{
    __block bool result = false;
    forEachSupportedPlatform(^(Platform platform, uint32_t minOS, uint32_t sdk) {
        switch (platform) {
        case Platform::macOS:
            switch (kind) {
            case Malformed::linkeditOrder:
            case Malformed::linkeditAlignment:
            case Malformed::dyldInfoAndlocalRelocs:
                // enforce these checks on new binaries only
                if (sdk >= 0x000A0E00) // macOS 10.14
                    result = true;
                break;
            case Malformed::segmentOrder:
            case Malformed::linkeditPermissions:
            case Malformed::textPermissions:
            case Malformed::executableData:
            case Malformed::codeSigAlignment:
                // enforce these checks on new binaries only
                if (sdk >= 0x000A0F00) // macOS 10.15
                    result = true;
                break;
            }
            break;
        case Platform::iOS:
            switch (kind) {
            case Malformed::linkeditOrder:
            case Malformed::dyldInfoAndlocalRelocs:
            case Malformed::textPermissions:
            case Malformed::executableData:
                result = true;
                break;
            case Malformed::linkeditAlignment:
            case Malformed::segmentOrder:
            case Malformed::linkeditPermissions:
            case Malformed::codeSigAlignment:
                // enforce these checks on new binaries only
                if (sdk >= 0x000D0000) // iOS 13
                    result = true;
                break;
            }
            break;
        default:
            result = true;
            break;
        }
    });
    // if binary is so old, there is no platform info, don't enforce malformed errors
    return result;
}

bool MachOAnalyzer::validEmbeddedPaths(Diagnostics& diag, Platform platform, const char* path) const
{
    __block int  index = 1;
    __block bool allGood = true;
    __block bool foundInstallName = false;
    __block int  dependentsCount = 0;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        const dylib_command* dylibCmd;
        const rpath_command* rpathCmd;
        switch ( cmd->cmd ) {
            case LC_ID_DYLIB:
                foundInstallName = true;
                // fall through
                [[clang::fallthrough]];
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_REEXPORT_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
                dylibCmd = (dylib_command*)cmd;
                if ( dylibCmd->dylib.name.offset > cmd->cmdsize ) {
                    diag.error("in '%s' load command #%d name offset (%u) outside its size (%u)", path, index, dylibCmd->dylib.name.offset, cmd->cmdsize);
                    stop = true;
                    allGood = false;
                }
                else {
                    bool foundEnd = false;
                    const char* start = (char*)dylibCmd + dylibCmd->dylib.name.offset;
                    const char* end   = (char*)dylibCmd + cmd->cmdsize;
                    for (const char* s=start; s < end; ++s) {
                        if ( *s == '\0' ) {
                            foundEnd = true;
                            break;
                        }
                    }
                    if ( !foundEnd ) {
                        diag.error("in '%s' load command #%d string extends beyond end of load command", path, index);
                        stop = true;
                        allGood = false;
                    }
                }
                if ( cmd->cmd  != LC_ID_DYLIB )
                    ++dependentsCount;
                break;
            case LC_RPATH:
                rpathCmd = (rpath_command*)cmd;
                if ( rpathCmd->path.offset > cmd->cmdsize ) {
                    diag.error("in '%s' load command #%d path offset (%u) outside its size (%u)", path, index, rpathCmd->path.offset, cmd->cmdsize);
                    stop = true;
                    allGood = false;
                }
                else {
                    bool foundEnd = false;
                    const char* start = (char*)rpathCmd + rpathCmd->path.offset;
                    const char* end   = (char*)rpathCmd + cmd->cmdsize;
                    for (const char* s=start; s < end; ++s) {
                        if ( *s == '\0' ) {
                            foundEnd = true;
                            break;
                        }
                    }
                    if ( !foundEnd ) {
                        diag.error("in '%s' load command #%d string extends beyond end of load command", path, index);
                        stop = true;
                        allGood = false;
                    }
                }
                break;
        }
        ++index;
    });
    if ( !allGood )
        return false;

    if ( this->filetype == MH_DYLIB ) {
        if ( !foundInstallName ) {
            diag.error("in '%s' MH_DYLIB is missing LC_ID_DYLIB", path);
            return false;
        }
    }
    else {
        if ( foundInstallName ) {
            diag.error("in '%s' LC_ID_DYLIB found in non-MH_DYLIB", path);
            return false;
        }
    }

    if ( (dependentsCount == 0) && (this->filetype == MH_EXECUTE)  ) {
        diag.error("in '%s' missing LC_LOAD_DYLIB (must link with at least libSystem.dylib)", path);
        return false;
    }

    return true;
}

bool MachOAnalyzer::validSegments(Diagnostics& diag, const char* path, size_t fileLen) const
{
    // check segment load command size
    __block bool badSegmentLoadCommand = false;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_SEGMENT_64 ) {
            const segment_command_64* seg = (segment_command_64*)cmd;
            int32_t sectionsSpace = cmd->cmdsize - sizeof(segment_command_64);
            if ( sectionsSpace < 0 ) {
               diag.error("in '%s' load command size too small for LC_SEGMENT_64", path);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( (sectionsSpace % sizeof(section_64)) != 0 ) {
               diag.error("in '%s' segment load command size 0x%X will not fit whole number of sections", path, cmd->cmdsize);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( sectionsSpace != (seg->nsects * sizeof(section_64)) ) {
               diag.error("in '%s' load command size 0x%X does not match nsects %d", path, cmd->cmdsize, seg->nsects);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( greaterThanAddOrOverflow(seg->fileoff, seg->filesize, fileLen) ) {
                diag.error("in '%s' segment load command content extends beyond end of file", path);
                badSegmentLoadCommand = true;
                stop = true;
            }
            else if ( (seg->filesize > seg->vmsize) && ((seg->vmsize != 0) || ((seg->flags & SG_NORELOC) == 0)) ) {
                // <rdar://problem/19986776> dyld should support non-allocatable __LLVM segment
                diag.error("in '%s' segment filesize exceeds vmsize", path);
                badSegmentLoadCommand = true;
                stop = true;
            }
        }
        else if ( cmd->cmd == LC_SEGMENT ) {
            const segment_command* seg = (segment_command*)cmd;
            int32_t sectionsSpace = cmd->cmdsize - sizeof(segment_command);
            if ( sectionsSpace < 0 ) {
               diag.error("in '%s' load command size too small for LC_SEGMENT", path);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( (sectionsSpace % sizeof(section)) != 0 ) {
               diag.error("in '%s' segment load command size 0x%X will not fit whole number of sections", path, cmd->cmdsize);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( sectionsSpace != (seg->nsects * sizeof(section)) ) {
               diag.error("in '%s' load command size 0x%X does not match nsects %d", path, cmd->cmdsize, seg->nsects);
               badSegmentLoadCommand = true;
               stop = true;
            }
            else if ( (seg->filesize > seg->vmsize) && ((seg->vmsize != 0) || ((seg->flags & SG_NORELOC) == 0)) ) {
                // <rdar://problem/19986776> dyld should support non-allocatable __LLVM segment
                diag.error("in '%s' segment filesize exceeds vmsize", path);
                badSegmentLoadCommand = true;
                stop = true;
            }
        }
    });
     if ( badSegmentLoadCommand )
         return false;

    // check mapping permissions of segments
    __block bool badPermissions = false;
    __block bool badSize        = false;
    __block bool hasTEXT        = false;
    __block bool hasLINKEDIT    = false;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 ) {
            if ( (info.protections != (VM_PROT_READ|VM_PROT_EXECUTE)) && enforceFormat(Malformed::textPermissions) ) {
                diag.error("in '%s' __TEXT segment permissions is not 'r-x'", path);
                badPermissions = true;
                stop = true;
            }
            hasTEXT = true;
        }
        else if ( strcmp(info.segName, "__LINKEDIT") == 0 ) {
            if ( (info.protections != VM_PROT_READ) && enforceFormat(Malformed::linkeditPermissions) ) {
                diag.error("in '%s' __LINKEDIT segment permissions is not 'r--'", path);
                badPermissions = true;
                stop = true;
            }
            hasLINKEDIT = true;
        }
        else if ( (info.protections & 0xFFFFFFF8) != 0 ) {
            diag.error("in '%s' %s segment permissions has invalid bits set", path, info.segName);
            badPermissions = true;
            stop = true;
        }
        if ( greaterThanAddOrOverflow(info.fileOffset, info.fileSize, fileLen) ) {
            diag.error("in '%s' %s segment content extends beyond end of file", path, info.segName);
            badSize = true;
            stop = true;
        }
        if ( is64() ) {
            if ( info.vmAddr+info.vmSize < info.vmAddr ) {
                diag.error("in '%s' %s segment vm range wraps", path, info.segName);
                badSize = true;
                stop = true;
            }
       }
       else {
            if ( (uint32_t)(info.vmAddr+info.vmSize) < (uint32_t)(info.vmAddr) ) {
                diag.error("in '%s' %s segment vm range wraps", path, info.segName);
                badSize = true;
                stop = true;
            }
       }
    });
    if ( badPermissions || badSize )
        return false;
    if ( !hasTEXT ) {
       diag.error("in '%s' missing __TEXT segment", path);
       return false;
    }
    if ( !hasLINKEDIT ) {
       diag.error("in '%s' missing __LINKEDIT segment", path);
       return false;
    }

    // check for overlapping segments
    __block bool badSegments = false;
    forEachSegment(^(const SegmentInfo& info1, bool& stop1) {
        uint64_t seg1vmEnd   = info1.vmAddr + info1.vmSize;
        uint64_t seg1FileEnd = info1.fileOffset + info1.fileSize;
        forEachSegment(^(const SegmentInfo& info2, bool& stop2) {
            if ( info1.segIndex == info2.segIndex )
                return;
            uint64_t seg2vmEnd   = info2.vmAddr + info2.vmSize;
            uint64_t seg2FileEnd = info2.fileOffset + info2.fileSize;
            if ( ((info2.vmAddr <= info1.vmAddr) && (seg2vmEnd > info1.vmAddr) && (seg1vmEnd > info1.vmAddr )) || ((info2.vmAddr >= info1.vmAddr ) && (info2.vmAddr < seg1vmEnd) && (seg2vmEnd > info2.vmAddr)) ) {
                diag.error("in '%s' segment %s vm range overlaps segment %s", path, info1.segName, info2.segName);
                badSegments = true;
                stop1 = true;
                stop2 = true;
            }
             if ( ((info2.fileOffset  <= info1.fileOffset) && (seg2FileEnd > info1.fileOffset) && (seg1FileEnd > info1.fileOffset)) || ((info2.fileOffset  >= info1.fileOffset) && (info2.fileOffset  < seg1FileEnd) && (seg2FileEnd > info2.fileOffset )) ) {
                diag.error("in '%s' segment %s file content overlaps segment %s", path, info1.segName, info2.segName);
                badSegments = true;
                stop1 = true;
                stop2 = true;
            }
            if ( (info1.segIndex < info2.segIndex) && !stop1 ) {
                if ( (info1.vmAddr > info2.vmAddr) || ((info1.fileOffset > info2.fileOffset ) && (info1.fileOffset != 0) && (info2.fileOffset  != 0)) ){
                    if ( !inDyldCache() && enforceFormat(Malformed::segmentOrder) ) {
                        // dyld cache __DATA_* segments are moved around
                        diag.error("in '%s' segment load commands out of order with respect to layout for %s and %s", path, info1.segName, info2.segName);
                        badSegments = true;
                        stop1 = true;
                        stop2 = true;
                    }
                }
            }
        });
    });
    if ( badSegments )
        return false;

    // check sections are within segment
    __block bool badSections = false;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_SEGMENT_64 ) {
            const segment_command_64* seg = (segment_command_64*)cmd;
            const section_64* const sectionsStart = (section_64*)((char*)seg + sizeof(struct segment_command_64));
            const section_64* const sectionsEnd   = &sectionsStart[seg->nsects];
            for (const section_64* sect=sectionsStart; (sect < sectionsEnd); ++sect) {
                if ( (int64_t)(sect->size) < 0 ) {
                    diag.error("in '%s' section %s size too large 0x%llX", path, sect->sectname, sect->size);
                    badSections = true;
                }
                else if ( sect->addr < seg->vmaddr ) {
                    diag.error("in '%s' section %s start address 0x%llX is before containing segment's address 0x%0llX", path, sect->sectname, sect->addr, seg->vmaddr);
                    badSections = true;
                }
                else if ( sect->addr+sect->size > seg->vmaddr+seg->vmsize ) {
                    diag.error("in '%s' section %s end address 0x%llX is beyond containing segment's end address 0x%0llX", path, sect->sectname, sect->addr+sect->size, seg->vmaddr+seg->vmsize);
                    badSections = true;
                }
            }
        }
        else if ( cmd->cmd == LC_SEGMENT ) {
            const segment_command* seg = (segment_command*)cmd;
            const section* const sectionsStart = (section*)((char*)seg + sizeof(struct segment_command));
            const section* const sectionsEnd   = &sectionsStart[seg->nsects];
            for (const section* sect=sectionsStart; !stop && (sect < sectionsEnd); ++sect) {
               if ( (int64_t)(sect->size) < 0 ) {
                    diag.error("in '%s' section %s size too large 0x%X", path, sect->sectname, sect->size);
                    badSections = true;
                }
                else if ( sect->addr < seg->vmaddr ) {
                    diag.error("in '%s' section %s start address 0x%X is before containing segment's address 0x%0X", path,  sect->sectname, sect->addr, seg->vmaddr);
                    badSections = true;
                }
                else if ( sect->addr+sect->size > seg->vmaddr+seg->vmsize ) {
                    diag.error("in '%s' section %s end address 0x%X is beyond containing segment's end address 0x%0X", path, sect->sectname, sect->addr+sect->size, seg->vmaddr+seg->vmsize);
                    badSections = true;
                }
            }
        }
    });

    return !badSections;
}


bool MachOAnalyzer::validMain(Diagnostics& diag, const char* path) const
{
    __block uint64_t textSegStartAddr = 0;
    __block uint64_t textSegStartSize = 0;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 ) {
            textSegStartAddr = info.vmAddr;
            textSegStartSize = info.vmSize;
            stop = true;
       }
    });

    __block int mainCount   = 0;
    __block int threadCount = 0;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        entry_point_command* mainCmd;
        uint64_t startAddress;
        switch (cmd->cmd) {
            case LC_MAIN:
                ++mainCount;
                mainCmd = (entry_point_command*)cmd;
                if ( mainCmd->entryoff >= textSegStartSize ) {
                    startAddress = preferredLoadAddress() + mainCmd->entryoff;
                    __block bool foundSegment = false;
                    forEachSegment(^(const SegmentInfo& info, bool& stopSegment) {
                        // Skip segments which don't contain this address
                        if ( (startAddress < info.vmAddr) || (startAddress >= info.vmAddr+info.vmSize) )
                            return;
                        foundSegment = true;
                        if ( (info.protections & VM_PROT_EXECUTE) == 0 )
                            diag.error("LC_MAIN points to non-executable segment");
                        stopSegment = true;
                    });
                    if (!foundSegment)
                        diag.error("LC_MAIN entryoff is out of range");
                    stop = true;
                }
                break;
            case LC_UNIXTHREAD:
                ++threadCount;
                startAddress = entryAddrFromThreadCmd((thread_command*)cmd);
                if ( startAddress == 0 ) {
                    diag.error("LC_UNIXTHREAD not valid for arch %s", archName());
                    stop = true;
                }
                else if ( (startAddress < textSegStartAddr) || (startAddress >= textSegStartAddr+textSegStartSize) ) {
                    diag.error("LC_UNIXTHREAD entry not in __TEXT segment");
                    stop = true;
                }
                break;
        }
    });
    if ( diag.hasError() )
        return false;
    if ( diag.noError() && (mainCount+threadCount == 1) )
        return true;

    if ( mainCount + threadCount == 0 )
        diag.error("missing LC_MAIN or LC_UNIXTHREAD");
    else
        diag.error("only one LC_MAIN or LC_UNIXTHREAD is allowed");
    return false;
}


namespace {
    struct LinkEditContentChunk
    {
        const char* name;
        uint32_t    alignment;
        uint32_t    fileOffsetStart;
        uint32_t    size;

        static int compareByFileOffset(const void* l, const void* r) {
            if ( ((LinkEditContentChunk*)l)->fileOffsetStart < ((LinkEditContentChunk*)r)->fileOffsetStart )
                return -1;
            else
                return 1;
        }
    };
} // anonymous namespace



bool MachOAnalyzer::validLinkeditLayout(Diagnostics& diag, const char* path) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return false;
    const uint32_t ptrSize = pointerSize();

    // build vector of all blobs in LINKEDIT
    LinkEditContentChunk blobs[32];
    LinkEditContentChunk* bp = blobs;
    if ( leInfo.dyldInfo != nullptr ) {
        if ( leInfo.dyldInfo->rebase_size != 0 )
            *bp++ = {"rebase opcodes",          ptrSize, leInfo.dyldInfo->rebase_off, leInfo.dyldInfo->rebase_size};
        if ( leInfo.dyldInfo->bind_size != 0 )
            *bp++ = {"bind opcodes",            ptrSize, leInfo.dyldInfo->bind_off, leInfo.dyldInfo->bind_size};
        if ( leInfo.dyldInfo->weak_bind_size != 0 )
            *bp++ = {"weak bind opcodes",       ptrSize, leInfo.dyldInfo->weak_bind_off, leInfo.dyldInfo->weak_bind_size};
        if ( leInfo.dyldInfo->lazy_bind_size != 0 )
            *bp++ = {"lazy bind opcodes",       ptrSize, leInfo.dyldInfo->lazy_bind_off, leInfo.dyldInfo->lazy_bind_size};
        if ( leInfo.dyldInfo->export_size!= 0 )
            *bp++ = {"exports trie",            ptrSize, leInfo.dyldInfo->export_off, leInfo.dyldInfo->export_size};
    }
    if ( leInfo.exportsTrie != nullptr ) {
        if ( leInfo.exportsTrie->datasize != 0 )
            *bp++ = {"exports trie",            ptrSize, leInfo.exportsTrie->dataoff, leInfo.exportsTrie->datasize};
    }
    
    if ( leInfo.dynSymTab != nullptr ) {
        if ( leInfo.dynSymTab->nlocrel != 0 )
            *bp++ = {"local relocations",       ptrSize, leInfo.dynSymTab->locreloff, static_cast<uint32_t>(leInfo.dynSymTab->nlocrel*sizeof(relocation_info))};
        if ( leInfo.dynSymTab->nextrel != 0 )
            *bp++ = {"external relocations",    ptrSize, leInfo.dynSymTab->extreloff, static_cast<uint32_t>(leInfo.dynSymTab->nextrel*sizeof(relocation_info))};
        if ( leInfo.dynSymTab->nindirectsyms != 0 )
            *bp++ = {"indirect symbol table",   4,       leInfo.dynSymTab->indirectsymoff, leInfo.dynSymTab->nindirectsyms*4};
    }
    if ( leInfo.splitSegInfo != nullptr ) {
        if ( leInfo.splitSegInfo->datasize != 0 )
            *bp++ = {"shared cache info",       ptrSize, leInfo.splitSegInfo->dataoff, leInfo.splitSegInfo->datasize};
    }
    if ( leInfo.functionStarts != nullptr ) {
        if ( leInfo.functionStarts->datasize != 0 )
            *bp++ = {"function starts",         ptrSize, leInfo.functionStarts->dataoff, leInfo.functionStarts->datasize};
    }
    if ( leInfo.dataInCode != nullptr ) {
        if ( leInfo.dataInCode->datasize != 0 )
            *bp++ = {"data in code",            ptrSize, leInfo.dataInCode->dataoff, leInfo.dataInCode->datasize};
    }
    if ( leInfo.symTab != nullptr ) {
        if ( leInfo.symTab->nsyms != 0 )
            *bp++ = {"symbol table",            ptrSize, leInfo.symTab->symoff, static_cast<uint32_t>(leInfo.symTab->nsyms*(ptrSize == 8 ? sizeof(nlist_64) : sizeof(struct nlist)))};
        if ( leInfo.symTab->strsize != 0 )
            *bp++ = {"symbol table strings",    1,       leInfo.symTab->stroff, leInfo.symTab->strsize};
    }
    if ( leInfo.codeSig != nullptr ) {
        if ( leInfo.codeSig->datasize != 0 )
            *bp++ = {"code signature",          ptrSize, leInfo.codeSig->dataoff, leInfo.codeSig->datasize};
    }

    // check for bad combinations
    if ( (leInfo.dyldInfo != nullptr) && (leInfo.dyldInfo->cmd == LC_DYLD_INFO_ONLY) && (leInfo.dynSymTab != nullptr) ) {
        if ( (leInfo.dynSymTab->nlocrel != 0) && enforceFormat(Malformed::dyldInfoAndlocalRelocs) ) {
            diag.error("in '%s' malformed mach-o contains LC_DYLD_INFO_ONLY and local relocations", path);
            return false;
        }
        if ( leInfo.dynSymTab->nextrel != 0 ) {
            diag.error("in '%s' malformed mach-o contains LC_DYLD_INFO_ONLY and external relocations", path);
            return false;
        }
    }
    if ( (leInfo.dyldInfo == nullptr) && (leInfo.dynSymTab == nullptr) ) {
        diag.error("in '%s' malformed mach-o misssing LC_DYLD_INFO and LC_DYSYMTAB", path);
        return false;
    }
    const unsigned long blobCount = bp - blobs;
    if ( blobCount == 0 ) {
        diag.error("in '%s' malformed mach-o misssing LINKEDIT", path);
        return false;
    }

    uint32_t linkeditFileEnd = leInfo.layout.linkeditFileOffset + leInfo.layout.linkeditFileSize;


    // sort blobs by file-offset and error on overlaps
    ::qsort(blobs, blobCount, sizeof(LinkEditContentChunk), &LinkEditContentChunk::compareByFileOffset);
    uint32_t     prevEnd = leInfo.layout.linkeditFileOffset;
    const char*  prevName = "start of LINKEDIT";
    for (unsigned long i=0; i < blobCount; ++i) {
        const LinkEditContentChunk& blob = blobs[i];
        if ( blob.fileOffsetStart < prevEnd ) {
            diag.error("in '%s' LINKEDIT overlap of %s and %s", path, prevName, blob.name);
            return false;
        }
        if (greaterThanAddOrOverflow(blob.fileOffsetStart, blob.size, linkeditFileEnd)) {
            diag.error("in '%s' LINKEDIT content '%s' extends beyond end of segment", path, blob.name);
            return false;
        }
        if ( (blob.fileOffsetStart & (blob.alignment-1)) != 0 ) {
            // <rdar://problem/51115705> relax code sig alignment for pre iOS13
            Malformed kind = (strcmp(blob.name, "code signature") == 0) ? Malformed::codeSigAlignment : Malformed::linkeditAlignment;
            if ( enforceFormat(kind) )
                diag.error("in '%s' mis-aligned LINKEDIT content '%s'", path, blob.name);
        }
        prevEnd  = blob.fileOffsetStart + blob.size;
        prevName = blob.name;
    }

    // Check for invalid symbol table sizes
    if ( leInfo.symTab != nullptr ) {
        if ( leInfo.symTab->nsyms > 0x10000000 ) {
            diag.error("in '%s' malformed mach-o image: symbol table too large", path);
            return false;
        }
        if ( leInfo.dynSymTab != nullptr ) {
            // validate indirect symbol table
            if ( leInfo.dynSymTab->nindirectsyms != 0 ) {
                if ( leInfo.dynSymTab->nindirectsyms > 0x10000000 ) {
                    diag.error("in '%s' malformed mach-o image: indirect symbol table too large", path);
                    return false;
                }
            }
            if ( (leInfo.dynSymTab->nlocalsym > leInfo.symTab->nsyms) || (leInfo.dynSymTab->ilocalsym > leInfo.symTab->nsyms) ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table local symbol count exceeds total symbols", path);
                return false;
            }
            if ( leInfo.dynSymTab->ilocalsym + leInfo.dynSymTab->nlocalsym < leInfo.dynSymTab->ilocalsym  ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table local symbol count wraps", path);
                return false;
            }
            if ( (leInfo.dynSymTab->nextdefsym > leInfo.symTab->nsyms) || (leInfo.dynSymTab->iextdefsym > leInfo.symTab->nsyms) ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table extern symbol count exceeds total symbols", path);
                return false;
            }
            if ( leInfo.dynSymTab->iextdefsym + leInfo.dynSymTab->nextdefsym < leInfo.dynSymTab->iextdefsym  ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table extern symbol count wraps", path);
                return false;
            }
            if ( (leInfo.dynSymTab->nundefsym > leInfo.symTab->nsyms) || (leInfo.dynSymTab->iundefsym > leInfo.symTab->nsyms) ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table undefined symbol count exceeds total symbols", path);
                return false;
            }
            if ( leInfo.dynSymTab->iundefsym + leInfo.dynSymTab->nundefsym < leInfo.dynSymTab->iundefsym  ) {
                diag.error("in '%s' malformed mach-o image: indirect symbol table undefined symbol count wraps", path);
                return false;
            }
        }
    }

    return true;
}



bool MachOAnalyzer::invalidRebaseState(Diagnostics& diag, const char* opcodeName, const char* path, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                      bool segIndexSet, uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type) const
{
    if ( !segIndexSet ) {
        diag.error("in '%s' %s missing preceding REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB", path, opcodeName);
        return true;
    }
    if ( segmentIndex >= leInfo.layout.linkeditSegIndex )  {
        diag.error("in '%s' %s segment index %d too large", path, opcodeName, segmentIndex);
        return true;
    }
    if ( segmentOffset > (segments[segmentIndex].vmSize-ptrSize) ) {
        diag.error("in '%s' %s current segment offset 0x%08llX beyond segment size (0x%08llX)", path, opcodeName, segmentOffset, segments[segmentIndex].vmSize);
        return true;
    }
    switch ( type )  {
        case REBASE_TYPE_POINTER:
            if ( !segments[segmentIndex].writable() ) {
                diag.error("in '%s' %s pointer rebase is in non-writable segment", path, opcodeName);
                return true;
            }
            if ( segments[segmentIndex].executable() && enforceFormat(Malformed::executableData) ) {
                diag.error("in '%s' %s pointer rebase is in executable segment", path, opcodeName);
                return true;
            }
            break;
        case REBASE_TYPE_TEXT_ABSOLUTE32:
        case REBASE_TYPE_TEXT_PCREL32:
            if ( !segments[segmentIndex].textRelocs ) {
                diag.error("in '%s' %s text rebase is in segment that does not support text relocations", path, opcodeName);
                return true;
            }
            if ( segments[segmentIndex].writable() ) {
                diag.error("in '%s' %s text rebase is in writable segment", path, opcodeName);
                return true;
            }
            if ( !segments[segmentIndex].executable() ) {
                diag.error("in '%s' %s pointer rebase is in non-executable segment", path, opcodeName);
                return true;
            }
            break;
        default:
            diag.error("in '%s' %s unknown rebase type %d", path, opcodeName, type);
            return true;
    }
    return false;
}


void MachOAnalyzer::getAllSegmentsInfos(Diagnostics& diag, SegmentInfo segments[]) const
{
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        segments[info.segIndex] = info;
    });
}


bool MachOAnalyzer::validRebaseInfo(Diagnostics& diag, const char* path) const
{
    forEachRebase(diag, ^(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                          bool segIndexSet, uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, bool& stop) {
        if ( invalidRebaseState(diag, opcodeName, path, leInfo, segments, segIndexSet, ptrSize, segmentIndex, segmentOffset, type) )
            stop = true;
    });
    return diag.noError();
}


void MachOAnalyzer::forEachTextRebase(Diagnostics& diag, void (^handler)(uint64_t runtimeOffset, bool& stop)) const
{
    __block bool     startVmAddrSet = false;
    __block uint64_t startVmAddr    = 0;
    forEachRebase(diag, ^(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                          bool segIndexSet, uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, bool& stop) {
        if ( type != REBASE_TYPE_TEXT_ABSOLUTE32 )
            return;
        if ( !startVmAddrSet ) {
            for (int i=0; i <= segmentIndex; ++i) {
                if ( strcmp(segments[i].segName, "__TEXT") == 0 ) {
                    startVmAddr = segments[i].vmAddr;
                    startVmAddrSet = true;
                    break;
               }
            }
        }
        uint64_t rebaseVmAddr  = segments[segmentIndex].vmAddr + segmentOffset;
        uint64_t runtimeOffset = rebaseVmAddr - startVmAddr;
        handler(runtimeOffset, stop);
    });
}


void MachOAnalyzer::forEachRebase(Diagnostics& diag, bool ignoreLazyPointers, void (^handler)(uint64_t runtimeOffset, bool& stop)) const
{
    __block bool     startVmAddrSet = false;
    __block uint64_t startVmAddr    = 0;
    __block uint64_t lpVmAddr       = 0;
    __block uint64_t lpEndVmAddr    = 0;
    __block uint64_t shVmAddr       = 0;
    __block uint64_t shEndVmAddr    = 0;
    if ( ignoreLazyPointers ) {
        forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool &stop) {
            if ( (info.sectFlags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS ) {
                lpVmAddr    = info.sectAddr;
                lpEndVmAddr = info.sectAddr + info.sectSize;
            }
            else if ( (info.sectFlags & S_ATTR_PURE_INSTRUCTIONS) && (strcmp(info.sectName, "__stub_helper") == 0) ) {
                shVmAddr    = info.sectAddr;
                shEndVmAddr = info.sectAddr + info.sectSize;
            }
        });
    }
    forEachRebase(diag, ^(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                          bool segIndexSet, uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, bool& stop) {
        if ( type != REBASE_TYPE_POINTER )
            return;
        if ( !startVmAddrSet ) {
            for (int i=0; i < segmentIndex; ++i) {
                if ( strcmp(segments[i].segName, "__TEXT") == 0 ) {
                    startVmAddr = segments[i].vmAddr;
                    startVmAddrSet = true;
                    break;
               }
            }
        }
        uint64_t rebaseVmAddr  = segments[segmentIndex].vmAddr + segmentOffset;
        bool skipRebase = false;
        if ( (rebaseVmAddr >= lpVmAddr) && (rebaseVmAddr < lpEndVmAddr) ) {
            // rebase is in lazy pointer section
            uint64_t lpValue = 0;
            if ( ptrSize == 8 )
                lpValue = *((uint64_t*)(rebaseVmAddr-startVmAddr+(uint8_t*)this));
            else
                lpValue = *((uint32_t*)(rebaseVmAddr-startVmAddr+(uint8_t*)this));
            if ( (lpValue >= shVmAddr) && (lpValue < shEndVmAddr) ) {
                // content is into stub_helper section
                uint64_t lpTargetImageOffset = lpValue - startVmAddr;
                const uint8_t* helperContent = (uint8_t*)this + lpTargetImageOffset;
                bool isLazyStub = contentIsRegularStub(helperContent);
                // ignore rebases for normal lazy pointers, but leave rebase for resolver helper stub
                if ( isLazyStub )
                    skipRebase = true;
            }
            else {
                // if lazy pointer does not point into stub_helper, then it points to weak-def symbol and we need rebase
            }
        }
        if ( !skipRebase ) {
            uint64_t runtimeOffset = rebaseVmAddr - startVmAddr;
            handler(runtimeOffset, stop);
        }
    });
}


bool MachOAnalyzer::contentIsRegularStub(const uint8_t* helperContent) const
{
    switch (this->cputype) {
        case CPU_TYPE_X86_64:
            return ( (helperContent[0] == 0x68) && (helperContent[5] == 0xE9) ); // push $xxx / JMP pcRel
        case CPU_TYPE_I386:
            return ( (helperContent[0] == 0x68) && (helperContent[5] == 0xFF) && (helperContent[2] == 0x26) ); // push $xxx / JMP *pcRel
        case CPU_TYPE_ARM:
            return ( (helperContent[0] == 0x00) && (helperContent[1] == 0xC0) && (helperContent[2] == 0x9F) && (helperContent[3] == 0xE5) ); // ldr  ip, [pc, #0]
        case CPU_TYPE_ARM64:
            return ( (helperContent[0] == 0x50) && (helperContent[1] == 0x00) && (helperContent[2] == 0x00) && (helperContent[3] == 0x18) ); // ldr w16, L0

    }
    return false;
}

static int uint32Sorter(const void* l, const void* r) {
    if ( *((uint32_t*)l) < *((uint32_t*)r) )
        return -1;
    else
        return 1;
}


void MachOAnalyzer::forEachRebase(Diagnostics& diag,
                                 void (^handler)(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                                 bool segIndexSet, uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset,
                                                 uint8_t type, bool& stop)) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    BLOCK_ACCCESSIBLE_ARRAY(SegmentInfo, segmentsInfo, leInfo.layout.linkeditSegIndex+1);
    getAllSegmentsInfos(diag, segmentsInfo);
    if ( diag.hasError() )
        return;

    if ( leInfo.dyldInfo != nullptr ) {
        const uint8_t* const start = getLinkEditContent(leInfo.layout, leInfo.dyldInfo->rebase_off);
        const uint8_t* const end   = start + leInfo.dyldInfo->rebase_size;
        const uint8_t* p           = start;
        const uint32_t ptrSize     = pointerSize();
        uint8_t  type = 0;
        int      segIndex = 0;
        uint64_t segOffset = 0;
        uint64_t count;
        uint64_t skip;
        bool     segIndexSet = false;
        bool     stop = false;
        while ( !stop && diag.noError() && (p < end) ) {
            uint8_t immediate = *p & REBASE_IMMEDIATE_MASK;
            uint8_t opcode = *p & REBASE_OPCODE_MASK;
            ++p;
            switch (opcode) {
                case REBASE_OPCODE_DONE:
                    if ( (end - p) > 8 )
                        diag.error("rebase opcodes terminated early at offset %d of %d", (int)(p-start), (int)(end-start));
                    stop = true;
                    break;
                case REBASE_OPCODE_SET_TYPE_IMM:
                    type = immediate;
                    break;
                case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                    segIndex = immediate;
                    segOffset = read_uleb128(diag, p, end);
                    segIndexSet = true;
                    break;
                case REBASE_OPCODE_ADD_ADDR_ULEB:
                    segOffset += read_uleb128(diag, p, end);
                    break;
                case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
                    segOffset += immediate*ptrSize;
                    break;
                case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
                    for (int i=0; i < immediate; ++i) {
                        handler("REBASE_OPCODE_DO_REBASE_IMM_TIMES", leInfo, segmentsInfo, segIndexSet, ptrSize, segIndex, segOffset, type, stop);
                        segOffset += ptrSize;
                        if ( stop )
                            break;
                    }
                    break;
                case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
                    count = read_uleb128(diag, p, end);
                    for (uint32_t i=0; i < count; ++i) {
                        handler("REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB", leInfo, segmentsInfo, segIndexSet, ptrSize, segIndex, segOffset, type, stop);
                        segOffset += ptrSize;
                        if ( stop )
                            break;
                    }
                    break;
                case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
                    handler("REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB", leInfo, segmentsInfo, segIndexSet, ptrSize, segIndex, segOffset, type, stop);
                    segOffset += read_uleb128(diag, p, end) + ptrSize;
                    break;
                case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
                    count = read_uleb128(diag, p, end);
                    if ( diag.hasError() )
                        break;
                    skip = read_uleb128(diag, p, end);
                    for (uint32_t i=0; i < count; ++i) {
                        handler("REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB", leInfo, segmentsInfo, segIndexSet, ptrSize, segIndex, segOffset, type, stop);
                        segOffset += skip + ptrSize;
                        if ( stop )
                            break;
                    }
                    break;
                default:
                    diag.error("unknown rebase opcode 0x%02X", opcode);
            }
        }
    }
    else {
        // old binary, walk relocations
        const uint64_t                  relocsStartAddress = relocBaseAddress(segmentsInfo, leInfo.layout.linkeditSegIndex);
        const relocation_info* const    relocsStart = (relocation_info*)getLinkEditContent(leInfo.layout, leInfo.dynSymTab->locreloff);
        const relocation_info* const    relocsEnd   = &relocsStart[leInfo.dynSymTab->nlocrel];
        bool                            stop = false;
        const uint8_t                   relocSize = (is64() ? 3 : 2);
        const uint8_t                   ptrSize   = pointerSize();
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(uint32_t, relocAddrs, 2048);
        for (const relocation_info* reloc=relocsStart; (reloc < relocsEnd) && !stop; ++reloc) {
            if ( reloc->r_length != relocSize ) {
                diag.error("local relocation has wrong r_length");
                break;
            }
            if ( reloc->r_type != 0 ) { // 0 == X86_64_RELOC_UNSIGNED == GENERIC_RELOC_VANILLA ==  ARM64_RELOC_UNSIGNED
                diag.error("local relocation has wrong r_type");
                break;
            }
            relocAddrs.push_back(reloc->r_address);
        }
        if ( !relocAddrs.empty() ) {
            ::qsort(&relocAddrs[0], relocAddrs.count(), sizeof(uint32_t), &uint32Sorter);
            for (uint32_t addrOff : relocAddrs) {
                uint32_t segIndex  = 0;
                uint64_t segOffset = 0;
                if ( segIndexAndOffsetForAddress(relocsStartAddress+addrOff, segmentsInfo, leInfo.layout.linkeditSegIndex, segIndex, segOffset) ) {
                    uint8_t type = REBASE_TYPE_POINTER;
                    if ( this->cputype == CPU_TYPE_I386 ) {
                        if ( segmentsInfo[segIndex].executable() )
                            type = REBASE_TYPE_TEXT_ABSOLUTE32;
                    }
                    handler("local relocation", leInfo, segmentsInfo, true, ptrSize, segIndex, segOffset, type , stop);
                }
                else {
                    diag.error("local relocation has out of range r_address");
                    break;
                }
            }
        }
        // then process indirect symbols
        forEachIndirectPointer(diag, ^(uint64_t address, bool bind, int bindLibOrdinal,
                                       const char* bindSymbolName, bool bindWeakImport, bool bindLazy, bool selfModifyingStub, bool& indStop) {
            if ( bind )
               return;
            uint32_t segIndex  = 0;
            uint64_t segOffset = 0;
            if ( segIndexAndOffsetForAddress(address, segmentsInfo, leInfo.layout.linkeditSegIndex, segIndex, segOffset) ) {
                handler("local relocation", leInfo, segmentsInfo, true, ptrSize, segIndex, segOffset, REBASE_TYPE_POINTER, indStop);
            }
            else {
                diag.error("local relocation has out of range r_address");
                indStop = true;
            }
        });
    }
}

bool MachOAnalyzer::segIndexAndOffsetForAddress(uint64_t addr, const SegmentInfo segmentsInfos[], uint32_t segCount, uint32_t& segIndex, uint64_t& segOffset) const
{
    for (uint32_t i=0; i < segCount; ++i) {
        if ( (segmentsInfos[i].vmAddr <= addr) && (addr < segmentsInfos[i].vmAddr+segmentsInfos[i].vmSize) ) {
            segIndex  = i;
            segOffset = addr - segmentsInfos[i].vmAddr;
            return true;
        }
    }
    return false;
}

uint64_t MachOAnalyzer::relocBaseAddress(const SegmentInfo segmentsInfos[], uint32_t segCount) const
{
    if ( is64() ) {
        // x86_64 reloc base address is first writable segment
        for (uint32_t i=0; i < segCount; ++i) {
            if ( segmentsInfos[i].writable() )
                return segmentsInfos[i].vmAddr;
        }
    }
    return segmentsInfos[0].vmAddr;
}



void MachOAnalyzer::forEachIndirectPointer(Diagnostics& diag, void (^handler)(uint64_t pointerAddress, bool bind, int bindLibOrdinal, const char* bindSymbolName, 
                                                                             bool bindWeakImport, bool bindLazy, bool selfModifyingStub, bool& stop)) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    // find lazy and non-lazy pointer sections
    const bool              is64Bit                  = is64();
    const uint32_t* const   indirectSymbolTable      = (uint32_t*)getLinkEditContent(leInfo.layout, leInfo.dynSymTab->indirectsymoff);
    const uint32_t          indirectSymbolTableCount = leInfo.dynSymTab->nindirectsyms;
    const uint32_t          ptrSize                  = pointerSize();
    const void*             symbolTable              = getLinkEditContent(leInfo.layout, leInfo.symTab->symoff);
    const struct nlist_64*  symbols64                = (nlist_64*)symbolTable;
    const struct nlist*     symbols32                = (struct nlist*)symbolTable;
    const char*             stringPool               = (char*)getLinkEditContent(leInfo.layout, leInfo.symTab->stroff);
    uint32_t                symCount                 = leInfo.symTab->nsyms;
    uint32_t                poolSize                 = leInfo.symTab->strsize;
    __block bool            stop                     = false;
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& sectInfo, bool malformedSectionRange, bool& sectionStop) {
        uint8_t  sectionType  = (sectInfo.sectFlags & SECTION_TYPE);
        bool selfModifyingStub = (sectionType == S_SYMBOL_STUBS) && (sectInfo.sectFlags & S_ATTR_SELF_MODIFYING_CODE) && (sectInfo.reserved2 == 5) && (this->cputype == CPU_TYPE_I386);
        if ( (sectionType != S_LAZY_SYMBOL_POINTERS) && (sectionType != S_NON_LAZY_SYMBOL_POINTERS) && !selfModifyingStub )
            return;
        if ( (flags & S_ATTR_SELF_MODIFYING_CODE) && !selfModifyingStub ) {
            diag.error("S_ATTR_SELF_MODIFYING_CODE section type only valid in old i386 binaries");
            sectionStop = true;
            return;
        }
        uint32_t elementSize = selfModifyingStub ? sectInfo.reserved2 : ptrSize;
        uint32_t elementCount = (uint32_t)(sectInfo.sectSize/elementSize);
        if ( greaterThanAddOrOverflow(sectInfo.reserved1, elementCount, indirectSymbolTableCount) ) {
            diag.error("section %s overflows indirect symbol table", sectInfo.sectName);
            sectionStop = true;
            return;
        }

        for (uint32_t i=0; (i < elementCount) && !stop; ++i) {
            uint32_t symNum = indirectSymbolTable[sectInfo.reserved1 + i];
            if ( symNum == INDIRECT_SYMBOL_ABS )
                continue;
            if ( symNum == INDIRECT_SYMBOL_LOCAL ) {
                handler(sectInfo.sectAddr+i*elementSize, false, 0, "", false, false, false, stop);
                continue;
            }
            if ( symNum > symCount ) {
                diag.error("indirect symbol[%d] = %d which is invalid symbol index", sectInfo.reserved1 + i, symNum);
                sectionStop = true;
                return;
            }
            uint16_t n_desc = is64Bit ? symbols64[symNum].n_desc : symbols32[symNum].n_desc;
            uint32_t libOrdinal = libOrdinalFromDesc(n_desc);
            uint32_t strOffset = is64Bit ? symbols64[symNum].n_un.n_strx : symbols32[symNum].n_un.n_strx;
            if ( strOffset > poolSize ) {
               diag.error("symbol[%d] string offset out of range", sectInfo.reserved1 + i);
                sectionStop = true;
                return;
            }
            const char* symbolName  = stringPool + strOffset;
            bool        weakImport  = (n_desc & N_WEAK_REF);
            bool        lazy        = (sectionType == S_LAZY_SYMBOL_POINTERS);
            handler(sectInfo.sectAddr+i*elementSize, true, libOrdinal, symbolName, weakImport, lazy, selfModifyingStub, stop);
        }
        sectionStop = stop;
    });
}

int MachOAnalyzer::libOrdinalFromDesc(uint16_t n_desc) const
{
    // -flat_namespace is always flat lookup
    if ( (this->flags & MH_TWOLEVEL) == 0 )
        return BIND_SPECIAL_DYLIB_FLAT_LOOKUP;

    // extract byte from undefined symbol entry
    int libIndex = GET_LIBRARY_ORDINAL(n_desc);
    switch ( libIndex ) {
        case SELF_LIBRARY_ORDINAL:
            return BIND_SPECIAL_DYLIB_SELF;

        case DYNAMIC_LOOKUP_ORDINAL:
            return BIND_SPECIAL_DYLIB_FLAT_LOOKUP;

        case EXECUTABLE_ORDINAL:
            return BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE;
    }

    return libIndex;
}

bool MachOAnalyzer::validBindInfo(Diagnostics& diag, const char* path) const
{
    forEachBind(diag, ^(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                         bool segIndexSet, bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal,
                         uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset,
                         uint8_t type, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
        if ( invalidBindState(diag, opcodeName, path, leInfo, segments, segIndexSet, libraryOrdinalSet, dylibCount,
                              libOrdinal, ptrSize, segmentIndex, segmentOffset, type, symbolName) ) {
            stop = true;
        }
    }, ^(const char* symbolName) {
    }, ^() { });
    return diag.noError();
}

bool MachOAnalyzer::invalidBindState(Diagnostics& diag, const char* opcodeName, const char* path, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                    bool segIndexSet,  bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal, uint32_t ptrSize,
                                    uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type, const char* symbolName) const
{
    if ( !segIndexSet ) {
        diag.error("in '%s' %s missing preceding BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB", path, opcodeName);
        return true;
    }
    if ( segmentIndex >= leInfo.layout.linkeditSegIndex )  {
        diag.error("in '%s' %s segment index %d too large", path, opcodeName, segmentIndex);
        return true;
    }
    if ( segmentOffset > (segments[segmentIndex].vmSize-ptrSize) ) {
        diag.error("in '%s' %s current segment offset 0x%08llX beyond segment size (0x%08llX)", path, opcodeName, segmentOffset, segments[segmentIndex].vmSize);
        return true;
    }
    if ( symbolName == NULL ) {
        diag.error("in '%s' %s missing preceding BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM", path, opcodeName);
        return true;
    }
    if ( !libraryOrdinalSet ) {
        diag.error("in '%s' %s missing preceding BIND_OPCODE_SET_DYLIB_ORDINAL", path, opcodeName);
        return true;
    }
    if ( libOrdinal > (int)dylibCount ) {
        diag.error("in '%s' %s has library ordinal too large (%d) max (%d)", path, opcodeName, libOrdinal, dylibCount);
        return true;
    }
    if ( libOrdinal < BIND_SPECIAL_DYLIB_WEAK_LOOKUP ) {
        diag.error("in '%s' %s has unknown library special ordinal (%d)", path, opcodeName, libOrdinal);
        return true;
    }
    switch ( type )  {
        case BIND_TYPE_POINTER:
            if ( !segments[segmentIndex].writable() ) {
                diag.error("in '%s' %s pointer bind is in non-writable segment", path, opcodeName);
                return true;
            }
            if ( segments[segmentIndex].executable() && enforceFormat(Malformed::executableData) ) {
                diag.error("in '%s' %s pointer bind is in executable segment", path, opcodeName);
                return true;
            }
            break;
        case BIND_TYPE_TEXT_ABSOLUTE32:
        case BIND_TYPE_TEXT_PCREL32:
            if ( !segments[segmentIndex].textRelocs ) {
                diag.error("in '%s' %s text bind is in segment that does not support text relocations", path, opcodeName);
                return true;
            }
            if ( segments[segmentIndex].writable() ) {
                diag.error("in '%s' %s text bind is in writable segment", path, opcodeName);
                return true;
            }
            if ( !segments[segmentIndex].executable() ) {
                diag.error("in '%s' %s pointer bind is in non-executable segment", path, opcodeName);
                return true;
            }
            break;
        default:
            diag.error("in '%s' %s unknown bind type %d", path, opcodeName, type);
            return true;
    }
    return false;
}

void MachOAnalyzer::forEachBind(Diagnostics& diag, void (^handler)(uint64_t runtimeOffset, int libOrdinal, const char* symbolName,
                                                                  bool weakImport, bool lazyBind, uint64_t addend, bool& stop),
                                                   void (^strongHandler)(const char* symbolName),
                                                   void (^missingLazyBindHandler)()) const
{
    __block bool     startVmAddrSet = false;
    __block uint64_t startVmAddr    = 0;
    forEachBind(diag, ^(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                        bool segIndexSet, bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal,
                        uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset,
                        uint8_t type, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
       if ( !startVmAddrSet ) {
            for (int i=0; i <= segmentIndex; ++i) {
                if ( strcmp(segments[i].segName, "__TEXT") == 0 ) {
                    startVmAddr = segments[i].vmAddr;
                    startVmAddrSet = true;
                    break;
               }
            }
        }
        uint64_t bindVmOffset  = segments[segmentIndex].vmAddr + segmentOffset;
        uint64_t runtimeOffset = bindVmOffset - startVmAddr;
        handler(runtimeOffset, libOrdinal, symbolName, weakImport, lazyBind, addend, stop);
    }, ^(const char* symbolName) {
        strongHandler(symbolName);
    }, ^() {
        missingLazyBindHandler();
    });
}

void MachOAnalyzer::forEachBind(Diagnostics& diag,
                                 void (^handler)(const char* opcodeName, const LinkEditInfo& leInfo, const SegmentInfo segments[],
                                                 bool segIndexSet,  bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal,
                                                 uint32_t ptrSize, uint8_t segmentIndex, uint64_t segmentOffset, uint8_t type,
                                                 const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop),
                                 void (^strongHandler)(const char* symbolName),
                                 void (^missingLazyBindHandler)()) const
{
    const uint32_t  ptrSize = this->pointerSize();
    bool            stop    = false;

    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    BLOCK_ACCCESSIBLE_ARRAY(SegmentInfo, segmentsInfo, leInfo.layout.linkeditSegIndex+1);
    getAllSegmentsInfos(diag, segmentsInfo);
    if ( diag.hasError() )
        return;



    const uint32_t dylibCount = dependentDylibCount();

    if ( leInfo.dyldInfo != nullptr ) {
        // process bind opcodes
        const uint8_t*  p    = getLinkEditContent(leInfo.layout, leInfo.dyldInfo->bind_off);
        const uint8_t*  end  = p + leInfo.dyldInfo->bind_size;
        uint8_t         type = 0;
        uint64_t        segmentOffset = 0;
        uint8_t         segmentIndex = 0;
        const char*     symbolName = NULL;
        int             libraryOrdinal = 0;
        bool            segIndexSet = false;
        bool            libraryOrdinalSet = false;

        int64_t         addend = 0;
        uint64_t        count;
        uint64_t        skip;
        bool            weakImport = false;
        while ( !stop && diag.noError() && (p < end) ) {
            uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
            uint8_t opcode = *p & BIND_OPCODE_MASK;
            ++p;
            switch (opcode) {
                case BIND_OPCODE_DONE:
                    stop = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                    libraryOrdinal = immediate;
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                    libraryOrdinal = (int)read_uleb128(diag, p, end);
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                    // the special ordinals are negative numbers
                    if ( immediate == 0 )
                        libraryOrdinal = 0;
                    else {
                        int8_t signExtended = BIND_OPCODE_MASK | immediate;
                        libraryOrdinal = signExtended;
                    }
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                    weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                    symbolName = (char*)p;
                    while (*p != '\0')
                        ++p;
                    ++p;
                    break;
                case BIND_OPCODE_SET_TYPE_IMM:
                    type = immediate;
                    break;
                case BIND_OPCODE_SET_ADDEND_SLEB:
                    addend = read_sleb128(diag, p, end);
                    break;
                case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                    segmentIndex = immediate;
                    segmentOffset = read_uleb128(diag, p, end);
                    segIndexSet = true;
                    break;
                case BIND_OPCODE_ADD_ADDR_ULEB:
                    segmentOffset += read_uleb128(diag, p, end);
                    break;
                case BIND_OPCODE_DO_BIND:
                    handler("BIND_OPCODE_DO_BIND", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                            ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                    segmentOffset += ptrSize;
                    break;
                case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                    handler("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                            ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                    segmentOffset += read_uleb128(diag, p, end) + ptrSize;
                    break;
                case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                    handler("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                            ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                    segmentOffset += immediate*ptrSize + ptrSize;
                    break;
                case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                    count = read_uleb128(diag, p, end);
                    skip = read_uleb128(diag, p, end);
                    for (uint32_t i=0; i < count; ++i) {
                        handler("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                        segmentOffset += skip + ptrSize;
                        if ( stop )
                            break;
                    }
                    break;
                default:
                    diag.error("bad bind opcode 0x%02X", *p);
            }
        }
        if ( diag.hasError() )
            return;

        // process lazy bind opcodes
        uint32_t lazyDoneCount = 0;
        uint32_t lazyBindCount = 0;
        if ( leInfo.dyldInfo->lazy_bind_size != 0 ) {
            p               = getLinkEditContent(leInfo.layout, leInfo.dyldInfo->lazy_bind_off);
            end             = p + leInfo.dyldInfo->lazy_bind_size;
            type            = BIND_TYPE_POINTER;
            segmentOffset   = 0;
            segmentIndex    = 0;
            symbolName      = NULL;
            libraryOrdinal  = 0;
            segIndexSet     = false;
            libraryOrdinalSet= false;
            addend          = 0;
            weakImport      = false;
            stop            = false;
            while (  !stop && diag.noError() && (p < end) ) {
                uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
                uint8_t opcode = *p & BIND_OPCODE_MASK;
                ++p;
                switch (opcode) {
                    case BIND_OPCODE_DONE:
                        // this opcode marks the end of each lazy pointer binding
                        ++lazyDoneCount;
                        break;
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                        libraryOrdinal = immediate;
                        libraryOrdinalSet = true;
                        break;
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                        libraryOrdinal = (int)read_uleb128(diag, p, end);
                        libraryOrdinalSet = true;
                        break;
                    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                        // the special ordinals are negative numbers
                        if ( immediate == 0 )
                            libraryOrdinal = 0;
                        else {
                            int8_t signExtended = BIND_OPCODE_MASK | immediate;
                            libraryOrdinal = signExtended;
                        }
                        libraryOrdinalSet = true;
                        break;
                    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                        weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                        symbolName = (char*)p;
                        while (*p != '\0')
                            ++p;
                        ++p;
                        break;
                    case BIND_OPCODE_SET_ADDEND_SLEB:
                        addend = read_sleb128(diag, p, end);
                        break;
                    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                        segmentIndex = immediate;
                        segmentOffset = read_uleb128(diag, p, end);
                        segIndexSet = true;
                        break;
                    case BIND_OPCODE_DO_BIND:
                        handler("BIND_OPCODE_DO_BIND", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, true, addend, stop);
                        segmentOffset += ptrSize;
                        ++lazyBindCount;
                        break;
                    case BIND_OPCODE_SET_TYPE_IMM:
                    case BIND_OPCODE_ADD_ADDR_ULEB:
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                    default:
                        diag.error("bad lazy bind opcode 0x%02X", opcode);
                        break;
                }
            }
            if ( lazyDoneCount > lazyBindCount+7 )
                missingLazyBindHandler();
            //    diag.error("lazy bind opcodes missing binds");
        }
        if ( diag.hasError() )
            return;

        // process weak bind info
        if ( leInfo.dyldInfo->weak_bind_size != 0 ) {
            p               = getLinkEditContent(leInfo.layout, leInfo.dyldInfo->weak_bind_off);
            end             = p + leInfo.dyldInfo->weak_bind_size;
            type            = BIND_TYPE_POINTER;
            segmentOffset   = 0;
            segmentIndex    = 0;
            symbolName      = NULL;
            libraryOrdinal  = BIND_SPECIAL_DYLIB_WEAK_LOOKUP;
            segIndexSet     = false;
            libraryOrdinalSet= true;
            addend          = 0;
            weakImport      = false;
            stop            = false;
            while ( !stop && diag.noError() && (p < end) ) {
                uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
                uint8_t opcode = *p & BIND_OPCODE_MASK;
                ++p;
                switch (opcode) {
                    case BIND_OPCODE_DONE:
                        stop = true;
                        break;
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                        diag.error("unexpected dylib ordinal in weak_bind");
                        break;
                    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                        weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                        symbolName = (char*)p;
                        while (*p != '\0')
                            ++p;
                        ++p;
                        if ( immediate & BIND_SYMBOL_FLAGS_NON_WEAK_DEFINITION ) {
                            strongHandler(symbolName);
                        }
                        break;
                    case BIND_OPCODE_SET_TYPE_IMM:
                        type = immediate;
                        break;
                    case BIND_OPCODE_SET_ADDEND_SLEB:
                        addend = read_sleb128(diag, p, end);
                        break;
                    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                        segmentIndex = immediate;
                        segmentOffset = read_uleb128(diag, p, end);
                        segIndexSet = true;
                        break;
                    case BIND_OPCODE_ADD_ADDR_ULEB:
                        segmentOffset += read_uleb128(diag, p, end);
                        break;
                    case BIND_OPCODE_DO_BIND:
                        handler("BIND_OPCODE_DO_BIND", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                        segmentOffset += ptrSize;
                        break;
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
                        handler("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                        segmentOffset += read_uleb128(diag, p, end) + ptrSize;
                        break;
                    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
                        handler("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                        segmentOffset += immediate*ptrSize + ptrSize;
                        break;
                    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:
                        count = read_uleb128(diag, p, end);
                        skip = read_uleb128(diag, p, end);
                        for (uint32_t i=0; i < count; ++i) {
                            handler("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB", leInfo, segmentsInfo, segIndexSet, libraryOrdinalSet, dylibCount, libraryOrdinal,
                                    ptrSize, segmentIndex, segmentOffset, type, symbolName, weakImport, false, addend, stop);
                            segmentOffset += skip + ptrSize;
                            if ( stop )
                                break;
                        }
                        break;
                    default:
                        diag.error("bad bind opcode 0x%02X", *p);
                }
            }
        }
    }
    else {
        // old binary, process external relocations
        const uint64_t                  relocsStartAddress = relocBaseAddress(segmentsInfo, leInfo.layout.linkeditSegIndex);
        const relocation_info* const    relocsStart = (relocation_info*)getLinkEditContent(leInfo.layout, leInfo.dynSymTab->extreloff);
        const relocation_info* const    relocsEnd   = &relocsStart[leInfo.dynSymTab->nextrel];
        bool                            is64Bit     = is64() ;
        const uint8_t                   relocSize   = (is64Bit ? 3 : 2);
        const void*                     symbolTable = getLinkEditContent(leInfo.layout, leInfo.symTab->symoff);
        const struct nlist_64*          symbols64   = (nlist_64*)symbolTable;
        const struct nlist*             symbols32   = (struct nlist*)symbolTable;
        const char*                     stringPool  = (char*)getLinkEditContent(leInfo.layout, leInfo.symTab->stroff);
        uint32_t                        symCount    = leInfo.symTab->nsyms;
        uint32_t                        poolSize    = leInfo.symTab->strsize;
        for (const relocation_info* reloc=relocsStart; (reloc < relocsEnd) && !stop; ++reloc) {
            if ( reloc->r_length != relocSize ) {
                diag.error("external relocation has wrong r_length");
                break;
            }
            if ( reloc->r_type != 0 ) { // 0 == X86_64_RELOC_UNSIGNED == GENERIC_RELOC_VANILLA == ARM64_RELOC_UNSIGNED
                diag.error("external relocation has wrong r_type");
                break;
            }
            uint32_t segIndex  = 0;
            uint64_t segOffset = 0;
            if ( segIndexAndOffsetForAddress(relocsStartAddress+reloc->r_address, segmentsInfo, leInfo.layout.linkeditSegIndex, segIndex, segOffset) ) {
                uint32_t symbolIndex = reloc->r_symbolnum;
                if ( symbolIndex > symCount ) {
                    diag.error("external relocation has out of range r_symbolnum");
                    break;
                }
                else {
                    uint32_t strOffset  = is64Bit ? symbols64[symbolIndex].n_un.n_strx : symbols32[symbolIndex].n_un.n_strx;
                    uint16_t n_desc     = is64Bit ? symbols64[symbolIndex].n_desc : symbols32[symbolIndex].n_desc;
                    uint32_t libOrdinal = libOrdinalFromDesc(n_desc);
                    if ( strOffset >= poolSize ) {
                        diag.error("external relocation has r_symbolnum=%d which has out of range n_strx", symbolIndex);
                        break;
                    }
                    else {
                        const char*     symbolName = stringPool + strOffset;
                        bool            weakImport = (n_desc & N_WEAK_REF);
                        const uint8_t*  content    = (uint8_t*)this + segmentsInfo[segIndex].vmAddr - leInfo.layout.textUnslidVMAddr + segOffset;
                        uint64_t        addend     = is64Bit ? *((uint64_t*)content) : *((uint32_t*)content);
                        handler("external relocation", leInfo, segmentsInfo, true, true, dylibCount, libOrdinal,
                                ptrSize, segIndex, segOffset, BIND_TYPE_POINTER, symbolName, weakImport, false, addend, stop);
                    }
                }
            }
            else {
                diag.error("local relocation has out of range r_address");
                break;
            }
        }
        // then process indirect symbols
        forEachIndirectPointer(diag, ^(uint64_t address, bool bind, int bindLibOrdinal,
                                       const char* bindSymbolName, bool bindWeakImport, bool bindLazy, bool selfModifyingStub, bool& indStop) {
            if ( !bind )
               return;
            uint32_t segIndex  = 0;
            uint64_t segOffset = 0;
            if ( segIndexAndOffsetForAddress(address, segmentsInfo, leInfo.layout.linkeditSegIndex, segIndex, segOffset) ) {
                handler("indirect symbol", leInfo, segmentsInfo, true, true, dylibCount, bindLibOrdinal,
                         ptrSize, segIndex, segOffset, BIND_TYPE_POINTER, bindSymbolName, bindWeakImport, bindLazy, 0, indStop);
            }
            else {
                diag.error("indirect symbol has out of range address");
                indStop = true;
            }
        });
    }

}


bool MachOAnalyzer::validChainedFixupsInfo(Diagnostics& diag, const char* path) const
{
    __block uint32_t maxTargetCount = 0;
    __block uint32_t currentTargetCount = 0;
    parseOrgArm64eChainedFixups(diag,
        ^(uint32_t totalTargets, bool& stop) {
            maxTargetCount = totalTargets;
        },
        ^(const LinkEditInfo& leInfo, const SegmentInfo segments[], bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal, uint8_t type, const char* symbolName, uint64_t addend, bool weakImport, bool& stop) {
           if ( symbolName == NULL ) {
                diag.error("in '%s' missing BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM", path);
            }
            else if ( !libraryOrdinalSet ) {
                diag.error("in '%s' missing BIND_OPCODE_SET_DYLIB_ORDINAL", path);
            }
            else if ( libOrdinal > (int)dylibCount ) {
                diag.error("in '%s' has library ordinal too large (%d) max (%d)", path, libOrdinal, dylibCount);
            }
            else if ( libOrdinal < BIND_SPECIAL_DYLIB_WEAK_LOOKUP ) {
                diag.error("in '%s' has unknown library special ordinal (%d)", path, libOrdinal);
            }
            else if ( type != BIND_TYPE_POINTER ) {
                diag.error("in '%s' unknown bind type %d", path, type);
            }
            else if ( currentTargetCount > maxTargetCount ) {
                diag.error("in '%s' chained target counts exceeds BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB", path);
            }
            ++currentTargetCount;
            if ( diag.hasError() )
                stop = true;
        },
        ^(const LinkEditInfo& leInfo, const SegmentInfo segments[], uint8_t segmentIndex, bool segIndexSet, uint64_t segmentOffset, uint16_t format, bool& stop) {
           if ( !segIndexSet ) {
                diag.error("in '%s' missing BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB", path);
            }
            else if ( segmentIndex >= leInfo.layout.linkeditSegIndex )  {
                diag.error("in '%s' segment index %d too large", path, segmentIndex);
            }
            else if ( segmentOffset > (segments[segmentIndex].vmSize-8) ) {
                diag.error("in '%s' current segment offset 0x%08llX beyond segment size (0x%08llX)", path, segmentOffset, segments[segmentIndex].vmSize);
            }
            else if ( !segments[segmentIndex].writable() ) {
                diag.error("in '%s' pointer bind is in non-writable segment", path);
            }
            else if ( segments[segmentIndex].executable() ) {
                diag.error("in '%s' pointer bind is in executable segment", path);
            }
            if ( diag.hasError() )
                stop = true;
        }
    );

    return diag.noError();
}



void MachOAnalyzer::parseOrgArm64eChainedFixups(Diagnostics& diag, void (^targetCount)(uint32_t totalTargets, bool& stop),
                                                                   void (^addTarget)(const LinkEditInfo& leInfo, const SegmentInfo segments[], bool libraryOrdinalSet, uint32_t dylibCount, int libOrdinal, uint8_t type, const char* symbolName, uint64_t addend, bool weakImport, bool& stop),
                                                                   void (^addChainStart)(const LinkEditInfo& leInfo, const SegmentInfo segments[], uint8_t segmentIndex, bool segIndexSet, uint64_t segmentOffset, uint16_t format, bool& stop)) const
{
    bool            stop    = false;

    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    BLOCK_ACCCESSIBLE_ARRAY(SegmentInfo, segmentsInfo, leInfo.layout.linkeditSegIndex+1);
    getAllSegmentsInfos(diag, segmentsInfo);
    if ( diag.hasError() )
        return;

    const uint32_t dylibCount = dependentDylibCount();

    if ( leInfo.dyldInfo != nullptr ) {
        // process bind opcodes
        const uint8_t*  p    = getLinkEditContent(leInfo.layout, leInfo.dyldInfo->bind_off);
        const uint8_t*  end  = p + leInfo.dyldInfo->bind_size;
        uint8_t         type = 0;
        uint64_t        segmentOffset = 0;
        uint8_t         segmentIndex = 0;
        const char*     symbolName = NULL;
        int             libraryOrdinal = 0;
        bool            segIndexSet = false;
        bool            libraryOrdinalSet = false;
        uint64_t        targetTableCount;
        uint64_t        addend = 0;
        bool            weakImport = false;
        while ( !stop && diag.noError() && (p < end) ) {
            uint8_t immediate = *p & BIND_IMMEDIATE_MASK;
            uint8_t opcode = *p & BIND_OPCODE_MASK;
            ++p;
            switch (opcode) {
                case BIND_OPCODE_DONE:
                    stop = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
                    libraryOrdinal = immediate;
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
                    libraryOrdinal = (int)read_uleb128(diag, p, end);
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
                    // the special ordinals are negative numbers
                    if ( immediate == 0 )
                        libraryOrdinal = 0;
                    else {
                        int8_t signExtended = BIND_OPCODE_MASK | immediate;
                        libraryOrdinal = signExtended;
                    }
                    libraryOrdinalSet = true;
                    break;
                case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
                    weakImport = ( (immediate & BIND_SYMBOL_FLAGS_WEAK_IMPORT) != 0 );
                    symbolName = (char*)p;
                    while (*p != '\0')
                        ++p;
                    ++p;
                    break;
                case BIND_OPCODE_SET_TYPE_IMM:
                    type = immediate;
                    break;
                case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
                    segmentIndex = immediate;
                    segmentOffset = read_uleb128(diag, p, end);
                    segIndexSet = true;
                    break;
                case BIND_OPCODE_SET_ADDEND_SLEB:
                    addend = read_sleb128(diag, p, end);
                    break;
                case BIND_OPCODE_DO_BIND:
                    if ( addTarget )
                        addTarget(leInfo, segmentsInfo, libraryOrdinalSet, dylibCount, libraryOrdinal, type, symbolName, addend, weakImport, stop);
                    break;
                case BIND_OPCODE_THREADED:
                    switch (immediate) {
                        case BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB:
                            targetTableCount = read_uleb128(diag, p, end);
                            if ( targetTableCount > 65535 ) {
                                diag.error("BIND_SUBOPCODE_THREADED_SET_BIND_ORDINAL_TABLE_SIZE_ULEB size too large");
                                stop = true;
                            }
                            else {
                                if ( targetCount )
                                    targetCount((uint32_t)targetTableCount, stop);
                            }
                            break;
                        case BIND_SUBOPCODE_THREADED_APPLY:
                            if ( addChainStart )
                                addChainStart(leInfo, segmentsInfo, segmentIndex, segIndexSet, segmentOffset, DYLD_CHAINED_PTR_ARM64E, stop);
                            break;
                        default:
                            diag.error("bad BIND_OPCODE_THREADED sub-opcode 0x%02X", immediate);
                    }
                    break;
                default:
                    diag.error("bad bind opcode 0x%02X", immediate);
            }
        }
        if ( diag.hasError() )
            return;
    }
}

void MachOAnalyzer::forEachChainedFixupTarget(Diagnostics& diag, void (^callback)(int libOrdinal, const char* symbolName, uint64_t addend, bool weakImport, bool& stop)) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    BLOCK_ACCCESSIBLE_ARRAY(SegmentInfo, segmentsInfo, leInfo.layout.linkeditSegIndex+1);
    getAllSegmentsInfos(diag, segmentsInfo);
    if ( diag.hasError() )
        return;

    bool stop    = false;
    if ( leInfo.dyldInfo != nullptr ) {
        parseOrgArm64eChainedFixups(diag, nullptr, ^(const LinkEditInfo& leInfo2, const SegmentInfo segments[], bool libraryOrdinalSet, uint32_t dylibCount,
                                                    int libOrdinal, uint8_t type, const char* symbolName, uint64_t fixAddend, bool weakImport, bool& stopChain) {
            callback(libOrdinal, symbolName, fixAddend, weakImport, stopChain);
        }, nullptr);
    }
    else if ( leInfo.chainedFixups != nullptr ) {
        const dyld_chained_fixups_header*  header = (dyld_chained_fixups_header*)getLinkEditContent(leInfo.layout, leInfo.chainedFixups->dataoff);
        if ( (header->imports_offset > leInfo.chainedFixups->datasize) || (header->symbols_offset > leInfo.chainedFixups->datasize) ) {
            diag.error("malformed import table");
            return;
        }
        const dyld_chained_import*          imports;
        const dyld_chained_import_addend*   importsA32;
        const dyld_chained_import_addend64* importsA64;
        const char*                         symbolsPool     = (char*)header + header->symbols_offset;
        uint32_t                            maxSymbolOffset = leInfo.chainedFixups->datasize - header->symbols_offset;
        int                                 libOrdinal;
        switch (header->imports_format) {
            case DYLD_CHAINED_IMPORT:
                imports = (dyld_chained_import*)((uint8_t*)header + header->imports_offset);
                for (uint32_t i=0; i < header->imports_count; ++i) {
                    const char* symbolName = &symbolsPool[imports[i].name_offset];
                    if ( imports[i].name_offset > maxSymbolOffset ) {
                        diag.error("malformed import table, string overflow");
                        return;
                    }
                    uint8_t libVal = imports[i].lib_ordinal;
                    if ( libVal > 0xF0 )
                        libOrdinal = (int8_t)libVal;
                    else
                        libOrdinal = libVal;
                    callback(libOrdinal, symbolName, 0, imports[i].weak_import, stop);
                }
                break;
            case DYLD_CHAINED_IMPORT_ADDEND:
                importsA32 = (dyld_chained_import_addend*)((uint8_t*)header + header->imports_offset);
                for (uint32_t i=0; i < header->imports_count; ++i) {
                    const char* symbolName = &symbolsPool[importsA32[i].name_offset];
                    if ( importsA32[i].name_offset > maxSymbolOffset ) {
                        diag.error("malformed import table, string overflow");
                        return;
                    }
                    uint8_t libVal = importsA32[i].lib_ordinal;
                    if ( libVal > 0xF0 )
                        libOrdinal = (int8_t)libVal;
                    else
                        libOrdinal = libVal;
                   callback(libOrdinal, symbolName, importsA32[i].addend, importsA32[i].weak_import, stop);
                }
                break;
            case DYLD_CHAINED_IMPORT_ADDEND64:
                importsA64 = (dyld_chained_import_addend64*)((uint8_t*)header + header->imports_offset);
                for (uint32_t i=0; i < header->imports_count; ++i) {
                    const char* symbolName = &symbolsPool[importsA64[i].name_offset];
                    if ( importsA64[i].name_offset > maxSymbolOffset ) {
                        diag.error("malformed import table, string overflow");
                        return;
                    }
                    uint16_t libVal = importsA64[i].lib_ordinal;
                    if ( libVal > 0xFFF0 )
                        libOrdinal = (int16_t)libVal;
                    else
                        libOrdinal = libVal;
                    callback(libOrdinal, symbolName, importsA64[i].addend, importsA64[i].weak_import, stop);
                }
                break;
           default:
                diag.error("unknown imports format");
                return;
        }
    }
}

uint32_t MachOAnalyzer::segmentCount() const
{
    __block uint32_t count   = 0;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        ++count;
    });
    return count;
}

bool MachOAnalyzer::hasCodeSignature(uint32_t& fileOffset, uint32_t& size) const
{
    fileOffset = 0;
    size = 0;

    Diagnostics diag;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_CODE_SIGNATURE ) {
            const linkedit_data_command* sigCmd = (linkedit_data_command*)cmd;
            fileOffset = sigCmd->dataoff;
            size       = sigCmd->datasize;
            stop = true;
        }
    });
    diag.assertNoError();   // any malformations in the file should have been caught by earlier validate() call

    // early exist if no LC_CODE_SIGNATURE
    if ( fileOffset == 0 )
        return false;

    // <rdar://problem/13622786> ignore code signatures in macOS binaries built with pre-10.9 tools
    if ( (this->cputype == CPU_TYPE_X86_64) || (this->cputype == CPU_TYPE_I386) ) {
        __block bool foundPlatform = false;
        __block bool badSignature  = false;
        forEachSupportedPlatform(^(Platform platform, uint32_t minOS, uint32_t sdk) {
            foundPlatform = true;
            if ( (platform == Platform::macOS) && (sdk < 0x000A0900) )
                badSignature = true;
        });
        return foundPlatform && !badSignature;
    }

    return true;
}

bool MachOAnalyzer::hasInitializer(Diagnostics& diag, bool contentRebased, const void* dyldCache) const
{
    __block bool result = false;
    forEachInitializer(diag, contentRebased, ^(uint32_t offset) {
        result = true;
    }, dyldCache);
    return result;
}

void MachOAnalyzer::forEachInitializerPointerSection(Diagnostics& diag, void (^callback)(uint32_t sectionOffset, uint32_t sectionSize, const uint8_t* content, bool& stop)) const
{
    const unsigned ptrSize     = pointerSize();
    const uint64_t baseAddress = preferredLoadAddress();
    const uint64_t slide       = (uint64_t)this - baseAddress;
    forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& sectStop) {
        if ( (info.sectFlags & SECTION_TYPE) == S_MOD_INIT_FUNC_POINTERS ) {
            if ( (info.sectSize % ptrSize) != 0 ) {
                diag.error("initializer section %s/%s has bad size", info.segInfo.segName, info.sectName);
                sectStop = true;
                return;
            }
            if ( malformedSectionRange ) {
                diag.error("initializer section %s/%s extends beyond its segment", info.segInfo.segName, info.sectName);
                sectStop = true;
                return;
            }
            const uint8_t* content = (uint8_t*)(info.sectAddr + slide);
            if ( ((long)content % ptrSize) != 0 ) {
                diag.error("initializer section %s/%s is not pointer aligned", info.segInfo.segName, info.sectName);
                sectStop = true;
                return;
            }
            callback((uint32_t)(info.sectAddr - baseAddress), (uint32_t)info.sectSize, content, sectStop);
        }
    });
}

struct VIS_HIDDEN SegmentRanges
{
    struct SegmentRange {
        uint64_t vmAddrStart;
        uint64_t vmAddrEnd;
        uint32_t fileSize;
    };

    bool contains(uint64_t vmAddr) const {
        for (const SegmentRange& range : segments) {
            if ( (range.vmAddrStart <= vmAddr) && (vmAddr < range.vmAddrEnd) )
                return true;
        }
        return false;
    }

private:
    SegmentRange localAlloc[1];

public:
    dyld3::OverflowSafeArray<SegmentRange> segments { localAlloc, sizeof(localAlloc) / sizeof(localAlloc[0]) };
};

void MachOAnalyzer::forEachInitializer(Diagnostics& diag, bool contentRebased, void (^callback)(uint32_t offset), const void* dyldCache) const
{
    __block SegmentRanges executableSegments;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( (info.protections & VM_PROT_EXECUTE) != 0 ) {
            executableSegments.segments.push_back({ info.vmAddr, info.vmAddr + info.vmSize, (uint32_t)info.fileSize });
        }
    });

    if (executableSegments.segments.empty()) {
        diag.error("no exeutable segments");
        return;
    }

    uint64_t loadAddress = preferredLoadAddress();
    intptr_t slide = getSlide();

    // if dylib linked with -init linker option, that initializer is first
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_ROUTINES ) {
            const routines_command* routines = (routines_command*)cmd;
            uint64_t dashInit = routines->init_address;
            if ( executableSegments.contains(dashInit) )
                callback((uint32_t)(dashInit - loadAddress));
            else
                diag.error("-init does not point within __TEXT segment");
        }
        else if ( cmd->cmd == LC_ROUTINES_64 ) {
            const routines_command_64* routines = (routines_command_64*)cmd;
            uint64_t dashInit = routines->init_address;
            if ( executableSegments.contains(dashInit) )
                callback((uint32_t)(dashInit - loadAddress));
            else
                diag.error("-init does not point within __TEXT segment");
        }
    });

    // next any function pointers in mod-init section
    const unsigned ptrSize          = pointerSize();
    const bool     useChainedFixups = hasChainedFixups();
    const uint16_t pointerFormat    = useChainedFixups ? this->chainedPointerFormat() : 0;
    forEachInitializerPointerSection(diag, ^(uint32_t sectionOffset, uint32_t sectionSize, const uint8_t* content, bool& stop) {
        if ( ptrSize == 8 ) {
            const uint64_t* initsStart = (uint64_t*)content;
            const uint64_t* initsEnd   = (uint64_t*)((uint8_t*)content + sectionSize);
            for (const uint64_t* p=initsStart; p < initsEnd; ++p) {
                uint64_t anInit = *p;
                if ( contentRebased ) {
                    // The function pointer may have been signed.  Strip the signature if that is the case
#if __has_feature(ptrauth_calls)
                    anInit = (uint64_t)__builtin_ptrauth_strip((void*)anInit, ptrauth_key_asia);
#endif
                    anInit -= slide;
                }
                else if ( useChainedFixups ) {
                    uint64_t initFuncRuntimeOffset;
                    ChainedFixupPointerOnDisk* aChainedInit = (ChainedFixupPointerOnDisk*)p;
                    if ( aChainedInit->isRebase(pointerFormat, loadAddress, initFuncRuntimeOffset) ) {
                        anInit = loadAddress+initFuncRuntimeOffset;
                    }
                    else {
                        diag.error("initializer is not rebased");
                        stop = true;
                        break;
                    }
                }
                if ( !executableSegments.contains(anInit) ) {
                     diag.error("initializer 0x%0llX does not point within executable segment", anInit);
                     stop = true;
                     break;
                }
                callback((uint32_t)(anInit - loadAddress));
            }
        }
        else {
            const uint32_t* initsStart = (uint32_t*)content;
            const uint32_t* initsEnd   = (uint32_t*)((uint8_t*)content + sectionSize);
            for (const uint32_t* p=initsStart; p < initsEnd; ++p) {
                uint32_t anInit = *p;
                if ( contentRebased ) {
                    anInit -= slide;
                }
                else if ( useChainedFixups ) {
                    uint64_t initFuncRuntimeOffset;
                    ChainedFixupPointerOnDisk* aChainedInit = (ChainedFixupPointerOnDisk*)p;
                    if ( aChainedInit->isRebase(pointerFormat, loadAddress, initFuncRuntimeOffset) ) {
                        anInit = (uint32_t)(loadAddress+initFuncRuntimeOffset);
                    }
                    else {
                        diag.error("initializer is not rebased");
                        stop = true;
                        break;
                    }
                }
                if ( !executableSegments.contains(anInit) ) {
                     diag.error("initializer 0x%0X does not point within executable segment", anInit);
                     stop = true;
                     break;
                }
                callback(anInit - (uint32_t)loadAddress);
            }
        }
    });

    forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& stop) {
        if ( (info.sectFlags & SECTION_TYPE) != S_INIT_FUNC_OFFSETS )
            return;
        const uint8_t* content = (uint8_t*)(info.sectAddr + slide);
        if ( info.segInfo.writable() ) {
            diag.error("initializer offsets section %s/%s must be in read-only segment", info.segInfo.segName, info.sectName);
            stop = true;
            return;
        }
        if ( (info.sectSize % 4) != 0 ) {
            diag.error("initializer offsets section %s/%s has bad size", info.segInfo.segName, info.sectName);
            stop = true;
            return;
        }
        if ( malformedSectionRange ) {
            diag.error("initializer offsets section %s/%s extends beyond the end of the segment", info.segInfo.segName, info.sectName);
            stop = true;
            return;
        }
        if ( (info.sectAddr % 4) != 0 ) {
            diag.error("initializer offsets section %s/%s is not 4-byte aligned", info.segInfo.segName, info.sectName);
            stop = true;
            return;
        }
        const uint32_t* initsStart = (uint32_t*)content;
        const uint32_t* initsEnd   = (uint32_t*)((uint8_t*)content + info.sectSize);
        for (const uint32_t* p=initsStart; p < initsEnd; ++p) {
            uint32_t anInitOffset = *p;
            if ( anInitOffset > executableSegments.segments[0].fileSize ) {
                 diag.error("initializer 0x%0X is not an offset within __TEXT segment", anInitOffset);
                 stop = true;
                 break;
            }
            callback(anInitOffset);
        }
    });
}

bool MachOAnalyzer::hasTerminators(Diagnostics& diag, bool contentRebased) const
{
    __block bool result = false;
    forEachTerminator(diag, contentRebased, ^(uint32_t offset) {
        result = true;
    });
    return result;
}

void MachOAnalyzer::forEachTerminator(Diagnostics& diag, bool contentRebased, void (^callback)(uint32_t offset)) const
{
    __block SegmentRanges executableSegments;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( (info.protections & VM_PROT_EXECUTE) != 0 ) {
            executableSegments.segments.push_back({ info.vmAddr, info.vmAddr + info.vmSize, (uint32_t)info.fileSize });
        }
    });

    if (executableSegments.segments.empty()) {
        diag.error("no exeutable segments");
        return;
    }

    uint64_t loadAddress = preferredLoadAddress();
    intptr_t slide = getSlide();

    // next any function pointers in mod-term section
    const unsigned ptrSize          = pointerSize();
    const bool     useChainedFixups = hasChainedFixups();
    forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& stop) {
        if ( (info.sectFlags & SECTION_TYPE) == S_MOD_TERM_FUNC_POINTERS ) {
            uint64_t initFuncRuntimeOffset;
            const uint16_t pointerFormat = useChainedFixups ? this->chainedPointerFormat() : 0;
            const uint8_t* content;
            content = (uint8_t*)(info.sectAddr + slide);
            if ( (info.sectSize % ptrSize) != 0 ) {
                diag.error("terminator section %s/%s has bad size", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            if ( malformedSectionRange ) {
                diag.error("terminator section %s/%s extends beyond its segment", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            if ( ((long)content % ptrSize) != 0 ) {
                diag.error("terminator section %s/%s is not pointer aligned", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            if ( ptrSize == 8 ) {
                const uint64_t* initsStart = (uint64_t*)content;
                const uint64_t* initsEnd   = (uint64_t*)((uint8_t*)content + info.sectSize);
                for (const uint64_t* p=initsStart; p < initsEnd; ++p) {
                    uint64_t anInit = *p;
                    if ( contentRebased ) {
                        // The function pointer may have been signed.  Strip the signature if that is the case
#if __has_feature(ptrauth_calls)
                        anInit = (uint64_t)__builtin_ptrauth_strip((void*)anInit, ptrauth_key_asia);
#endif
                        anInit -= slide;
                    }
                    else if ( useChainedFixups ) {
                        ChainedFixupPointerOnDisk* aChainedInit = (ChainedFixupPointerOnDisk*)p;
                        if ( aChainedInit->isRebase(pointerFormat, loadAddress, initFuncRuntimeOffset) ) {
                            anInit = loadAddress+initFuncRuntimeOffset;
                        }
                        else {
                            diag.error("terminator is not rebased");
                            stop = true;
                            break;
                        }
                    }
                    if ( !executableSegments.contains(anInit) ) {
                         diag.error("terminator 0x%0llX does not point within executable segment", anInit);
                         stop = true;
                         break;
                    }
                    callback((uint32_t)(anInit - loadAddress));
                }
            }
            else {
                const uint32_t* initsStart = (uint32_t*)content;
                const uint32_t* initsEnd   = (uint32_t*)((uint8_t*)content + info.sectSize);
                for (const uint32_t* p=initsStart; p < initsEnd; ++p) {
                    uint32_t anInit = *p;
                    if ( contentRebased ) {
                        anInit -= slide;
                    }
                    else if ( useChainedFixups ) {
                        ChainedFixupPointerOnDisk* aChainedInit = (ChainedFixupPointerOnDisk*)p;
                        if ( aChainedInit->isRebase(pointerFormat, loadAddress, initFuncRuntimeOffset) ) {
                            anInit = (uint32_t)(loadAddress+initFuncRuntimeOffset);
                        }
                        else {
                            diag.error("terminator is not rebased");
                            stop = true;
                            break;
                        }
                    }
                    if ( !executableSegments.contains(anInit) ) {
                         diag.error("terminator 0x%0X does not point within executable segment", anInit);
                         stop = true;
                         break;
                    }
                    callback(anInit - (uint32_t)loadAddress);
                }
            }
        }
    });
}



void MachOAnalyzer::forEachRPath(void (^callback)(const char* rPath, bool& stop)) const
{
    Diagnostics diag;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
         if ( cmd->cmd == LC_RPATH ) {
            const char* rpath = (char*)cmd + ((struct rpath_command*)cmd)->path.offset;
            callback(rpath, stop);
        }
    });
    diag.assertNoError();   // any malformations in the file should have been caught by earlier validate() call
}


bool MachOAnalyzer::hasObjC() const
{
    __block bool result = false;
    forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& stop) {
        if ( (strcmp(info.sectName, "__objc_imageinfo") == 0) && (strncmp(info.segInfo.segName, "__DATA", 6) == 0) ) {
            result = true;
            stop = true;
        }
        if ( (this->cputype == CPU_TYPE_I386) && (strcmp(info.sectName, "__image_info") == 0) && (strcmp(info.segInfo.segName, "__OBJC") == 0) ) {
            result = true;
            stop = true;
        }
    });
    return result;
}

bool MachOAnalyzer::hasPlusLoadMethod(Diagnostics& diag) const
{
    __block bool result = false;
    if ( (this->cputype == CPU_TYPE_I386) && supportsPlatform(Platform::macOS) ) {
        // old objc runtime has no special section for +load methods, scan for string
        int64_t slide = getSlide();
        forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& stop) {
            if ( ( (info.sectFlags & SECTION_TYPE) == S_CSTRING_LITERALS ) ) {
                if ( malformedSectionRange ) {
                    diag.error("cstring section %s/%s extends beyond the end of the segment", info.segInfo.segName, info.sectName);
                    stop = true;
                    return;
                }
                const uint8_t* content = (uint8_t*)(info.sectAddr + slide);
                const char* s   = (char*)content;
                const char* end = s + info.sectSize;
                while ( s < end ) {
                    if ( strcmp(s, "load") == 0 ) {
                        result = true;
                        stop = true;
                        return;
                    }
                    while (*s != '\0' )
                        ++s;
                    ++s;
                }
            }
        });
    }
    else {
        // in new objc runtime compiler puts classes/categories with +load method in specical section
        forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool& stop) {
            if ( strncmp(info.segInfo.segName, "__DATA", 6) != 0 )
                return;
            if ( (strcmp(info.sectName, "__objc_nlclslist") == 0) || (strcmp(info.sectName, "__objc_nlcatlist") == 0)) {
                result = true;
                stop = true;
            }
        });
    }
    return result;
}

const void* MachOAnalyzer::getRebaseOpcodes(uint32_t& size) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.dyldInfo == nullptr) )
        return nullptr;

    size = leInfo.dyldInfo->rebase_size;
    return getLinkEditContent(leInfo.layout, leInfo.dyldInfo->rebase_off);
}

const void* MachOAnalyzer::getBindOpcodes(uint32_t& size) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.dyldInfo == nullptr) )
        return nullptr;

    size = leInfo.dyldInfo->bind_size;
    return getLinkEditContent(leInfo.layout, leInfo.dyldInfo->bind_off);
}

const void* MachOAnalyzer::getLazyBindOpcodes(uint32_t& size) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.dyldInfo == nullptr) )
        return nullptr;

    size = leInfo.dyldInfo->lazy_bind_size;
    return getLinkEditContent(leInfo.layout, leInfo.dyldInfo->lazy_bind_off);
}

const void* MachOAnalyzer::getSplitSeg(uint32_t& size) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.splitSegInfo == nullptr) )
        return nullptr;

    size = leInfo.splitSegInfo->datasize;
    return getLinkEditContent(leInfo.layout, leInfo.splitSegInfo->dataoff);
}


uint64_t MachOAnalyzer::segAndOffsetToRuntimeOffset(uint8_t targetSegIndex, uint64_t targetSegOffset) const
{
    __block uint64_t textVmAddr = 0;
    __block uint64_t result     = 0;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 )
            textVmAddr = info.vmAddr;
        if ( info.segIndex == targetSegIndex ) {
            result = (info.vmAddr - textVmAddr) + targetSegOffset;
        }
    });
    return result;
}

bool MachOAnalyzer::hasLazyPointers(uint32_t& runtimeOffset, uint32_t& size) const
{
    size = 0;
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool &stop) {
        if ( (info.sectFlags & SECTION_TYPE) == S_LAZY_SYMBOL_POINTERS ) {
            runtimeOffset = (uint32_t)(info.sectAddr - preferredLoadAddress());
            size          = (uint32_t)info.sectSize;
            stop = true;
        }
    });
    return (size != 0);
}

uint64_t MachOAnalyzer::preferredLoadAddress() const
{
    __block uint64_t textVmAddr = 0;
    forEachSegment(^(const SegmentInfo& info, bool& stop) {
        if ( strcmp(info.segName, "__TEXT") == 0 ) {
            textVmAddr = info.vmAddr;
            stop = true;
        }
    });
    return textVmAddr;
}


bool MachOAnalyzer::getEntry(uint32_t& offset, bool& usesCRT) const
{
    Diagnostics diag;
    offset = 0;
    forEachLoadCommand(diag, ^(const load_command* cmd, bool& stop) {
        if ( cmd->cmd == LC_MAIN ) {
            entry_point_command* mainCmd = (entry_point_command*)cmd;
            usesCRT = false;
            offset = (uint32_t)mainCmd->entryoff;
            stop = true;
        }
        else if ( cmd->cmd == LC_UNIXTHREAD ) {
            stop = true;
            usesCRT = true;
            uint64_t startAddress = entryAddrFromThreadCmd((thread_command*)cmd);
            offset = (uint32_t)(startAddress - preferredLoadAddress());
        }
    });
    return (offset != 0);
}

uint64_t MachOAnalyzer::entryAddrFromThreadCmd(const thread_command* cmd) const
{
    assert(cmd->cmd == LC_UNIXTHREAD);
    const uint32_t* regs32 = (uint32_t*)(((char*)cmd) + 16);
    const uint64_t* regs64 = (uint64_t*)(((char*)cmd) + 16);
    uint64_t startAddress = 0;
    switch ( this->cputype ) {
        case CPU_TYPE_I386:
            startAddress = regs32[10]; // i386_thread_state_t.eip
            break;
        case CPU_TYPE_X86_64:
            startAddress = regs64[16]; // x86_thread_state64_t.rip
            break;
        case CPU_TYPE_ARM:
            startAddress = regs32[15]; // arm_thread_state_t.pc
            break;
        case CPU_TYPE_ARM64:
            startAddress = regs64[32]; // arm_thread_state64_t.__pc
            break;
    }
    return startAddress;
}


void MachOAnalyzer::forEachInterposingSection(Diagnostics& diag, void (^handler)(uint64_t vmOffset, uint64_t vmSize, bool& stop)) const
{
    const unsigned ptrSize   = pointerSize();
    const unsigned entrySize = 2 * ptrSize;
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool &stop) {
        if ( ((info.sectFlags & SECTION_TYPE) == S_INTERPOSING) || ((strcmp(info.sectName, "__interpose") == 0) && (strcmp(info.segInfo.segName, "__DATA") == 0)) ) {
            if ( info.sectSize % entrySize != 0 ) {
                diag.error("interposing section %s/%s has bad size", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            if ( malformedSectionRange ) {
                diag.error("interposing section %s/%s extends beyond the end of the segment", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            if ( (info.sectAddr % ptrSize) != 0 ) {
                diag.error("interposing section %s/%s is not pointer aligned", info.segInfo.segName, info.sectName);
                stop = true;
                return;
            }
            handler(info.sectAddr - preferredLoadAddress(), info.sectSize, stop);
        }
    });
}

void MachOAnalyzer::forEachDOFSection(Diagnostics& diag, void (^callback)(uint32_t offset)) const
{
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool &stop) {
        if ( ( (info.sectFlags & SECTION_TYPE) == S_DTRACE_DOF ) && !malformedSectionRange ) {
            callback((uint32_t)(info.sectAddr - info.segInfo.vmAddr));
        }
    });
}

void MachOAnalyzer::forEachCDHash(void (^handler)(const uint8_t cdHash[20])) const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.codeSig == nullptr) )
        return;

    forEachCDHashOfCodeSignature(getLinkEditContent(leInfo.layout, leInfo.codeSig->dataoff),
                                 leInfo.codeSig->datasize,
                                 handler);
}

bool MachOAnalyzer::isRestricted() const
{
    __block bool result = false;
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& info, bool malformedSectionRange, bool &stop) {
        if ( (strcmp(info.segInfo.segName, "__RESTRICT") == 0) && (strcmp(info.sectName, "__restrict") == 0) ) {
            result = true;
            stop = true;
        }
    });
    return result;
}

bool MachOAnalyzer::usesLibraryValidation() const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.codeSig == nullptr) )
        return false;

    // check for CS_REQUIRE_LV in CS_CodeDirectory.flags
    __block bool requiresLV = false;
    forEachCodeDirectoryBlob(getLinkEditContent(leInfo.layout, leInfo.codeSig->dataoff),
                             leInfo.codeSig->datasize,
                             ^(const void *cdBuffer) {
         const CS_CodeDirectory* cd = (const CS_CodeDirectory*)cdBuffer;
         requiresLV |= (htonl(cd->flags) & CS_REQUIRE_LV);
    });

    return requiresLV;
}

bool MachOAnalyzer::canHavePrecomputedDlopenClosure(const char* path, void (^failureReason)(const char*)) const
{
    __block bool retval = true;

    // only dylibs can go in cache
    if ( (this->filetype != MH_DYLIB) && (this->filetype != MH_BUNDLE) ) {
        retval = false;
        failureReason("not MH_DYLIB or MH_BUNDLE");
    }

    // flat namespace files cannot go in cache
    if ( (this->flags & MH_TWOLEVEL) == 0 ) {
        retval = false;
        failureReason("not built with two level namespaces");
    }

    // can only depend on other dylibs with absolute paths
    __block bool allDepPathsAreGood = true;
    forEachDependentDylib(^(const char* loadPath, bool isWeak, bool isReExport, bool isUpward, uint32_t compatVersion, uint32_t curVersion, bool& stop) {
        if ( loadPath[0] != '/' ) {
            allDepPathsAreGood = false;
            stop = true;
        }
    });
    if ( !allDepPathsAreGood ) {
        retval = false;
        failureReason("depends on dylibs that are not absolute paths");
    }

    // dylibs with interposing info cannot have dlopen closure pre-computed
    __block bool hasInterposing = false;
    forEachSection(^(const SectionInfo& info, bool malformedSectionRange, bool &stop) {
        if ( ((info.sectFlags & SECTION_TYPE) == S_INTERPOSING) || ((strcmp(info.sectName, "__interpose") == 0) && (strcmp(info.segInfo.segName, "__DATA") == 0)) )
            hasInterposing = true;
    });
    if ( hasInterposing ) {
        retval = false;
        failureReason("has interposing tuples");
    }

    // images that use dynamic_lookup, bundle_loader, or have weak-defs cannot have dlopen closure pre-computed
    Diagnostics diag;
    auto checkBind = ^(int libOrdinal, bool& stop) {
        switch (libOrdinal) {
            case BIND_SPECIAL_DYLIB_WEAK_LOOKUP:
                failureReason("has weak externals");
                retval = false;
                stop = true;
                break;
            case BIND_SPECIAL_DYLIB_FLAT_LOOKUP:
                failureReason("has dynamic_lookup binds");
                retval = false;
                stop = true;
                break;
            case BIND_SPECIAL_DYLIB_MAIN_EXECUTABLE:
                failureReason("has reference to main executable (bundle loader)");
                retval = false;
                stop = true;
                break;
        }
    };

    if (hasChainedFixups()) {
        forEachChainedFixupTarget(diag, ^(int libOrdinal, const char *symbolName, uint64_t addend, bool weakImport, bool &stop) {
            checkBind(libOrdinal, stop);
        });
    } else {
        forEachBind(diag, ^(uint64_t runtimeOffset, int libOrdinal, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
            checkBind(libOrdinal, stop);
        },
        ^(const char* symbolName) {
        },
        ^() {
        });
    }

    // special system dylib overrides cannot have closure pre-computed
    if ( strncmp(path, "/usr/lib/system/introspection/", 30) == 0 ) {
        retval = false;
        failureReason("override of OS dylib");
    }
    
    // Don't precompute iOSMac for now until dyld3 support is there.
    if ( supportsPlatform(Platform::iOSMac) && !supportsPlatform(Platform::macOS) ) {
        retval = false;
        failureReason("UIKitForMac binary");
    }

    return retval;
}


bool MachOAnalyzer::hasUnalignedPointerFixups() const
{
    // only look at 64-bit architectures
    if ( pointerSize() == 4 )
        return false;

    __block Diagnostics diag;
    __block bool result = false;
    if ( hasChainedFixups() ) {
        withChainStarts(diag, chainStartsOffset(), ^(const dyld_chained_starts_in_image* startsInfo) {
            forEachFixupInAllChains(diag, startsInfo, false, ^(MachOLoaded::ChainedFixupPointerOnDisk* fixupLoc, const dyld_chained_starts_in_segment* segInfo, bool& fixupsStop) {
                if ( ((long)(fixupLoc) & 7) != 0 ) {
                    result = true;
                    fixupsStop = true;
                }
           });
        });
    }
    else {
        forEachBind(diag, ^(uint64_t runtimeOffset, int libOrdinal, const char* symbolName, bool weakImport, bool lazyBind, uint64_t addend, bool& stop) {
            if ( (runtimeOffset & 7) != 0 ) {
                result = true;
                stop = true;
            }
        },
        ^(const char* symbolName) {
        },
        ^() {
        });
        forEachRebase(diag, true, ^(uint64_t runtimeOffset, bool& stop) {
            if ( (runtimeOffset & 7) != 0 ) {
                result = true;
                stop = true;
            }
        });
    }

    return result;
}

void MachOAnalyzer::recurseTrie(Diagnostics& diag, const uint8_t* const start, const uint8_t* p, const uint8_t* const end,
                                OverflowSafeArray<char>& cummulativeString, int curStrOffset, bool& stop, ExportsCallback callback) const
{
    if ( p >= end ) {
        diag.error("malformed trie, node past end");
        return;
    }
    const uint64_t terminalSize = read_uleb128(diag, p, end);
    const uint8_t* children = p + terminalSize;
    if ( terminalSize != 0 ) {
        uint64_t    imageOffset = 0;
        uint64_t    flags       = read_uleb128(diag, p, end);
        uint64_t    other       = 0;
        const char* importName  = nullptr;
        if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
            other = read_uleb128(diag, p, end); // dylib ordinal
            importName = (char*)p;
        }
        else {
            imageOffset = read_uleb128(diag, p, end);
            if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
                other = read_uleb128(diag, p, end);
            else
                other = 0;
        }
        if ( diag.hasError() )
            return;
        callback(cummulativeString.begin(), imageOffset, flags, other, importName, stop);
        if ( stop )
            return;
    }
    if ( children > end ) {
        diag.error("malformed trie, terminalSize extends beyond trie data");
        return;
    }
    const uint8_t childrenCount = *children++;
    const uint8_t* s = children;
    for (uint8_t i=0; i < childrenCount; ++i) {
        int edgeStrLen = 0;
        while (*s != '\0') {
            cummulativeString.resize(curStrOffset+edgeStrLen + 1);
            cummulativeString[curStrOffset+edgeStrLen] = *s++;
            ++edgeStrLen;
            if ( s > end ) {
                diag.error("malformed trie node, child node extends past end of trie\n");
                return;
            }
       }
        cummulativeString.resize(curStrOffset+edgeStrLen + 1);
        cummulativeString[curStrOffset+edgeStrLen] = *s++;
        uint64_t childNodeOffset = read_uleb128(diag, s, end);
        if (childNodeOffset == 0) {
            diag.error("malformed trie, childNodeOffset==0");
            return;
        }
        recurseTrie(diag, start, start+childNodeOffset, end, cummulativeString, curStrOffset+edgeStrLen, stop, callback);
        if ( diag.hasError() || stop )
            return;
    }
}

void MachOAnalyzer::forEachExportedSymbol(Diagnostics& diag, ExportsCallback callback) const
{
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;
    uint64_t trieSize;
    if ( const uint8_t* trieStart = getExportsTrie(leInfo, trieSize) ) {
        const uint8_t* trieEnd   = trieStart + trieSize;
        bool stop = false;
        STACK_ALLOC_OVERFLOW_SAFE_ARRAY(char, cummulativeString, 4096);
        recurseTrie(diag, trieStart, trieStart, trieEnd, cummulativeString, 0, stop, callback);
   }
}

bool MachOAnalyzer::canBePlacedInDyldCache(const char* path, void (^failureReason)(const char*)) const
{
    if (!MachOFile::canBePlacedInDyldCache(path, failureReason))
        return false;
    if ( !(isArch("x86_64") || isArch("x86_64h")) )
        return true;

    __block bool rebasesOk = true;
    Diagnostics diag;
    uint64_t startVMAddr = preferredLoadAddress();
    uint64_t endVMAddr = startVMAddr + mappedSize();
    forEachRebase(diag, false, ^(uint64_t runtimeOffset, bool &stop) {
        // We allow TBI for x86_64 dylibs, but then require that the remainder of the offset
        // is a 32-bit offset from the mach-header.
        uint64_t value = *(uint64_t*)((uint8_t*)this + runtimeOffset);
        value &= 0x00FFFFFFFFFFFFFFULL;
        if ( (value < startVMAddr) || (value >= endVMAddr) ) {
            failureReason("rebase value out of range of dylib");
            rebasesOk = false;
            stop = true;
            return;
        }

        // Also error if the rebase location is anything other than 4/8 byte aligned
        if ( (runtimeOffset & 0x3) != 0 ) {
            failureReason("rebase value is not 4-byte aligned");
            rebasesOk = false;
            stop = true;
            return;
        }
    });
    return rebasesOk;
}

uint64_t MachOAnalyzer::chainStartsOffset() const
{
    Diagnostics diag;
    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() || (leInfo.chainedFixups == nullptr) )
        return 0;

    const dyld_chained_fixups_header* header = (dyld_chained_fixups_header*)getLinkEditContent(leInfo.layout, leInfo.chainedFixups->dataoff);
    return header->starts_offset + ((uint8_t*)header - (uint8_t*)this);
}

uint16_t MachOAnalyzer::chainedPointerFormat() const
{
    uint64_t infoOffset = chainStartsOffset();
    if ( infoOffset != 0 ) {
        // get pointer format from chain info struct in LINKEDIT
        const dyld_chained_starts_in_image* startsInfo = (dyld_chained_starts_in_image*)((uint8_t*)this + infoOffset);
        for (uint32_t i=0; i < startsInfo->seg_count; ++i) {
            uint32_t segInfoOffset = startsInfo->seg_info_offset[i];
            // 0 offset means this segment has no fixups
            if ( segInfoOffset == 0 )
                continue;
            const dyld_chained_starts_in_segment* segInfo = (dyld_chained_starts_in_segment*)((uint8_t*)startsInfo + segInfoOffset);
            if ( segInfo->page_count != 0 )
                return segInfo->pointer_format;
        }
    }
    assert(this->cputype == CPU_TYPE_ARM64 && this->cpusubtype == CPU_SUBTYPE_ARM64E && "chainedPointerFormat() called on non-chained binary");
    return DYLD_CHAINED_PTR_ARM64E;
}

#if (BUILDING_DYLD || BUILDING_LIBDYLD) && !__arm64e__
  #define SUPPORT_OLD_ARM64E_FORMAT 0
#else
  #define SUPPORT_OLD_ARM64E_FORMAT 1
#endif

// find dyld_chained_starts_in_image* in image
// if old arm64e binary, synthesize dyld_chained_starts_in_image*
void MachOAnalyzer::withChainStarts(Diagnostics& diag, uint64_t startsStructOffsetHint, void (^callback)(const dyld_chained_starts_in_image*)) const
{
    if ( startsStructOffsetHint != 0 ) {
        // we have a pre-computed offset into LINKEDIT for dyld_chained_starts_in_image
        callback((dyld_chained_starts_in_image*)((uint8_t*)this + startsStructOffsetHint));
        return;
    }

    LinkEditInfo leInfo;
    getLinkEditPointers(diag, leInfo);
    if ( diag.hasError() )
        return;

    if ( leInfo.chainedFixups != nullptr ) {
        // find dyld_chained_starts_in_image from dyld_chained_fixups_header
        const dyld_chained_fixups_header* header = (dyld_chained_fixups_header*)getLinkEditContent(leInfo.layout, leInfo.chainedFixups->dataoff);
        callback((dyld_chained_starts_in_image*)((uint8_t*)header + header->starts_offset));
    }
#if SUPPORT_OLD_ARM64E_FORMAT
    // don't want this code in non-arm64e dyld because it causes a stack protector which dereferences a GOT pointer before GOT is set up
    else if ( (leInfo.dyldInfo != nullptr) && (this->cputype == CPU_TYPE_ARM64) && (this->cpusubtype == CPU_SUBTYPE_ARM64E) ) {
        // old arm64e binary, create a dyld_chained_starts_in_image for caller
        uint64_t baseAddress = preferredLoadAddress();
        BLOCK_ACCCESSIBLE_ARRAY(uint8_t, buffer, leInfo.dyldInfo->bind_size + 512);
        dyld_chained_starts_in_image* header = (dyld_chained_starts_in_image*)buffer;
        header->seg_count = leInfo.layout.linkeditSegIndex;
        for (uint32_t i=0; i < header->seg_count; ++i)
            header->seg_info_offset[i] = 0;
        __block uint8_t curSegIndex = 0;
        __block dyld_chained_starts_in_segment* curSeg = (dyld_chained_starts_in_segment*)(&(header->seg_info_offset[header->seg_count]));
        parseOrgArm64eChainedFixups(diag, nullptr, nullptr, ^(const LinkEditInfo& leInfo2, const SegmentInfo segments[], uint8_t segmentIndex,
                                                              bool segIndexSet, uint64_t segmentOffset, uint16_t format, bool& stop) {
            uint32_t pageIndex = (uint32_t)(segmentOffset/0x1000);
            if ( segmentIndex != curSegIndex ) {
                if ( curSegIndex == 0 ) {
                    header->seg_info_offset[segmentIndex] = (uint32_t)((uint8_t*)curSeg - buffer);
                }
                else {
                    header->seg_info_offset[segmentIndex] = (uint32_t)((uint8_t*)(&curSeg->page_start[curSeg->page_count]) - buffer);
                    curSeg = (dyld_chained_starts_in_segment*)((uint8_t*)header+header->seg_info_offset[segmentIndex]);
               }
               curSeg->page_count = 0;
               curSegIndex = segmentIndex;
            }
            while ( curSeg->page_count != pageIndex ) {
                curSeg->page_start[curSeg->page_count] = 0xFFFF;
                curSeg->page_count++;
            }
            curSeg->size                  = (uint32_t)((uint8_t*)(&curSeg->page_start[pageIndex]) - (uint8_t*)curSeg);
            curSeg->page_size             = 0x1000; // old arm64e encoding used 4KB pages
            curSeg->pointer_format        = DYLD_CHAINED_PTR_ARM64E;
            curSeg->segment_offset        = segments[segmentIndex].vmAddr - baseAddress;
            curSeg->max_valid_pointer     = 0;
            curSeg->page_count            = pageIndex+1;
            curSeg->page_start[pageIndex] = segmentOffset & 0xFFF;
            //fprintf(stderr, "segment_offset=0x%llX, vmAddr=0x%llX\n", curSeg->segment_offset, segments[segmentIndex].vmAddr );
            //printf("segIndex=%d, segOffset=0x%08llX, page_start[%d]=0x%04X, page_start[%d]=0x%04X\n",
            //        segmentIndex, segmentOffset, pageIndex, curSeg->page_start[pageIndex], pageIndex-1, pageIndex ? curSeg->page_start[pageIndex-1] : 0);
        });
        callback(header);
    }
#endif
    else {
        diag.error("image does not use chained fixups");
    }
}

MachOAnalyzer::ObjCInfo MachOAnalyzer::getObjCInfo() const
{
    __block ObjCInfo result;
    result.selRefCount      = 0;
    result.classDefCount    = 0;
    result.protocolDefCount = 0;

    const uint32_t ptrSize  = pointerSize();
    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) == 0 ) {
            if ( strcmp(sectInfo.sectName, "__objc_selrefs") == 0 )
                result.selRefCount += (sectInfo.sectSize/ptrSize);
            else if ( strcmp(sectInfo.sectName, "__objc_classlist") == 0 )
                result.classDefCount += (sectInfo.sectSize/ptrSize);
            else if ( strcmp(sectInfo.sectName, "__objc_protolist") == 0 )
                result.protocolDefCount += (sectInfo.sectSize/ptrSize);
        }
        else if ( (this->cputype == CPU_TYPE_I386) && (strcmp(sectInfo.segInfo.segName, "__OBJC") == 0) ) {
            if ( strcmp(sectInfo.sectName, "__message_refs") == 0 )
                result.selRefCount += (sectInfo.sectSize/4);
            else if ( strcmp(sectInfo.sectName, "__class") == 0 )
                result.classDefCount += (sectInfo.sectSize/48);
            else if ( strcmp(sectInfo.sectName, "__protocol") == 0 )
                result.protocolDefCount += (sectInfo.sectSize/20);
        }
   });

    return result;
}

// Convert from a (possibly) live pointer to a vmAddr
static uint64_t convertToVMAddr(uint64_t value, MachOAnalyzer::VMAddrConverter vmAddrConverter) {
    if ( vmAddrConverter.contentRebased ) {
        // The value may have been signed.  Strip the signature if that is the case
#if __has_feature(ptrauth_calls)
        value = (uint64_t)__builtin_ptrauth_strip((void*)value, ptrauth_key_asia);
#endif
        value -= vmAddrConverter.slide;
    }
    else if ( vmAddrConverter.chainedPointerFormat != 0 ) {
        auto* chainedValue = (MachOAnalyzer::ChainedFixupPointerOnDisk*)&value;
        uint64_t targetRuntimeOffset;
        if ( chainedValue->isRebase(vmAddrConverter.chainedPointerFormat, vmAddrConverter.preferredLoadAddress,
                                    targetRuntimeOffset) ) {
            value = vmAddrConverter.preferredLoadAddress + targetRuntimeOffset;
        }
    }

    return value;
}

uint64_t MachOAnalyzer::ObjCClassInfo::getReadOnlyDataField(ObjCClassInfo::ReadOnlyDataField field, uint32_t pointerSize) const {
    if (pointerSize == 8) {
        typedef uint64_t PtrTy;
        struct class_ro_t {
            uint32_t flags;
            uint32_t instanceStart;
            // Note there is 4-bytes of alignment padding between instanceSize and ivarLayout
            // on 64-bit archs, but no padding on 32-bit archs.
            // This union is a way to model that.
            union {
                uint32_t   instanceSize;
                PtrTy   pad;
            } instanceSize;
            PtrTy ivarLayoutVMAddr;
            PtrTy nameVMAddr;
            PtrTy baseMethodsVMAddr;
            PtrTy baseProtocolsVMAddr;
            PtrTy ivarsVMAddr;
            PtrTy weakIvarLayoutVMAddr;
            PtrTy basePropertiesVMAddr;
        };
        const class_ro_t* classData = (const class_ro_t*)(dataVMAddr + vmAddrConverter.slide);
        switch (field) {
        case ObjCClassInfo::ReadOnlyDataField::name:
            return convertToVMAddr(classData->nameVMAddr, vmAddrConverter);
        case ObjCClassInfo::ReadOnlyDataField::baseMethods:
            return convertToVMAddr(classData->baseMethodsVMAddr, vmAddrConverter);
        }
    } else {
        typedef uint32_t PtrTy;
        struct class_ro_t {
            uint32_t flags;
            uint32_t instanceStart;
            // Note there is 4-bytes of alignment padding between instanceSize and ivarLayout
            // on 64-bit archs, but no padding on 32-bit archs.
            // This union is a way to model that.
            union {
                uint32_t   instanceSize;
                PtrTy   pad;
            } instanceSize;
            PtrTy ivarLayoutVMAddr;
            PtrTy nameVMAddr;
            PtrTy baseMethodsVMAddr;
            PtrTy baseProtocolsVMAddr;
            PtrTy ivarsVMAddr;
            PtrTy weakIvarLayoutVMAddr;
            PtrTy basePropertiesVMAddr;
        };
        const class_ro_t* classData = (const class_ro_t*)(dataVMAddr + vmAddrConverter.slide);
        switch (field) {
            case ObjCClassInfo::ReadOnlyDataField::name:
                return convertToVMAddr(classData->nameVMAddr, vmAddrConverter);
            case ObjCClassInfo::ReadOnlyDataField::baseMethods:
                return convertToVMAddr(classData->baseMethodsVMAddr, vmAddrConverter);
        }
    }
}

const char* MachOAnalyzer::getPrintableString(uint64_t stringVMAddr, MachOAnalyzer::PrintableStringResult& result,
                                              SectionCache* sectionCache,
                                              bool (^sectionHandler)(const SectionInfo& sectionInfo)) const {
    if ( sectionCache != nullptr ) {
        // Make sure the string is pointing in to one of the supported sections
        __block const dyld3::MachOAnalyzer::SectionInfo* nameSectionInfo = nullptr;
        for (const dyld3::MachOAnalyzer::SectionInfo& sectionInfo : sectionCache->sectionInfos) {
            if ( stringVMAddr < sectionInfo.sectAddr ) {
                continue;
            }
            if ( stringVMAddr >= ( sectionInfo.sectAddr + sectionInfo.sectSize) ) {
                continue;
            }
            nameSectionInfo = &sectionInfo;
            break;
        }

        if ( nameSectionInfo != nullptr ) {
            // The section handler may also reject this section
            if ( sectionHandler != nullptr ) {
                if (!sectionHandler(*nameSectionInfo)) {
                    result = PrintableStringResult::UnknownSection;
                    return nullptr;
                }
            }

            result = PrintableStringResult::CanPrint;
            return (const char*)(stringVMAddr + getSlide());
        }
    }

    // If the name isn't in the cache then find the section its in

    uint32_t fairplayTextOffsetStart;
    uint32_t fairplayTextOffsetEnd;
    uint32_t fairplaySize;
    if ( isFairPlayEncrypted(fairplayTextOffsetStart, fairplaySize) ) {
        fairplayTextOffsetEnd = fairplayTextOffsetStart + fairplaySize;
    } else {
        fairplayTextOffsetEnd = 0;
    }

    result = PrintableStringResult::UnknownSection;
    forEachSection(^(const MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
        if ( stringVMAddr < sectInfo.sectAddr ) {
            return;
        }
        if ( stringVMAddr >= ( sectInfo.sectAddr + sectInfo.sectSize) ) {
            return;
        }

        // We can't scan this section if its protected or not cstrings.
        if ( sectInfo.segInfo.isProtected || ( (sectInfo.sectFlags & SECTION_TYPE) != S_CSTRING_LITERALS ) ) {
            result = PrintableStringResult::ProtectedSection;
            stop = true;
            return;
        }

        // We can't scan this section if it overlaps with the fairplay range
        if ( fairplayTextOffsetEnd < sectInfo.sectFileOffset ) {
            // Fairplay range ends before section
        } else if ( fairplayTextOffsetStart > (sectInfo.sectFileOffset + sectInfo.sectSize) ) {
            // Fairplay range starts after section
        } else {
            // Must overlap
            result = PrintableStringResult::FairPlayEncrypted;
            stop = true;
            return;
        }

        // The section handler may also reject this section
        if ( sectionHandler != nullptr ) {
            if (!sectionHandler(sectInfo)) {
                result = PrintableStringResult::UnknownSection;
                stop = true;
                return;
            }
        }
        // Cache this section for later.
        if ( sectionCache != nullptr ) {
            sectionCache->sectionInfos.push_back(sectInfo);
        }
        result = PrintableStringResult::CanPrint;
        stop = true;
    });

    if (result == PrintableStringResult::CanPrint)
        return (const char*)(stringVMAddr + getSlide());
    return nullptr;
}

bool MachOAnalyzer::SectionCache::findSectionForVMAddr(uint64_t vmAddr, bool (^sectionHandler)(const SectionInfo& sectionInfo)) {

    // Make sure the string is pointing in to one of the supported sections
    __block const dyld3::MachOAnalyzer::SectionInfo* foundSectionInfo = nullptr;
    for (const dyld3::MachOAnalyzer::SectionInfo& sectionInfo : sectionInfos) {
        if ( vmAddr < sectionInfo.sectAddr ) {
            continue;
        }
        if ( vmAddr >= ( sectionInfo.sectAddr + sectionInfo.sectSize) ) {
            continue;
        }
        foundSectionInfo = &sectionInfo;
        break;
    }

    if ( foundSectionInfo != nullptr ) {
        // The section handler may also reject this section
        if ( sectionHandler != nullptr ) {
            if (!sectionHandler(*foundSectionInfo)) {
                return nullptr;
            }
        }

        // Found a section, so return true
        return true;
    }

    // If the name isn't in the cache then find the section its in

    uint32_t fairplayTextOffsetStart;
    uint32_t fairplayTextOffsetEnd;
    uint32_t fairplaySize;
    if ( ma->isFairPlayEncrypted(fairplayTextOffsetStart, fairplaySize) ) {
        fairplayTextOffsetEnd = fairplayTextOffsetStart + fairplaySize;
    } else {
        fairplayTextOffsetEnd = 0;
    }

    __block bool foundValidSection = false;
    ma->forEachSection(^(const MachOAnalyzer::SectionInfo &sectInfo, bool malformedSectionRange, bool &stop) {
        if ( vmAddr < sectInfo.sectAddr ) {
            return;
        }
        if ( vmAddr >= ( sectInfo.sectAddr + sectInfo.sectSize) ) {
            return;
        }

        // We can't scan this section if it overlaps with the fairplay range
        if ( fairplayTextOffsetEnd < sectInfo.sectFileOffset ) {
            // Fairplay range ends before section
        } else if ( fairplayTextOffsetStart > (sectInfo.sectFileOffset + sectInfo.sectSize) ) {
            // Fairplay range starts after section
        } else {
            // Must overlap
            stop = true;
            return;
        }

        // The section handler may also reject this section
        if ( sectionHandler != nullptr ) {
            if (!sectionHandler(sectInfo)) {
                stop = true;
                return;
            }
        }
        // Cache this section for later.
        sectionInfos.push_back(sectInfo);
        foundValidSection = true;
        stop = true;
    });

    return foundValidSection;
}
    
void MachOAnalyzer::forEachObjCClass(Diagnostics& diag, bool contentRebased,
                                     void (^handler)(Diagnostics& diag, uint64_t classVMAddr,
                                                     uint64_t classSuperclassVMAddr, uint64_t classDataVMAddr,
                                                     const ObjCClassInfo& objcClass, bool isMetaClass)) const {
    const uint64_t ptrSize = pointerSize();
    intptr_t slide = getSlide();

    MachOAnalyzer::VMAddrConverter vmAddrConverter;
    vmAddrConverter.preferredLoadAddress   = preferredLoadAddress();
    vmAddrConverter.slide                  = slide;
    vmAddrConverter.chainedPointerFormat   = hasChainedFixups() ? chainedPointerFormat() : 0;
    vmAddrConverter.contentRebased         = contentRebased;

    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_classlist") != 0 )
            return;
        const uint8_t*  classList       = (uint8_t*)(sectInfo.sectAddr + slide);
        uint64_t        classListSize   = sectInfo.sectSize;
        
        if ( (classListSize % ptrSize) != 0 ) {
            diag.error("Invalid objc class section size");
            return;
        }

        if ( ptrSize == 8 ) {
            typedef uint64_t PtrTy;
            struct objc_class_t {
                uint64_t isaVMAddr;
                uint64_t superclassVMAddr;
                uint64_t methodCacheBuckets;
                uint64_t methodCacheProperties;
                uint64_t dataVMAddrAndFastFlags;
            };
            // This matches "struct TargetClassMetadata" from Metadata.h in Swift
            struct swift_class_metadata_t : objc_class_t {
                uint32_t swiftClassFlags;
            };
            enum : uint64_t {
                FAST_DATA_MASK = 0x00007ffffffffff8ULL
            };
            for (uint64_t i = 0; i != classListSize; i += sizeof(PtrTy)) {
                uint64_t classVMAddr = convertToVMAddr(*(PtrTy*)(classList + i), vmAddrConverter);
                uint64_t classSuperclassVMAddr = classVMAddr + offsetof(objc_class_t, superclassVMAddr);
                uint64_t classDataVMAddr       = classVMAddr + offsetof(objc_class_t, dataVMAddrAndFastFlags);

                // First call the handler on the class
                const objc_class_t*           classPtr      = (const objc_class_t*)(classVMAddr + slide);
                const swift_class_metadata_t* swiftClassPtr = (const swift_class_metadata_t*)classPtr;
                ObjCClassInfo objcClass;
                objcClass.isaVMAddr         = convertToVMAddr(classPtr->isaVMAddr, vmAddrConverter);
                objcClass.superclassVMAddr  = convertToVMAddr(classPtr->superclassVMAddr, vmAddrConverter);
                objcClass.dataVMAddr        = convertToVMAddr(classPtr->dataVMAddrAndFastFlags, vmAddrConverter) & FAST_DATA_MASK;
                objcClass.vmAddrConverter   = vmAddrConverter;
                objcClass.isSwiftLegacy     = classPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_LEGACY;
                objcClass.isSwiftStable     = classPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_STABLE;
                // The Swift class flags are only present if the class is swift
                objcClass.swiftClassFlags   = (objcClass.isSwiftLegacy || objcClass.isSwiftStable) ? swiftClassPtr->swiftClassFlags : 0;
                handler(diag, classVMAddr, classSuperclassVMAddr, classDataVMAddr, objcClass, false);
                if (diag.hasError())
                    return;

                // Then call it on the metaclass
                const objc_class_t*             metaClassPtr        = (const objc_class_t*)(objcClass.isaVMAddr + slide);
                const swift_class_metadata_t*   swiftMetaClassPtr   = (const swift_class_metadata_t*)metaClassPtr;
                ObjCClassInfo objcMetaClass;
                objcMetaClass.isaVMAddr         = convertToVMAddr(metaClassPtr->isaVMAddr, vmAddrConverter);
                objcMetaClass.superclassVMAddr  = convertToVMAddr(metaClassPtr->superclassVMAddr, vmAddrConverter);
                objcMetaClass.dataVMAddr        = convertToVMAddr(metaClassPtr->dataVMAddrAndFastFlags, vmAddrConverter) & FAST_DATA_MASK;
                objcMetaClass.vmAddrConverter   = vmAddrConverter;
                objcMetaClass.isSwiftLegacy     = metaClassPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_LEGACY;
                objcMetaClass.isSwiftStable     = metaClassPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_STABLE;
                // The Swift class flags are only present if the class is swift
                objcMetaClass.swiftClassFlags   = (objcMetaClass.isSwiftLegacy || objcMetaClass.isSwiftStable) ? swiftMetaClassPtr->swiftClassFlags : 0;
                classSuperclassVMAddr = objcClass.isaVMAddr + offsetof(objc_class_t, superclassVMAddr);
                classDataVMAddr       = objcClass.isaVMAddr + offsetof(objc_class_t, dataVMAddrAndFastFlags);
                handler(diag, classVMAddr, classSuperclassVMAddr, classDataVMAddr, objcMetaClass, true);
                if (diag.hasError())
                    return;
            }
        } else {
            typedef uint32_t PtrTy;
            struct objc_class_t {
                uint32_t isaVMAddr;
                uint32_t superclassVMAddr;
                uint32_t methodCacheBuckets;
                uint32_t methodCacheProperties;
                uint32_t dataVMAddrAndFastFlags;
            };
            // This matches "struct TargetClassMetadata" from Metadata.h in Swift
            struct swift_class_metadata_t : objc_class_t {
                uint32_t swiftClassFlags;
            };
            enum : uint32_t {
                FAST_DATA_MASK = 0xfffffffcUL
            };
            for (uint64_t i = 0; i != classListSize; i += sizeof(PtrTy)) {
                uint64_t classVMAddr = convertToVMAddr(*(PtrTy*)(classList + i), vmAddrConverter);
                uint64_t classSuperclassVMAddr = classVMAddr + offsetof(objc_class_t, superclassVMAddr);
                uint64_t classDataVMAddr       = classVMAddr + offsetof(objc_class_t, dataVMAddrAndFastFlags);

                // First call the handler on the class
                const objc_class_t*           classPtr      = (const objc_class_t*)(classVMAddr + slide);
                const swift_class_metadata_t* swiftClassPtr = (const swift_class_metadata_t*)classPtr;
                ObjCClassInfo objcClass;
                objcClass.isaVMAddr         = convertToVMAddr(classPtr->isaVMAddr, vmAddrConverter);
                objcClass.superclassVMAddr  = convertToVMAddr(classPtr->superclassVMAddr, vmAddrConverter);
                objcClass.dataVMAddr        = convertToVMAddr(classPtr->dataVMAddrAndFastFlags, vmAddrConverter) & FAST_DATA_MASK;
                objcClass.vmAddrConverter   = vmAddrConverter;
                objcClass.isSwiftLegacy     = classPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_LEGACY;
                objcClass.isSwiftStable     = classPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_STABLE;
                // The Swift class flags are only present if the class is swift
                objcClass.swiftClassFlags   = (objcClass.isSwiftLegacy || objcClass.isSwiftStable) ? swiftClassPtr->swiftClassFlags : 0;
                handler(diag, classVMAddr, classSuperclassVMAddr, classDataVMAddr, objcClass, false);
                if (diag.hasError())
                    return;

                // Then call it on the metaclass
                const objc_class_t*             metaClassPtr        = (const objc_class_t*)(objcClass.isaVMAddr + slide);
                const swift_class_metadata_t*   swiftMetaClassPtr   = (const swift_class_metadata_t*)metaClassPtr;
                ObjCClassInfo objcMetaClass;
                objcMetaClass.isaVMAddr         = convertToVMAddr(metaClassPtr->isaVMAddr, vmAddrConverter);
                objcMetaClass.superclassVMAddr  = convertToVMAddr(metaClassPtr->superclassVMAddr, vmAddrConverter);
                objcMetaClass.dataVMAddr        = convertToVMAddr(metaClassPtr->dataVMAddrAndFastFlags, vmAddrConverter) & FAST_DATA_MASK;
                objcMetaClass.vmAddrConverter   = vmAddrConverter;
                objcMetaClass.isSwiftLegacy     = metaClassPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_LEGACY;
                objcMetaClass.isSwiftStable     = metaClassPtr->dataVMAddrAndFastFlags & ObjCClassInfo::FAST_IS_SWIFT_STABLE;
                // The Swift class flags are only present if the class is swift
                objcMetaClass.swiftClassFlags   = (objcMetaClass.isSwiftLegacy || objcMetaClass.isSwiftStable) ? swiftMetaClassPtr->swiftClassFlags : 0;
                classSuperclassVMAddr = objcClass.isaVMAddr + offsetof(objc_class_t, superclassVMAddr);
                classDataVMAddr       = objcClass.isaVMAddr + offsetof(objc_class_t, dataVMAddrAndFastFlags);
                handler(diag, classVMAddr, classSuperclassVMAddr, classDataVMAddr, objcMetaClass, true);
                if (diag.hasError())
                    return;
            }
        }
    });
}

void MachOAnalyzer::forEachObjCCategory(Diagnostics& diag, bool contentRebased,
                                        void (^handler)(Diagnostics& diag, uint64_t categoryVMAddr,
                                                        const dyld3::MachOAnalyzer::ObjCCategory& objcCategory)) const {
    const uint64_t ptrSize = pointerSize();
    intptr_t slide = getSlide();

    MachOAnalyzer::VMAddrConverter vmAddrConverter;
    vmAddrConverter.preferredLoadAddress   = preferredLoadAddress();
    vmAddrConverter.slide                  = slide;
    vmAddrConverter.chainedPointerFormat   = hasChainedFixups() ? chainedPointerFormat() : 0;
    vmAddrConverter.contentRebased         = contentRebased;

    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_catlist") != 0 )
            return;
        const uint8_t*  categoryList       = (uint8_t*)(sectInfo.sectAddr + slide);
        uint64_t        categoryListSize   = sectInfo.sectSize;

        if ( (categoryListSize % ptrSize) != 0 ) {
            diag.error("Invalid objc category section size");
            return;
        }

        if ( ptrSize == 8 ) {
            typedef uint64_t PtrTy;
            struct objc_category_t {
                PtrTy nameVMAddr;
                PtrTy clsVMAddr;
                PtrTy instanceMethodsVMAddr;
                PtrTy classMethodsVMAddr;
                PtrTy protocolsVMAddr;
                PtrTy instancePropertiesVMAddr;
            };
            for (uint64_t i = 0; i != categoryListSize; i += sizeof(PtrTy)) {
                uint64_t categoryVMAddr = convertToVMAddr(*(PtrTy*)(categoryList + i), vmAddrConverter);

                const objc_category_t* categoryPtr = (const objc_category_t*)(categoryVMAddr + slide);
                ObjCCategory objCCategory;
                objCCategory.nameVMAddr                 = convertToVMAddr(categoryPtr->nameVMAddr, vmAddrConverter);
                objCCategory.clsVMAddr                  = convertToVMAddr(categoryPtr->clsVMAddr, vmAddrConverter);
                objCCategory.instanceMethodsVMAddr      = convertToVMAddr(categoryPtr->instanceMethodsVMAddr, vmAddrConverter);
                objCCategory.classMethodsVMAddr         = convertToVMAddr(categoryPtr->classMethodsVMAddr, vmAddrConverter);
                objCCategory.protocolsVMAddr            = convertToVMAddr(categoryPtr->protocolsVMAddr, vmAddrConverter);
                objCCategory.instancePropertiesVMAddr   = convertToVMAddr(categoryPtr->instancePropertiesVMAddr, vmAddrConverter);
                handler(diag, categoryVMAddr, objCCategory);
                if (diag.hasError())
                    return;
            }
        } else {
            typedef uint32_t PtrTy;
            struct objc_category_t {
                PtrTy nameVMAddr;
                PtrTy clsVMAddr;
                PtrTy instanceMethodsVMAddr;
                PtrTy classMethodsVMAddr;
                PtrTy protocolsVMAddr;
                PtrTy instancePropertiesVMAddr;
            };
            for (uint64_t i = 0; i != categoryListSize; i += sizeof(PtrTy)) {
                uint64_t categoryVMAddr = convertToVMAddr(*(PtrTy*)(categoryList + i), vmAddrConverter);

                const objc_category_t* categoryPtr = (const objc_category_t*)(categoryVMAddr + slide);
                ObjCCategory objCCategory;
                objCCategory.nameVMAddr                 = convertToVMAddr(categoryPtr->nameVMAddr, vmAddrConverter);
                objCCategory.clsVMAddr                  = convertToVMAddr(categoryPtr->clsVMAddr, vmAddrConverter);
                objCCategory.instanceMethodsVMAddr      = convertToVMAddr(categoryPtr->instanceMethodsVMAddr, vmAddrConverter);
                objCCategory.classMethodsVMAddr         = convertToVMAddr(categoryPtr->classMethodsVMAddr, vmAddrConverter);
                objCCategory.protocolsVMAddr            = convertToVMAddr(categoryPtr->protocolsVMAddr, vmAddrConverter);
                objCCategory.instancePropertiesVMAddr   = convertToVMAddr(categoryPtr->instancePropertiesVMAddr, vmAddrConverter);
                handler(diag, categoryVMAddr, objCCategory);
                if (diag.hasError())
                    return;
            }
        }
    });
}

void MachOAnalyzer::forEachObjCProtocol(Diagnostics& diag, bool contentRebased,
                                        void (^handler)(Diagnostics& diag, uint64_t categoryVMAddr,
                                                        const dyld3::MachOAnalyzer::ObjCProtocol& objCProtocol)) const {
    const uint64_t ptrSize = pointerSize();
    intptr_t slide = getSlide();

    MachOAnalyzer::VMAddrConverter vmAddrConverter;
    vmAddrConverter.preferredLoadAddress   = preferredLoadAddress();
    vmAddrConverter.slide                  = slide;
    vmAddrConverter.chainedPointerFormat   = hasChainedFixups() ? chainedPointerFormat() : 0;
    vmAddrConverter.contentRebased         = contentRebased;

    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_protolist") != 0 )
            return;
        const uint8_t*  protocolList       = (uint8_t*)(sectInfo.sectAddr + slide);
        uint64_t        protocolListSize   = sectInfo.sectSize;

        if ( (protocolListSize % ptrSize) != 0 ) {
            diag.error("Invalid objc protocol section size");
            return;
        }

        if ( ptrSize == 8 ) {
            typedef uint64_t PtrTy;
            struct protocol_t {
                PtrTy    isaVMAddr;
                PtrTy    nameVMAddr;
                PtrTy    protocolsVMAddr;
                PtrTy    instanceMethodsVMAddr;
                PtrTy    classMethodsVMAddr;
                PtrTy    optionalInstanceMethodsVMAddr;
                PtrTy    optionalClassMethodsVMAddr;
                PtrTy    instancePropertiesVMAddr;
                uint32_t size;
                uint32_t flags;
                // Fields below this point are not always present on disk.
                PtrTy    extendedMethodTypesVMAddr;
                PtrTy    demangledNameVMAddr;
                PtrTy    classPropertiesVMAddr;
            };
            for (uint64_t i = 0; i != protocolListSize; i += sizeof(PtrTy)) {
                uint64_t protocolVMAddr = convertToVMAddr(*(PtrTy*)(protocolList + i), vmAddrConverter);

                const protocol_t* protocolPtr = (const protocol_t*)(protocolVMAddr + slide);
                ObjCProtocol objCProtocol;
                objCProtocol.isaVMAddr                          = convertToVMAddr(protocolPtr->isaVMAddr, vmAddrConverter);
                objCProtocol.nameVMAddr                         = convertToVMAddr(protocolPtr->nameVMAddr, vmAddrConverter);
                objCProtocol.instanceMethodsVMAddr              = convertToVMAddr(protocolPtr->instanceMethodsVMAddr, vmAddrConverter);
                objCProtocol.classMethodsVMAddr                 = convertToVMAddr(protocolPtr->classMethodsVMAddr, vmAddrConverter);
                objCProtocol.optionalInstanceMethodsVMAddr      = convertToVMAddr(protocolPtr->optionalInstanceMethodsVMAddr, vmAddrConverter);
                objCProtocol.optionalClassMethodsVMAddr         = convertToVMAddr(protocolPtr->optionalClassMethodsVMAddr, vmAddrConverter);

                // Track if this protocol needs a reallocation in objc
                objCProtocol.requiresObjCReallocation           = protocolPtr->size < sizeof(protocol_t);

                handler(diag, protocolVMAddr, objCProtocol);
                if (diag.hasError())
                    return;
            }
        } else {
            typedef uint32_t PtrTy;
            struct protocol_t {
                PtrTy    isaVMAddr;
                PtrTy    nameVMAddr;
                PtrTy    protocolsVMAddr;
                PtrTy    instanceMethodsVMAddr;
                PtrTy    classMethodsVMAddr;
                PtrTy    optionalInstanceMethodsVMAddr;
                PtrTy    optionalClassMethodsVMAddr;
                PtrTy    instancePropertiesVMAddr;
                uint32_t size;
                uint32_t flags;
                // Fields below this point are not always present on disk.
                PtrTy    extendedMethodTypesVMAddr;
                PtrTy    demangledNameVMAddr;
                PtrTy    classPropertiesVMAddr;
            };
            for (uint64_t i = 0; i != protocolListSize; i += sizeof(PtrTy)) {
                uint64_t protocolVMAddr = convertToVMAddr(*(PtrTy*)(protocolList + i), vmAddrConverter);

                const protocol_t* protocolPtr = (const protocol_t*)(protocolVMAddr + slide);
                ObjCProtocol objCProtocol;
                objCProtocol.isaVMAddr                          = convertToVMAddr(protocolPtr->isaVMAddr, vmAddrConverter);
                objCProtocol.nameVMAddr                         = convertToVMAddr(protocolPtr->nameVMAddr, vmAddrConverter);
                objCProtocol.instanceMethodsVMAddr              = convertToVMAddr(protocolPtr->instanceMethodsVMAddr, vmAddrConverter);
                objCProtocol.classMethodsVMAddr                 = convertToVMAddr(protocolPtr->classMethodsVMAddr, vmAddrConverter);
                objCProtocol.optionalInstanceMethodsVMAddr      = convertToVMAddr(protocolPtr->optionalInstanceMethodsVMAddr, vmAddrConverter);
                objCProtocol.optionalClassMethodsVMAddr         = convertToVMAddr(protocolPtr->optionalClassMethodsVMAddr, vmAddrConverter);

                // Track if this protocol needs a reallocation in objc
                objCProtocol.requiresObjCReallocation           = protocolPtr->size < sizeof(protocol_t);

                handler(diag, protocolVMAddr, objCProtocol);
                if (diag.hasError())
                    return;
            }
        }
    });
}

void MachOAnalyzer::forEachObjCMethod(uint64_t methodListVMAddr, bool contentRebased,
                                      void (^handler)(uint64_t methodVMAddr, const ObjCMethod& method)) const {
    if ( methodListVMAddr == 0 )
        return;

    const uint64_t ptrSize = pointerSize();
    intptr_t slide = getSlide();

    MachOAnalyzer::VMAddrConverter vmAddrConverter;
    vmAddrConverter.preferredLoadAddress   = preferredLoadAddress();
    vmAddrConverter.slide                  = slide;
    vmAddrConverter.chainedPointerFormat   = hasChainedFixups() ? chainedPointerFormat() : 0;
    vmAddrConverter.contentRebased         = contentRebased;

    if ( ptrSize == 8 ) {
        typedef uint64_t PtrTy;
        struct method_list_t {
            uint32_t    entsize;
            uint32_t    count;
            PtrTy       methodArrayBase; // Note this is the start the array method_t[0]

            uint32_t getEntsize() const {
                return (entsize) & ~(uint32_t)3;
            }
        };

        struct method_t {
            PtrTy nameVMAddr;   // SEL
            PtrTy typesVMAddr;  // const char *
            PtrTy impVMAddr;    // IMP
        };

        const method_list_t* methodList = (const method_list_t*)(methodListVMAddr + slide);
        uint64_t methodListArrayBaseVMAddr = methodListVMAddr + offsetof(method_list_t, methodArrayBase);
        for (unsigned i = 0; i != methodList->count; ++i) {
            uint64_t methodEntryOffset = i * methodList->getEntsize();
            uint64_t methodVMAddr = methodListArrayBaseVMAddr + methodEntryOffset;
            const method_t* methodPtr = (const method_t*)(methodVMAddr + slide);
            ObjCMethod method;
            method.nameVMAddr   = convertToVMAddr(methodPtr->nameVMAddr, vmAddrConverter);
            method.typesVMAddr  = convertToVMAddr(methodPtr->typesVMAddr, vmAddrConverter);
            method.impVMAddr    = convertToVMAddr(methodPtr->impVMAddr, vmAddrConverter);
            method.nameLocationVMAddr = methodVMAddr + offsetof(method_t, nameVMAddr);
            handler(methodVMAddr, method);
        }
    } else {
        typedef uint32_t PtrTy;
        struct method_list_t {
            uint32_t    entsize;
            uint32_t    count;
            PtrTy       methodArrayBase; // Note this is the start the array method_t[0]

            uint32_t getEntsize() const {
                return (entsize) & ~(uint32_t)3;
            }
        };

        struct method_t {
            PtrTy nameVMAddr;   // SEL
            PtrTy typesVMAddr;  // const char *
            PtrTy impVMAddr;    // IMP
        };

        const method_list_t* methodList = (const method_list_t*)(methodListVMAddr + slide);
        uint64_t methodListArrayBaseVMAddr = methodListVMAddr + offsetof(method_list_t, methodArrayBase);
        for (unsigned i = 0; i != methodList->count; ++i) {
            uint64_t methodEntryOffset = i * methodList->getEntsize();
            uint64_t methodVMAddr = methodListArrayBaseVMAddr + methodEntryOffset;
            const method_t* methodPtr = (const method_t*)(methodVMAddr + slide);
            ObjCMethod method;
            method.nameVMAddr   = convertToVMAddr(methodPtr->nameVMAddr, vmAddrConverter);
            method.typesVMAddr  = convertToVMAddr(methodPtr->typesVMAddr, vmAddrConverter);
            method.impVMAddr    = convertToVMAddr(methodPtr->impVMAddr, vmAddrConverter);
            method.nameLocationVMAddr = methodVMAddr + offsetof(method_t, nameVMAddr);
            handler(methodVMAddr, method);
        }
    }
}


void MachOAnalyzer::forEachObjCSelectorReference(Diagnostics& diag, bool contentRebased,
                                                 void (^handler)(uint64_t selRefVMAddr, uint64_t selRefTargetVMAddr)) const {
    const uint64_t ptrSize = pointerSize();
    intptr_t slide = getSlide();

    MachOAnalyzer::VMAddrConverter vmAddrConverter;
    vmAddrConverter.preferredLoadAddress   = preferredLoadAddress();
    vmAddrConverter.slide                  = slide;
    vmAddrConverter.chainedPointerFormat   = hasChainedFixups() ? chainedPointerFormat() : 0;
    vmAddrConverter.contentRebased         = contentRebased;

    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_selrefs") != 0 )
            return;
        uint64_t        selRefSectionVMAddr = sectInfo.sectAddr;
        const uint8_t*  selRefs       = (uint8_t*)(selRefSectionVMAddr + slide);
        uint64_t        selRefsSize   = sectInfo.sectSize;

        if ( (selRefsSize % ptrSize) != 0 ) {
            diag.error("Invalid sel ref section size");
            return;
        }

        if ( ptrSize == 8 ) {
            typedef uint64_t PtrTy;
            for (uint64_t i = 0; i != selRefsSize; i += sizeof(PtrTy)) {
                uint64_t selRefVMAddr = selRefSectionVMAddr + i;
                uint64_t selRefTargetVMAddr = convertToVMAddr(*(PtrTy*)(selRefs + i), vmAddrConverter);
                handler(selRefVMAddr, selRefTargetVMAddr);
                if (diag.hasError()) {
                    stop = true;
                    return;
                }
            }
        } else {
            typedef uint32_t PtrTy;
            for (uint64_t i = 0; i != selRefsSize; i += sizeof(PtrTy)) {
                uint64_t selRefVMAddr = selRefSectionVMAddr + i;
                uint64_t selRefTargetVMAddr = convertToVMAddr(*(PtrTy*)(selRefs + i), vmAddrConverter);
                handler(selRefVMAddr, selRefTargetVMAddr);
                if (diag.hasError()) {
                    stop = true;
                    return;
                }
            }
        }
    });
}

void MachOAnalyzer::forEachObjCMethodName(void (^handler)(const char* methodName)) const {
    intptr_t slide = getSlide();
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strcmp(sectInfo.segInfo.segName, "__TEXT") != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_methname") != 0 )
            return;
        if ( sectInfo.segInfo.isProtected || ( (sectInfo.sectFlags & SECTION_TYPE) != S_CSTRING_LITERALS ) ) {
            stop = true;
            return;
        }
        if ( malformedSectionRange ) {
            stop = true;
            return;
        }

        const char* content       = (const char*)(sectInfo.sectAddr + slide);
        uint64_t    sectionSize   = sectInfo.sectSize;

        const char* s   = (const char*)content;
        const char* end = s + sectionSize;
        while ( s < end ) {
            handler(s);
            s += strlen(s) + 1;
        }
    });
}


bool MachOAnalyzer::hasObjCMessageReferences() const {

    __block bool foundSection = false;
    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if ( strcmp(sectInfo.sectName, "__objc_msgrefs") != 0 )
            return;
        foundSection = true;
        stop = true;
    });
    return foundSection;
}

const MachOAnalyzer::ObjCImageInfo* MachOAnalyzer::objcImageInfo() const {
    int64_t slide = getSlide();

    __block bool foundInvalidObjCImageInfo = false;
    __block const ObjCImageInfo* imageInfo = nullptr;
    forEachSection(^(const dyld3::MachOAnalyzer::SectionInfo& sectionInfo, bool malformedSectionRange, bool& stop) {
        if ( strncmp(sectionInfo.segInfo.segName, "__DATA", 6) != 0 )
            return;
        if (strcmp(sectionInfo.sectName, "__objc_imageinfo") != 0)
            return;
        if ( malformedSectionRange ) {
            stop = true;
            return;
        }
        if ( sectionInfo.sectSize != 8 ) {
            stop = true;
            return;
        }
        imageInfo = (const ObjCImageInfo*)(sectionInfo.sectAddr + slide);
        if ( (imageInfo->flags & ObjCImageInfo::dyldPreoptimized) != 0 ) {
            foundInvalidObjCImageInfo = true;
            stop = true;
            return;
        }
        stop = true;
    });
    if ( foundInvalidObjCImageInfo )
        return nullptr;
    return imageInfo;
}

uint32_t MachOAnalyzer::loadCommandsFreeSpace() const
{
    __block uint32_t firstSectionFileOffset = 0;
    __block uint32_t firstSegmentFileOffset = 0;
    forEachSection(^(const SectionInfo& sectInfo, bool malformedSectionRange, bool& stop) {
        firstSectionFileOffset = sectInfo.sectFileOffset;
        firstSegmentFileOffset = (uint32_t)sectInfo.segInfo.fileOffset;
        stop = true;
    });

    uint32_t headerSize = (this->magic == MH_MAGIC_64) ? sizeof(mach_header_64) : sizeof(mach_header);
    uint32_t existSpaceUsed = this->sizeofcmds + headerSize;
    return firstSectionFileOffset - firstSegmentFileOffset - existSpaceUsed;
}

void MachOAnalyzer::forEachWeakDef(Diagnostics& diag,
                                   void (^handler)(const char* symbolName, uintptr_t imageOffset, bool isFromExportTrie)) const {
    uint64_t baseAddress = preferredLoadAddress();
    forEachGlobalSymbol(diag, ^(const char *symbolName, uint64_t n_value, uint8_t n_type, uint8_t n_sect, uint16_t n_desc, bool &stop) {
        if ( (n_desc & N_WEAK_DEF) != 0 ) {
            handler(symbolName, n_value - baseAddress, false);
        }
    });
    forEachExportedSymbol(diag, ^(const char *symbolName, uint64_t imageOffset, uint64_t flags, uint64_t other, const char *importName, bool &stop) {
        if ( (flags & EXPORT_SYMBOL_FLAGS_WEAK_DEFINITION ) == 0 )
            return;
        // Skip resolvers and re-exports
        if ( (flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) != 0 )
            return;
        if ( (flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) != 0 )
            return;
        handler(symbolName, imageOffset, true);
    });
}

} // dyld3


