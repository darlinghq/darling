// The x86_64 signal frame the FEXCore shim builds, against the Linux x86_64 ABI a guest handler relies on.
// Needs FEX's source tree for its x86_64 ucontext definitions (header only), not FEXCore itself.
#include "fex/darling_fexcore_sigframe_layout.h"

#include <cstddef>
#include <cstdio>

namespace X64 = FEXCore::x86_64;
using namespace darling_fex;

static int failures = 0;

static void check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) {
    failures++;
  }
}

int main() {
  // struct ucontext as the x86_64 kernel writes it up to uc_sigmask (uapi asm/ucontext.h, asm/sigcontext.h):
  // uc_flags, uc_link, a 24-byte stack_t, then a 256-byte sigcontext with r8..r15, rdi, rsi, rbp, rbx, rdx,
  // rax, rcx, rsp, rip, eflags, cs/gs/fs/ss, err, trapno, oldmask, cr2 (glibc's REG_* order), fpstate.
  check(offsetof(X64::ucontext_t, uc_link) == 8 && offsetof(X64::ucontext_t, uc_stack) == 16, "ucontext header");
  check(sizeof(X64::stack_t) == 24 && offsetof(X64::stack_t, ss_flags) == 8 && offsetof(X64::stack_t, ss_size) == 16,
        "stack_t: ss_sp, ss_flags, ss_size");
  check(offsetof(X64::ucontext_t, uc_mcontext) == 40, "uc_mcontext at 40");
  check(X64::FEX_REG_R8 == 0 && X64::FEX_REG_RDI == 8 && X64::FEX_REG_RAX == 13 && X64::FEX_REG_RSP == 15 &&
          X64::FEX_REG_RIP == 16 && X64::FEX_REG_EFL == 17 && X64::FEX_REG_CSGSFS == 18 && X64::FEX_REG_TRAPNO == 20 &&
          X64::FEX_REG_CR2 == 22,
        "gregs in the kernel's sigcontext order");
  check(offsetof(X64::mcontext_t, fpregs) == 23 * 8, "fpstate pointer after the 23 gregs");
  check(offsetof(X64::ucontext_t, uc_sigmask) == 40 + 256, "uc_sigmask right after the 256-byte sigcontext");

  // The fxsave layout of the fp state, and the xstate extension marker.
  check(sizeof(X64::_libc_fpstate) == 512, "fxsave area is 512 bytes");
  check(offsetof(X64::_libc_fpstate, mxcsr) == 24 && offsetof(X64::_libc_fpstate, _st) == 32 &&
          offsetof(X64::_libc_fpstate, _xmm) == 160 && offsetof(X64::_libc_fpstate, sw_reserved) == 464,
        "mxcsr, st, xmm and the software bytes at their fxsave offsets");
  check(sizeof(X64::xstate) == 840 && offsetof(X64::xstate, xstate_hdr) == 512 && offsetof(X64::xstate, ymmh) == 576,
        "xstate: header at 512, ymm high halves at 576");

  check(sizeof(siginfo_t) == 128, "siginfo_t is 128 bytes on the host, as on x86_64");

  // rt_sigframe order: return address at the stack pointer, then ucontext, then siginfo. (Linux's ucontext is
  // 304 bytes there; this one keeps glibc's 128-byte uc_sigmask, so siginfo sits later. A handler finds both
  // through its rsi and rdx arguments, and rt_sigreturn through its stack pointer.)
  check(kFrameUContext == 8 && kFrameSigInfo == 8 + sizeof(X64::ucontext_t) && kFrameBackup == kFrameSigInfo + 128,
        "return address, ucontext, siginfo, backup address");

  bool aligned = true, ordered = true, below = true;
  for (uint64_t top = 0x7fff0000; top < 0x7fff0000 + 256; top += 8) {
    for (bool avx : {false, true}) {
      const X64FrameLayout l = x64_frame_layout(top, avx);
      const uint64_t fp_size = avx ? sizeof(X64::xstate) : sizeof(X64::_libc_fpstate);
      aligned &= l.entry_sp % 16 == 8 && l.fpstate % 64 == 0;
      ordered &= l.ucontext == l.entry_sp + 8 && l.siginfo == l.ucontext + sizeof(X64::ucontext_t) &&
                 l.backup + 8 <= l.fpstate;
      below &= l.fpstate + fp_size <= top;
    }
  }
  // The saved host context rides in the frame (a handler left by longjmp never runs rt_sigreturn, so a copy on
  // the host stack would be lost): inside the frame, 16-byte aligned, clear of the siginfo and the fp state.
  constexpr uint64_t kSaved = 2192; // size of the shim's SignalBackup; any size must fit
  bool in_frame = true, same_without = true, found = true;
  for (uint64_t top = 0x7fff0000; top < 0x7fff0000 + 256; top += 8) {
    for (bool avx : {false, true}) {
      for (uint64_t size : {uint64_t(16), kSaved, uint64_t(4104)}) {
        const X64FrameLayout l = x64_frame_layout(top, avx, size);
        in_frame &= l.backup_data % 16 == 0 && l.backup_data >= l.backup + 8 && l.backup_data + size <= l.fpstate &&
                    l.entry_sp % 16 == 8 && l.fpstate + (avx ? sizeof(X64::xstate) : sizeof(X64::_libc_fpstate)) <= top;
        found &= backup_data_at(l.entry_sp) == l.backup_data;
      }
      const X64FrameLayout none = x64_frame_layout(top, avx);
      same_without &= none.entry_sp == x64_frame_layout(top, avx, 0).entry_sp && none.backup_data + 8 <= none.fpstate;
    }
  }
  check(in_frame, "the saved host context lies inside the frame, aligned, below the fp state");
  check(found, "rt_sigreturn finds the saved host context from the handler's stack pointer");
  check(same_without, "a frame without a saved host context keeps the plain layout");

  check(aligned, "rsp is 8 modulo 16 at handler entry and the fp state is 64-byte aligned");
  check(ordered, "the frame is laid out in order below the fp state");
  check(below, "nothing is written above the given stack pointer");

  return failures ? 1 : 0;
}
