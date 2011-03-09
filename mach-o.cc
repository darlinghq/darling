// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mach-o.h"
#include "mach-o/loader.h"

#ifdef NOLOG
# define LOGF(...) if (0) fprintf(stderr, __VA_ARGS__)
#else
# define LOGF(...) fprintf(stderr, __VA_ARGS__)
#endif

typedef long long ll;
typedef unsigned long long ull;

struct sym {
  uint32_t name;
  uint32_t addr;
  uint32_t flag;
};

struct sym64 {
  uint32_t name;
  uint64_t addr;
  uint32_t flag;
};

static uint64_t uleb128(const uint8_t*& p) {
  uint64_t r = 0;
  int s = 0;
  for (;;) {
    uint8_t b = *p++;
    if (b < 0x80) {
      r += b << s;
      break;
    }
    r += (b & 0x7f) << s;
    s += 7;
  }
  return r;
}

static int64_t sleb128(const uint8_t*& p) {
  int64_t r = 0;
  int s = 0;
  for (;;) {
    uint8_t b = *p++;
    if (b < 0x80) {
      if (b & 0x40) {
        r -= (0x80 - b) << s;
      }
      else {
        r += (b & 0x3f) << s;
      }
      break;
    }
    r += (b & 0x7f) << s;
    s += 7;
  }
  return r;
}

void MachO::readBind(const uint8_t* p, const uint8_t* end) {
  uint8_t ordinal = 0;
  const char* sym_name;
  uint8_t type = BIND_TYPE_POINTER;
  int64_t addend = 0;
  int seg_index = 0;
  uint64_t seg_offset = 0;

  while (p < end) {
    uint8_t op = *p & BIND_OPCODE_MASK;
    uint8_t imm = *p & BIND_IMMEDIATE_MASK;
    p++;
    LOGF("bind: op=%x imm=%d\n", op, imm);
    switch (op) {
    case BIND_OPCODE_DONE:
      break;

    case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
      ordinal = imm;
      break;

    case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
      ordinal = uleb128(p);
      break;

    case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
      if (imm == 0) {
        ordinal = 0;
      } else {
        ordinal = BIND_OPCODE_MASK | imm;
      }
      break;

    case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
      sym_name = reinterpret_cast<const char*>(p);
      p += strlen(sym_name) + 1;
      LOGF("sym_name=%s\n", sym_name);
      break;

    case BIND_OPCODE_SET_TYPE_IMM:
      type = imm;
      break;

    case BIND_OPCODE_SET_ADDEND_SLEB:
      addend = sleb128(p);
      break;

    case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
      seg_index = imm;
      seg_offset = uleb128(p);
      break;

    case BIND_OPCODE_ADD_ADDR_ULEB:
      seg_offset += uleb128(p);
      break;

    case BIND_OPCODE_DO_BIND: {
      MachO::Bind* bind = new MachO::Bind();
      uint64_t vmaddr;
      if (is64_) {
        vmaddr = segments64_[seg_index]->vmaddr;
      } else {
        vmaddr = segments_[seg_index]->vmaddr;
      }
      LOGF("do bind! %s seg_index=%d seg_offset=%llu "
           "type=%d ordinal=%d addend=%lld vmaddr=%p\n",
           sym_name, seg_index, (ull)seg_offset,
           type, ordinal, (ll)addend, (void*)vmaddr);
      bind->name = sym_name;
      bind->vmaddr = vmaddr + seg_offset;
      bind->type = type;
      bind->ordinal = ordinal;
      binds_.push_back(bind);

      seg_offset += is64_ ? 8 : 4;

      break;
    }

    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
      fprintf(stderr, "not impl\n");
      break;

    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB:

    default:
      fprintf(stderr, "unknown op\n");
    }
  }
}

