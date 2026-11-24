// Delivery of host signals to x86_64 guest handlers.
//
// The mechanism is FEX's: the host handler, on the host signal stack, builds an x86_64 rt_sigframe on the
// guest stack and points the host context at the dispatcher, so that returning from the host handler runs
// the guest handler as ordinary emulated code on the interrupted host stack. The interrupted host context
// is kept in that rt_sigframe (FEX keeps it on the host stack, where a handler left by longjmp strands it).
// The guest handler comes back through rt_sigreturn, which traps at the dispatcher's signal return address,
// and that trap puts the saved host context back. Nothing here calls guest code from a signal handler.
//
// Adapted from FEX (commit 8e95a994a, SignalDelegator.cpp), Copyright (c) 2019 Ryan Houdek, MIT License;
// see THIRD-PARTY-NOTICES.md.

#include "darling_fexcore_signals.h"
#include "fex_callback_frames.h"
#include "fex_signal_checks.h"

#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Utils/ArchHelpers/Arm64.h>
#include <FEXCore/Utils/MathUtils.h>

#include <cerrno>
#include <cstdio>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <sys/mman.h>
#include <sys/random.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <ucontext.h>
#include <unistd.h>

namespace HostCtx = FEX::ArchHelpers::Context;
namespace X64 = FEXCore::x86_64;

