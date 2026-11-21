#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/HostFeatures.h>
#include <FEXCore/Core/SignalDelegator.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/HLE/SyscallHandler.h>
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

class DarlingSignalDelegator final : public FEXCore::SignalDelegator {
public:
  ~DarlingSignalDelegator() override = default;
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
static DarlingSyscallHandler* g_syscall_handler = nullptr;

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
