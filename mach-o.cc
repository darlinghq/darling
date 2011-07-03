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

#include "fat.h"
#include "log.h"
#include "mach-o.h"
#include "mach-o/loader.h"

DEFINE_bool(READ_SYMTAB,
#ifdef NDEBUG
            false,
#else
            true,
#endif
            "Read symtab for better backtrace");
DEFINE_bool(READ_DYSYMTAB, false, "Read dysymtab");

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

// See mach-o/nlist.h for this layout
struct nlist {
  uint32_t n_strx;
  uint8_t n_type;
  uint8_t n_sect;
  uint16_t n_desc;
  uint64_t n_value;
};

static uint64_t uleb128(const uint8_t*& p) {
  uint64_t r = 0;
  int s = 0;
  do {
    r |= (uint64_t)(*p & 0x7f) << s;
    s += 7;
  } while (*p++ >= 0x80);
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
        r |= (b & 0x3f) << s;
      }
      break;
    }
    r |= (b & 0x7f) << s;
    s += 7;
  }
  return r;
}

class MachOImpl : public MachO {
 public:
  // Take ownership of fd.
  // If len is 0, the size of file will be used as len.
  MachOImpl(const char* filename, int fd, size_t offset, size_t len,
            bool need_exports);
  virtual ~MachOImpl();
  virtual void close();

 private:
  class RebaseState;
  friend class MachOImpl::RebaseState;
  class BindState;
  friend class MachOImpl::BindState;

  template <class segment_command, class section>
  void readSegment(char* cmds_ptr,
                   vector<segment_command*>* segments,
                   vector<section*>* bind_sections);
  void readRebase(const uint8_t* p, const uint8_t* end);
  void readBind(const uint8_t* p, const uint8_t* end, bool is_weak);
  void readExport(const uint8_t* start, const uint8_t* p, const uint8_t* end,
                  string* name_buf);

  template <class section>
  void readClassicBind(const section& sec,
                       uint32_t* dysyms,
                       uint32_t* symtab,
                       const char* symstrtab) {
    uint32_t indirect_offset = sec.reserved1;
    int count = sec.size / ptrsize_;
    for (int i = 0; i < count; i++) {
      uint32_t dysym = dysyms[indirect_offset + i];
      uint32_t index = dysym & 0x3fffffff;
      uint32_t* sym = symtab;
      sym += index * (is64_ ? 4 : 3);

      MachO::Bind* bind = new MachO::Bind();
      bind->name = symstrtab + sym[0];
      bind->vmaddr = sec.addr + i * ptrsize_;
      bind->addend = 0;
      bind->type = BIND_TYPE_POINTER;
      bind->ordinal = 1;
      binds_.push_back(bind);
    }
  }

  char* mapped_;
  size_t mapped_size_;
  bool need_exports_;
};

