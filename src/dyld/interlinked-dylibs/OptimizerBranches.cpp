/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*- 
 *
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

#include "OptimizerBranches.h"
#include "Trie.hpp"
#include "MachOFileAbstraction.hpp"
#include "Logging.h"

#include "dyld_cache_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <string>
#include <unordered_map>
#include <unordered_set>

#include <CommonCrypto/CommonDigest.h>


static const bool verbose = false;

// These are functions that are interposed by Instruments.app or ASan
static const char* sNeverStubEliminateSymbols[] = {
    "___bzero",
    "___cxa_atexit",
    "___cxa_throw", 
    "__longjmp", 
    "__objc_autoreleasePoolPop", 
    "_accept", 
    "_access", 
    "_asctime", 
    "_asctime_r", 
    "_asprintf", 
    "_atoi", 
    "_atol", 
    "_atoll", 
    "_calloc", 
    "_chmod", 
    "_chown", 
    "_close", 
    "_confstr", 
    "_ctime", 
    "_ctime_r", 
    "_dispatch_after", 
    "_dispatch_after_f", 
    "_dispatch_async", 
    "_dispatch_async_f", 
    "_dispatch_barrier_async_f", 
    "_dispatch_group_async", 
    "_dispatch_group_async_f", 
    "_dispatch_source_set_cancel_handler", 
    "_dispatch_source_set_event_handler", 
    "_dispatch_sync_f", 
    "_dlclose", 
    "_dlopen", 
    "_dup", 
    "_dup2", 
    "_endgrent", 
    "_endpwent", 
    "_ether_aton", 
    "_ether_hostton", 
    "_ether_line", 
    "_ether_ntoa", 
    "_ether_ntohost", 
    "_fchmod", 
    "_fchown", 
    "_fclose", 
    "_fdopen", 
    "_fflush", 
    "_fopen", 
    "_fork", 
    "_fprintf", 
    "_free", 
    "_freopen", 
    "_frexp", 
    "_frexpf", 
    "_frexpl", 
    "_fscanf", 
    "_fstat", 
    "_fstatfs", 
    "_fstatfs64", 
    "_fsync", 
    "_ftime", 
    "_getaddrinfo", 
    "_getattrlist", 
    "_getcwd", 
    "_getgrent", 
    "_getgrgid", 
    "_getgrgid_r", 
    "_getgrnam", 
    "_getgrnam_r", 
    "_getgroups", 
    "_gethostbyaddr", 
    "_gethostbyname", 
    "_gethostbyname2", 
    "_gethostent", 
    "_getifaddrs", 
    "_getitimer", 
    "_getnameinfo", 
    "_getpass", 
    "_getpeername", 
    "_getpwent", 
    "_getpwnam", 
    "_getpwnam_r", 
    "_getpwuid", 
    "_getpwuid_r", 
    "_getsockname", 
    "_getsockopt", 
    "_gmtime", 
    "_gmtime_r", 
    "_if_indextoname", 
    "_if_nametoindex", 
    "_index", 
    "_inet_aton", 
    "_inet_ntop", 
    "_inet_pton", 
    "_initgroups", 
    "_ioctl", 
    "_lchown", 
    "_lgamma", 
    "_lgammaf", 
    "_lgammal", 
    "_link", 
    "_listxattr", 
    "_localtime", 
    "_localtime_r", 
    "_longjmp", 
    "_lseek", 
    "_lstat", 
    "_malloc", 
    "_malloc_create_zone", 
    "_malloc_default_purgeable_zone", 
    "_malloc_default_zone", 
    "_malloc_good_size", 
    "_malloc_make_nonpurgeable", 
    "_malloc_make_purgeable", 
    "_malloc_set_zone_name", 
    "_mbsnrtowcs", 
    "_mbsrtowcs", 
    "_mbstowcs", 
    "_memchr", 
    "_memcmp", 
    "_memcpy", 
    "_memmove", 
    "_memset", 
    "_mktime", 
    "_mlock", 
    "_mlockall", 
    "_modf", 
    "_modff", 
    "_modfl", 
    "_munlock", 
    "_munlockall", 
    "_objc_autoreleasePoolPop", 
    "_objc_setProperty", 
    "_objc_setProperty_atomic", 
    "_objc_setProperty_atomic_copy", 
    "_objc_setProperty_nonatomic", 
    "_objc_setProperty_nonatomic_copy", 
    "_objc_storeStrong", 
    "_open", 
    "_opendir", 
    "_poll", 
    "_posix_memalign", 
    "_pread", 
    "_printf", 
    "_pthread_attr_getdetachstate", 
    "_pthread_attr_getguardsize", 
    "_pthread_attr_getinheritsched", 
    "_pthread_attr_getschedparam", 
    "_pthread_attr_getschedpolicy", 
    "_pthread_attr_getscope", 
    "_pthread_attr_getstack", 
    "_pthread_attr_getstacksize", 
    "_pthread_condattr_getpshared", 
    "_pthread_create", 
    "_pthread_getschedparam", 
    "_pthread_join", 
    "_pthread_mutex_lock", 
    "_pthread_mutex_unlock", 
    "_pthread_mutexattr_getprioceiling", 
    "_pthread_mutexattr_getprotocol", 
    "_pthread_mutexattr_getpshared", 
    "_pthread_mutexattr_gettype", 
    "_pthread_rwlockattr_getpshared", 
    "_pwrite", 
    "_rand_r", 
    "_read", 
    "_readdir", 
    "_readdir_r", 
    "_readv", 
    "_readv$UNIX2003", 
    "_realloc", 
    "_realpath", 
    "_recv", 
    "_recvfrom", 
    "_recvmsg", 
    "_remquo", 
    "_remquof", 
    "_remquol", 
    "_scanf", 
    "_send", 
    "_sendmsg", 
    "_sendto", 
    "_setattrlist", 
    "_setgrent", 
    "_setitimer", 
    "_setlocale", 
    "_setpwent", 
    "_shm_open", 
    "_shm_unlink", 
    "_sigaction", 
    "_sigemptyset", 
    "_sigfillset", 
    "_siglongjmp", 
    "_signal", 
    "_sigpending", 
    "_sigprocmask", 
    "_sigwait", 
    "_snprintf", 
    "_sprintf", 
    "_sscanf", 
    "_stat", 
    "_statfs", 
    "_statfs64", 
    "_strcasecmp", 
    "_strcat", 
    "_strchr", 
    "_strcmp", 
    "_strcpy", 
    "_strdup", 
    "_strerror", 
    "_strerror_r", 
    "_strlen", 
    "_strncasecmp", 
    "_strncat", 
    "_strncmp", 
    "_strncpy", 
    "_strptime", 
    "_strtoimax", 
    "_strtol", 
    "_strtoll", 
    "_strtoumax", 
    "_tempnam", 
    "_time", 
    "_times", 
    "_tmpnam", 
    "_tsearch", 
    "_unlink", 
    "_valloc", 
    "_vasprintf", 
    "_vfprintf", 
    "_vfscanf", 
    "_vprintf", 
    "_vscanf", 
    "_vsnprintf", 
    "_vsprintf", 
    "_vsscanf", 
    "_wait", 
    "_wait$UNIX2003", 
    "_wait3", 
    "_wait4", 
    "_waitid", 
    "_waitid$UNIX2003", 
    "_waitpid", 
    "_waitpid$UNIX2003", 
    "_wcslen", 
    "_wcsnrtombs", 
    "_wcsrtombs", 
    "_wcstombs", 
    "_wordexp", 
    "_write", 
    "_writev", 
    "_writev$UNIX2003",
    // <rdar://problem/22050956> always use stubs for C++ symbols that can be overridden
    "__ZdaPv",
    "__ZdlPv",
    "__Znam",
    "__Znwm",

    nullptr
};


// These are dylibs that are interposed and stubs calling into them should never be bypassed
static const char* sNeverStubEliminateDylibs[] = {
    "/usr/lib/system/libdispatch.dylib",
    nullptr
};



