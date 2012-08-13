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
#include "FatMachO.h"
#include "log.h"
#include "MachO.h"
#include "FileMap.h"
#include "arch.h"

using namespace std;
using namespace std::tr1;

DEFINE_bool(TRACE_FUNCTIONS, false, "Show calling functions");
DEFINE_bool(PRINT_TIME, false, "Print time spent in this loader");

class MachO;

static map<string, string> g_rename;
static vector<string> g_bound_names;
static set<string> g_no_trampoline;

static FileMap g_file_map;

static char* g_darwin_executable_path;

static Timer g_timer;

class MachOLoader;
static MachOLoader* g_loader;

/*
static void initRename() {
#define RENAME(src, dst) g_rename.insert(make_pair(#src, #dst));
#define WRAP(src) RENAME(src, __darwin_ ## src)
#include "rename.tab"
#undef RENAME
#undef WRAP
}
*/







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
	// TODO: this is very wrong
	/*
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
	*/

    
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
  
  // TODO: add 'apple'!
  
  __asm__ volatile(" mov %1, %%rax;\n"
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

/*
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
*/

/*
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
*/

// TODO: OMG
static string ld_mac_dlerror_buf;
static bool ld_mac_dlerror_is_set;

// TODO: broken - should support multiple loads and native libs
void* __darwin_dlopen(const char* filename, int flag) {
  LOG << "ld_mac_dlopen: " << filename << " " << flag << endl;

  // TODO: should return main executable on filename == NULL
  Timer timer;
  timer.start();

  // TODO(hamaji): Handle failures.
  auto_ptr<MachO> dylib_mach(loadDylib(filename));

  MachOLoader* loader = g_loader;
  CHECK(loader);
  Exports* exports = new Exports();
  loader->load(*dylib_mach, exports);

  timer.print(filename);

  loader->runInitFuncs(-1, NULL, NULL, NULL);
  return exports;
}

// TODO: broken - should decrease the reference count
static int __darwin_dlclose(void* handle) {
  LOG << "ld_mac_dlclose" << endl;

  delete (Exports*)handle;
  return 0;
}

static const char* __darwin_dlerror(void) {
  LOG << "ld_mac_dlerror" << endl;

  if (!ld_mac_dlerror_is_set)
    return NULL;
  ld_mac_dlerror_is_set = false;
  return ld_mac_dlerror_buf.c_str();
}

static void* __darwin_dlsym(void* handle, const char* symbol) {
  LOG << "ld_mac_dlsym: " << symbol << endl;
  
  // TODO: support RTLD_DEFAULT

  Exports* exports = (Exports*)handle;
  Exports::const_iterator found = exports->find(string("_") + symbol);
  if (found == exports->end()) {
    ld_mac_dlerror_is_set = true;
    ld_mac_dlerror_buf = string("undefined symbol: ") + symbol;
    return NULL;
  }
  return (void*)found->second.addr;
}

/*
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
*/
