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

// A Mach-O loader for linux.

#include <assert.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <execinfo.h>
#include <fcntl.h>
#include <limits.h>
#include <memory>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <ucontext.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tr1/unordered_map>
#include <vector>

#include "env_flags.h"
#include "fat.h"
#include "log.h"
#include "mach-o.h"

using namespace std;
using namespace std::tr1;

DEFINE_bool(TRACE_FUNCTIONS, false, "Show calling functions");
DEFINE_bool(PRINT_TIME, false, "Print time spent in this loader");

class MachO;

static map<string, string> g_rename;
static vector<string> g_bound_names;
static set<string> g_no_trampoline;

struct Timer {
  Timer() : start_time(0) {}

  void start() {
    if (FLAGS_PRINT_TIME) {
      start_time = clock();
    }
  }

  void print(const char* name) {
    if (FLAGS_PRINT_TIME) {
      double elapsed = ((double)clock() - start_time) / CLOCKS_PER_SEC;
      printf("Elapsed time (%s): %f sec\n", name, elapsed);
    }
  }

  clock_t start_time;
};

class FileMap {
 public:
  void add(const MachO& mach, uintptr_t slide, uintptr_t base) {
    SymbolMap* symbol_map = new SymbolMap();
    symbol_map->filename = mach.filename();
    symbol_map->base = base;
    if (!maps_.insert(make_pair(base, symbol_map)).second) {
      err(1, "dupicated base addr: %p in %s",
          (void*)base, mach.filename().c_str());
    }

    for (size_t i = 0; i < mach.symbols().size(); i++) {
      MachO::Symbol sym = mach.symbols()[i];
      if (sym.name.empty() || sym.name[0] != '_')
        continue;
      sym.addr += slide;
      if (sym.addr < base)
        continue;
      symbol_map->symbols.insert(make_pair(sym.addr, sym.name.substr(1)));
    }
  }

  void addWatchDog(uintptr_t addr) {
    bool r = maps_.insert(make_pair(addr, (SymbolMap*)NULL)).second;
    CHECK(r);
  }

  const char* dumpSymbol(void* p) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    map<uintptr_t, SymbolMap*>::const_iterator found = maps_.upper_bound(addr);
    if (found == maps_.begin() || found == maps_.end()) {
      return NULL;
    }

    --found;
    return dumpSymbolFromMap(*found->second, addr);
  }

 private:
  struct SymbolMap {
    string filename;
    map<uintptr_t, string> symbols;
    uintptr_t base;
  };

  const char* dumpSymbolFromMap(const SymbolMap& symbol_map, uintptr_t addr) {
    uintptr_t file_offset = addr - symbol_map.base;

    // Use lower_bound as PC may be in just after call.
    map<uintptr_t, string>::const_iterator found =
        symbol_map.symbols.lower_bound(addr);
    if (found == symbol_map.symbols.begin()) {
      snprintf(dumped_stack_frame_buf_, 4095, "%s [%p(%lx)]",
               symbol_map.filename.c_str(), (void*)addr, (long)file_offset);
      return dumped_stack_frame_buf_;
    }

    --found;
    const char* name = found->second.c_str();
    uintptr_t func_offset = addr - found->first;
    snprintf(dumped_stack_frame_buf_, 4095, "%s(%s+%lx) [%p(%lx)]",
             symbol_map.filename.c_str(), name, (long)func_offset,
             (void*)addr, (long)file_offset);
    return dumped_stack_frame_buf_;
  }

  map<uintptr_t, SymbolMap*> maps_;
  char dumped_stack_frame_buf_[4096];
};

static FileMap g_file_map;

#ifdef __x86_64__
static const char* ARCH_NAME = "x86-64";
static const int BITS = 64;
#else
static const char* ARCH_NAME = "i386";
static const int BITS = 32;
#endif

static char* g_darwin_executable_path;

static Timer g_timer;

// TODO(hamaji): Need a static type...
class MachOLoader;
static MachOLoader* g_loader;