uint64_t branchPoolTextSize(ArchPair arch)
{
 if ( arch.arch == CPU_TYPE_ARM64 )
    return 0x0000C000;  // 48KB
  else
    return 0;
}

uint64_t branchPoolLinkEditSize(ArchPair arch)
{
 if ( arch.arch == CPU_TYPE_ARM64 )
    return 0x00100000;  // 1MB
  else
    return 0;
}


uint64_t branchReach(ArchPair arch)
{
  if ( arch.arch == CPU_TYPE_ARM64 )
    return 0x08000000 - (2*branchPoolTextSize(arch));  // 128MB
  else
    return 0;
}

template <typename P>
class BranchPoolDylib {
public:
                            BranchPoolDylib(ArchPair arch, void* cacheBuffer, uint64_t cacheSize, uint64_t startAddr,
                                            uint64_t textRegionStartAddr, uint64_t poolLinkEditStartAddr, uint64_t poolLinkEditStartOffset);

    uint64_t                addr() { return _startAddr; }
    uint64_t                getForwardBranch(uint64_t finalTargetAddr, const char* name, std::vector<BranchPoolDylib<P>*>& branchIslandPools);
    uint64_t                getBackBranch(uint64_t finalTargetAddr, const char* name, std::vector<BranchPoolDylib<P>*>& branchIslandPools);
    void                    finalizeLoadCommands();
    void                    printStats();

private:
    uint64_t                indexToAddr(uint32_t index) { return _startAddr + _firstStubOffset + sizeof(uint32_t)*index; }

    static const int64_t b128MegLimit = 0x07FFFFFF;

    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    ArchPair                                    _arch;
    void*                                       _cacheBuffer;
    uint64_t                                    _startAddr;
	std::unordered_map<uint64_t, uint32_t>      _targetToIslandIndex;
	std::unordered_map<uint32_t, const char*>   _islandIndexToName;
    macho_symtab_command<P>*                    _symbolTableCmd;
	macho_dysymtab_command<P>*                  _dynamicSymbolTableCmd;
    macho_uuid_command<P>*                      _uuidCmd;
    uint32_t                                    _maxStubs;
    uint32_t                                    _nextIndex;
    uint32_t                                    _firstStubOffset;
    uint32_t*                                   _stubInstructions;
    macho_nlist<P>*                             _symbolTable;
    char*                                       _nextString;
    char*                                       _stringPoolStart;
    char*                                       _stringPoolEnd;
};


template <typename P>
BranchPoolDylib<P>::BranchPoolDylib(ArchPair arch, void* cacheBuffer, uint64_t cacheSize, uint64_t poolStartAddr,
                                     uint64_t textRegionStartAddr, uint64_t poolLinkEditStartAddr, uint64_t poolLinkEditStartOffset)
    : _arch(arch), _cacheBuffer(cacheBuffer),_startAddr(poolStartAddr), _nextIndex(0), _firstStubOffset(0x280)
{
    bool is64 = (sizeof(typename P::uint_t) == 8);

    const uint64_t textSegSize = branchPoolTextSize(arch);
    const uint64_t linkEditSegSize = branchPoolLinkEditSize(arch);
    const unsigned stubCount = (unsigned)((textSegSize - _firstStubOffset)/4);
    const uint32_t linkeditOffsetSymbolTable = 0;
    const uint32_t linkeditOffsetIndirectSymbolTable = stubCount*sizeof(macho_nlist<P>);
    const uint32_t linkeditOffsetSymbolPoolOffset = linkeditOffsetIndirectSymbolTable + stubCount*sizeof(uint32_t);
    _maxStubs = stubCount;

    // write mach_header and load commands for pseudo dylib
    macho_header<P>* mh = (macho_header<P>*)((uint8_t*)cacheBuffer + poolStartAddr - textRegionStartAddr);
    mh->set_magic(is64 ? MH_MAGIC_64 : MH_MAGIC);
    mh->set_cputype(arch.arch);
    mh->set_cpusubtype(arch.subtype);
    mh->set_filetype(MH_DYLIB);
    mh->set_ncmds(6);
    mh->set_sizeofcmds(is64 ? 0x210 : 100); // FIXME: 32-bit size
    mh->set_flags(0x80000000);
    // LC_SEGMENT
    macho_load_command<P>* cmd = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    macho_segment_command<P>* textSegCmd = (macho_segment_command<P>*)cmd;
    textSegCmd->set_cmd(is64 ? LC_SEGMENT_64 : LC_SEGMENT);
    textSegCmd->set_cmdsize(sizeof(macho_segment_command<P>)*2+sizeof(macho_section<P>));
    textSegCmd->set_segname("__TEXT");
    textSegCmd->set_vmaddr(poolStartAddr);
    textSegCmd->set_vmsize(textSegSize);
    textSegCmd->set_fileoff(poolStartAddr - textRegionStartAddr);
    textSegCmd->set_filesize(branchPoolTextSize(arch));
    textSegCmd->set_maxprot(PROT_READ|PROT_EXEC);
    textSegCmd->set_initprot(PROT_READ|PROT_EXEC);
    textSegCmd->set_nsects(1);
    textSegCmd->set_flags(0);
    macho_section<P>* stubSection = (macho_section<P>*)((uint8_t*)textSegCmd + sizeof(macho_segment_command<P>));
    stubSection->set_sectname("__stubs");
    stubSection->set_segname("__TEXT");
    stubSection->set_addr(poolStartAddr + _firstStubOffset);
    stubSection->set_size(textSegSize - _firstStubOffset);
    stubSection->set_offset((uint32_t)(poolStartAddr + _firstStubOffset - textRegionStartAddr));
    stubSection->set_align(2);
    stubSection->set_reloff(0);
    stubSection->set_nreloc(0);
    stubSection->set_flags(S_SYMBOL_STUBS | S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS);
    stubSection->set_reserved1(0); // start index in indirect table
    stubSection->set_reserved2(4); // size of stubs
    // LC_SEGMENT
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    macho_segment_command<P>* linkEditSegCmd = (macho_segment_command<P>*)cmd;
    linkEditSegCmd->set_cmd(is64 ? LC_SEGMENT_64 :  LC_SEGMENT);
    linkEditSegCmd->set_cmdsize(sizeof(macho_segment_command<P>));
    linkEditSegCmd->set_segname("__LINKEDIT");
    linkEditSegCmd->set_vmaddr(poolLinkEditStartAddr);
    linkEditSegCmd->set_vmsize(linkEditSegSize);
    linkEditSegCmd->set_fileoff(poolLinkEditStartOffset);
    linkEditSegCmd->set_filesize(linkEditSegSize);
    linkEditSegCmd->set_maxprot(PROT_READ);
    linkEditSegCmd->set_initprot(PROT_READ);
    linkEditSegCmd->set_nsects(0);
    linkEditSegCmd->set_flags(0);
    // LC_ID_DYLIB
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    macho_dylib_command<P>* installNameCmd = (macho_dylib_command<P>*)cmd;
    installNameCmd->set_cmd(LC_ID_DYLIB);
    installNameCmd->set_cmdsize(sizeof(macho_dylib_command<P>) + 48);
	installNameCmd->set_timestamp(2);
	installNameCmd->set_current_version(0x10000);
	installNameCmd->set_compatibility_version(0x10000);
	installNameCmd->set_name_offset();
	strcpy((char*)cmd + sizeof(macho_dylib_command<P>), "dyld_shared_cache_branch_islands");
    // LC_SYMTAB
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	_symbolTableCmd = (macho_symtab_command<P>*)cmd;
	_symbolTableCmd->set_cmd(LC_SYMTAB);
	_symbolTableCmd->set_cmdsize(sizeof(macho_symtab_command<P>));
	_symbolTableCmd->set_nsyms(stubCount);
	_symbolTableCmd->set_symoff((uint32_t)(poolLinkEditStartOffset + linkeditOffsetSymbolTable));
	_symbolTableCmd->set_stroff((uint32_t)(poolLinkEditStartOffset + linkeditOffsetSymbolPoolOffset));
	_symbolTableCmd->set_strsize((uint32_t)(linkEditSegSize - linkeditOffsetSymbolPoolOffset));
    // LC_DYSYMTAB
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	_dynamicSymbolTableCmd = (macho_dysymtab_command<P>*)cmd;
	_dynamicSymbolTableCmd->set_cmd(LC_DYSYMTAB);
	_dynamicSymbolTableCmd->set_cmdsize(sizeof(macho_dysymtab_command<P>));
	_dynamicSymbolTableCmd->set_ilocalsym(0);
	_dynamicSymbolTableCmd->set_nlocalsym(0);
	_dynamicSymbolTableCmd->set_iextdefsym(0);
	_dynamicSymbolTableCmd->set_nextdefsym(0);
	_dynamicSymbolTableCmd->set_iundefsym(0);
	_dynamicSymbolTableCmd->set_nundefsym(stubCount);
	_dynamicSymbolTableCmd->set_tocoff(0);
	_dynamicSymbolTableCmd->set_ntoc(0);
	_dynamicSymbolTableCmd->set_modtaboff(0);
	_dynamicSymbolTableCmd->set_nmodtab(0);
	_dynamicSymbolTableCmd->set_extrefsymoff(0);
    _dynamicSymbolTableCmd->set_nextrefsyms(0);
	_dynamicSymbolTableCmd->set_indirectsymoff((uint32_t)(poolLinkEditStartOffset + linkeditOffsetIndirectSymbolTable));
	_dynamicSymbolTableCmd->set_nindirectsyms(stubCount);
    _dynamicSymbolTableCmd->set_extreloff(0);
    _dynamicSymbolTableCmd->set_nextrel(0);
    _dynamicSymbolTableCmd->set_locreloff(0);
    _dynamicSymbolTableCmd->set_nlocrel(0);
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    // LC_UUID
	_uuidCmd = (macho_uuid_command<P>*)cmd;
	_uuidCmd->set_cmd(LC_UUID);
	_uuidCmd->set_cmdsize(sizeof(macho_uuid_command<P>));
    cmd = (macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());

    // write stubs section content
    _stubInstructions = (uint32_t*)((uint8_t*)mh + _firstStubOffset);
    for (int i=0; i < stubCount; ++i) {
        E::set32(_stubInstructions[i], 0xD4200000);
    }

    // write linkedit content
    uint8_t* linkeditBufferStart = (uint8_t*)cacheBuffer + poolLinkEditStartOffset;
    // write symbol table
    _symbolTable = (macho_nlist<P>*)(linkeditBufferStart);
    for (int i=0; i < stubCount; ++i) {
        _symbolTable[i].set_n_strx(1);
        _symbolTable[i].set_n_type(N_UNDF | N_EXT);
        _symbolTable[i].set_n_sect(0);
        _symbolTable[i].set_n_desc(0);
        _symbolTable[i].set_n_value(0);
    }
    // write indirect symbol table
    uint32_t* indirectSymboTable = (uint32_t*)(linkeditBufferStart + linkeditOffsetIndirectSymbolTable);
    for (int i=0; i < stubCount; ++i) {
        P::E::set32(indirectSymboTable[i], i);
    }
    // write string pool
    _stringPoolStart = (char*)(linkeditBufferStart + linkeditOffsetSymbolPoolOffset);
    _stringPoolEnd = _stringPoolStart + linkEditSegSize - linkeditOffsetSymbolPoolOffset;
    _stringPoolStart[0] = '\0';
    strcpy(&_stringPoolStart[1], "<unused>");
    _nextString = &_stringPoolStart[10];
}


