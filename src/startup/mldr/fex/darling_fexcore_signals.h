#ifndef _MLDR_FEX_DARLING_FEXCORE_SIGNALS_H_
#define _MLDR_FEX_DARLING_FEXCORE_SIGNALS_H_

#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/SignalDelegator.h>
#include <FEXCore/Debug/InternalThreadState.h>

#include "Tools/LinuxEmulation/ArchHelpers/MContext.h"

#include "darling_fexcore_sigframe_layout.h"

#include <csignal>
#include <cstddef>
#include <cstdint>

#include "fex_bridge.h"

namespace darling_fex {

// The delegator FEXCore fills with the dispatcher's signal entry points during InitCore. A guest callback
// returns to `thunk_callback_ret`.
FEXCore::SignalDelegator* signal_delegator(uintptr_t thunk_callback_ret);

// Called once InitCore has run.
void signals_init(FEXCore::Context::Context* ctx, bool supports_avx, bool supports_sve256);

// Records `thread` as the emulator thread running on this host thread, after making sure the host thread has
// the signal stack the host handler runs on. Returns false if that stack cannot be set up. `previous` receives
// the thread to put back with signals_leave_thread (callbacks nest runs on one host thread).
bool signals_enter_thread(FEXCore::Core::InternalThreadState* thread, FEXCore::Core::InternalThreadState** previous);
void signals_leave_thread(FEXCore::Core::InternalThreadState* previous);

// What a host handler knows about the host thread it runs on (darling_fexcore_sigstack.cpp).
constexpr size_t kMaxDeferredSignals = 8;
struct DeferredSignal {
  int sig;
  siginfo_t info;
  uint64_t mask;
};
struct HostThreadSignals {
  uint64_t magic;
  FEXCore::Core::InternalThreadState* thread; // null while no emulated code runs on the thread
  size_t deferred_count;
  DeferredSignal deferred[kMaxDeferredSignals];
};

void host_signal_stacks_init();
// The state of the host thread a host handler runs on, or null when the thread has no host signal stack or
// the handler interrupted another one.
HostThreadSignals* host_signals_in_handler(void* host_ucontext);
// The same, outside a signal handler.
HostThreadSignals* host_signals_of_this_thread();


struct FrameRequest {
  int sig;
  const siginfo_t* info;
  void* host_ucontext;
  const fex_guest_stack* altstack;
  int32_t stack_flags;     // the guest's alternate stack state at the signal: SS_DISABLE, SS_ONSTACK or 0
  uint64_t guest_sp;       // where the frame goes (red zone already skipped, alternate stack chosen)
  uint64_t restorer;       // 0 when the action has no SA_RESTORER
  uint64_t old_mask;       // the guest's blocked set when the signal came
  uint32_t eflags;
  bool supports_avx;
  size_t backup_size;      // bytes of saved host context kept in the frame (0 for a handler run at a syscall return)
};

// Writes the frame below req.guest_sp and points the guest's argument registers at it. Returns the stack
// pointer for handler entry (Linux's alignment: 8 modulo 16).
// `original_rip` is where the handler returns to; `fault_to_top` says the signal is a guest exception the
// generated code raised. `backup` (null for a handler run at a syscall return) is req.backup_size bytes of the
// saved host context, which the frame keeps.
uint64_t setup_x64_frame(FEXCore::Context::Context* ctx, FEXCore::Core::InternalThreadState* thread, uint64_t original_rip,
                         bool fault_to_top, const void* backup, const FrameRequest& req);

// The guest alternate stack a running handler of this thread was put on (flags SS_DISABLE when none is).
const fex_guest_stack* handler_altstack();

// Runs the handler `request` names when the current syscall returns: called from the shim's syscall handler
// after the syscall's result is in the thread's state.
void deliver_at_syscall_return(FEXCore::Core::InternalThreadState* thread, const fex_signal_request* request);

// Loads the guest registers rt_sigreturn returns to from the frame's ucontext.
void restore_x64_registers(FEXCore::Context::Context* ctx, FEXCore::Core::InternalThreadState* thread,
                           const FEXCore::x86_64::ucontext_t* guest_uc, bool supports_avx);

} // namespace darling_fex

#endif // _MLDR_FEX_DARLING_FEXCORE_SIGNALS_H_