namespace darling_fex {
namespace {

void note_handler_return(uint64_t guest_sp);

class DarlingSignalDelegator final : public FEXCore::SignalDelegator {
public:
  // Guest code FEX's callback entry pushes as the return address of a guest callback.
  uintptr_t GetThunkCallbackRET() const override {
    return thunk_callback_ret;
  }
  uintptr_t thunk_callback_ret = 0;
};

DarlingSignalDelegator g_delegator;
FEXCore::Context::Context* g_ctx = nullptr;
bool g_supports_avx = false;
bool g_supports_sve256 = false;

// The interrupted host context, kept in the guest's signal frame while a guest handler runs. A guest that
// leaves the handler with longjmp never runs rt_sigreturn, and the frame goes with the guest stack pointer.
struct SignalBackup {
  HostCtx::ContextBackup host;
  uint64_t cookie; // g_backup_cookie while the context is live; the guest cannot know it
  int saved_errno;
};
uint64_t g_backup_cookie = 0;
size_t g_host_page_size = 0;
// FEX's REG_CALLRET_SP (Arm64Emitter.h).
constexpr uint32_t kCallRetSpReg = 25;

HostThreadSignals* current_host_signals(void* ucontext) {
  return g_ctx ? host_signals_in_handler(ucontext) : nullptr;
}

uint64_t get_host_mask(void* ucontext) {
  uint64_t mask;
  memcpy(&mask, &HostCtx::GetUContext(ucontext)->uc_sigmask, sizeof(mask));
  return mask;
}

void set_host_mask(void* ucontext, uint64_t mask) {
  mask &= ~(FEX_JIT_SIGNALS | FEX_SIGBIT(SIGKILL) | FEX_SIGBIT(SIGSTOP));
  memcpy(&HostCtx::GetUContext(ucontext)->uc_sigmask, &mask, sizeof(mask));
}

// A signal raised by the kernel for the instruction being executed must be handled now; anything else may
// wait until the emulator leaves a section where it cannot be interrupted.
bool is_async_signal(const siginfo_t* info, int sig) {
  if (info->si_code <= SI_USER) {
    return true;
  }
  return sig != SIGBUS && sig != SIGFPE && sig != SIGILL && sig != SIGSEGV && sig != SIGTRAP;
}

void spill_static_registers(FEXCore::Core::InternalThreadState* thread, void* ucontext, uint32_t ignore_mask) {
  auto* frame = thread->CurrentFrame;
  const auto& config = g_delegator.GetConfig();
  frame->State.rip = g_ctx->RestoreRIPFromHostPC(thread, HostCtx::GetPc(ucontext));

  for (size_t i = 0; i < config.SRAGPRCount; i++) {
    const uint8_t host_reg = config.SRAGPRMapping[i];
    if (!(ignore_mask & (1U << host_reg))) {
      frame->State.gregs[i] = HostCtx::GetArmReg(ucontext, host_reg);
    }
  }
  for (size_t i = 0; i < config.SRAFPRCount; i++) {
    const __uint128_t fpr = HostCtx::GetArmFPR(ucontext, config.SRAFPRMapping[i]);
    void* dst = (g_supports_avx && g_supports_sve256) ? static_cast<void*>(&frame->State.xmm.avx.data[i][0])
                                                      : static_cast<void*>(&frame->State.xmm.sse.data[i][0]);
    memcpy(dst, &fpr, sizeof(fpr));
  }
  const uint32_t eflags = g_ctx->ReconstructCompactedEFLAGS(thread, true, HostCtx::GetArmGPRs(ucontext), HostCtx::GetArmPState(ucontext));
  g_ctx->SetFlagsFromCompactedEFLAGS(thread, eflags);
}

void resume_in_dispatcher(FEXCore::Core::CpuStateFrame* frame, void* ucontext) {
  HostCtx::SetPc(ucontext, g_delegator.GetConfig().AbsoluteLoopTopAddressFillSRA);
  HostCtx::SetFillSRASingleInst(ucontext, false);
  HostCtx::SetState(ucontext, reinterpret_cast<uint64_t>(frame));
}

// Reads guest memory without faulting in the caller: a frame address comes from guest registers.
bool read_guest(void* dst, uint64_t guest_addr, size_t len) {
  struct iovec local = {dst, len};
  struct iovec remote = {reinterpret_cast<void*>(guest_addr), len};
  return process_vm_readv(getpid(), &local, 1, &remote, 1, 0) == static_cast<ssize_t>(len);
}

// The same for writing: false when any of the bytes cannot be written.
bool write_guest(uint64_t guest_addr, const void* src, size_t len) {
  struct iovec local = {const_cast<void*>(src), len};
  struct iovec remote = {reinterpret_cast<void*>(guest_addr), len};
  return process_vm_writev(getpid(), &local, 1, &remote, 1, 0) == static_cast<ssize_t>(len);
}

// Linux's answer to a signal frame that does not fit the alternate stack: the process dies by SIGSEGV.
[[noreturn]] void die_without_frame() {
  static const char kWhy[] = "[mldr/fex] fatal: the signal frame does not fit the alternate stack\n";
  (void)!write(STDERR_FILENO, kWhy, sizeof(kWhy) - 1);
  struct sigaction dfl {};
  dfl.sa_handler = SIG_DFL;
  sigaction(SIGSEGV, &dfl, nullptr);
  sigset_t segv;
  sigemptyset(&segv);
  sigaddset(&segv, SIGSEGV);
  pthread_sigmask(SIG_UNBLOCK, &segv, nullptr);
  syscall(SYS_tgkill, getpid(), gettid(), SIGSEGV);
  abort();
}

// rt_sigreturn of a handler that interrupted emulated code: the trap at the dispatcher's return address
// brings the thread here, in a host handler; put the interrupted host context back.
bool restore_interrupted_frame(FEXCore::Core::InternalThreadState* thread, HostThreadSignals* hs, void* ucontext, uint64_t* guest_mask) {
  auto* frame = thread->CurrentFrame;
  const uint64_t guest_sp = frame->State.gregs[FEXCore::X86State::REG_RSP];
  // darling_fexcore_signal_return read this frame with read_guest before it trapped.
  const auto* guest_uc = reinterpret_cast<const X64::ucontext_t*>(guest_sp);

  // The frame sits in memory the guest writes: take a copy first, and accept only the context this process
  // saved for this very frame, from a host stack pointer the thread can be running below.
  const uint64_t frame_at = guest_sp - kFrameUContext;
  const uint64_t data_at = backup_data_at(frame_at);
  uint64_t slot = 0;
  SignalBackup saved;
  memset(&saved, 0, sizeof(saved));
  const bool readable = read_guest(&slot, frame_at + kFrameBackup, sizeof(slot)) && read_guest(&saved, data_at, sizeof(saved));
  if (fex_backup_check(readable, slot, data_at, saved.cookie, g_backup_cookie, saved.host.UContextLocation, guest_sp,
                       saved.host.PrevSP, HostCtx::GetSp(ucontext)) != FEX_BACKUP_OK) {
    return false;
  }
  // A frame is used once.
  const uint64_t used = 0;
  if (!write_guest(data_at + (reinterpret_cast<uintptr_t>(&saved.cookie) - reinterpret_cast<uintptr_t>(&saved)), &used, sizeof(used))) {
    return false;
  }
  SignalBackup* backup = &saved;

  HostCtx::RestoreContext(ucontext, &backup->host);
  memcpy(&frame->State, &backup->host.GuestState, sizeof(frame->State));
  frame->InSyscallInfo = backup->host.InSyscallInfo;
  *guest_mask = guest_uc->uc_sigmask.val[0];
  set_host_mask(ucontext, *guest_mask);

  // The handler may have moved the thread elsewhere (or the signal came from a fault that does not resume):
  // continue from the frame's registers at the top of the dispatcher.
  if (backup->host.OriginalRIP != guest_uc->uc_mcontext.gregs[X64::FEX_REG_RIP] || backup->host.FaultToTopAndGeneratedException) {
    resume_in_dispatcher(frame, ucontext);
    restore_x64_registers(g_ctx, thread, guest_uc, g_supports_avx);
  }

  --frame->SignalHandlerRefCounter;
  note_handler_return(frame->State.gregs[FEXCore::X86State::REG_RSP]);
  if (hs->deferred_count != 0) {
    mprotect(thread->InterruptFaultPage, sizeof(thread->InterruptFaultPage), PROT_NONE);
  }
  errno = backup->saved_errno;
  return true;
}

void defer_signal(FEXCore::Core::InternalThreadState* thread, HostThreadSignals* hs, int sig, const siginfo_t* info, void* ucontext) {
  if (hs->deferred_count == kMaxDeferredSignals) {
    // Leave it pending on the kernel, blocked until the deferred ones have run. Queueing with the original
    // siginfo is refused on a thread other than the main one; then only the signal number survives.
    if (syscall(SYS_rt_tgsigqueueinfo, getpid(), gettid(), sig, info) != 0 && syscall(SYS_tgkill, getpid(), gettid(), sig) != 0) {
      abort();
    }
  } else {
    hs->deferred[hs->deferred_count++] = DeferredSignal {sig, *info, get_host_mask(ucontext)};
  }
  // Nothing else may arrive before the deferred signal is taken, at the end of the section.
  set_host_mask(ucontext, ~0ULL);
  mprotect(thread->InterruptFaultPage, sizeof(thread->InterruptFaultPage), PROT_NONE);
}

// The guest alternate stack a running handler of this thread was put on (SS_DISABLE when none is).
thread_local fex_guest_stack t_handler_altstack = {0, FEX_GUEST_SS_DISABLE, 0, 0};

// rt_sigreturn put back `guest_sp`: off the alternate stack, no handler of this thread runs on it any more.
void note_handler_return(uint64_t guest_sp) {
  if (!fex_on_guest_stack(&t_handler_altstack, guest_sp)) {
    t_handler_altstack = {0, FEX_GUEST_SS_DISABLE, 0, 0};
  }
}

// Linux's get_sigframe: skip the red zone, switch to the alternate stack if asked and not already on it.
uint64_t signal_stack_pointer(uint64_t guest_sp, const fex_guest_sigaction* action, const fex_guest_stack* altstack, int32_t* stack_flags) {
  guest_sp -= kRedZone;
  const bool enabled = !(altstack->flags & FEX_GUEST_SS_DISABLE) && altstack->size != 0;
  const bool on_altstack = enabled && guest_sp > altstack->sp && guest_sp - altstack->sp <= altstack->size;
  *stack_flags = !enabled ? FEX_GUEST_SS_DISABLE : on_altstack ? FEX_GUEST_SS_ONSTACK : 0;
  if ((action->flags & FEX_GUEST_SA_ONSTACK) && enabled && !on_altstack) {
    t_handler_altstack = *altstack;
    return altstack->sp + altstack->size;
  }
  return guest_sp;
}

// The registers a handler starts with, past the frame setup_x64_frame wrote.
void enter_handler(FEXCore::Core::CPUState& state, uint64_t handler, uint64_t guest_sp) {
  state.rip = handler;
  state.gregs[FEXCore::X86State::REG_RSP] = guest_sp;
  // Linux clears DF, RF and TF and resets CS and SS for the handler.
  state.flags[FEXCore::X86State::RFLAG_DF_RAW_LOC] = 1;
  state.flags[FEXCore::X86State::RFLAG_RF_LOC] = 0;
  state.flags[FEXCore::X86State::RFLAG_TF_RAW_LOC] = 0;
  state.cs_idx = FEXCore::Core::CPUState::DEFAULT_USER_CS << 3;
  state.ss_idx = 0;
  state.cs_cached = state.CalculateGDTBase(*state.GetSegmentFromIndex(state, state.cs_idx));
  state.ss_cached = state.CalculateGDTBase(*state.GetSegmentFromIndex(state, state.ss_idx));
  // The handler starts with a clean FPU and the default MXCSR; rt_sigreturn restores the interrupted ones.
  memset(state.xmm.avx.data, 0, sizeof(state.xmm));
  memset(state.mm, 0, sizeof(state.mm));
  state.FCW = 0x37F;
  state.AbridgedFTW = 0;
  state.mxcsr = 0x1F80;
}

} // namespace

const fex_guest_stack* handler_altstack() {
  return &t_handler_altstack;
}

FEXCore::SignalDelegator* signal_delegator(uintptr_t thunk_callback_ret) {
  g_delegator.thunk_callback_ret = thunk_callback_ret;
  return &g_delegator;
}

void signals_init(FEXCore::Context::Context* ctx, bool supports_avx, bool supports_sve256) {
  g_supports_avx = supports_avx;
  g_supports_sve256 = supports_sve256;
  g_ctx = ctx;
  g_host_page_size = static_cast<size_t>(sysconf(_SC_PAGESIZE));
  // Without a secret the saved contexts in guest frames could be forged: no signal may be delivered then.
  do {
    if (getrandom(&g_backup_cookie, sizeof(g_backup_cookie), 0) != static_cast<ssize_t>(sizeof(g_backup_cookie))) {
      fprintf(stderr, "[mldr/fex] fatal: no random numbers for the signal frame cookie\n");
      abort();
    }
  } while (g_backup_cookie == 0);
}

void deliver_at_syscall_return(FEXCore::Core::InternalThreadState* thread, const fex_signal_request* request) {
  auto* frame = thread->CurrentFrame;
  siginfo_t info;
  static_assert(sizeof(info) == sizeof(request->info));
  memcpy(&info, request->info, sizeof(info));

  int32_t stack_flags = 0;
  const uint64_t sp = signal_stack_pointer(frame->State.gregs[FEXCore::X86State::REG_RSP], &request->action, &request->altstack, &stack_flags);
  const FrameRequest frame_request {
    .sig = request->sig,
    .info = &info,
    .host_ucontext = nullptr,
    .altstack = &request->altstack,
    .stack_flags = stack_flags,
    .guest_sp = sp,
    .restorer = (request->action.flags & FEX_GUEST_SA_RESTORER) ? request->action.restorer : 0,
    .old_mask = request->old_mask,
    .eflags = g_ctx->ReconstructCompactedEFLAGS(thread, false, nullptr, 0),
    .supports_avx = g_supports_avx,
    .backup_size = 0,
  };
  if (!fex_frame_fits(x64_frame_layout(sp, g_supports_avx).entry_sp, sp, &request->altstack)) {
    die_without_frame();
  }
  // No host context to keep: the frame's registers are all rt_sigreturn needs (a null backup address).
  const uint64_t entry_sp = setup_x64_frame(g_ctx, thread, frame->State.rip, false, nullptr, frame_request);
  enter_handler(frame->State, request->action.handler, entry_sp);
}

} // namespace darling_fex

