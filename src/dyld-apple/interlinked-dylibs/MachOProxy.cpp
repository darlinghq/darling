//
//  DylibProxy.cpp
//  dyld
//
//  Created by Louis Gerbarg on 1/27/16.
//
//

#include <mach-o/loader.h>
#include <mach-o/fat.h>

#include "mega-dylib-utils.h"
#include "Logging.h"

#include "MachOProxy.h"

namespace {
std::map<std::string, MachOProxy*> mapMachOFile( const std::string& path ) {
    std::map<std::string, MachOProxy*> retval;
    const uint8_t* p = (uint8_t*)( -1 );
    struct stat stat_buf;
	bool rootless;

    std::tie( p, stat_buf, rootless ) = fileCache.cacheLoad( path );

    if ( p == (uint8_t*)( -1 ) ) {
        return retval;
    }

    // if fat file, process each architecture
    const fat_header* fh = (fat_header*)p;
    const mach_header* mh = (mach_header*)p;
    if ( OSSwapBigToHostInt32( fh->magic ) == FAT_MAGIC ) {
        // Fat header is always big-endian
        const fat_arch* slices = (const fat_arch*)( (char*)fh + sizeof( fat_header ) );
        const uint32_t sliceCount = OSSwapBigToHostInt32( fh->nfat_arch );
        for ( uint32_t i = 0; i < sliceCount; ++i ) {
            // FIXME Should we validate the fat header matches the slices?
            ArchPair arch( OSSwapBigToHostInt32( slices[i].cputype ), OSSwapBigToHostInt32( slices[i].cpusubtype ) );
            uint32_t fileOffset = OSSwapBigToHostInt32( slices[i].offset );
            const mach_header* th = (mach_header*)(p+fileOffset);
            if ( ( OSSwapLittleToHostInt32( th->magic ) == MH_MAGIC ) || ( OSSwapLittleToHostInt32( th->magic ) == MH_MAGIC_64 ) ) {
                uint32_t fileSize = static_cast<uint32_t>( stat_buf.st_size );
                retval[stringForArch( arch )] = new MachOProxy( path, stat_buf.st_ino, stat_buf.st_mtime, fileOffset, fileSize, rootless );
            }
        }
    } else if ( ( OSSwapLittleToHostInt32( mh->magic ) == MH_MAGIC ) || ( OSSwapLittleToHostInt32( mh->magic ) == MH_MAGIC_64 ) ) {
        ArchPair arch( OSSwapLittleToHostInt32( mh->cputype ), OSSwapLittleToHostInt32( mh->cpusubtype ) );
        uint32_t fileOffset = OSSwapBigToHostInt32( 0 );
        uint32_t fileSize = static_cast<uint32_t>( stat_buf.st_size );
        retval[stringForArch( arch )] = new MachOProxy( path, stat_buf.st_ino, stat_buf.st_mtime, fileOffset, fileSize, rootless );
    } else {
        //    warning( "file '%s' is not contain requested a MachO", path.c_str() );
    }
    return retval;
}
}