template <class segment_command, class section>
void MachOImpl::readSegment(char* cmds_ptr,
                            vector<segment_command*>* segments,
                            vector<section*>* bind_sections) {
  segment_command* segment =
    reinterpret_cast<segment_command*>(cmds_ptr);
  segments->push_back(segment);

  LOGF("segment %s: vmaddr=%p vmsize=%llu "
       "fileoff=%llu filesize=%llu "
       "maxprot=%d initprot=%d nsects=%u flags=%u\n",
       segment->segname,
       (void*)segment->vmaddr, (ull)segment->vmsize,
       (ull)segment->fileoff, (ull)segment->filesize,
       segment->maxprot, segment->initprot,
       segment->nsects, segment->flags);

  section* sections = reinterpret_cast<section*>(
    cmds_ptr + sizeof(segment_command));
  for (uint32_t j = 0; j < segment->nsects; j++) {
    const section& sec = sections[j];
    LOGF("section %s in %s: "
         "addr=%p size=%llu offset=%u align=%u "
         "reloff=%u nreloc=%u flags=%u "
         "reserved1=%u reserved2=%u\n",
         sec.sectname, sec.segname,
         (void*)sec.addr, (ull)sec.size,
         sec.offset, sec.align,
         sec.reloff, sec.nreloc, sec.flags,
         sec.reserved1, sec.reserved2);

    if (!strcmp(sec.sectname, "__dyld") &&
        !strcmp(sec.segname, "__DATA")) {
      dyld_data_ = sec.addr;
    }

    int section_type = sec.flags & SECTION_TYPE;
    switch (section_type) {
    case S_REGULAR:
        /* Regular section: nothing to do */
        break;

    case S_MOD_INIT_FUNC_POINTERS: {
      for (uint64_t p = sec.addr; p < sec.addr + sec.size; p += ptrsize_) {
        init_funcs_.push_back(p);
      }
      break;
    }
    case S_NON_LAZY_SYMBOL_POINTERS:
    case S_LAZY_SYMBOL_POINTERS: {
      bind_sections->push_back(sections + j);
      break;
    }
    case S_ZEROFILL:
    case S_CSTRING_LITERALS:
    case S_4BYTE_LITERALS:
    case S_8BYTE_LITERALS:
    case S_LITERAL_POINTERS:
    case S_SYMBOL_STUBS:
    case S_MOD_TERM_FUNC_POINTERS:
      // TODO(hamaji): Support term_funcs.
    case S_COALESCED:
    case S_GB_ZEROFILL:
    case S_INTERPOSING:
    case S_16BYTE_LITERALS:
    case S_DTRACE_DOF:
    case S_LAZY_DYLIB_SYMBOL_POINTERS:
      LOGF("FIXME: section type %d will not be handled for %s in %s\n",
           section_type, sec.sectname, sec.segname);
      break;

    default:
        fprintf(stderr, "Unknown section type: %d\n", section_type);
        abort();
        break;
    }
  }
}

struct MachOImpl::RebaseState {
  explicit RebaseState(MachOImpl* mach0)
    : mach(mach0), type(0), seg_index(0), seg_offset(0) {}

  bool readRebaseOp(const uint8_t*& p) {
    uint8_t op = *p & REBASE_OPCODE_MASK;
    uint8_t imm = *p & REBASE_IMMEDIATE_MASK;
    p++;
    switch (op) {
    case REBASE_OPCODE_DONE:
      return false;

    case REBASE_OPCODE_SET_TYPE_IMM:
      type = imm;
      break;

    case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
      seg_index = imm;
      seg_offset = uleb128(p);
      break;

    case REBASE_OPCODE_ADD_ADDR_ULEB:
      seg_offset += uleb128(p);
      break;

    case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
      seg_offset += imm * mach->ptrsize_;
      break;

    case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
      for (int i = 0; i < imm; i++) {
        addRebase();
      }
      break;

    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES: {
      int count = uleb128(p);
      for (int i = 0; i < count; i++) {
        addRebase();
      }
      break;
    }

    case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
      addRebase();
      seg_offset += uleb128(p);
      break;

    case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB: {
      int count = uleb128(p);
      uint64_t skip = uleb128(p);
      for (int i = 0; i < count; i++) {
        addRebase();
        seg_offset += skip;
      }
      break;
    }

    default:
      fprintf(stderr, "unknown op: %x\n", op);
    }

    return true;
  }

  void addRebase() {
    MachO::Rebase* rebase = new MachO::Rebase();
    uint64_t vmaddr;
    if (mach->is64_) {
      vmaddr = mach->segments64_[seg_index]->vmaddr;
    } else {
      vmaddr = mach->segments_[seg_index]->vmaddr;
    }
    LOGF("add rebase! seg_index=%d seg_offset=%llu type=%d vmaddr=%p\n",
         seg_index, (ull)seg_offset, type, (void*)vmaddr);
    rebase->vmaddr = vmaddr + seg_offset;
    rebase->type = type;
    mach->rebases_.push_back(rebase);

    seg_offset += mach->ptrsize_;
  }

  MachOImpl* mach;
  uint8_t type;
  int seg_index;
  uint64_t seg_offset;
};

void MachOImpl::readRebase(const uint8_t* p, const uint8_t* end) {
  RebaseState state(this);
  while (p < end) {
    if (!state.readRebaseOp(p))
      break;
  }
}

