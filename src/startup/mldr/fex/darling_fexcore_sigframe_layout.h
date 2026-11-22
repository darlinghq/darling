#ifndef _MLDR_FEX_DARLING_FEXCORE_SIGFRAME_LAYOUT_H_
#define _MLDR_FEX_DARLING_FEXCORE_SIGFRAME_LAYOUT_H_

// Where the x86_64 signal frame goes on the guest stack. Only FEX's x86_64 ucontext definitions are needed,
// so tests/mldr/fex_sigframe_layout.cpp checks this without FEXCore.

#include "Tools/LinuxEmulation/ArchHelpers/UContext.h"

#include <csignal>
#include <cstddef>
#include <cstdint>

namespace darling_fex {

// From the stack pointer at handler entry: return address, ucontext, siginfo, then the host address of the
// saved host context. The fp state sits above it, 64-byte aligned. A handler returns with `ret`, so
// rt_sigreturn finds the ucontext at its stack pointer.
constexpr size_t kFrameUContext = sizeof(uint64_t);
constexpr size_t kFrameSigInfo = kFrameUContext + sizeof(FEXCore::x86_64::ucontext_t);
constexpr size_t kFrameBackup = kFrameSigInfo + sizeof(siginfo_t);
constexpr size_t kFrameSize = kFrameBackup + sizeof(uint64_t);
constexpr size_t kRedZone = 128;

struct X64FrameLayout {
  uint64_t entry_sp; // the return address; Linux's alignment for a function entry, 8 modulo 16
  uint64_t ucontext;
  uint64_t siginfo;
  uint64_t backup;
  uint64_t fpstate;
};

// `sp` is the stack pointer the frame goes below (red zone skipped, alternate stack chosen).
constexpr X64FrameLayout x64_frame_layout(uint64_t sp, bool supports_avx) {
  const uint64_t fp_size = supports_avx ? sizeof(FEXCore::x86_64::xstate) : sizeof(FEXCore::x86_64::_libc_fpstate);
  const uint64_t fpstate = (sp - fp_size) & ~uint64_t(63);
  const uint64_t entry_sp = ((fpstate - kFrameSize) & ~uint64_t(15)) - 8;
  return {entry_sp, entry_sp + kFrameUContext, entry_sp + kFrameSigInfo, entry_sp + kFrameBackup, fpstate};
}

} // namespace darling_fex

#endif // _MLDR_FEX_DARLING_FEXCORE_SIGFRAME_LAYOUT_H_