template <typename P>
std::string MachOProxy::machoParser(bool ignoreUncacheableDylibsInExecutables)
{
    const uint8_t*                     buffer = getBuffer();
    bool                               hasSplitSegInfo = false;
    bool                               hasDylidInfo = false;
    const macho_header<P>*             mh = (const macho_header<P>*)buffer;
    const macho_symtab_command<P>*     symTab = nullptr;
    const macho_dysymtab_command<P>*   dynSymTab = nullptr;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t                     cmd_count = mh->ncmds();
    const macho_load_command<P>*       cmd = cmds;
    _dylib = (mh->filetype() == MH_DYLIB);
    _executable = (mh->filetype() == MH_EXECUTE);
    if (mh->filetype() == MH_DYLIB_STUB) {
        return "stub dylib";
    }
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd()) {
        case LC_ID_DYLIB: {
            macho_dylib_command<P>* dylib = (macho_dylib_command<P>*)cmd;
            if (dylib->name()[0] != '/') {
                if (strncmp(dylib->name(), "@rpath", 6) == 0)
                    return "@rpath cannot be used in -install_name for OS dylibs";
                else
                    return "-install_name is not an absolute path";
            }
            installName = dylib->name();
            installNameOffsetInTEXT = (uint32_t)((uint8_t*)cmd - buffer) + dylib->name_offset();
        } break;
        case LC_UUID: {
            const macho_uuid_command<P>* uuidCmd = (macho_uuid_command<P>*)cmd;
            ::memcpy(uuid, uuidCmd->uuid(), sizeof(uuid_t));
        } break;
        case LC_LOAD_DYLIB:
        case LC_LOAD_WEAK_DYLIB:
        case LC_REEXPORT_DYLIB:
        case LC_LOAD_UPWARD_DYLIB: {
            macho_dylib_command<P>* dylib = (macho_dylib_command<P>*)cmd;
            std::string             depName = dylib->name();
            if ( _executable && ignoreUncacheableDylibsInExecutables && !has_prefix(depName, "/usr/lib/") && !has_prefix(depName, "/System/Library/") ) {
                // <rdar://problem/25918268> in update_dyld_shared_cache don't warn if root executable links with something not eligible for shared cache
                break;
            }
            else if ( depName[0] != '/' ) {
                return "linked against a dylib whose -install_name was non-absolute (e.g. @rpath)";
            }
            dependencies.insert(depName);
        } break;
        case macho_segment_command<P>::CMD: {
            const macho_segment_command<P>* segCmd = (macho_segment_command<P>*)cmd;
            MachOProxy::Segment             seg;
            seg.name = segCmd->segname();
            seg.size = align(segCmd->vmsize(), 12);
            seg.diskSize = (uint32_t)segCmd->filesize();
            seg.fileOffset = (uint32_t)segCmd->fileoff();
            seg.protection = segCmd->initprot();
            if (segCmd->nsects() > 0) {
                seg.p2align = 0;
                const macho_section<P>* const sectionsStart = (macho_section<P>*)((uint8_t*)segCmd + sizeof(macho_segment_command<P>));
                const macho_section<P>* const sectionsLast = &sectionsStart[segCmd->nsects() - 1];
                const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                for (const macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                    if (sect->align() > seg.p2align)
                        seg.p2align = sect->align();
                }
                seg.sizeOfSections = sectionsLast->addr() + sectionsLast->size() - segCmd->vmaddr();
            } else {
                seg.p2align = 12;
            }
            segments.push_back(seg);
        } break;
        case LC_SEGMENT_SPLIT_INFO:
            hasSplitSegInfo = true;
            break;
        case LC_SYMTAB:
            symTab = (macho_symtab_command<P>*)cmd;
            break;
        case LC_DYSYMTAB:
            dynSymTab = (macho_dysymtab_command<P>*)cmd;
            break;
        case LC_DYLD_INFO:
        case LC_DYLD_INFO_ONLY:
            hasDylidInfo = true;
            break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd) + cmd->cmdsize());
    }

    if (!_dylib) {
        return "";
    }

    if (!hasDylidInfo) {
        return "built for old OS";
    }

    if ((mh->flags() & MH_TWOLEVEL) == 0) {
        return "built with -flat_namespace";
    }

    if (!hasSplitSegInfo) {
        bool inUsrLib = (installName.size() > 9) && (installName.substr(0, 9) == "/usr/lib/");
        bool inSystemLibrary = (installName.size() > 16) && (installName.substr(0, 16) == "/System/Library/");
        if (!inUsrLib && !inSystemLibrary) {
            return "-install_name not /usr/lib/* or /System/Library/*";
        }
        return "no shared region info";
    }

    if ((symTab == nullptr) && (dynSymTab == nullptr)) {
        return "no symbol table";
    }

    if (installName.empty()) {
        return "dylib missing install name";
    }

    // scan undefines looking for invalid ordinals
    const macho_nlist<P>* symbolTable = (macho_nlist<P>*)((uint8_t*)mh + symTab->symoff());
    const uint32_t        startUndefs = dynSymTab->iundefsym();
    const uint32_t        endUndefs = startUndefs + dynSymTab->nundefsym();
    for (uint32_t i = startUndefs; i < endUndefs; ++i) {
        uint8_t ordinal = GET_LIBRARY_ORDINAL(symbolTable[i].n_desc());
        if (ordinal == DYNAMIC_LOOKUP_ORDINAL) {
            return "built with '-undefined dynamic_lookup'";
        } else if (ordinal == EXECUTABLE_ORDINAL) {
            return "built with -bundle_loader";
        }
    }

    return "";
}

const bool MachOProxy::isDylib()
{
    return _dylib;
}

const bool MachOProxy::isExecutable()
{
    return _executable;
}

std::map<std::string, MachOProxy*> MachOProxy::findDylibInfo(const std::string& path, bool warnOnProblems, bool ignoreUncacheableDylibsInExecutables) {
    std::map<std::string, MachOProxy*> slices = mapMachOFile( path );
    std::vector<std::string> errorSlices;

    for ( auto& slice : slices ) {
        std::string errorMessage;
        verboseLog( "analyzing file '%s'", path.c_str() );
        switch ( archForString( slice.first ).arch ) {
            case CPU_TYPE_ARM:
            case CPU_TYPE_I386:
                errorMessage = slice.second->machoParser<Pointer32<LittleEndian>>(ignoreUncacheableDylibsInExecutables);
                break;
            case CPU_TYPE_X86_64:
            case CPU_TYPE_ARM64:
                errorMessage = slice.second->machoParser<Pointer64<LittleEndian>>(ignoreUncacheableDylibsInExecutables);
                break;
            default:
                errorMessage = "unsupported arch '" + slice.first + "'";
                break;
        }

        if ( !errorMessage.empty() ) {
			if ( warnOnProblems )
				warning( "%s (%s)", errorMessage.c_str(), path.c_str() );
            errorSlices.push_back( slice.first );
        }
    }

    for ( const auto& slice : errorSlices ) {
        slices.erase( slice );
    }

    return slices;
}

const uint8_t* MachOProxy::getBuffer() {
    const uint8_t* p = (uint8_t*)( -1 );
    struct stat stat_buf;
	bool rootless;
	std::tie(p, stat_buf,rootless) = fileCache.cacheLoad(path);
    return p + fatFileOffset;
}

bool MachOProxy::addAlias( const std::string& alias ) {
    if (!has_prefix(alias, "/usr/lib/") && !has_prefix(alias, "/System/Library/"))
        return false;
    if ( alias != installName && installNameAliases.count( alias ) == 0 ) {
        installNameAliases.insert( alias );
        return true;
    }
    return false;
}