template <typename P>
void BranchPoolDylib<P>::finalizeLoadCommands()
{
    _symbolTableCmd->set_nsyms(_nextIndex);
	_symbolTableCmd->set_strsize((uint32_t)(_nextString - _stringPoolStart));
    _dynamicSymbolTableCmd->set_nundefsym(_nextIndex);

    uint8_t digest[CC_MD5_DIGEST_LENGTH];
    CC_MD5(_stubInstructions, _maxStubs*sizeof(uint64_t), digest);
	_uuidCmd->set_uuid(digest);

    if ( verbose ) {
        verboseLog("branch islands in image at 0x%0llX:", _startAddr);
        for (uint32_t i=0; i < _nextIndex; ++i) {
            verboseLog("   0x%llX  %s", indexToAddr(i), _islandIndexToName[i]);
        }
    }
}

template <typename P>
uint64_t BranchPoolDylib<P>::getForwardBranch(uint64_t finalTargetAddr, const char* name, std::vector<BranchPoolDylib<P>*>& branchIslandPools)
{
    // check if we can re-used existing branch island
    const auto& pos = _targetToIslandIndex.find(finalTargetAddr);
    if ( pos != _targetToIslandIndex.end() )
        return indexToAddr(pos->second);

    // skip if instruction pool is full
    if ( _nextIndex >= _maxStubs )
        return 0;

    // skip if string pool is full
    if ( (_nextString + strlen(name)+1) >= _stringPoolEnd )
        return 0;

    uint64_t branchIslandTargetAddr = finalTargetAddr;
    // if final target is too far, we need to use branch island in next pool
    if ( (finalTargetAddr - _startAddr) > b128MegLimit ) {
        BranchPoolDylib<P>* nextPool = nullptr;
        for (size_t i=0; i < branchIslandPools.size()-1; ++i) {
            if ( branchIslandPools[i] == this ) {
                nextPool = branchIslandPools[i+1];
                break;
            }
        }

        if (nextPool == nullptr) {
             warning("BranchPoolDylib<P>::getForwardBranch: nextPool unreachable");
            return 0;
        }

        branchIslandTargetAddr = nextPool->getForwardBranch(finalTargetAddr, name, branchIslandPools);
        if ( branchIslandTargetAddr == 0 )
            return 0; // next pool is full
    }

    // write branch instruction in stubs section
    uint32_t index = _nextIndex++;
    int64_t branchDelta =  branchIslandTargetAddr - indexToAddr(index);
    uint32_t branchInstr = 0x14000000 + ((branchDelta/4) & 0x03FFFFFF);
    E::set32(_stubInstructions[index], branchInstr);

    // update symbol table
    _symbolTable[index].set_n_strx((uint32_t)(_nextString - _stringPoolStart));
    strcpy(_nextString, name);
    _nextString += (strlen(name) +1);

    // record island
	_targetToIslandIndex[finalTargetAddr] = index;
    _islandIndexToName[index] = name;
    return indexToAddr(index);
}

template <typename P>
uint64_t BranchPoolDylib<P>::getBackBranch(uint64_t finalTargetAddr, const char* name, std::vector<BranchPoolDylib<P>*>& branchIslandPools)
{
    // check if we can re-used existing branch island
    const auto& pos = _targetToIslandIndex.find(finalTargetAddr);
    if ( pos != _targetToIslandIndex.end() )
        return indexToAddr(pos->second);

     // skip if instruction pool is full
    if ( _nextIndex >= _maxStubs )
        return 0;

    // skip if string pool is full
    if ( (_nextString + strlen(name)+1) >= _stringPoolEnd )
        return 0;

    uint64_t branchIslandTargetAddr = finalTargetAddr;
    // if final target is too far, we need to use branch island in next pool
    if ( (indexToAddr(_nextIndex) - finalTargetAddr) > b128MegLimit ) {
        BranchPoolDylib<P>* nextPool = nullptr;
        for (long i=branchIslandPools.size()-1; i > 0; --i) {
            if ( branchIslandPools[i] == this ) {
                nextPool = branchIslandPools[i-1];
                break;
            }
        }

        if (nextPool == nullptr) {
            warning("BranchPoolDylib<P>::getBackBranch: nextPool unreachable");
            return 0;
        }

        branchIslandTargetAddr = nextPool->getBackBranch(finalTargetAddr, name, branchIslandPools);
        if ( branchIslandTargetAddr == 0 )
            return 0; // next pool is full
    }

    // write branch instruction in stubs section
    uint32_t index = _nextIndex++;
    int64_t branchDelta =  branchIslandTargetAddr - indexToAddr(index);
    uint32_t branchInstr = 0x14000000 + ((branchDelta/4) & 0x03FFFFFF);
    E::set32(_stubInstructions[index], branchInstr);

    // update symbol table
    _symbolTable[index].set_n_strx((uint32_t)(_nextString - _stringPoolStart));
    strcpy(_nextString, name);
    _nextString += (strlen(name) +1);

    // record island
	_targetToIslandIndex[finalTargetAddr] = index;
    _islandIndexToName[index] = name;
    return indexToAddr(index);
}