using namespace darling_fex;

extern "C" int darling_fexcore_signal_preempt(int* sig, void* raw_info, void* ucontext, uint64_t* guest_mask) {
  auto* info = static_cast<siginfo_t*>(raw_info);
  HostThreadSignals* hs = current_host_signals(ucontext);
  FEXCore::Core::InternalThreadState* thread = hs ? hs->thread : nullptr;
  if (!thread) {
    return FEX_SIGNAL_NO_GUEST_THREAD;
  }
  auto* frame = thread->CurrentFrame;
  const auto& config = g_delegator.GetConfig();
  const uint64_t pc = HostCtx::GetPc(ucontext);

  if (*sig == SIGILL && (pc == config.SignalHandlerReturnAddressRT || pc == config.SignalHandlerReturnAddress)) {
    if (restore_interrupted_frame(thread, hs, ucontext, guest_mask)) {
      return FEX_SIGNAL_RETURNED;
    }
    // An rt_sigreturn without a frame from us: the kernel answers that with SIGSEGV, and a handler that
    // returns comes back after the syscall instruction.
    frame->State.rip += 2;
    resume_in_dispatcher(frame, ucontext);
    memset(info, 0, sizeof(*info));
    info->si_signo = SIGSEGV;
    info->si_code = SI_KERNEL;
    *sig = SIGSEGV;
    return FEX_SIGNAL_FOR_GUEST;
  }

  // The call-return stack pointer ran into a guard page, as calls the guest left by longjmp pile up. The
  // entries are hints for the ret target: start again at the default place and run the access again. This is
  // FEX's answer too (SyscallsSMCTracking.cpp, HandleSegfault).
  const uint64_t callret_base = reinterpret_cast<uintptr_t>(thread->CallRetStackBase);
  const uint64_t callret_size = FEXCore::Core::InternalThreadState::CALLRET_STACK_SIZE;
  const bool in_emulator = g_ctx->IsAddressInCodeBuffer(thread, pc) || (pc >= config.DispatcherBegin && pc < config.DispatcherEnd);
  if (*sig == SIGSEGV && info->si_code == SEGV_ACCERR && in_emulator &&
      fex_callret_fault(reinterpret_cast<uintptr_t>(info->si_addr), callret_base, callret_size, g_host_page_size)) {
    HostCtx::SetArmReg(ucontext, kCallRetSpReg, fex_callret_start(callret_base, callret_size));
    return FEX_SIGNAL_CONSUMED;
  }

  // x86 allows unaligned locked and ordered accesses; the generated half-barrier atomics fault on them, and
  // FEXCore patches the access in place.
  if (*sig == SIGBUS && info->si_code == BUS_ADRALN && g_ctx->IsAddressInCodeBuffer(thread, pc)) {
    const auto patched = FEXCore::ArchHelpers::Arm64::HandleUnalignedAccess(
      thread, FEXCore::ArchHelpers::Arm64::UnalignedHandlerType::HalfBarrier, pc, HostCtx::GetArmGPRs(ucontext));
    if (patched) {
      HostCtx::SetPc(ucontext, pc + *patched);
      return FEX_SIGNAL_CONSUMED;
    }
    return FEX_SIGNAL_FOR_GUEST;
  }

  // The end of a section that deferred a signal touches this page.
  if (*sig == SIGSEGV && info->si_code == SEGV_ACCERR && info->si_addr == thread->InterruptFaultPage) {
    if (frame->State.DeferredSignalRefCount.Load() != 0) {
      // A nested section ended; the outermost one checks again. Skip the store.
      HostCtx::SetPc(ucontext, pc + 4);
      return FEX_SIGNAL_CONSUMED;
    }
    mprotect(thread->InterruptFaultPage, sizeof(thread->InterruptFaultPage), PROT_READ | PROT_WRITE);
    if (hs->deferred_count == 0) {
      return FEX_SIGNAL_CONSUMED;
    }
    const DeferredSignal& next = hs->deferred[--hs->deferred_count];
    *sig = next.sig;
    *info = next.info;
    set_host_mask(ucontext, next.mask);
    if (hs->deferred_count != 0) {
      // The rest are taken at the end of the next section, or when this handler returns.
      mprotect(thread->InterruptFaultPage, sizeof(thread->InterruptFaultPage), PROT_NONE);
    }
    // No lock is held here: FEX releases its locks before it ends a section.
    return FEX_SIGNAL_FOR_GUEST;
  }

  const bool async = is_async_signal(info, *sig);
  if (async && frame->State.DeferredSignalRefCount.Load() != 0) {
    defer_signal(thread, hs, *sig, info, ucontext);
    return FEX_SIGNAL_CONSUMED;
  }

  if (!async) {
    // A guest exception raised by the generated code (ud2, int3, ...) traps here with the guest's view of
    // it recorded in the frame. A fault cannot wait, wherever it happened.
    if (frame->SynchronousFaultData.FaultToTopAndGeneratedException) {
      const auto& fault = frame->SynchronousFaultData;
      *sig = fault.Signal;
      info->si_signo = fault.Signal;
      info->si_code = fault.si_code;
      info->si_addr = reinterpret_cast<void*>(fault.FaultAddress ? fault.FaultAddress : frame->State.rip);
    }
    return FEX_SIGNAL_FOR_GUEST;
  }

  // Inside a syscall (InSyscallInfo is set for the whole of the dispatcher's syscall call) the thread is in
  // host code that may hold host locks (malloc in a host library, a mutex in mldr) a guest handler would need
  // to compile its blocks: the handler runs when that syscall returns. Elsewhere the thread is in generated
  // code, the dispatcher, or a FEXCore helper called from generated code (x87 arithmetic, for one), and takes
  // it now as FEX's own frontend does: FEXCore holds its locks only inside the sections that defer signals
  // (ScopedDeferredSignalWithMutex), deferred above.
  const bool emulating = in_emulator;
  if (!emulating && frame->InSyscallInfo != 0) {
    return FEX_SIGNAL_AT_SYSCALL_RETURN;
  }
  return FEX_SIGNAL_FOR_GUEST;
}

