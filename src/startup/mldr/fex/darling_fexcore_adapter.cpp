#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/HostFeatures.h>
#include <FEXCore/Core/SignalDelegator.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/HLE/SyscallHandler.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/ArchHelpers/Arm64.h>
#include <FEXCore/Utils/LogManager.h>

#include "Common/HostFeatures.h"

#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <optional>
#include <sys/mman.h>
#include <ucontext.h>
#include <unistd.h>

#include "fex_bridge.h"

namespace {

thread_local fex_syscall_cb_t t_syscall_cb = nullptr;
thread_local jmp_buf* t_exit_jmp = nullptr;
thread_local FEXCore::Core::InternalThreadState* t_thread_state = nullptr;

// Guest code that FEX's callback entry pushes as the return address of a guest callback: the
// FEX-reserved instruction 0F 3E, which returns from HandleCallback to the host caller.
static uint8_t* g_callback_return_stub = nullptr;

class DarlingSignalDelegator final : public FEXCore::SignalDelegator {
public:
  ~DarlingSignalDelegator() override = default;

  uintptr_t GetThunkCallbackRET() const override {
    return reinterpret_cast<uintptr_t>(g_callback_return_stub);
  }
};

static void fex_state_to_darling(
    FEXCore::Context::Context* ctx,
    FEXCore::Core::InternalThreadState* thread,
    const FEXCore::Core::CPUState& src,
    struct fex_x86_64_state* dst) {
  dst->rax = src.gregs[FEXCore::X86State::REG_RAX];
  dst->rbx = src.gregs[FEXCore::X86State::REG_RBX];
  dst->rcx = src.gregs[FEXCore::X86State::REG_RCX];
  dst->rdx = src.gregs[FEXCore::X86State::REG_RDX];
  dst->rsi = src.gregs[FEXCore::X86State::REG_RSI];
  dst->rdi = src.gregs[FEXCore::X86State::REG_RDI];
  dst->rbp = src.gregs[FEXCore::X86State::REG_RBP];
  dst->rsp = src.gregs[FEXCore::X86State::REG_RSP];
  dst->r8  = src.gregs[FEXCore::X86State::REG_R8];
  dst->r9  = src.gregs[FEXCore::X86State::REG_R9];
  dst->r10 = src.gregs[FEXCore::X86State::REG_R10];
  dst->r11 = src.gregs[FEXCore::X86State::REG_R11];
  dst->r12 = src.gregs[FEXCore::X86State::REG_R12];
  dst->r13 = src.gregs[FEXCore::X86State::REG_R13];
  dst->r14 = src.gregs[FEXCore::X86State::REG_R14];
  dst->r15 = src.gregs[FEXCore::X86State::REG_R15];
  dst->rip = src.rip;
  dst->gs_base = src.gs_cached;
  dst->fs_base = src.fs_cached;
  dst->rflags = ctx->ReconstructCompactedEFLAGS(thread, false, nullptr, 0);
  for (int i = 0; i < 16; i++) {
    dst->xmm[i][0] = src.xmm.sse.data[i][0];
    dst->xmm[i][1] = src.xmm.sse.data[i][1];
  }
}

static void darling_to_fex_state(
    FEXCore::Context::Context* ctx,
    FEXCore::Core::InternalThreadState* thread,
    const struct fex_x86_64_state* src,
    FEXCore::Core::CPUState& dst) {
  dst.gregs[FEXCore::X86State::REG_RAX] = src->rax;
  dst.gregs[FEXCore::X86State::REG_RBX] = src->rbx;
  dst.gregs[FEXCore::X86State::REG_RCX] = src->rcx;
  dst.gregs[FEXCore::X86State::REG_RDX] = src->rdx;
  dst.gregs[FEXCore::X86State::REG_RSI] = src->rsi;
  dst.gregs[FEXCore::X86State::REG_RDI] = src->rdi;
  dst.gregs[FEXCore::X86State::REG_RBP] = src->rbp;
  dst.gregs[FEXCore::X86State::REG_RSP] = src->rsp;
  dst.gregs[FEXCore::X86State::REG_R8]  = src->r8;
  dst.gregs[FEXCore::X86State::REG_R9]  = src->r9;
  dst.gregs[FEXCore::X86State::REG_R10] = src->r10;
  dst.gregs[FEXCore::X86State::REG_R11] = src->r11;
  dst.gregs[FEXCore::X86State::REG_R12] = src->r12;
  dst.gregs[FEXCore::X86State::REG_R13] = src->r13;
  dst.gregs[FEXCore::X86State::REG_R14] = src->r14;
  dst.gregs[FEXCore::X86State::REG_R15] = src->r15;
  dst.rip = src->rip;
  dst.gs_cached = src->gs_base;
  dst.fs_cached = src->fs_base;
  ctx->SetFlagsFromCompactedEFLAGS(thread, static_cast<uint32_t>(src->rflags));
  for (int i = 0; i < 16; i++) {
    dst.xmm.sse.data[i][0] = src->xmm[i][0];
    dst.xmm.sse.data[i][1] = src->xmm[i][1];
  }
}

class DarlingSyscallHandler final : public FEXCore::HLE::SyscallHandler {
public:
  explicit DarlingSyscallHandler(FEXCore::Context::Context* ctx)
    : CTX(ctx) {}