template <typename P>
void BranchPoolDylib<P>::printStats()
{
    verboseLog("  island pool at 0x%0llX has %u stubs and stringPool size=%lu", _startAddr, _nextIndex, _nextString-_stringPoolStart);
}



template <typename P>
class StubOptimizer {
public:
                            StubOptimizer(void* cacheBuffer, macho_header<P>* mh);
    void                    buildStubMap(const std::unordered_set<std::string>& neverStubEliminate);
    void                    optimizeStubs(std::unordered_map<uint64_t,std::vector<uint64_t>>& targetToBranchIslands);
    void                    bypassStubs(std::unordered_map<uint64_t,std::vector<uint64_t>>& targetToBranchIslands);
    void                    optimizeCallSites(std::vector<BranchPoolDylib<P>*>& branchIslandPools);
    const char*             installName() { return _installName; }
    const uint8_t*          exportsTrie() { return (uint8_t*)_cacheBuffer + _dyldInfo->export_off(); }
    uint32_t                exportsTrieSize() { return _dyldInfo->export_size(); }

    uint32_t                                _stubCount           = 0;
    uint32_t                                _stubOptimizedCount  = 0;
    uint32_t                                _branchesCount       = 0;
    uint32_t                                _branchesModifiedCount = 0;
    uint32_t                                _branchesDirectCount = 0;
    uint32_t                                _branchesIslandCount = 0;

private:

    typedef std::function<bool(uint8_t callSiteKind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction)> CallSiteHandler;
    typedef typename P::uint_t pint_t;
    typedef typename P::E E;

    void                    forEachCallSiteToAStub(CallSiteHandler);
    void                    optimizeArm64CallSites(std::vector<BranchPoolDylib<P>*>& branchIslandPools);
    void                    optimizeArmCallSites();
    void                    optimizeArmStubs();
    uint64_t                lazyPointerAddrFromArm64Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr);
    uint32_t                lazyPointerAddrFromArmStub(const uint8_t* stubInstructions, uint32_t stubVMAddr);
    int32_t                 getDisplacementFromThumbBranch(uint32_t instruction, uint32_t instrAddr);
    uint32_t                setDisplacementInThumbBranch(uint32_t instruction,  uint32_t instrAddr,
                                                         int32_t displacement, bool targetIsThumb);


 	struct AddressAndName { pint_t targetVMAddr; const char* targetName; };
	typedef std::unordered_map<pint_t, AddressAndName> StubVMAddrToTarget;

    static const int64_t b128MegLimit = 0x07FFFFFF;
    static const int64_t b16MegLimit  = 0x00FFFFFF;


    macho_header<P>*                        _mh;
    void*                                   _cacheBuffer;
    uint32_t                                _linkeditSize        = 0;
    uint32_t                                _linkeditCacheOffset = 0;
    uint64_t                                _linkeditAddr        = 0;
    const uint8_t*                          _linkeditBias        = nullptr;
    const char*                             _installName         = nullptr;
    const macho_symtab_command<P>*          _symTabCmd           = nullptr;
    const macho_dysymtab_command<P>*        _dynSymTabCmd        = nullptr;
    const macho_dyld_info_command<P>*       _dyldInfo            = nullptr;
    macho_linkedit_data_command<P>*         _splitSegInfoCmd     = nullptr;
    const macho_section<P>*                 _textSection         = nullptr;
    const macho_section<P>*                 _stubSection         = nullptr;
    uint32_t                                _textSectionIndex    = 0;
    uint32_t                                _stubSectionIndex    = 0;
    pint_t                                  _textSegStartAddr    = 0;
    uint32_t                                _textSegCacheOffset  = 0;
    std::vector<macho_segment_command<P>*>  _segCmds;
	std::unordered_map<pint_t, pint_t>      _stubAddrToLPAddr;
	std::unordered_map<pint_t, pint_t>      _lpAddrToTargetAddr;
 	std::unordered_map<pint_t, const char*> _targetAddrToName;
};



template <typename P>
StubOptimizer<P>::StubOptimizer(void* cacheBuffer, macho_header<P>* mh)
: _mh(mh), _cacheBuffer(cacheBuffer)
{
    _linkeditBias = (uint8_t*)cacheBuffer;
    const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)mh + sizeof(macho_header<P>));
    const uint32_t cmd_count = mh->ncmds();
    macho_segment_command<P>* segCmd;
    uint32_t sectionIndex = 0;
    const macho_load_command<P>* cmd = cmds;
    for (uint32_t i = 0; i < cmd_count; ++i) {
        switch (cmd->cmd()) {
            case LC_ID_DYLIB:
                _installName = ((macho_dylib_command<P>*)cmd)->name();
                break;
            case LC_SYMTAB:
                _symTabCmd = (macho_symtab_command<P>*)cmd;
                break;
            case LC_DYSYMTAB:
                _dynSymTabCmd = (macho_dysymtab_command<P>*)cmd;
                break;
            case LC_SEGMENT_SPLIT_INFO:
                _splitSegInfoCmd = (macho_linkedit_data_command<P>*)cmd;
                break;
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
                _dyldInfo = (macho_dyld_info_command<P>*)cmd;
                break;
            case macho_segment_command<P>::CMD:
                segCmd =( macho_segment_command<P>*)cmd;
                _segCmds.push_back(segCmd);
                if ( strcmp(segCmd->segname(), "__LINKEDIT") == 0 ) {
                    _linkeditSize        = (uint32_t)segCmd->vmsize();
                    _linkeditCacheOffset = (uint32_t)segCmd->fileoff();
                    _linkeditAddr        = segCmd->vmaddr();
                }
                else if ( strcmp(segCmd->segname(), "__TEXT") == 0 ) {
                    _textSegStartAddr = (pint_t)segCmd->vmaddr();
                    _textSegCacheOffset = (uint32_t)((uint8_t*)mh - (uint8_t*)cacheBuffer);
                    const macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)segCmd + sizeof(macho_segment_command<P>));
                    const macho_section<P>* const sectionsEnd = &sectionsStart[segCmd->nsects()];
                    for (const macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
                        ++sectionIndex;
                        if ( strcmp(sect->sectname(), "__text") == 0 ) {
                            _textSection = sect;
                            _textSectionIndex = sectionIndex;
                        }
                        else if ( ((sect->flags() & SECTION_TYPE) == S_SYMBOL_STUBS) && (sect->size() != 0) ) {
                            _stubSection = sect;
                            _stubSectionIndex = sectionIndex;
                        }
                    }
                }
                break;
        }
        cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
    }
}



