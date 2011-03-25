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
#include <ucontext.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "fat.h"
#include "mach-o.h"

#ifdef NOLOG
# define LOG if (0) cout
//# define LOG cerr
#else
# define LOG cerr
#endif
#define ERR cerr

using namespace std;

class MachO;

static map<string, string> g_rename;
// TODO(hamaji): We might want to control this behavior with a flag.
#ifdef NOLOG
static bool g_use_trampoline = false;
#else
static bool g_use_trampoline = true;
#endif
static vector<const char*> g_bound_names;
static set<string> g_no_trampoline;
// TODO(hamaji): Need one more layer to filter invalid PCs.
static map<uintptr_t, string> g_exported_symbol_map;

// We only support x86-64 for now.
static const char* ARCH_NAME = "x86-64";

static char* g_darwin_executable_path;

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
  if (!g_bound_names[bound_name_id]) {
    fprintf(stderr, "%d: unbound function id\n", bound_name_id);
    return;
  }
  printf("calling %s(%d)\n", g_bound_names[bound_name_id], bound_name_id);
  fflush(stdout);
}

template <bool is64>
struct BitsHelpers {
  typedef uint64_t intptr;
  typedef segment_command_64 mach_segment;

  static const vector<mach_segment*>& segments(const MachO& mach) {
    return mach.segments64();
  }
};

template <>
struct BitsHelpers<false> {
  typedef uint32_t intptr;
  typedef segment_command mach_segment;

  static const vector<mach_segment*>& segments(const MachO& mach) {
    return mach.segments();
  }
};

template <bool is64>
class MachOLoader {
  typedef BitsHelpers<is64> Helpers;
  typedef typename Helpers::intptr intptr;
  typedef typename Helpers::mach_segment Segment;
 public:
  MachOLoader() {
    if (g_use_trampoline) {
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

    const vector<Segment*>& segments = Helpers::segments(mach);
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
        assert(i == 0);
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
        assert(vmsize > filesize);
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
      init_funcs_.push_back(mach.init_funcs()[i] + slide);
    }
  }

  void loadDylibs(const MachO& mach) {
    for (size_t i = 0; i < mach.dylibs().size(); i++) {
      string dylib = mach.dylibs()[i];
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

      auto_ptr<MachO> dylib_mach(readMachO(dylib.c_str(), ARCH_NAME));
      load(*dylib_mach);
    }
  }

  void doBind(const MachO& mach, intptr slide) {
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
        const char* name = bind->name + 1;

        map<string, string>::const_iterator found =
            g_rename.find(name);
        if (found != g_rename.end()) {
          LOG << "Applying renaming: " << name
              << " => " << found->second.c_str() << endl;
          name = found->second.c_str();
        }

        void** ptr = (void**)(bind->vmaddr + slide);
        void* sym = NULL;
        const map<string, MachO::Export>::const_iterator export_found =
            exports_.find(bind->name);
        if (export_found != exports_.end()) {
          sym = (void*)export_found->second.addr;
        }
        if (!sym) {
          sym = dlsym(RTLD_DEFAULT, name);
        }
        if (!sym) {
            ERR << name << ": undefined symbol" << endl;
            sym = (void*)&undefinedFunction;
        }

        LOG << "bind " << name << ": "
            << *ptr << " => " << sym << " @" << ptr << endl;

        if (g_use_trampoline && !g_no_trampoline.count(name)) {
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
  }

  void loadExports(const MachO& mach, intptr base) {
    for (size_t i = 0; i < mach.exports().size(); i++) {
      MachO::Export exp = mach.exports()[i];
      exp.addr += base;
      if (!exports_.insert(make_pair(exp.name, exp)).second) {
        fprintf(stderr, "duplicated exported symbol: %s\n", exp.name.c_str());
      }
      g_exported_symbol_map.insert(make_pair(exp.addr, exp.name.substr(1)));
    }
  }

  void load(const MachO& mach) {
    intptr slide = 0;
    intptr base = 0;

    loadSegments(mach, &slide, &base);

    doRebase(mach, slide);

    loadInitFuncs(mach, slide);

    loadDylibs(mach);

    doBind(mach, slide);

    loadExports(mach, base);
  }

  void run(const MachO& mach, int argc, char** argv, char** envp) {
    load(mach);

    char* trampoline_start_addr =
        (char*)(((uintptr_t)&trampoline_[0]) & ~0xfff);
    uint64_t trampoline_size =
        alignMem(&trampoline_[0] + trampoline_.size() - trampoline_start_addr,
                 0x1000);
    mprotect(trampoline_start_addr, trampoline_size,
             PROT_READ | PROT_WRITE | PROT_EXEC);

    for (size_t i = 0; i < init_funcs_.size(); i++) {
      void** init_func = (void**)init_funcs_[i];
      LOG << "calling initializer function " << *init_func << endl;
      ((void(*)())*init_func)();
    }

    LOG << "booting from " << (void*)mach.entry() << "..." << endl;
    fflush(stdout);
    assert(argc > 0);
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
  map<string, MachO::Export> exports_;
};

template <>
void MachOLoader<true>::boot(
    uint64_t entry, int argc, char** argv, char** envp) {
  __asm__ volatile(" mov %1, %%eax;\n"
                   " mov %2, %%rdx;\n"
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
}

template <>
void MachOLoader<false>::boot(
    uint64_t entry, int argc, char** argv, char** envp) {
  __asm__ volatile(""
                   ::"r"(entry), "r"(argc), "r"(argv), "r"(envp));
}

template <bool is64>
void runMachO(const MachO& mach, int argc, char** argv, char** envp) {
  MachOLoader<is64> loader;
  loader.run(mach, argc, argv, envp);
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

static bool dumpExportedSymbol(void* p) {
  uintptr_t addr = reinterpret_cast<uintptr_t>(p);
  map<uintptr_t, string>::const_iterator found =
      g_exported_symbol_map.lower_bound(addr);
  if (found == g_exported_symbol_map.begin()) {
    return false;
  }

  --found;
  fprintf(stderr, "%s(+%lx) [%p]\n",
          found->second.c_str(), addr - found->first, p);
  return true;
}

/* signal handler for fatal errors */
static void handleSignal(int signum, siginfo_t* siginfo, void* vuc) {
  ucontext_t *uc = (ucontext_t*)vuc;
  void* pc = (void*)uc->uc_mcontext.gregs[REG_RIP];

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
      if (!dumpExportedSymbol(trace[i])) {
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

int main(int argc, char* argv[], char* envp[]) {
  initSignalHandler();
  initRename();
  initNoTrampoline();

  if (!dlopen("libmac.so", RTLD_NOW | RTLD_GLOBAL)) {
    if (!dlopen("libmac/libmac.so", RTLD_NOW | RTLD_GLOBAL)) {
      fprintf(stderr, "libmac not found\n");
      exit(1);
    }
  }

  char* loader_path = (char*)dlsym(RTLD_DEFAULT, "__loader_path");
  realpath(argv[0], loader_path);

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
  realpath(argv[0], g_darwin_executable_path);

  auto_ptr<MachO> mach(readMachO(argv[0], ARCH_NAME));
  if (mach->is64()) {
    runMachO<true>(*mach, argc, argv, envp);
  } else {
    runMachO<false>(*mach, argc, argv, envp);
  }
}