struct MachOImpl::BindState {
  BindState(MachOImpl* mach0, bool is_weak0)
    : mach(mach0), ordinal(0), sym_name(NULL), type(BIND_TYPE_POINTER),
      addend(0), seg_index(0), seg_offset(0), is_weak(is_weak0) {}

  void readBindOp(const uint8_t*& p) {
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

    case BIND_OPCODE_DO_BIND:
      addBind();
      break;

    case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
      LOGF("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB\n");
      addBind();
      seg_offset += uleb128(p);
      break;

    case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
      LOGF("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED %d\n", (int)imm);
      addBind();
      seg_offset += imm * mach->ptrsize_;
      break;

    case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: {
      uint64_t count = uleb128(p);
      uint64_t skip = uleb128(p);
      LOGF("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB %u %u\n",
           (unsigned)count, (unsigned)skip);
      for (uint64_t i = 0; i < count; i++) {
        addBind();
        seg_offset += skip;
      }
      break;
    }

    default:
      fprintf(stderr, "unknown op: %x\n", op);
    }
  }

  void addBind() {
    MachO::Bind* bind = new MachO::Bind();
    uint64_t vmaddr;
    if (mach->is64_) {
      vmaddr = mach->segments64_[seg_index]->vmaddr;
    } else {
      vmaddr = mach->segments_[seg_index]->vmaddr;
    }
    LOGF("add bind! %s seg_index=%d seg_offset=%llu "
         "type=%d ordinal=%d addend=%lld vmaddr=%p\n",
         sym_name, seg_index, (ull)seg_offset,
         type, ordinal, (ll)addend, (void*)(vmaddr + seg_offset));
    bind->name = sym_name;
    bind->vmaddr = vmaddr + seg_offset;
    bind->addend = addend;
    bind->type = type;
    bind->ordinal = ordinal;
    bind->is_weak = is_weak;
    mach->binds_.push_back(bind);

    seg_offset += mach->ptrsize_;
  }

  MachOImpl* mach;
  uint8_t ordinal;
  const char* sym_name;
  uint8_t type;
  int64_t addend;
  int seg_index;
  uint64_t seg_offset;
  bool is_weak;
};

void MachOImpl::readBind(const uint8_t* p, const uint8_t* end, bool is_weak) {
  BindState state(this, is_weak);
  while (p < end) {
    state.readBindOp(p);
  }
}

void MachOImpl::readExport(const uint8_t* start,
                           const uint8_t* p,
                           const uint8_t* end,
                           string* name_buf) {
  LOGF("readExport: %p-%p\n", p, end);
#if 0
  char buf[17];
  buf[16] = '\0';
  for (int i = 0; i < 16*8; i++) {
    LOGF("%02x ", p[i]);
    buf[i % 16] = p[i] < 32 ? '?' : p[i];
    if (i % 16 == 15) LOGF("%s\n", buf);
  }
#endif

  if (p >= end) {
    fprintf(stderr, "broken export trie\n");
    exit(1);
  }

  if (uint8_t term_size = *p++) {
    const uint8_t* expected_term_end = p + term_size;
    Export* exp = new Export;
    exp->name = *name_buf;
    exp->flag = uleb128(p);
    exp->addr = uleb128(p);
    LOGF("export: %s %lu %p\n",
         name_buf->c_str(), (long)exp->flag, (void*)exp->addr);

    exports_.push_back(exp);

    CHECK(expected_term_end == p);
  }

  const uint8_t num_children = *p++;
  for (uint8_t i = 0; i < num_children; i++) {
    size_t orig_name_size = name_buf->size();
    while (*p) {
      name_buf->push_back(*p++);
    }
    p++;

    uint64_t off = uleb128(p);
    CHECK(off != 0);
    readExport(start, start + off, end, name_buf);

    name_buf->resize(orig_name_size);
  }
}