extern "C" int darling_fexcore_signal_deliver(int sig, const void* raw_info, void* ucontext, const fex_guest_sigaction* action,
                                              const fex_guest_stack* altstack, uint64_t old_mask) {
  const auto* info = static_cast<const siginfo_t*>(raw_info);
  HostThreadSignals* hs = current_host_signals(ucontext);
  FEXCore::Core::InternalThreadState* thread = hs ? hs->thread : nullptr;
  if (!thread) {
    return FEX_SIGNAL_NO_GUEST_THREAD;
  }
  auto* frame = thread->CurrentFrame;

  // The host handler runs on the signal stack, so the interrupted stack below its pointer is free for the
  // dispatcher that runs the guest handler. The interrupted context goes in the guest's signal frame, so
  // that a handler left by longjmp does not strand it on the host stack.
  SignalBackup backup;
  HostCtx::BackupContext(ucontext, &backup.host);
  backup.cookie = g_backup_cookie;
  backup.saved_errno = errno;
  backup.host.Signal = sig;
  backup.host.Flags = 0;
  memcpy(&backup.host.GuestState, &frame->State, sizeof(frame->State));
  backup.host.FaultToTopAndGeneratedException = frame->SynchronousFaultData.FaultToTopAndGeneratedException;
  frame->SynchronousFaultData.FaultToTopAndGeneratedException = false;
  HostCtx::SetSp(ucontext, FEXCore::AlignDown(HostCtx::GetSp(ucontext), 16));
  ++frame->SignalHandlerRefCounter;

  if (g_ctx->IsAddressInCodeBuffer(thread, backup.host.PrevPC)) {
    // Interrupted in generated code: the guest registers are partly in host registers.
    spill_static_registers(thread, ucontext, frame->InSyscallInfo & 0xFFFF);
    backup.host.Flags |= HostCtx::ContextFlags::CONTEXT_FLAG_INJIT;
  }
  // The handler runs as plain generated code: a signal nested in it must spill every static register.
  backup.host.InSyscallInfo = frame->InSyscallInfo;
  frame->InSyscallInfo = 0;

  int32_t stack_flags = 0;
  const uint64_t guest_sp = signal_stack_pointer(frame->State.gregs[FEXCore::X86State::REG_RSP], action, altstack, &stack_flags);
  backup.host.OriginalRIP = frame->State.rip;
  const X64FrameLayout layout = x64_frame_layout(guest_sp, g_supports_avx, sizeof(backup));
  if (!fex_frame_fits(layout.entry_sp, guest_sp, altstack)) {
    die_without_frame();
  }
  backup.host.UContextLocation = layout.ucontext;
  const FrameRequest request {
    .sig = sig,
    .info = info,
    .host_ucontext = ucontext,
    .altstack = altstack,
    .stack_flags = stack_flags,
    .guest_sp = guest_sp,
    .restorer = (action->flags & FEX_GUEST_SA_RESTORER) ? action->restorer : 0,
    .old_mask = old_mask,
    .eflags = g_ctx->ReconstructCompactedEFLAGS(thread, false, nullptr, 0),
    .supports_avx = g_supports_avx,
    .backup_size = sizeof(backup),
  };
  const uint64_t entry_sp =
    setup_x64_frame(g_ctx, thread, backup.host.OriginalRIP, backup.host.FaultToTopAndGeneratedException, &backup, request);
  enter_handler(frame->State, action->handler, entry_sp);

  resume_in_dispatcher(frame, ucontext);
  return FEX_SIGNAL_DELIVERED;
}