  void HandleSyscall(FEXCore::Core::CpuStateFrame* Frame) override {
    struct fex_x86_64_state guest{};
    fex_state_to_darling(CTX, t_thread_state, Frame->State, &guest);
    uint64_t nr = guest.rax;

    if (t_syscall_cb) {
      t_syscall_cb(&guest);
    }

    Frame->State.gregs[FEXCore::X86State::REG_RAX] = guest.rax;
    Frame->State.gs_cached = guest.gs_base;
    Frame->State.fs_cached = guest.fs_base;
    for (int i = 0; i < 16; i++) {
      Frame->State.xmm.sse.data[i][0] = guest.xmm[i][0];
      Frame->State.xmm.sse.data[i][1] = guest.xmm[i][1];
    }

    if ((nr == DARLING_FEX_CALLBACK_RETURN_SYSCALL || guest.unwind_thread) && t_exit_jmp) {
      longjmp(*t_exit_jmp, 1);
    }

    // FEX's HLE handler contract: step over the 2-byte syscall instruction before
    // re-entering the dispatcher (LinuxSyscalls/Syscalls.cpp:909).
    Frame->State.rip += 2;
  }

  FEXCore::HLE::ExecutableRangeInfo QueryGuestExecutableRange(
      FEXCore::Core::InternalThreadState*, uint64_t) override {
    return {0, UINT64_MAX, true};
  }