MachOImpl::MachOImpl(const char* filename, int fd, size_t offset, size_t len,
                     bool need_exports)
  : mapped_(NULL), mapped_size_(len) {
  filename_ = filename;
  need_exports_ = need_exports;
  dyld_data_ = 0;
  CHECK(fd);
  fd_ = fd;
  offset_ = offset;

  if (!mapped_size_) {
    mapped_size_ = lseek(fd_, 0, SEEK_END);
  }
  lseek(fd, 0, SEEK_SET);

  char* bin = mapped_ = reinterpret_cast<char*>(
    mmap(NULL, mapped_size_,
         PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd_, offset));
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
    fprintf(stderr, "Not mach-o: %s\n", filename);
    exit(1);
  }
  ptrsize_ = is64_ ? 8 : 4;

  if ((header->cputype & 0x00ffffff) != CPU_TYPE_X86) {
    fprintf(stderr, "Unsupported CPU\n");
    exit(1);
  }

  struct load_command* cmds_ptr = reinterpret_cast<struct load_command*>(
                                  bin + (is64_ ? sizeof(mach_header_64)
                                               : sizeof(mach_header)));

  uint32_t* symtab = NULL;
  uint32_t* dysyms = NULL;
  const char* symstrtab = NULL;
  dyld_info_command* dyinfo = NULL;
  vector<section_64*> bind_sections_64;
  vector<section*> bind_sections_32;

  for (uint32_t ii = 0; ii < header->ncmds; ii++) {
    LOGF("cmd type:%x\n", cmds_ptr->cmd);

    switch (cmds_ptr->cmd) {
    case LC_SEGMENT_64: {
      readSegment<segment_command_64, section_64>(
          (char *)cmds_ptr, &segments64_, &bind_sections_64);
      break;
    }

    case LC_SEGMENT: {
      readSegment<segment_command, section>(
          (char *)cmds_ptr, &segments_, &bind_sections_32);
      break;
    }

    case LC_DYLD_INFO_ONLY: {
      dyinfo = reinterpret_cast<dyld_info_command*>(cmds_ptr);
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
          bin + dyinfo->rebase_off);
        const uint8_t* end = p + dyinfo->rebase_size;
        if (dyinfo->rebase_off && dyinfo->rebase_size) {
          readRebase(p, end);
        }
      }

      {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->bind_off);
        const uint8_t* end = p + dyinfo->bind_size;
        readBind(p, end, false);
      }

      {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->lazy_bind_off);
        const uint8_t* end = p + dyinfo->lazy_bind_size;
        readBind(p, end, false);
      }

      {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->weak_bind_off);
        const uint8_t* end = p + dyinfo->weak_bind_size;
        readBind(p, end, true);
      }

      if (need_exports_) {
        const uint8_t* p = reinterpret_cast<uint8_t*>(
          bin + dyinfo->export_off);
        const uint8_t* end = p + dyinfo->export_size;
        if (dyinfo->export_off && dyinfo->export_size) {
          string buf;
          readExport(p, p, end, &buf);
        }
      }

      break;
    }

    case LC_SYMTAB: {
      symtab_command* symtab_cmd =
        reinterpret_cast<symtab_command*>(cmds_ptr);

      LOGF("symoff=%u nsysm=%u stroff=%u strsize=%u\n",
           symtab_cmd->symoff, symtab_cmd->nsyms,
           symtab_cmd->stroff, symtab_cmd->strsize);

      uint32_t* symtab_top = symtab =
          reinterpret_cast<uint32_t*>(bin + symtab_cmd->symoff);
      symstrtab = bin + symtab_cmd->stroff;

      if (FLAGS_READ_SYMTAB) {
        for (uint32_t i = 0; i < symtab_cmd->nsyms; i++) {
          Symbol sym;
          nlist* nl = (nlist*)symtab;
          sym.name = symstrtab + nl->n_strx;
          if (is64_) {
            sym.addr = nl->n_value;
            symtab += 4;
          } else {
            sym.addr = (uint32_t)nl->n_value;
            symtab += 3;
          }

          LOGF("%d %s(%d) %p\n",
               i, sym.name.c_str(), nl->n_strx, (void*)sym.addr);
          symbols_.push_back(sym);
        }
      }

      // Will be used by other load commands.
      symtab = symtab_top;

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

      if (dysymtab_cmd->nindirectsyms) {
          dysyms = reinterpret_cast<uint32_t*>(
              bin + dysymtab_cmd->indirectsymoff);
      }
      if (FLAGS_READ_DYSYMTAB) {
        for (uint32_t j = 0; j < dysymtab_cmd->nindirectsyms; j++) {
          uint32_t dysym = dysyms[j];
          uint32_t index = dysym & 0x3fffffff;
          const char* local =
            (dysym & INDIRECT_SYMBOL_LOCAL) ? " local" : "";
          const char* abs =
            (dysym & INDIRECT_SYMBOL_ABS) ? " abs" : "";

          uint32_t* sym = symtab;
          sym += index * (is64_ ? 4 : 3);

          LOGF("dysym %d %s(%u)%s%s\n",
               j, symstrtab + sym[0], index, local, abs);
        }

        uint32_t* dymods = reinterpret_cast<uint32_t*>(
            bin + dysymtab_cmd->modtaboff);
        for (uint32_t j = 0; j < dysymtab_cmd->nmodtab; j++) {
          LOGF("dymods: %u\n", dymods[j]);
        }
      }

      break;
    }

    case LC_LOAD_DYLINKER: {
      lc_str name = reinterpret_cast<struct dylinker_command*>(cmds_ptr)->name;
      LOGF("dylinker: %s\n", (char *)cmds_ptr + name.offset);
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
      if (is64_) {
        entry_ = reinterpret_cast<uint64_t*>(cmds_ptr)[18];
      } else {
        entry_ = reinterpret_cast<uint32_t*>(cmds_ptr)[14];
      }
      LOGF("entry=%llx\n", (ull)entry_);
      break;
    }

    case LC_LOAD_DYLIB: {
      dylib* lib = &reinterpret_cast<dylib_command*>(cmds_ptr)->dylib;
      LOGF("dylib: '%s'\n", (char *)cmds_ptr + lib->name.offset);
      dylibs_.push_back((char *)cmds_ptr + lib->name.offset);
      break;
    }

    }

    cmds_ptr = reinterpret_cast<load_command*>(reinterpret_cast<char *>(cmds_ptr) + cmds_ptr->cmdsize);
  }

  LOGF("%p vs %p\n", cmds_ptr, bin + mapped_size_);

  // No LC_DYLD_INFO_ONLY, we will read classic binding info.
  if (!dyinfo && dysyms && symtab && symstrtab) {
    for (size_t i = 0; i < bind_sections_64.size(); i++) {
      readClassicBind<section_64>(
          *bind_sections_64[i], dysyms, symtab, symstrtab);
    }
    for (size_t i = 0; i < bind_sections_32.size(); i++) {
      readClassicBind<section>(
          *bind_sections_32[i], dysyms, symtab, symstrtab);
    }
  }
}