// rt_sigreturn, called from the syscall dispatch (ordinary context, inside the shim's syscall handler).
extern "C" int darling_fexcore_signal_return(uint64_t* guest_mask) {
  HostThreadSignals* hs = host_signals_of_this_thread();
  FEXCore::Core::InternalThreadState* thread = hs ? hs->thread : nullptr;
  if (!thread) {
    return -EFAULT;
  }
  auto& state = thread->CurrentFrame->State;
  const uint64_t uc_at = state.gregs[FEXCore::X86State::REG_RSP];
  X64::ucontext_t uc;
  uint64_t backup = 0;
  if (!read_guest(&uc, uc_at, sizeof(uc)) || !read_guest(&backup, uc_at - kFrameUContext + kFrameBackup, sizeof(backup))) {
    return -EFAULT;
  }
  if (backup != 0) {
    // The handler interrupted emulated code: the trap's host handler puts that context back.
    using ReturnTrap = void (*)();
    reinterpret_cast<ReturnTrap>(g_delegator.GetConfig().SignalHandlerReturnAddressRT)();
    __builtin_unreachable();
  }

  // A handler that ran at a syscall return: its frame's registers are the whole state to go back to.
  X64::xstate xstate;
  const uint64_t fp_at = reinterpret_cast<uint64_t>(uc.uc_mcontext.fpregs);
  if (fp_at) {
    memset(&xstate, 0, sizeof(xstate));
    if (!read_guest(&xstate, fp_at, g_supports_avx ? sizeof(X64::xstate) : sizeof(X64::_libc_fpstate))) {
      return -EFAULT;
    }
    uc.uc_mcontext.fpregs = &xstate.fpstate;
  }
  restore_x64_registers(g_ctx, thread, &uc, g_supports_avx);
  note_handler_return(state.gregs[FEXCore::X86State::REG_RSP]);
  *guest_mask = uc.uc_sigmask.val[0];
  return 1;
}