  std::optional<FEXCore::ExecutableFileSectionInfo> LookupExecutableFileSection(
      FEXCore::Core::InternalThreadState*, uint64_t) override {
    return std::nullopt;
  }

private:
  FEXCore::Context::Context* CTX;
};

static std::once_flag g_ctx_once;
static fextl::unique_ptr<FEXCore::Context::Context> g_ctx;
static DarlingSignalDelegator* g_delegator = nullptr;
static fextl::vector<FEXCore::Allocator::MemoryRegion> g_reserved_upper_va;
static DarlingSyscallHandler* g_syscall_handler = nullptr;

// One FEX thread per host thread, created on first use and kept until the host thread exits. The guest
// code a host thread runs (its guest thread, and any guest callback a host library makes on it) all
// executes on that one FEX thread, so its block caches stay warm between callbacks. Cost: every host
// thread that ever runs guest code keeps about 256 MB of mostly untouched reserved address space (the
// lookup cache) and a 4 MB call-return stack until it exits.
struct HostThreadEmulator {
  FEXCore::Core::InternalThreadState* thread;
  void* callret_alloc;
  size_t callret_alloc_size;
  // Guest stack for callbacks made while no guest code runs on the host thread; mapped on first use.
  void* callback_stack;
  // FEX keeps the GDT per thread; with a zeroed CPUState segment_arrays stays null and the decoder
  // dereferences null when it reads the CS descriptor.
  FEXCore::Core::CPUState::gdt_segment gdt[32];
};

constexpr size_t kCallbackStackSize = 512 * 1024;

static pthread_key_t g_host_thread_key;
thread_local HostThreadEmulator* t_emulator = nullptr;

static void reset_callret_sp(HostThreadEmulator* emu) {
  // The quarter-in start is FEX's default (LinuxSyscalls/ThreadManager.cpp), leaving room for underflows.
  emu->thread->CurrentFrame->State.callret_sp =
      reinterpret_cast<uint64_t>(emu->thread->CallRetStackBase) + FEXCore::Core::InternalThreadState::CALLRET_STACK_SIZE / 4;
}

static void destroy_host_thread_emulator(void* p) {
  auto* emu = static_cast<HostThreadEmulator*>(p);
  if (!emu) {
    return;
  }
  if (t_thread_state == emu->thread) {
    // Destroying a thread that is still executing guest code would free the code under it.
    std::fprintf(stderr, "[mldr/fex] fatal: host thread exits while running guest code\n");
    std::abort();
  }
  g_ctx->DestroyThread(emu->thread);
  ::munmap(emu->callret_alloc, emu->callret_alloc_size);
  if (emu->callback_stack) {
    ::munmap(emu->callback_stack, kCallbackStackSize);
  }
  delete emu;
  t_emulator = nullptr;
}

static HostThreadEmulator* host_thread_emulator() {
  if (t_emulator) {
    return t_emulator;
  }

  FEXCore::Core::InternalThreadState* thread = g_ctx->CreateThread(nullptr);
  if (!thread) {
    return nullptr;
  }

  // FEX's dispatcher keeps a per-thread call-return stack and generated code writes below
  // State.callret_sp; a zero value faults at -8. Guarded allocation mirrors
  // LinuxSyscalls/ThreadManager.cpp:170-196.
  constexpr size_t kCallRetSize = FEXCore::Core::InternalThreadState::CALLRET_STACK_SIZE;
  const size_t host_page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
  const size_t callret_alloc_size = kCallRetSize + 2 * host_page;
  void* callret_alloc = ::mmap(nullptr, callret_alloc_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (callret_alloc == MAP_FAILED) {
    g_ctx->DestroyThread(thread);
    return nullptr;
  }
  void* callret_base = static_cast<uint8_t*>(callret_alloc) + host_page;
  if (::mprotect(callret_base, kCallRetSize, PROT_READ | PROT_WRITE) != 0) {
    ::munmap(callret_alloc, callret_alloc_size);
    g_ctx->DestroyThread(thread);
    return nullptr;
  }
  thread->CallRetStackBase = callret_base;

  auto* emu = new HostThreadEmulator {thread, callret_alloc, callret_alloc_size, nullptr, {}};

  // Mirrors the default setup in FEX's ThreadManager.cpp for a thread that inherits nothing.
  auto* frame = thread->CurrentFrame;
  frame->State.segment_arrays[FEXCore::Core::CPUState::SEGMENT_ARRAY_INDEX_GDT] = &emu->gdt[0];
  frame->State.segment_arrays[FEXCore::Core::CPUState::SEGMENT_ARRAY_INDEX_LDT] = &emu->gdt[0];
  frame->State.cs_idx = FEXCore::Core::CPUState::DEFAULT_USER_CS << 3;
  auto* cs = FEXCore::Core::CPUState::GetSegmentFromIndex(frame->State, frame->State.cs_idx);
  FEXCore::Core::CPUState::SetGDTBase(cs, 0);
  FEXCore::Core::CPUState::SetGDTLimit(cs, 0xF'FFFFU);
  frame->State.cs_cached = FEXCore::Core::CPUState::CalculateGDTBase(*cs);
  cs->L = 1;
  cs->D = 0;
  // ExecuteThread sets this too; a callback can run before the thread ever executes.
  frame->Pointers.ThunkCallbackRet = reinterpret_cast<uint64_t>(g_callback_return_stub);
  reset_callret_sp(emu);

  if (pthread_setspecific(g_host_thread_key, emu) != 0) {
    destroy_host_thread_emulator(emu);
    return nullptr;
  }
  t_emulator = emu;
  return emu;
}

// x86 allows unaligned locked and ordered accesses. FEX's generated code uses half-barrier atomics
// for them, which fault with BUS_ADRALN on an address that is not naturally aligned. FEXCore patches
// the access in place (HandleUnalignedAccess, with the half-barrier type FEX uses by default). A fault
// that is not in generated code, or that cannot be patched, is delivered as it would have been without
// this handler.
#if defined(__aarch64__)
static struct sigaction g_previous_sigbus;

static void sigbus_handler(int, siginfo_t* info, void* context) {
  auto* uc = static_cast<ucontext_t*>(context);
  const uintptr_t pc = uc->uc_mcontext.pc;
  FEXCore::Core::InternalThreadState* thread = t_thread_state;
  if (thread && info->si_code == BUS_ADRALN && g_ctx && g_ctx->IsAddressInCodeBuffer(thread, pc)) {
    const auto patched = FEXCore::ArchHelpers::Arm64::HandleUnalignedAccess(
        thread, FEXCore::ArchHelpers::Arm64::UnalignedHandlerType::HalfBarrier, pc, reinterpret_cast<uint64_t*>(&uc->uc_mcontext.regs[0]));
    if (patched) {
      uc->uc_mcontext.pc = pc + *patched;
      return;
    }
  }
  sigaction(SIGBUS, &g_previous_sigbus, nullptr);
}

static void install_sigbus_handler() {
  struct sigaction action {};
  action.sa_sigaction = sigbus_handler;
  action.sa_flags = SA_SIGINFO;
  sigemptyset(&action.sa_mask);
  sigaction(SIGBUS, &action, &g_previous_sigbus);
}
#endif

static void init_fex_context() {
  LogMan::Throw::InstallHandler([](const char* msg) {
    std::fprintf(stderr, "[FEXCore/assert] %s\n", msg);
  });
  LogMan::Msg::InstallHandler([](LogMan::DebugLevels, const char* msg) {
    std::fprintf(stderr, "[FEXCore] %s\n", msg);
  });

  // An x86_64 process only has the lower 47 bits of the address space, and Darwin's libraries rely on
  // it (they mask pointers to 47 bits). The host hands out addresses above that, so reserve them.
  const long host_page = sysconf(_SC_PAGESIZE);
  g_reserved_upper_va = FEXCore::Allocator::Setup48BitAllocatorIfExists(host_page > 0 ? host_page : 4096);

  void* ret_stub = ::mmap(nullptr, static_cast<size_t>(host_page), PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ret_stub == MAP_FAILED) {
    std::fprintf(stderr, "[mldr/fex] fatal: cannot map the guest callback return stub\n");
    std::abort();
  }
  g_callback_return_stub = static_cast<uint8_t*>(ret_stub);
  g_callback_return_stub[0] = 0x0f;
  g_callback_return_stub[1] = 0x3e;
  if (::mprotect(ret_stub, static_cast<size_t>(host_page), PROT_READ | PROT_EXEC) != 0) {
    std::fprintf(stderr, "[mldr/fex] fatal: cannot make the guest callback return stub executable\n");
    std::abort();
  }

  if (pthread_key_create(&g_host_thread_key, destroy_host_thread_emulator) != 0) {
    std::fprintf(stderr, "[mldr/fex] fatal: cannot create the per-thread emulator key\n");
    std::abort();
  }

  FEXCore::Config::Initialize();
  FEXCore::Config::Load();
  FEXCore::Config::Set(FEXCore::Config::CONFIG_IS64BIT_MODE, "1");

  // Detect the real host instead of hand-picking flags: the JIT emits instructions
  // for whatever is claimed here.
  FEXCore::HostFeatures features = FEX::FetchHostFeatures(true);

  g_ctx = FEXCore::Context::Context::CreateNewContext(features);
  g_delegator = new DarlingSignalDelegator();
  g_syscall_handler = new DarlingSyscallHandler(g_ctx.get());
  g_ctx->SetSignalDelegator(g_delegator);
  g_ctx->SetSyscallHandler(g_syscall_handler);

  // InitCore installs the SIGSEGV handler the JIT relies on to resolve page faults in
  // generated code; running without it faults in the host handler instead.
  if (!g_ctx->InitCore()) {
    std::fprintf(stderr, "[mldr/fex] fatal: FEXCore::Context::InitCore() failed\n");
    std::abort();
  }

#if defined(__aarch64__)
  install_sigbus_handler();
#endif
}
} // namespace

extern "C" int darling_fexcore_run_thread(
    struct fex_x86_64_state* state,
    fex_syscall_cb_t syscall_cb) {
  if (!state || !syscall_cb) {
    return -1;
  }

  std::call_once(g_ctx_once, init_fex_context);

  FEXCore::Core::InternalThreadState* thread = g_ctx->CreateThread(nullptr);
  if (!thread) {
    return -1;
  }

  // FEX's dispatcher keeps a per-thread call-return stack and generated code writes below
  // State.callret_sp; a zero value faults at -8. Guarded allocation mirrors
  // LinuxSyscalls/ThreadManager.cpp:170-196; the quarter-in start is FEX's default there,
  // leaving room for underflows.
  constexpr size_t kCallRetSize = FEXCore::Core::InternalThreadState::CALLRET_STACK_SIZE;
  const size_t host_page = static_cast<size_t>(sysconf(_SC_PAGESIZE));
  const size_t callret_alloc_size = kCallRetSize + 2 * host_page;
  void* callret_alloc = ::mmap(nullptr, callret_alloc_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (callret_alloc == MAP_FAILED) {
    g_ctx->DestroyThread(thread);
    return -1;
  }
  void* callret_base = static_cast<uint8_t*>(callret_alloc) + host_page;
  if (::mprotect(callret_base, kCallRetSize, PROT_READ | PROT_WRITE) != 0) {
    ::munmap(callret_alloc, callret_alloc_size);
    g_ctx->DestroyThread(thread);
    return -1;
  }
  thread->CallRetStackBase = callret_base;

  // FEX keeps the GDT per thread; with a zeroed CPUState segment_arrays stays null and the
  // decoder dereferences null when it reads the CS descriptor. Mirrors the default setup in
  // FEX's ThreadManager.cpp for a thread that inherits nothing.
  static thread_local FEXCore::Core::CPUState::gdt_segment gdt[32] {};
  auto* frame = thread->CurrentFrame;
  frame->State.segment_arrays[FEXCore::Core::CPUState::SEGMENT_ARRAY_INDEX_GDT] = &gdt[0];
  frame->State.segment_arrays[FEXCore::Core::CPUState::SEGMENT_ARRAY_INDEX_LDT] = &gdt[0];
  frame->State.cs_idx = FEXCore::Core::CPUState::DEFAULT_USER_CS << 3;
  auto* cs = FEXCore::Core::CPUState::GetSegmentFromIndex(frame->State, frame->State.cs_idx);
  FEXCore::Core::CPUState::SetGDTBase(cs, 0);
  FEXCore::Core::CPUState::SetGDTLimit(cs, 0xF'FFFFU);
  frame->State.cs_cached = FEXCore::Core::CPUState::CalculateGDTBase(*cs);
  cs->L = 1;
  cs->D = 0;
  frame->State.callret_sp = reinterpret_cast<uint64_t>(callret_base) + kCallRetSize / 4;

  darling_to_fex_state(g_ctx.get(), thread, state, thread->CurrentFrame->State);

  fex_syscall_cb_t prev_cb = t_syscall_cb;
  jmp_buf* prev_jmp = t_exit_jmp;
  FEXCore::Core::InternalThreadState* prev_thread = t_thread_state;

  jmp_buf exit_env{};
  t_syscall_cb = syscall_cb;
  t_exit_jmp = &exit_env;
  t_thread_state = thread;

  if (setjmp(exit_env) == 0) {
    g_ctx->ExecuteThread(thread);
  }

  fex_state_to_darling(g_ctx.get(), thread, thread->CurrentFrame->State, state);

  t_syscall_cb = prev_cb;
  t_exit_jmp = prev_jmp;
  t_thread_state = prev_thread;

  g_ctx->DestroyThread(thread);
  ::munmap(callret_alloc, callret_alloc_size);
  return static_cast<int>(state->rax);
}

// Runs the guest function at state->rip with the integer arguments in rdi, rsi, rdx, rcx, r8 and r9,
// and returns its rax and rdx. A host thread that is running guest code is inside a host call that guest
// code made, so the callback runs nested on the same FEX thread and guest stack, below the caller's
// frame, as it would natively. Otherwise it runs on the host thread's FEX thread with a stack of its own.
extern "C" int darling_fexcore_run_callback(
    struct fex_x86_64_state* state,
    fex_syscall_cb_t syscall_cb) {
  if (!state || !syscall_cb) {
    return -1;
  }

  std::call_once(g_ctx_once, init_fex_context);

  FEXCore::Core::InternalThreadState* thread = t_thread_state;
  if (!thread) {
    HostThreadEmulator* emu = host_thread_emulator();
    if (!emu) {
      return -1;
    }
    if (!emu->callback_stack) {
      void* stack = ::mmap(nullptr, kCallbackStackSize, PROT_READ | PROT_WRITE,
          MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
      if (stack == MAP_FAILED) {
        return -1;
      }
      // The bottom page is a guard.
      if (::mprotect(stack, static_cast<size_t>(sysconf(_SC_PAGESIZE)), PROT_NONE) != 0) {
        ::munmap(stack, kCallbackStackSize);
        return -1;
      }
      emu->callback_stack = stack;
    }
    thread = emu->thread;
    // The top keeps zeroed slack because a callback that tail-calls a host stub makes the bridge read
    // the stack words above its return address. The callback entry pushes a 16-byte slot holding the
    // return address, so starting 8 below an aligned top gives the callee the alignment a call does.
    uintptr_t sp = reinterpret_cast<uintptr_t>(emu->callback_stack) + kCallbackStackSize - 256;
    sp = (sp & ~uintptr_t(15)) - sizeof(uint64_t);
    thread->CurrentFrame->State.gregs[FEXCore::X86State::REG_RSP] = sp;
    reset_callret_sp(emu);
  } else if (thread->CurrentFrame->InSyscallInfo == 0) {
    // The guest's saved RSP is only current while it is stopped in a syscall, e.g. not from a signal handler.
    std::fprintf(stderr, "[mldr/fex] fatal: guest callback on a thread running guest code outside a host call\n");
    std::abort();
  }

  auto* frame = thread->CurrentFrame;
  auto& cpu = frame->State;

  // When nested, the caller is stopped in its syscall handler and resumes from this state afterwards.
  uint64_t saved_gregs[FEXCore::Core::CPUState::NUM_GPRS];
  std::memcpy(saved_gregs, cpu.gregs, sizeof(saved_gregs));
  const uint64_t saved_rip = cpu.rip;
  const uint64_t saved_callret_sp = cpu.callret_sp;
  const uint64_t saved_in_syscall = frame->InSyscallInfo;
  const auto saved_xmm = cpu.xmm;
  const uint32_t saved_eflags = g_ctx->ReconstructCompactedEFLAGS(thread, false, nullptr, 0);

  cpu.gregs[FEXCore::X86State::REG_RDI] = state->rdi;
  cpu.gregs[FEXCore::X86State::REG_RSI] = state->rsi;
  cpu.gregs[FEXCore::X86State::REG_RDX] = state->rdx;
  cpu.gregs[FEXCore::X86State::REG_RCX] = state->rcx;
  cpu.gregs[FEXCore::X86State::REG_R8] = state->r8;
  cpu.gregs[FEXCore::X86State::REG_R9] = state->r9;
  cpu.gs_cached = state->gs_base;
  frame->InSyscallInfo = 0;

  fex_syscall_cb_t prev_cb = t_syscall_cb;
  jmp_buf* prev_jmp = t_exit_jmp;
  FEXCore::Core::InternalThreadState* prev_thread = t_thread_state;

  jmp_buf exit_env{};
  t_syscall_cb = syscall_cb;
  t_exit_jmp = &exit_env;
  t_thread_state = thread;

  if (setjmp(exit_env) == 0) {
    g_ctx->HandleCallback(thread, state->rip);
  } else {
    // The guest thread is terminating and was unwound out of the callback, so the callback return that
    // balances the entry's count of nested JIT frames never ran.
    frame->SignalHandlerRefCounter--;
  }

  state->rax = cpu.gregs[FEXCore::X86State::REG_RAX];
  state->rdx = cpu.gregs[FEXCore::X86State::REG_RDX];
  state->gs_base = cpu.gs_cached;

  std::memcpy(cpu.gregs, saved_gregs, sizeof(saved_gregs));
  cpu.rip = saved_rip;
  cpu.callret_sp = saved_callret_sp;
  frame->InSyscallInfo = saved_in_syscall;
  cpu.xmm = saved_xmm;
  g_ctx->SetFlagsFromCompactedEFLAGS(thread, saved_eflags);

  t_syscall_cb = prev_cb;
  t_exit_jmp = prev_jmp;
  t_thread_state = prev_thread;
  return 0;
}

extern "C" void darling_fexcore_lock_before_fork(void) {
  if (g_ctx) {
    g_ctx->LockBeforeFork(t_thread_state);
  }
}

extern "C" void darling_fexcore_unlock_after_fork(bool is_child) {
  if (g_ctx) {
    g_ctx->UnlockAfterFork(t_thread_state, is_child);
  }
}