MachOImpl::~MachOImpl() {
  close();
}

void MachOImpl::close() {
  for (size_t i = 0; i < binds_.size(); i++) {
    delete binds_[i];
  }
  binds_.clear();
  for (size_t i = 0; i < rebases_.size(); i++) {
    delete rebases_[i];
  }
  rebases_.clear();
  for (size_t i = 0; i < exports_.size(); i++) {
    delete exports_[i];
  }
  exports_.clear();

  if (mapped_) {
    munmap(mapped_, mapped_size_);
    ::close(fd_);
    mapped_ = NULL;
    fd_ = -1;
  }
}

MachO* MachO::read(const char* path, const char* arch, bool need_exports) {
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "%s: %s\n", path, strerror(errno));
    exit(1);
  }

  size_t offset = 0, len = 0;
  map<string, fat_arch> archs;
  if (readFatInfo(fd, &archs)) {
    map<string, fat_arch>::const_iterator found = archs.find(arch);
    if (found == archs.end()) {
      fprintf(stderr,
              "%s is a fat binary, but doesn't contain %s binary\n",
              path, arch);
      exit(1);
    }
    offset = found->second.offset;
    len = found->second.size;
    LOGF("fat offset=%lu, len=%lu\n",
         (unsigned long)offset, (unsigned long)len);
  }

  return new MachOImpl(path, fd, offset, len, need_exports);
}