MachO::MachO(const char* filename) {
  fd_ = open(filename, O_RDONLY);
  if (fd_ < 0) {
    fprintf(stderr, "open %s: %s\n", filename, strerror(errno));
    exit(1);
  }

  off_t len = lseek(fd_, 0, SEEK_END);
  char* bin = reinterpret_cast<char*>(
    mmap(NULL, len,
         PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd_, 0));
  base_ = bin;

  mach_header* header = reinterpret_cast<mach_header*>(bin);
  LOGF("magic=%x cpu=%d cpusub=%d file=%d ncmds=%d sizecmd=%d flags=%x\n",
       header->magic, header->cputype, header->cpusubtype,
       header->filetype, header->ncmds, header->sizeofcmds,
       header->flags);

  is64_ = false;
  if (header->magic == MH_MAGIC_64) {
    is64_ = true;
  } else  if (header->magic != MH_MAGIC) {
    fprintf(stderr, "Not mach-o\n");
    exit(1);
  }

  if ((header->cputype & 0x00ffffff) != CPU_TYPE_X86) {
    fprintf(stderr, "Unsupported CPU\n");
    exit(1);
  }

  char* cmds_ptr = bin + sizeof(mach_header);
  if (is64_) {
    cmds_ptr += sizeof(uint32_t);
  }

  uint32_t* symtab = NULL;
  const char* symstrtab = NULL;

  for (uint32_t i = 0; i < header->ncmds; i++) {
    uint32_t cmd = *reinterpret_cast<uint32_t*>(cmds_ptr);
    LOGF("%x\n", cmd);

    switch (cmd) {
    case LC_SEGMENT_64: {
      segment_command_64* segment =
        reinterpret_cast<segment_command_64*>(cmds_ptr);
      segments64_.push_back(segment);

      LOGF("segment %s: vmaddr=%p vmsize=%llu "
           "fileoff=%llu filesize=%llu "
           "maxprot=%d initprot=%d nsects=%u flags=%u\n",
           segment->segname,
           (void*)segment->vmaddr, (ull)segment->vmsize,
           (ull)segment->fileoff, (ull)segment->filesize,
           segment->maxprot, segment->initprot,
           segment->nsects, segment->flags);

      section_64* sections = reinterpret_cast<section_64*>(
        cmds_ptr + sizeof(segment_command_64));
      for (uint32_t j = 0; j < segment->nsects; j++) {
        const section_64& sec = sections[j];
        LOGF("section %s in %s: "
             "addr=%p size=%llu offset=%u align=%u "
             "reloff=%u nreloc=%u flags=%u "
             "reserved1=%u reserved2=%u reserved3=%u\n",
             sec.sectname, sec.segname,
             (void*)sec.addr, (ull)sec.size,
             sec.offset, sec.align,
             sec.reloff, sec.nreloc, sec.flags,
             sec.reserved1, sec.reserved2, sec.reserved3);
      }

      break;
    }

    case LC_DYLD_INFO_ONLY: {
      dyld_info_command* dyinfo =
        reinterpret_cast<dyld_info_command*>(cmds_ptr);
      LOGF("dyld info: rebase_off=%u rebase_size=%u "
           "bind_off=%u bind_size=%u "
           "weak_bind_off=%u weak_bind_size=%u "
           "lazy_bind_off=%u lazy_bind_size=%u "
           "export_off=%u export_size=%u\n",
           dyinfo->rebase_off, dyinfo->rebase_size,
           dyinfo->bind_off, dyinfo->bind_size,
           dyinfo->weak_bind_off, dyinfo->weak_bind_size,
           dyinfo->lazy_bind_off, dyinfo->lazy_bind_size,
           dyinfo->export_off, dyinfo->export_size);

      {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->bind_off);
        const uint8_t* end = p + dyinfo->bind_size;
        readBind(p, end);
      }

      {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->lazy_bind_off);
        const uint8_t* end = p + dyinfo->lazy_bind_size;
        readBind(p, end);
      }

      break;
    }

    case LC_SYMTAB: {
      symtab_command* symtab_cmd =
        reinterpret_cast<symtab_command*>(cmds_ptr);

      LOGF("symoff=%u nsysm=%u stroff=%u strsize=%u\n",
           symtab_cmd->symoff, symtab_cmd->nsyms,
           symtab_cmd->stroff, symtab_cmd->strsize);

      symtab = reinterpret_cast<uint32_t*>(bin + symtab_cmd->symoff);
      symstrtab = bin + symtab_cmd->stroff;
      for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
        if (is64_) {
          LOGF("%d %s(%d) %p %d\n",
               i, symstrtab + symtab[0], symtab[0],
               (void*)*(uint64_t*)(symtab + 1),
               symtab[3]);
          symtab += 4;
        } else {
          LOGF("%d %s(%d) %p %d\n",
               i, symstrtab + symtab[0], symtab[0],
               (void*)*(uint32_t*)(symtab + 1),
               symtab[2]);
          symtab += 3;
        }
      }
      symtab = reinterpret_cast<uint32_t*>(bin + symtab_cmd->symoff);
      break;
    }

    case LC_DYSYMTAB: {
      dysymtab_command* dysymtab_cmd =
        reinterpret_cast<dysymtab_command*>(cmds_ptr);

      LOGF("dysym:\n"
           " ilocalsym=%u nlocalsym=%u\n"
           " iextdefsym=%u nextdefsym=%u\n"
           " iundefsym=%u nundefsym=%u\n"
           " tocoff=%u ntoc=%u\n"
           " modtaboff=%u nmodtab=%u\n"
           " extrefsymoff=%u nextrefsyms=%u\n"
           " indirectsymoff=%u nindirectsyms=%u\n"
           " extreloff=%u nextrel=%u\n"
           " locreloff=%u nlocrel=%u\n"
           ,
           dysymtab_cmd->ilocalsym, dysymtab_cmd->nlocalsym,
           dysymtab_cmd->iextdefsym, dysymtab_cmd->nextdefsym,
           dysymtab_cmd->iundefsym, dysymtab_cmd->nundefsym,
           dysymtab_cmd->tocoff, dysymtab_cmd->ntoc,
           dysymtab_cmd->modtaboff, dysymtab_cmd->nmodtab,
           dysymtab_cmd->extrefsymoff, dysymtab_cmd->nextrefsyms,
           dysymtab_cmd->indirectsymoff, dysymtab_cmd->nindirectsyms,
           dysymtab_cmd->extreloff, dysymtab_cmd->nextrel,
           dysymtab_cmd->locreloff, dysymtab_cmd->nlocrel);

      uint32_t* dysyms = reinterpret_cast<uint32_t*>(
        bin + dysymtab_cmd->indirectsymoff);
      for (uint32_t j = 0; j < dysymtab_cmd->nindirectsyms; j++) {
        uint32_t dysym = dysyms[j];
        uint32_t index = dysym & 0x3fffffff;
        const char* local =
          (dysym & INDIRECT_SYMBOL_LOCAL) ? " local" : "";
        const char* abs =
          (dysym & INDIRECT_SYMBOL_ABS) ? " abs" : "";

        uint32_t* sym = symtab;
        sym += index * (is64_ ? 4 : 3);

        LOGF("dylib %d %s(%u)%s%s\n",
             j, symstrtab + sym[0], index, local, abs);
      }

      uint32_t* dymods = reinterpret_cast<uint32_t*>(
        bin + dysymtab_cmd->modtaboff);
      for (uint32_t j = 0; j < dysymtab_cmd->nmodtab; j++) {
        LOGF("dymods: %u\n", dymods[j]);
      }

      break;
    }

    case LC_LOAD_DYLINKER: {
      lc_str name = *reinterpret_cast<lc_str*>(
        cmds_ptr + sizeof(uint32_t) * 2);
      LOGF("dylinker: %s\n", cmds_ptr + name.offset);
      break;
    }

    case LC_UUID:
      break;

    case LC_UNIXTHREAD: {
      uint32_t* p = reinterpret_cast<uint32_t*>(cmds_ptr);
      LOGF("UNIXTHREAD");
      for (uint32_t i = 2; i < p[1]; i++) {
        LOGF(" %d:%x", i, p[i]);
      }
      LOGF("\n");
      entry_ = reinterpret_cast<uint64_t*>(cmds_ptr)[18];
      LOGF("entry=%llx\n", (ull)entry_);
      break;
    }

    case LC_LOAD_DYLIB: {
      dylib* lib = reinterpret_cast<dylib*>(
        cmds_ptr + sizeof(uint32_t) * 2);
      LOGF("dylib: %s\n", cmds_ptr + lib->name.offset);
      break;
    }

    }

    cmds_ptr += reinterpret_cast<uint32_t*>(cmds_ptr)[1];
  }

  LOGF("%p vs %p\n", cmds_ptr, bin + len);
}

MachO::~MachO() {
  for (size_t i = 0; i < binds_.size(); i++) {
    delete binds_[i];
  }
  // need munmap
  close(fd_);
}