static void initRename() {
#define RENAME(src, dst) g_rename.insert(make_pair(#src, #dst));
#define WRAP(src) RENAME(src, __darwin_ ## src)
#include "rename.tab"
#undef RENAME
#undef WRAP
}

static void initNoTrampoline() {
#define NO_TRAMPOLINE(name) g_no_trampoline.insert(#name);
#include "no_trampoline.tab"
#undef NO_TRAMPOLINE
}

static void undefinedFunction() {
  fprintf(stderr, "Undefined function called\n");
  abort();
}

static void doNothing() {
}

static bool lookupDyldFunction(const char* name, uint64_t* addr) {
  LOG << "lookupDyldFunction: " << name << endl;
  *addr = (int64_t)&doNothing;
  return true;
}

static uint64_t alignMem(uint64_t p, uint64_t a) {
  a--;
  return (p + a) & ~a;
}

static void dumpInt(int bound_name_id) {
  if (bound_name_id < 0) {
    fprintf(stderr, "%d: negative bound function id\n", bound_name_id);
    return;
  }
  if (bound_name_id >= (int)g_bound_names.size()) {
    fprintf(stderr, "%d: bound function id overflow\n", bound_name_id);
    return;
  }
  if (g_bound_names[bound_name_id].empty()) {
    fprintf(stderr, "%d: unbound function id\n", bound_name_id);
    return;
  }
  printf("calling %s(%d)\n",
         g_bound_names[bound_name_id].c_str(), bound_name_id);
  fflush(stdout);
}

typedef unordered_map<string, MachO::Export> Exports;

class MachOLoader {
#ifdef __x86_64__
  typedef uint64_t intptr;
  typedef segment_command_64 Segment;

  static const vector<Segment*>& getSegments(const MachO& mach) {
    return mach.segments64();
  }
#else
  typedef uint32_t intptr;
  typedef segment_command Segment;

  static const vector<Segment*>& getSegments(const MachO& mach) {
    return mach.segments();
  }
#endif