template <typename P>
uint32_t StubOptimizer<P>::lazyPointerAddrFromArmStub(const uint8_t* stubInstructions, uint32_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions+4));
    uint32_t stubInstr3 = E::get32(*(uint32_t*)(stubInstructions+8));
    int32_t  stubData   = E::get32(*(uint32_t*)(stubInstructions+12));
    if ( stubInstr1 != 0xe59fc004 ) {
        warning("first instruction of stub (0x%08X) is not 'ldr ip, pc + 12' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    if ( stubInstr2 != 0xe08fc00c ) {
        warning("second instruction of stub (0x%08X) is not 'add ip, pc, ip' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    if ( stubInstr3 != 0xe59cf000 ) {
        warning("third instruction of stub (0x%08X) is not 'ldr pc, [ip]' for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    return stubVMAddr + 12 + stubData;
}


template <typename P>
uint64_t StubOptimizer<P>::lazyPointerAddrFromArm64Stub(const uint8_t* stubInstructions, uint64_t stubVMAddr)
{
    uint32_t stubInstr1 = E::get32(*(uint32_t*)stubInstructions);
    if ( (stubInstr1 & 0x9F00001F) != 0x90000010 ) {
        warning("first instruction of stub (0x%08X) is not ADRP for stub at addr 0x%0llX in %s",
                stubInstr1, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    int32_t adrpValue = ((stubInstr1 & 0x00FFFFE0) >> 3) | ((stubInstr1 & 0x60000000) >> 29);
    if ( stubInstr1 & 0x00800000 )
        adrpValue |= 0xFFF00000;
    uint32_t stubInstr2 = E::get32(*(uint32_t*)(stubInstructions + 4));
    if ( (stubInstr2 & 0xFFC003FF) != 0xF9400210 ) {
        warning("second instruction of stub (0x%08X) is not LDR for stub at addr 0x%0llX in %s",
                stubInstr2, (uint64_t)stubVMAddr, _installName);
        return 0;
    }
    uint32_t ldrValue = ((stubInstr2 >> 10) & 0x00000FFF);
    return (stubVMAddr & (-4096)) + adrpValue*4096 + ldrValue*8;
}



template <typename P>
void StubOptimizer<P>::buildStubMap(const std::unordered_set<std::string>& neverStubEliminate)
{
    // find all stubs and lazy pointers
    const macho_nlist<P>* symbolTable = (const macho_nlist<P>*)(((uint8_t*)_cacheBuffer) + _symTabCmd->symoff());
    const char* symbolStrings = (char*)_cacheBuffer + _symTabCmd->stroff();
	const uint32_t* const indirectTable = (uint32_t*)(((uint8_t*)_cacheBuffer) + _dynSymTabCmd->indirectsymoff());
	const macho_load_command<P>* const cmds = (macho_load_command<P>*)((uint8_t*)_mh + sizeof(macho_header<P>));
	const uint32_t cmd_count = _mh->ncmds();
	const macho_load_command<P>* cmd = cmds;
	for (uint32_t i = 0; i < cmd_count; ++i) {
		if ( cmd->cmd() == macho_segment_command<P>::CMD ) {
			macho_segment_command<P>* seg = (macho_segment_command<P>*)cmd;
			macho_section<P>* const sectionsStart = (macho_section<P>*)((char*)seg + sizeof(macho_segment_command<P>));
			macho_section<P>* const sectionsEnd = &sectionsStart[seg->nsects()];
			for(macho_section<P>* sect = sectionsStart; sect < sectionsEnd; ++sect) {
				if ( sect->size() == 0 ) 
					continue;
				unsigned sectionType = (sect->flags() & SECTION_TYPE);
				const uint32_t indirectTableOffset = sect->reserved1();
				if ( sectionType == S_SYMBOL_STUBS ) {
					const uint32_t stubSize = sect->reserved2();
                    _stubCount = (uint32_t)(sect->size() / stubSize);
					pint_t stubVMAddr = (pint_t)sect->addr();
					for (uint32_t j=0; j < _stubCount; ++j, stubVMAddr += stubSize) {
						uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
						switch ( symbolIndex ) {
							case INDIRECT_SYMBOL_ABS:
							case INDIRECT_SYMBOL_LOCAL:
								break;
							default:
                                if ( symbolIndex >= _symTabCmd->nsyms() ) {
                                    warning("symbol index out of range (%d of %d) for stub at addr 0x%0llX in %s",
                                            symbolIndex, _symTabCmd->nsyms(), (uint64_t)stubVMAddr, _installName);
                                    continue;
                                }
                                const macho_nlist<P>* sym = &symbolTable[symbolIndex];
                                uint32_t stringOffset = sym->n_strx();
                                if ( stringOffset > _symTabCmd->strsize() ) {
                                    warning("symbol string offset out of range (%u of %u) for stub at addr 0x%0llX in %s",
                                            stringOffset, sym->n_strx(), (uint64_t)stubVMAddr, _installName);
                                    continue;
                                }
                                const char* symName = &symbolStrings[stringOffset];
                                if ( neverStubEliminate.count(symName) ) {
                                    //verboseLog("not bypassing stub to %s in %s because target is interposable\n", symName, _installName);
                                    continue;
                                }
                                const uint8_t* stubInstrs = (uint8_t*)_cacheBuffer + sect->offset() + stubVMAddr - sect->addr();
                                pint_t targetLPAddr = 0;
                                switch ( _mh->cputype() ) {
                                    case CPU_TYPE_ARM64:
                                        targetLPAddr = (pint_t)lazyPointerAddrFromArm64Stub(stubInstrs, stubVMAddr);
                                        break;
                                    case CPU_TYPE_ARM:
                                        targetLPAddr = (pint_t)lazyPointerAddrFromArmStub(stubInstrs, (uint32_t)stubVMAddr);
                                        break;
                                }
                                if ( targetLPAddr != 0 )
                                    _stubAddrToLPAddr[stubVMAddr] = targetLPAddr;
                                break;
						}
					}
				}
				else if ( sectionType == S_LAZY_SYMBOL_POINTERS ) {
					pint_t lpVMAddr = (pint_t)sect->addr();
					pint_t* lpContent = (pint_t*)(((uint8_t*)_cacheBuffer) + sect->offset());
					uint32_t elementCount = (uint32_t)(sect->size() / sizeof(pint_t));
                    uint64_t textSegStartAddr = _segCmds[0]->vmaddr();
                    uint64_t textSegEndAddr   = _segCmds[0]->vmaddr() + _segCmds[0]->vmsize();
                    pint_t lpValue;
 					for (uint32_t j=0; j < elementCount; ++j) {
						uint32_t symbolIndex = E::get32(indirectTable[indirectTableOffset + j]); 
						switch ( symbolIndex ) {
							case INDIRECT_SYMBOL_ABS:
							case INDIRECT_SYMBOL_LOCAL:
								break;
							default:
                                lpValue = (pint_t)P::getP(lpContent[j]);
                                if ( symbolIndex >= _symTabCmd->nsyms() ) {
                                    warning("symbol index out of range (%d of %d) for lazy pointer at addr 0x%0llX in %s",
                                            symbolIndex, _symTabCmd->nsyms(), (uint64_t)lpVMAddr, _installName);
                                    continue;
                                }
                                const macho_nlist<P>* sym = &symbolTable[symbolIndex];
                                uint32_t stringOffset = sym->n_strx();
                                if ( stringOffset > _symTabCmd->strsize() ) {
                                    warning("symbol string offset out of range (%u of %u) for lazy pointer at addr 0x%0llX in %s",
                                            stringOffset, sym->n_strx(), (uint64_t)lpVMAddr, _installName);
                                    continue;
                                }
                                const char* symName = &symbolStrings[stringOffset];
                                if ( (lpValue > textSegStartAddr) && (lpValue< textSegEndAddr) ) {
                                    //verboseLog("skipping lazy pointer at 0x%0lX to %s in %s because target is within dylib\n", lpVMAddr, symName, _installName);
                                }
                                else if ( (sizeof(pint_t) == 8) && ((lpValue % 4) != 0) ) {
                                    warning("lazy pointer at 0x%0llX does not point to 4-byte aligned address(0x%0llX) in %s",
                                                (uint64_t)lpVMAddr, (uint64_t)lpValue, _installName);
                                }
                                else {
                                   _lpAddrToTargetAddr[lpVMAddr] = lpValue;
                                   _targetAddrToName[lpValue] = symName;
                                }
 								break;
						}
                        lpVMAddr += sizeof(pint_t);
					}
				}
			}
		}
		cmd = (const macho_load_command<P>*)(((uint8_t*)cmd)+cmd->cmdsize());
	}
}


template <typename P>
void StubOptimizer<P>::forEachCallSiteToAStub(CallSiteHandler handler)
{
	const uint8_t* infoStart = &_linkeditBias[_splitSegInfoCmd->dataoff()];
	const uint8_t* infoEnd = &infoStart[_splitSegInfoCmd->datasize()];
	if ( *infoStart++ != DYLD_CACHE_ADJ_V2_FORMAT )
		terminate("malformed split seg info in %s", _installName);

    uint8_t* textSectionContent = (uint8_t*)_cacheBuffer + _textSegCacheOffset + _textSection->addr() -_textSegStartAddr;

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
		uint64_t toSectionOffset = 0;
		for (uint64_t j=0; j < toOffsetCount; ++j) {
			uint64_t toSectionDelta = read_uleb128(p, infoEnd);
			uint64_t fromOffsetCount = read_uleb128(p, infoEnd);
			toSectionOffset += toSectionDelta;
			for (uint64_t k=0; k < fromOffsetCount; ++k) {
				uint64_t kind = read_uleb128(p, infoEnd);
                if ( kind > 12 )
                    terminate("bad kind (%llu) value in %s", kind, _installName);
				uint64_t fromSectDeltaCount = read_uleb128(p, infoEnd);
				uint64_t fromSectionOffset = 0;
				for (uint64_t l=0; l < fromSectDeltaCount; ++l) {
					uint64_t delta = read_uleb128(p, infoEnd);
					fromSectionOffset += delta;
                    if ( (fromSectionIndex == _textSectionIndex) && (toSectionIndex == _stubSectionIndex) ) {
                        uint32_t* instrPtr = (uint32_t*)(textSectionContent + fromSectionOffset);
                        uint64_t instrAddr = _textSection->addr() + fromSectionOffset;
                        uint64_t stubAddr = _stubSection->addr() + toSectionOffset;
                        uint32_t instruction = E::get32(*instrPtr);
                        _branchesCount++;
                        if ( handler(kind, instrAddr, stubAddr, instruction) ) {
                            _branchesModifiedCount++;
                            E::set32(*instrPtr, instruction);
                        }
                    }
				}
			}
		}
	}
}


/// Extract displacement from a thumb b/bl/blx instruction.
template <typename P>
int32_t StubOptimizer<P>::getDisplacementFromThumbBranch(uint32_t instruction, uint32_t instrAddr)
{
    bool is_blx    = ((instruction & 0xD000F800) == 0xC000F000);
    uint32_t s     = (instruction >> 10) & 0x1;
    uint32_t j1    = (instruction >> 29) & 0x1;
    uint32_t j2    = (instruction >> 27) & 0x1;
    uint32_t imm10 = instruction & 0x3FF;
    uint32_t imm11 = (instruction >> 16) & 0x7FF;
    uint32_t i1    = (j1 == s);
    uint32_t i2    = (j2 == s);
    uint32_t dis   = (s << 24) | (i1 << 23) | (i2 << 22) | (imm10 << 12) | (imm11 << 1);
    int32_t  sdis  = dis;
    int32_t result = s ? (sdis | 0xFE000000) : sdis;
    if ( is_blx && (instrAddr & 0x2) ) {
        // The thumb blx instruction always has low bit of imm11 as zero.  The way
        // a 2-byte aligned blx can branch to a 4-byte aligned ARM target is that
        // the blx instruction always 4-byte aligns the pc before adding the
        // displacement from the blx.  We must emulate that when decoding this.
        result -= 2;
    }
    return result;
}

/// Update a thumb b/bl/blx instruction, switching bl <-> blx as needed.
template <typename P>
uint32_t StubOptimizer<P>::setDisplacementInThumbBranch(uint32_t instruction,  uint32_t instrAddr,
                                                        int32_t displacement, bool targetIsThumb) {
    if ( (displacement > 16777214) || (displacement < (-16777216)) )
        terminate("thumb branch out of range at 0x%0X in %s", instrAddr, _installName);
	bool is_bl = ((instruction & 0xD000F800) == 0xD000F000);
	bool is_blx = ((instruction & 0xD000F800) == 0xC000F000);
	bool is_b = ((instruction & 0xD000F800) == 0x9000F000);
    uint32_t newInstruction = (instruction & 0xD000F800);
    if (is_bl || is_blx) {
        if (targetIsThumb) {
            newInstruction = 0xD000F000; // Use bl
        }
        else {
            newInstruction = 0xC000F000; // Use blx
            // See note in getDisplacementFromThumbBranch() about blx.
            if (instrAddr & 0x2)
                displacement += 2;
        }
    }
    else if (is_b) {
        if ( !targetIsThumb )
            terminate("no pc-rel thumb branch instruction that switches to arm mode at 0x%0X in %s", instrAddr, _installName);
    }
    else {
        terminate("not b/bl/blx at 0x%0X in %s", instrAddr, _installName);
    }
    uint32_t s = (uint32_t)(displacement >> 24) & 0x1;
    uint32_t i1 = (uint32_t)(displacement >> 23) & 0x1;
    uint32_t i2 = (uint32_t)(displacement >> 22) & 0x1;
    uint32_t imm10 = (uint32_t)(displacement >> 12) & 0x3FF;
    uint32_t imm11 = (uint32_t)(displacement >> 1) & 0x7FF;
    uint32_t j1 = (i1 == s);
    uint32_t j2 = (i2 == s);
    uint32_t nextDisp = (j1 << 13) | (j2 << 11) | imm11;
    uint32_t firstDisp = (s << 10) | imm10;
    newInstruction |= (nextDisp << 16) | firstDisp;
    return newInstruction;
}


template <typename P>
void StubOptimizer<P>::optimizeArmCallSites()
{
    forEachCallSiteToAStub([&](uint8_t kind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction) -> bool {
        if ( kind == DYLD_CACHE_ADJ_V2_THUMB_BR22 ) {
			bool is_bl = ((instruction & 0xD000F800) == 0xD000F000);
			bool is_blx = ((instruction & 0xD000F800) == 0xC000F000);
			bool is_b = ((instruction & 0xD000F800) == 0x9000F000);
            if ( !is_bl && !is_blx && !is_b ){
                warning("non-branch instruction at 0x%0llX in %s", callSiteAddr, _installName);
                return false;
            }
            int32_t brDelta = getDisplacementFromThumbBranch(instruction, (uint32_t)callSiteAddr);
            pint_t targetAddr = (pint_t)callSiteAddr + 4 + brDelta;
            if ( targetAddr != stubAddr ) {
                warning("stub target mismatch at callsite 0x%0llX in %s", callSiteAddr, _installName);
                return false;
            }
            // ignore branch if not to a known stub
            const auto& pos = _stubAddrToLPAddr.find(targetAddr);
            if ( pos == _stubAddrToLPAddr.end() )
                return false;
            // ignore branch if lazy pointer is not known (could be resolver based)
            pint_t lpAddr = pos->second;
            const auto& pos2 = _lpAddrToTargetAddr.find(lpAddr);
            if ( pos2 == _lpAddrToTargetAddr.end() )
                return false;
            uint64_t finalTargetAddr = pos2->second;
            int64_t deltaToFinalTarget = finalTargetAddr - (callSiteAddr + 4);
            // if final target within range, change to branch there directly
            if ( (deltaToFinalTarget > -b16MegLimit) && (deltaToFinalTarget < b16MegLimit) ) {
                bool targetIsThumb = finalTargetAddr & 1;
                instruction = setDisplacementInThumbBranch(instruction, (uint32_t)callSiteAddr, (int32_t)deltaToFinalTarget, targetIsThumb);
                _branchesDirectCount++;
                return true;
            }
        }
        else if ( kind == DYLD_CACHE_ADJ_V2_ARM_BR24 ) {
            // too few of these to be worth trying to optimize
        }

        return false;
    });
}


template <typename P>
void StubOptimizer<P>::optimizeArmStubs()
{
    for (const auto& stubEntry : _stubAddrToLPAddr) {
        pint_t stubVMAddr = stubEntry.first;
        pint_t lpVMAddr   = stubEntry.second;
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            return;
        pint_t targetVMAddr = pos->second;

        int32_t delta = (int32_t)(targetVMAddr - (stubVMAddr + 12));
        const uint32_t* stubInstructions = (uint32_t*)((uint8_t*)_cacheBuffer + _stubSection->offset() + stubVMAddr - _stubSection->addr());
        E::set32(*(uint32_t*)&stubInstructions[0], 0xe59fc000);  //      ldr    ip, L0
        E::set32(*(uint32_t*)&stubInstructions[1], 0xe08ff00c);  //      add    pc, pc, ip
        E::set32(*(uint32_t*)&stubInstructions[2], delta);       // L0:  .long  xxxx
        E::set32(*(uint32_t*)&stubInstructions[3], 0xe7ffdefe);  //      trap
        _stubOptimizedCount++;
    }
}




template <typename P>
void StubOptimizer<P>::optimizeArm64CallSites(std::vector<BranchPoolDylib<P>*>& branchIslandPools)
{
    forEachCallSiteToAStub([&](uint8_t kind, uint64_t callSiteAddr, uint64_t stubAddr, uint32_t& instruction) -> bool {
        if ( kind != DYLD_CACHE_ADJ_V2_ARM64_BR26 )
            return false;
		// skip all but BL or B
		if ( (instruction & 0x7C000000) != 0x14000000 )
			return false;
		// compute target of branch instruction
        int32_t brDelta = (instruction & 0x03FFFFFF) << 2;
        if ( brDelta & 0x08000000 )
            brDelta |= 0xF0000000;
        uint64_t targetAddr = callSiteAddr + (int64_t)brDelta;
        if ( targetAddr != stubAddr ) {
            warning("stub target mismatch");
			return false;
        }
        // ignore branch if not to a known stub
        const auto& pos = _stubAddrToLPAddr.find((pint_t)targetAddr);
        if ( pos == _stubAddrToLPAddr.end() )
			return false;
        // ignore branch if lazy pointer is not known (could be resolver based)
        uint64_t lpAddr = pos->second;
        const auto& pos2 = _lpAddrToTargetAddr.find((pint_t)lpAddr);
        if ( pos2 == _lpAddrToTargetAddr.end() )
			return false;
        uint64_t finalTargetAddr = pos2->second;
        int64_t deltaToFinalTarget = finalTargetAddr - callSiteAddr;
        // if final target within range, change to branch there directly
        if ( (deltaToFinalTarget > -b128MegLimit) && (deltaToFinalTarget < b128MegLimit) ) {
            instruction= (instruction & 0xFC000000) | ((deltaToFinalTarget >> 2) & 0x03FFFFFF);
            _branchesDirectCount++;
			return true;
        }
        // find closest branch island pool between instruction and target and get island
        const auto& pos3 = _targetAddrToName.find((pint_t)finalTargetAddr);
        if ( pos3 == _targetAddrToName.end() )
			return false;
        const char* targetName = pos3->second;
        if ( finalTargetAddr > callSiteAddr ) {
            // target is after branch so find first pool after branch
            for ( BranchPoolDylib<P>* pool : branchIslandPools ) {
                if ( (pool->addr() > callSiteAddr) && (pool->addr() < finalTargetAddr) ) {
                    uint64_t brIslandAddr = pool->getForwardBranch(finalTargetAddr, targetName, branchIslandPools);
                    if ( brIslandAddr == 0 ) {
                        // branch island pool full
                        warning("pool full. Can't optimizer branch to %s from 0x%llX in %s\n", targetName, callSiteAddr, _installName);
                        break;
                    }
                    int64_t deltaToTarget = brIslandAddr - callSiteAddr;
                    instruction = (instruction & 0xFC000000) | ((deltaToTarget >> 2) & 0x03FFFFFF);
                    _branchesIslandCount++;
                    return true;
                }
            }
        }
        else {
            // target is before branch so find closest pool before branch
            for (size_t j = branchIslandPools.size(); j > 0; --j) {
                BranchPoolDylib<P>* pool = branchIslandPools[j-1];
                if ( (pool->addr() < callSiteAddr) && (pool->addr() > finalTargetAddr) ) {
                    uint64_t brIslandAddr = pool->getBackBranch(finalTargetAddr, targetName, branchIslandPools);
                    if ( brIslandAddr == 0 ) {
                        // branch island pool full
                        warning("pool full. Can't optimizer branch to %s from 0x%llX in %s\n", targetName, callSiteAddr, _installName);
                        break;
                    }
                    int64_t deltaToTarget = brIslandAddr - callSiteAddr;
                    instruction = (instruction & 0xFC000000) | ((deltaToTarget >> 2) & 0x03FFFFFF);
                    _branchesIslandCount++;
                    return true;
                }
            }
        }
        return false;
    });
}


template <typename P>
void StubOptimizer<P>::optimizeCallSites(std::vector<BranchPoolDylib<P>*>& branchIslandPools)
{
    if ( _textSection == NULL )
        return;
    if ( _stubSection == NULL )
        return;


    switch ( _mh->cputype() ) {
        case CPU_TYPE_ARM64:
            optimizeArm64CallSites(branchIslandPools);
             if ( verbose ) {
                verboseLog("%5u branches in __text, %5u changed to direct branches, %5u changed to use islands for %s",
                            _branchesCount, _branchesDirectCount, _branchesIslandCount, _installName);
            }
           break;
        case CPU_TYPE_ARM:
            optimizeArmCallSites();
            optimizeArmStubs();
            if ( verbose ) {
                verboseLog("%3u of %3u stubs optimized. %5u branches in __text, %5u changed to direct branches for %s",
                            _stubOptimizedCount, _stubCount, _branchesCount, _branchesDirectCount, _installName);
            }
            break;
    }
}


template <typename P>
void SharedCache::bypassStubs(const std::vector<uint64_t>& branchPoolStartAddrs)
{
    verboseLog("Stub elimination optimization:");

    // construct a StubOptimizer for each image
    std::vector<StubOptimizer<P>*> optimizers;
    forEachImage([&](const void* mh, const char*, time_t, ino_t, const std::vector<MachOProxy::Segment>&) {
        optimizers.push_back(new StubOptimizer<P>(_buffer.get(), (macho_header<P>*)mh));
    });

     // construct a BranchPoolDylib for each pool
    std::vector<BranchPoolDylib<P>*> pools;

    if ( _arch.arch == CPU_TYPE_ARM64 ) {
        // Find hole at end of linkedit region for branch pool linkedits
        uint64_t textRegionStartAddr = 0;
        uint64_t linkEditRegionStartAddr = 0;
        uint64_t linkEditRegionEndAddr = 0;
        uint64_t linkEditRegionStartCacheOffset = 0;
        forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
            if ( permissions == (PROT_READ|PROT_EXEC) ) {
                textRegionStartAddr = vmAddr;
            }
            else if ( permissions == PROT_READ ) {
                linkEditRegionStartAddr = vmAddr;
                linkEditRegionEndAddr = vmAddr + size;
                linkEditRegionStartCacheOffset = (char*)content - (char*)_buffer.get();
            }
        });
        uint64_t lastLinkEditRegionUsedOffset = 0;
        forEachImage([&](const void* mh, const char*, time_t, ino_t, const std::vector<MachOProxy::Segment>& segs) {
            for (MachOProxy::Segment seg : segs) {
                if ( seg.name != "__LINKEDIT" )
                    continue;
                if ( seg.fileOffset >= lastLinkEditRegionUsedOffset )
                    lastLinkEditRegionUsedOffset = seg.fileOffset + seg.size;
            }
        });
        uint64_t allPoolsLinkEditStartOffset = lastLinkEditRegionUsedOffset;
        uint64_t allPoolsLinkEditStartAddr =  linkEditRegionStartAddr + allPoolsLinkEditStartOffset - linkEditRegionStartCacheOffset;
        uint64_t allPoolsLinkEditSize = linkEditRegionEndAddr - allPoolsLinkEditStartAddr;
        if ( !branchPoolStartAddrs.empty() ) {
            uint64_t poolLinkEditStartAddr = allPoolsLinkEditStartAddr;
            uint64_t poolLinkEditStartOffset = allPoolsLinkEditStartOffset;
            const uint64_t poolSize = (allPoolsLinkEditSize/branchPoolStartAddrs.size()) & (-4096);
            for (uint64_t poolAddr : branchPoolStartAddrs) {
                pools.push_back(new BranchPoolDylib<P>(_arch, _buffer.get(), _fileSize, poolAddr,
                                                                             textRegionStartAddr, poolLinkEditStartAddr, poolLinkEditStartOffset));
                poolLinkEditStartAddr += poolSize;
                poolLinkEditStartOffset += poolSize;
            }
        }
    }

    // build set of functions to never stub-eliminate because tools may need to override them
    std::unordered_set<std::string> neverStubEliminate;
    for (const char** p=sNeverStubEliminateSymbols; *p != nullptr; ++p) {
        neverStubEliminate.insert(*p);
    }
    for (const char** d=sNeverStubEliminateDylibs; *d != nullptr; ++d) {
        for (StubOptimizer<P>* op : optimizers) {
            if ( strcmp(op->installName(), *d) == 0 ) {
                // add all exports
                const uint8_t* exportsStart = op->exportsTrie();
                const uint8_t* exportsEnd = exportsStart + op->exportsTrieSize();
                std::vector<ExportInfoTrie::Entry> exports;
                if ( !ExportInfoTrie::parseTrie(exportsStart, exportsEnd, exports) ) {
                    terminate("malformed exports trie in %s", *d);
                }
                for(const ExportInfoTrie::Entry& entry : exports) {
                    neverStubEliminate.insert(entry.name);
               }
            }
        }
    }

    // build maps of stubs-to-lp and lp-to-target
    for (StubOptimizer<P>* op : optimizers)
        op->buildStubMap(neverStubEliminate);

    // optimize call sites to by-pass stubs or jump through island
    for (StubOptimizer<P>* op : optimizers)
        op->optimizeCallSites(pools);

   // final fix ups in branch pools
    for (BranchPoolDylib<P>* pool : pools) {
        pool->finalizeLoadCommands();
        pool->printStats();
    }

    // write total optimization info
    uint32_t callSiteCount = 0;
    uint32_t callSiteDirectOptCount = 0;
    uint32_t callSiteOneHopOptCount = 0;
    for (StubOptimizer<P>* op : optimizers) {
        callSiteCount           += op->_branchesCount;
        callSiteDirectOptCount  += op->_branchesDirectCount;
        callSiteOneHopOptCount  += op->_branchesIslandCount;
    }
    verboseLog("  cache contains %u call sites of which %u were direct bound and %u were bound through islands", callSiteCount, callSiteDirectOptCount, callSiteOneHopOptCount);

    // clean up
    for (StubOptimizer<P>* op : optimizers)
        delete op;
   for (BranchPoolDylib<P>* p : pools)
        delete p;

}


void SharedCache::bypassStubs(const std::vector<uint64_t>& branchPoolStartAddrs) {
    switch( _arch.arch ) {
        case CPU_TYPE_ARM:
            bypassStubs<Pointer32<LittleEndian>>(branchPoolStartAddrs);
            break;
        case CPU_TYPE_ARM64:
            bypassStubs<Pointer64<LittleEndian>>(branchPoolStartAddrs);
            break;
        default:
            // no stub optimization done for other arches
            break;
    }
}


/*
template <typename P>
void StubOptimizer<P>::optimizeStubs(std::unordered_map<uint64_t,std::vector<uint64_t>>& targetToBranchIslands)
{
    for (const auto& stubEntry : _stubAddrToLPAddr) {
        pint_t stubVMAddr = stubEntry.first;
        pint_t lpVMAddr   = stubEntry.second;
        const auto& pos = _lpAddrToTargetAddr.find(lpVMAddr);
        if ( pos == _lpAddrToTargetAddr.end() )
            continue;
        pint_t targetVMAddr = pos->second;
        int64_t delta = targetVMAddr - stubVMAddr;
        if ( (delta > -b128MegLimit) && (delta < b128MegLimit) ) {
            // target within reach, change stub to direct branch
            uint32_t* stubInstructions = (uint32_t*)((uint8_t*)_cacheBuffer + _textSegCacheOffset + stubVMAddr -_textSegStartAddr);
            uint32_t stubInstr1 = E::get32(stubInstructions[0]);
            if ( (stubInstr1 & 0x9F00001F) != 0x90000010 ) {
                warning("first instruction of stub (0x%08X) is no longer ADRP for stub at addr 0x%0X in %s\n",
                        stubInstr1,  stubVMAddr, _installName);
                continue;
            }
            uint32_t directBranchInstr = 0x14000000 + ((delta/4) & 0x03FFFFFF);
            E::set32(stubInstructions[0], directBranchInstr);
            uint32_t brkInstr = 0xD4200000;
            E::set32(stubInstructions[1], brkInstr);
            E::set32(stubInstructions[2], brkInstr);
            _stubOptimizedCount++;
            targetToBranchIslands[targetVMAddr].push_back(stubVMAddr);
        }
    }
    verboseLog("%3u of %3u stubs optimized for %s\n", _stubOptimizedCount, _stubCount, _installName);
}


template <typename P>
void StubOptimizer<P>::bypassStubs(std::unordered_map<uint64_t,std::vector<uint64_t>>& targetToBranchIslands)
{
    if ( _textSection == NULL )
        return;

    // scan __text section looking for B(L) instructions that branch to a stub
	unsigned instructionCount = (unsigned)(_textSection->size() / 4);
    uint32_t* instructions = (uint32_t*)((uint8_t*)_cacheBuffer + _textSegCacheOffset + _textSection->addr() -_textSegStartAddr);
	for (unsigned i=0; i < instructionCount; ++i) {
		uint32_t instr = E::get32(instructions[i]); 
		// skip all but BL or B
		if ( (instr & 0x7C000000) != 0x14000000 ) 
			continue;
		// compute target of branch instruction
        int32_t brDelta = (instr & 0x03FFFFFF) << 2;
        if ( brDelta & 0x08000000 )
            brDelta |= 0xF0000000;
		uint64_t branchAddr = _textSection->addr() + i*4;
        uint64_t targetAddr = branchAddr + (int64_t)brDelta;
        // ignore branch if not to a known stub
        const auto& pos = _stubAddrToLPAddr.find(targetAddr);
        if ( pos == _stubAddrToLPAddr.end() )
            continue;
        _branchesCount++;
        // ignore branch if lazy pointer is not known (could be resolver based)
        const auto& pos2 = _lpAddrToTargetAddr.find(pos->second);
        if ( pos2 == _lpAddrToTargetAddr.end() )
            continue;
        uint64_t finalTargetAddr = pos2->second;
        int64_t deltaToFinalTarget = finalTargetAddr - branchAddr;
        // if final target within range, change to branch there directly
        if ( (deltaToFinalTarget > -b128MegLimit) && (deltaToFinalTarget < b128MegLimit) ) {
            uint32_t newInstr = (instr & 0xFC000000) | ((deltaToFinalTarget >> 2) & 0x03FFFFFF);
            E::set32(instructions[i], newInstr);
            _branchesDirectCount++;
           continue;
        }
        // see if there is an existing branch island in range that can be used
        std::vector<uint64_t>& existingBranchIslands = targetToBranchIslands[finalTargetAddr];
        for (uint64_t branchIslandAddr : existingBranchIslands) {
            int64_t deltaToBranchIsland = branchIslandAddr - branchAddr;
            // if final target within range, change to branch deltaToBranchIsland directly
            if ( (deltaToBranchIsland > -b128MegLimit) && (deltaToFinalTarget < b128MegLimit) ) {
                uint32_t newInstr = (instr & 0xFC000000) | ((deltaToBranchIsland >> 2) & 0x03FFFFFF);
                E::set32(instructions[i], newInstr);
                _branchesIslandCount++;
                break;
            }
        }
    }
    if ( verbose ) {
        verboseLog("%5u branches in __text, %5u changed to direct branches, %5u changed to indirect for %s\n",
                    _branchesCount, _branchesDirectCount, _branchesIslandCount, _installName);
    }
}
*/