 public:
  MachOLoader()
    : last_addr_(0) {
    dylib_to_so_.insert(make_pair(
                          "/System/Library/Frameworks/CoreFoundation.framework"
                          "/Versions/A/CoreFoundation",
                          "libCoreFoundation.so"));
    dylib_to_so_.insert(make_pair(
                          "/usr/lib/libncurses.5.4.dylib",
                          "libncurses.so"));

    symbol_to_so_.insert(make_pair("uuid_clear", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_compare", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_copy", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_generate", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_generate_random", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_generate_time", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_is_null", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_pack", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_parse", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_unpack", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_unparse", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_unparse_lower", "libuuid.so"));
    symbol_to_so_.insert(make_pair("uuid_unparse_upper", "libuuid.so"));

    if (FLAGS_TRACE_FUNCTIONS) {
      // Push all arguments into stack.

      // push %rax
      pushTrampolineCode(0x50);
      // push %rdi
      pushTrampolineCode(0x57);
      // push %rsi
      pushTrampolineCode(0x56);
      // push %rdx
      pushTrampolineCode(0x52);
      // push %rcx
      pushTrampolineCode(0x51);
      // push %r8
      pushTrampolineCode(0x5041);
      // push %r9
      pushTrampolineCode(0x5141);

      // push %xmm0..%xmm7
      for (int i = 0; i < 8; i++) {
        // sub $8, %rsp
        pushTrampolineCode(0x08ec8348);

        // movq %xmmN, (%rsp)
        pushTrampolineCode(0xd60f66);
        pushTrampolineCode(4 + i * 8);
        pushTrampolineCode(0x24);
      }

      // mov %r10, %rdi
      pushTrampolineCode(0xd7894c);

      // mov $func, %rdx
      pushTrampolineCode(0xba48);
      pushTrampolineCode64((unsigned long long)(void*)&dumpInt);

      // call *%rdx
      pushTrampolineCode(0xd2ff);

      // pop %xmm7..%xmm0
      for (int i = 7; i >= 0; i--) {
        // movq (%rsp), %xmmN
        pushTrampolineCode(0x7e0ff3);
        pushTrampolineCode(4 + i * 8);
        pushTrampolineCode(0x24);

        // add $8, %rsp
        pushTrampolineCode(0x08c48348);
      }

      // pop %r9
      pushTrampolineCode(0x5941);
      // pop %r8
      pushTrampolineCode(0x5841);
      // pop %rcx
      pushTrampolineCode(0x59);
      // pop %rdx
      pushTrampolineCode(0x5a);
      // pop %rsi
      pushTrampolineCode(0x5e);
      // pop %rdi
      pushTrampolineCode(0x5f);
      // pop %rax
      pushTrampolineCode(0x58);

      // ret
      pushTrampolineCode(0xc3);
    }
  }

  void loadSegments(const MachO& mach, intptr* slide, intptr* base) {
    *base = 0;
    --*base;

    const vector<Segment*>& segments = getSegments(mach);
    for (size_t i = 0; i < segments.size(); i++) {
      Segment* seg = segments[i];
      const char* name = seg->segname;
      if (!strcmp(name, SEG_PAGEZERO) || !strcmp(name, SEG_LINKEDIT)) {
        continue;
      }

      LOG << seg->segname << ": "
          << "fileoff=" << seg->fileoff
          << ", vmaddr=" << seg->vmaddr << endl;

      int prot = 0;
      if (seg->initprot & VM_PROT_READ) {
        prot |= PROT_READ;
      }
      if (seg->initprot & VM_PROT_WRITE) {
        prot |= PROT_WRITE;
      }
      if (seg->initprot & VM_PROT_EXECUTE) {
        prot |= PROT_EXEC;
      }

      intptr filesize = alignMem(seg->filesize, 0x1000);
      intptr vmaddr = seg->vmaddr + *slide;
      if (vmaddr < last_addr_) {
        LOG << "will rebase: filename=" << mach.filename()
            << ", vmaddr=" << (void*)vmaddr
            << ", last_addr=" << (void*)last_addr_ << endl;
        CHECK(i == 0);
        vmaddr = last_addr_;
        *slide = vmaddr - seg->vmaddr;
      }
      *base = min(*base, vmaddr);

      intptr vmsize = seg->vmsize;
      LOG << "mmap(file) " << mach.filename() << ' ' << name
          << ": " << (void*)vmaddr << "-" << (void*)(vmaddr + filesize)
          << " offset=" << mach.offset() + seg->fileoff << endl;
      void* mapped = mmap((void*)vmaddr, filesize, prot,
                          MAP_PRIVATE | MAP_FIXED,
                          mach.fd(), mach.offset() + seg->fileoff);
      if (mapped == MAP_FAILED) {
        err(1, "%s mmap(file) failed", mach.filename().c_str());
      }

      if (vmsize != filesize) {
        CHECK(vmsize > filesize);
        LOG << "mmap(anon) " << mach.filename() << ' ' << name
            << ": " << (void*)(vmaddr + filesize) << "-"
            << (void*)(vmaddr + vmsize)
            << endl;
        void* mapped = mmap((void*)(vmaddr + filesize),
                            vmsize - filesize, prot,
                            MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,
                            0, 0);
        if (mapped == MAP_FAILED) {
          err(1, "%s mmap(anon) failed", mach.filename().c_str());
        }
      }

      last_addr_ = max(last_addr_, (intptr)vmaddr + vmsize);
    }
  }

  void doRebase(const MachO& mach, intptr slide) {
    for (size_t i = 0; i < mach.rebases().size(); i++) {
      const MachO::Rebase& rebase = *mach.rebases()[i];
      switch (rebase.type) {
      case REBASE_TYPE_POINTER: {
        char** ptr = (char**)(rebase.vmaddr + slide);
        LOG << "rebase: " << i << ": " << (void*)rebase.vmaddr << ' '
            << (void*)*ptr << " => "
            << (void*)(*ptr + slide) << " @" << ptr << endl;
        *ptr += slide;
        break;
      }

      default:
        fprintf(stderr, "Unknown rebase type: %d\n", rebase.type);
        exit(1);
      }
    }
  }

  void loadInitFuncs(const MachO& mach, intptr slide) {
    for (size_t i = 0; i < mach.init_funcs().size(); i++) {
      intptr addr = mach.init_funcs()[i] + slide;
      LOG << "Registering init func " << (void*)addr
          << " from " << mach.filename() << endl;
      init_funcs_.push_back(addr);
    }
  }

  void loadDylibs(const MachO& mach) {
    for (size_t i = 0; i < mach.dylibs().size(); i++) {
      string dylib = mach.dylibs()[i];

      if (!loaded_dylibs_.insert(dylib).second)
        continue;

      const string so = dylib_to_so_[dylib];
      if (!so.empty()) {
        LOG << "Loading " << so << " for " << dylib << endl;
        if (!dlopen(so.c_str(), RTLD_LAZY | RTLD_GLOBAL)) {
          fprintf(stderr, "Couldn't load %s for %s\n",
                  so.c_str(), dylib.c_str());
        }
      }

      // For now, we assume a dylib is a system library if its path
      // starts with /
      // TODO(hamaji): Do something?
      if (dylib[0] == '/') {
        continue;
      }

      static const char executable_str[] = "@executable_path";
      static const size_t executable_str_len = strlen(executable_str);
      if (!strncmp(dylib.c_str(), executable_str, executable_str_len)) {
        string dir = g_darwin_executable_path;
        size_t found = dir.rfind('/');
        if (found == string::npos) {
          dir = ".";
        } else {
          dir = dir.substr(0, found);
        }
        dylib.replace(0, executable_str_len, dir);
      }

      auto_ptr<MachO> dylib_mach(MachO::read(dylib.c_str(), ARCH_NAME));
      load(*dylib_mach);
    }
  }

  void doBind(const MachO& mach, intptr slide) {
    string last_weak_name = "";
    char* last_weak_sym = NULL;
    vector<pair<string, char*> >::iterator
        seen_weak_bind_iter = seen_weak_binds_.begin(),
        seen_weak_bind_end = seen_weak_binds_.end();
    size_t seen_weak_binds_orig_size = seen_weak_binds_.size();

    unsigned int common_code_size = (unsigned int)trampoline_.size();
    // Ensure that we won't change the address.
    trampoline_.reserve(common_code_size +
                        (1 + 6 + 5 + 10 + 3 + 2 + 1) * mach.binds().size());
    g_bound_names.resize(mach.binds().size());

    for (size_t i = 0; i < mach.binds().size(); i++) {
      MachO::Bind* bind = mach.binds()[i];
      if (bind->name[0] != '_') {
        LOG << bind->name << ": skipping" << endl;
        continue;
      }

      if (bind->type == BIND_TYPE_POINTER) {
        string name = bind->name + 1;
        void** ptr = (void**)(bind->vmaddr + slide);
        char* sym = NULL;

        if (bind->is_weak) {
          if (last_weak_name == name) {
            sym = last_weak_sym;
          } else {
            last_weak_name = name;
            if (seen_weak_bind_iter != seen_weak_bind_end &&
                !strcmp(seen_weak_bind_iter->first.c_str(), name.c_str())) {
              last_weak_sym = sym = seen_weak_bind_iter->second;
              seen_weak_bind_iter++;
            } else {
              last_weak_sym = (char*)*ptr;
              seen_weak_binds_.push_back(make_pair(name, last_weak_sym));
              while (seen_weak_bind_iter != seen_weak_bind_end &&
                     strcmp(seen_weak_bind_iter->first.c_str(),
                            name.c_str()) <= 0) {
                seen_weak_bind_iter++;
              }
              continue;
            }
          }
        } else {
#ifndef __x86_64__
          static const char* SUF_UNIX03 = "$UNIX2003";
          static const size_t SUF_UNIX03_LEN = strlen(SUF_UNIX03);
          if (name.size() > SUF_UNIX03_LEN &&
              !strcmp(name.c_str() + name.size() - SUF_UNIX03_LEN,
                      SUF_UNIX03)) {
            name = name.substr(0, name.size() - SUF_UNIX03_LEN);
          }
#endif

          map<string, string>::const_iterator found =
              g_rename.find(name);
          if (found != g_rename.end()) {
            LOG << "Applying renaming: " << name
                << " => " << found->second.c_str() << endl;
            name = found->second.c_str();
          }

          const Exports::const_iterator export_found =
              exports_.find(bind->name);
          if (export_found != exports_.end()) {
            sym = (char*)export_found->second.addr;
          }
          if (!sym) {
            sym = (char*)dlsym(RTLD_DEFAULT, name.c_str());
            if (!sym) {
              map<string, string>::const_iterator iter =
                symbol_to_so_.find(name);
              if (iter != symbol_to_so_.end()) {
                if (dlopen(iter->second.c_str(), RTLD_LAZY | RTLD_GLOBAL)) {
                  sym = (char*)dlsym(RTLD_DEFAULT, name.c_str());
                } else {
                  fprintf(stderr, "Couldn't load %s for %s\n",
                          iter->second.c_str(), name.c_str());
                }
              }
            }
          }
          if (!sym) {
            ERR << name << ": undefined symbol" << endl;
            sym = (char*)&undefinedFunction;
          }
          sym += bind->addend;
        }

        LOG << "bind " << name << ": "
            << *ptr << " => " << (void*)sym << " @" << ptr << endl;

        if (FLAGS_TRACE_FUNCTIONS && !g_no_trampoline.count(name)) {
          LOG << "Generating trampoline for " << name << "..." << endl;

          *ptr = &trampoline_[0] + trampoline_.size();
          g_bound_names[i] = name;

          // push %rax  ; to adjust alignment for sse
          pushTrampolineCode(0x50);

          // mov $i, %r10d
          pushTrampolineCode(0xba41);
          pushTrampolineCode32((unsigned int)i);

          // call &trampoline_[0]
          pushTrampolineCode(0xe8);
          pushTrampolineCode32((unsigned int)(-4-trampoline_.size()));

          // mov $sym, %r10
          pushTrampolineCode(0xba49);
          pushTrampolineCode64((unsigned long long)(void*)sym);
          // call *%r10
          pushTrampolineCode(0xd2ff41);

          // pop %r10
          pushTrampolineCode(0x5a41);

          // ret
          pushTrampolineCode(0xc3);
        } else {
          *ptr = sym;
        }
      } else {
        fprintf(stderr, "Unknown bind type: %d\n", bind->type);
        abort();
      }
    }

    inplace_merge(seen_weak_binds_.begin(),
                  seen_weak_binds_.begin() + seen_weak_binds_orig_size,
                  seen_weak_binds_.end());
  }

  void loadExports(const MachO& mach, intptr base, Exports* exports) {
    exports->rehash(exports->size() + mach.exports().size());
    for (size_t i = 0; i < mach.exports().size(); i++) {
      MachO::Export exp = *mach.exports()[i];
      exp.addr += base;
      // TODO(hamaji): Not 100% sure, but we may need to consider weak symbols.
      if (!exports->insert(make_pair(exp.name, exp)).second) {
        fprintf(stderr, "duplicated exported symbol: %s\n", exp.name.c_str());
      }
    }
  }

  void loadSymbols(const MachO& mach, intptr slide, intptr base) {
    g_file_map.add(mach, slide, base);
  }

  void load(const MachO& mach, Exports* exports = NULL) {
    if (!exports) {
      exports = &exports_;
    }
    intptr slide = 0;
    intptr base = 0;

    loadSegments(mach, &slide, &base);

    doRebase(mach, slide);

    loadInitFuncs(mach, slide);

    loadDylibs(mach);

    doBind(mach, slide);

    loadExports(mach, base, exports);

    loadSymbols(mach, slide, base);
  }

  void setupDyldData(const MachO& mach) {
    if (!mach.dyld_data())
      return;

    void** dyld_data = (void**)mach.dyld_data();
    dyld_data[1] = reinterpret_cast<void*>(&lookupDyldFunction);
  }

  void runInitFuncs(int argc, char** argv, char** envp, char** apple) {
    for (size_t i = 0; i < init_funcs_.size(); i++) {
      void** init_func = (void**)init_funcs_[i];
      LOG << "calling initializer function " << *init_func << endl;
      if (argc >= 0) {
        ((void(*)(int, char**, char**, char**))*init_func)(
            argc, argv, envp, apple);
      } else {
        ((void(*)())*init_func)();
      }
    }
    init_funcs_.clear();
  }

  void run(MachO& mach, int argc, char** argv, char** envp) {
    // I don't understand what it is.
    char* apple[2];
    apple[0] = argv[0];
    apple[1] = NULL;

    load(mach);
    setupDyldData(mach);

    g_file_map.addWatchDog(last_addr_ + 1);

    char* trampoline_start_addr =
        (char*)(((uintptr_t)&trampoline_[0]) & ~0xfff);
    uint64_t trampoline_size =
        alignMem(&trampoline_[0] + trampoline_.size() - trampoline_start_addr,
                 0x1000);
    mprotect(trampoline_start_addr, trampoline_size,
             PROT_READ | PROT_WRITE | PROT_EXEC);

    g_timer.print(mach.filename().c_str());

    mach.close();

    runInitFuncs(argc, argv, envp, apple);

    LOG << "booting from " << (void*)mach.entry() << "..." << endl;
    fflush(stdout);
    CHECK(argc > 0);
    boot(mach.entry(), argc, argv, envp);
    /*
      int (*fp)(int, char**, char**) =
      (int(*)(int, char**, char**))mach.entry();
      int ret = fp(argc, argv, envp);
      exit(ret);
    */
  }

 private:
  void boot(uint64_t entry, int argc, char** argv, char** envp);

  void pushTrampolineCode(unsigned int c) {
    while (c) {
      trampoline_.push_back(c & 255);
      c = c >> 8;
    }
  }

  void pushTrampolineCode64(unsigned long long c) {
    for (int i = 0; i < 8; i++) {
      trampoline_.push_back(c & 255);
      c = c >> 8;
    }
  }

  void pushTrampolineCode32(unsigned int c) {
    for (int i = 0; i < 4; i++) {
      trampoline_.push_back(c & 255);
      c = c >> 8;
    }
  }

  string trampoline_;
  intptr last_addr_;
  vector<uint64_t> init_funcs_;
  Exports exports_;
  vector<pair<string, char*> > seen_weak_binds_;
  map<string, string> dylib_to_so_;
  map<string, string> symbol_to_so_;
  set<string> loaded_dylibs_;
};

void MachOLoader::boot(
    uint64_t entry, int argc, char** argv, char** envp) {
#ifdef __x86_64__
  // 0x08: argc
  // 0x10: argv[0]
  // 0x18: argv[1]
  //  ...: argv[n]
  //       0
  //       envp[0]
  //       envp[1]
  //       envp[n]
  __asm__ volatile(" mov %1, %%eax;\n"
                   " mov %2, %%rdx;\n"
                   " push $0;\n"
                   // TODO(hamaji): envp
                   " push $0;\n"
                   ".loop64:\n"
                   " sub $8, %%rdx;\n"
                   " push (%%rdx);\n"
                   " dec %%eax;\n"
                   " jnz .loop64;\n"
                   " mov %1, %%eax;\n"
                   " push %%rax;\n"
                   " jmp *%0;\n"
                   ::"r"(entry), "r"(argc), "r"(argv + argc), "r"(envp)
                   :"%rax", "%rdx");
  //fprintf(stderr, "done!\n");
#else
  __asm__ volatile(" mov %1, %%eax;\n"
                   " mov %2, %%edx;\n"
                   " push $0;\n"
                   ".loop32:\n"
                   " sub $4, %%edx;\n"
                   " push (%%edx);\n"
                   " dec %%eax;\n"
                   " jnz .loop32;\n"
                   " mov %1, %%eax;\n"
                   " push %%eax;\n"
                   " jmp *%0;\n"
                   // TODO(hamaji): Fix parameters
                   ::"r"(entry), "r"(argc), "r"(argv + argc), "g"(envp)
                   :"%eax", "%edx");
#endif
}

void runMachO(MachO& mach, int argc, char** argv, char** envp) {
  MachOLoader loader;
  g_loader = &loader;
  loader.run(mach, argc, argv, envp);
  g_loader = NULL;
}

#if 0
static int getBacktrace(void** trace, int max_depth) {
    typedef struct frame {
        struct frame *bp;
        void *ret;
    } frame;

    int depth;
    frame* bp = (frame*)__builtin_frame_address(0);
    for (depth = 0; bp && depth < max_depth; depth++) {
        trace[depth] = bp->ret;
        bp = bp->bp;
    }
    return depth;
}
#endif

extern "C" {
  const char* dumpSymbol(void* p) {
    return g_file_map.dumpSymbol(p);
  }
}

/* signal handler for fatal errors */
static void handleSignal(int signum, siginfo_t* siginfo, void* vuc) {
  ucontext_t *uc = (ucontext_t*)vuc;
  void* pc = (void*)uc->uc_mcontext.gregs[
#ifdef __x86_64__
    REG_RIP
#else
    REG_EIP
#endif
    ];

  fprintf(stderr, "%s(%d) %d (@%p) PC: %p\n\n",
          strsignal(signum), signum, siginfo->si_code, siginfo->si_addr, pc);

  void* trace[100];
  int len = backtrace(trace, 99);
  //int len = getBacktrace(trace, 99);
  char** syms = backtrace_symbols(trace, len);
  for (int i = len - 1; i > 0; i--) {
    if (syms[i] && syms[i][0] != '[') {
      fprintf(stderr, "%s\n", syms[i]);
    } else {
      const char* s = dumpSymbol(trace[i]);
      if (s) {
        fprintf(stderr, "%s\n", s);
      } else {
        fprintf(stderr, "%p\n", trace[i]);
      }
    }
  }
}

/* Generate a stack backtrace when a CPU exception occurs. */
static void initSignalHandler() {
  struct sigaction sigact;
  sigact.sa_flags = SA_SIGINFO | SA_RESETHAND;
  sigact.sa_sigaction = handleSignal;
  sigemptyset(&sigact.sa_mask);
  sigaction(SIGFPE, &sigact, NULL);
  sigaction(SIGILL, &sigact, NULL);
  sigaction(SIGSEGV, &sigact, NULL);
  sigaction(SIGBUS, &sigact, NULL);
  sigaction(SIGABRT, &sigact, NULL);
}

static bool loadLibMac(const char* mypath) {
  if (dlopen("libmac.so", RTLD_LAZY | RTLD_GLOBAL)) {
    return true;
  }

  char buf[PATH_MAX + 100];
  strcpy(buf, mypath);
  char* p = strrchr(buf, '/');
  if (!p) {
    fprintf(stderr, "Weird loader path: %s\n", mypath);
    exit(1);
  }
  strcpy(p, "/libmac.so");

  if (dlopen(buf, RTLD_LAZY | RTLD_GLOBAL)) {
    return true;
  }

  sprintf(p, "/libmac%d.so", BITS);
  if (dlopen(buf, RTLD_LAZY | RTLD_GLOBAL)) {
    return true;
  }

  return false;
}

static void initLibMac() {
  char mypath[PATH_MAX + 1];
  ssize_t l = readlink("/proc/self/exe", mypath, PATH_MAX);
  if (l < 0) {
    err(1, "readlink for /proc/self/exe");
  }
  mypath[l] = '\0';

  if (!loadLibMac(mypath)) {
    fprintf(stderr, "libmac not found\n");
    exit(1);
  }

  int* LIBMAC_LOG = (int*)dlsym(RTLD_DEFAULT, "LIBMAC_LOG");
  if (LIBMAC_LOG) {
    *LIBMAC_LOG = FLAGS_LOG;
  }

  char* loader_path = (char*)dlsym(RTLD_DEFAULT, "__loader_path");
  if (!loader_path) {
    fprintf(stderr, "wrong libmac: __loader_path not found\n");
    exit(1);
  }
  strcpy(loader_path, mypath);
}

static string ld_mac_dlerror_buf;
static bool ld_mac_dlerror_is_set;

static void* ld_mac_dlopen(const char* filename, int flag) {
  LOG << "ld_mac_dlopen: " << filename << " " << flag << endl;

  Timer timer;
  timer.start();

  // TODO(hamaji): Handle failures.
  auto_ptr<MachO> dylib_mach(MachO::read(filename, ARCH_NAME));

  // TODO(hamaji): Consider 32bit.
  MachOLoader* loader = g_loader;
  CHECK(loader);
  Exports* exports = new Exports();
  loader->load(*dylib_mach, exports);

  timer.print(filename);

  loader->runInitFuncs(-1, NULL, NULL, NULL);
  return exports;
}

static int ld_mac_dlclose(void* handle) {
  LOG << "ld_mac_dlclose" << endl;

  delete (Exports*)handle;
  return 0;
}

static const char* ld_mac_dlerror(void) {
  LOG << "ld_mac_dlerror" << endl;

  if (!ld_mac_dlerror_is_set)
    return NULL;
  ld_mac_dlerror_is_set = false;
  return ld_mac_dlerror_buf.c_str();
}

static void* ld_mac_dlsym(void* handle, const char* symbol) {
  LOG << "ld_mac_dlsym: " << symbol << endl;

  Exports* exports = (Exports*)handle;
  Exports::const_iterator found = exports->find(string("_") + symbol);
  if (found == exports->end()) {
    ld_mac_dlerror_is_set = true;
    ld_mac_dlerror_buf = string("undefined symbol: ") + symbol;
    return NULL;
  }
  return (void*)found->second.addr;
}

void initDlfcn() {
#define SET_DLFCN_FUNC(f)                                   \
  do {                                                      \
    void** p = (void**)dlsym(RTLD_DEFAULT, "ld_mac_" #f);   \
    *p = (void*)&ld_mac_ ## f;                              \
  } while (0)

  SET_DLFCN_FUNC(dlopen);
  SET_DLFCN_FUNC(dlclose);
  SET_DLFCN_FUNC(dlerror);
  SET_DLFCN_FUNC(dlsym);
}

int main(int argc, char* argv[], char* envp[]) {
  g_timer.start();
  initSignalHandler();
  initRename();
  initNoTrampoline();
  initLibMac();
  initDlfcn();

  argc--;
  argv++;
  for (;;) {
    if (argc == 0) {
      fprintf(stderr, "An argument required.\n");
      exit(1);
    }

    const char* arg = argv[0];
    if (arg[0] != '-') {
      break;
    }

    // TODO(hamaji): Do something for switches.

    argc--;
    argv++;
  }

  g_darwin_executable_path =
      (char*)dlsym(RTLD_DEFAULT, "__darwin_executable_path");
  if (!realpath(argv[0], g_darwin_executable_path)) {
  }

  auto_ptr<MachO> mach(MachO::read(argv[0], ARCH_NAME));
#ifdef __x86_64__
  if (!mach->is64()) {
    fprintf(stderr, "%s: not 64bit binary\n", argv[0]);
    exit(1);
  }
#else
  if (mach->is64()) {
    fprintf(stderr, "%s: not 32bit binary\n", argv[0]);
    exit(1);
  }
#endif
  runMachO(*mach, argc, argv, envp);
}
