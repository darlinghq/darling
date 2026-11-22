// The x86_64 rt_sigframe the guest's signal handlers get, and the register restore of rt_sigreturn.
//
// Adapted from FEX (commit 8e95a994a, SignalDelegator/GuestFramesManagement.cpp: SetupFrame_x64 and
// RestoreFrame_x64), Copyright (c) 2019 Ryan Houdek, MIT License; see THIRD-PARTY-NOTICES.md.

#include "darling_fexcore_signals.h"

#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Utils/MathUtils.h>

#include <cstring>

namespace HostCtx = FEX::ArchHelpers::Context;
namespace X64 = FEXCore::x86_64;
using FEXCore::X86State::X87FLAG_C0_LOC;
using FEXCore::X86State::X87FLAG_C1_LOC;
using FEXCore::X86State::X87FLAG_C2_LOC;
using FEXCore::X86State::X87FLAG_C3_LOC;
using FEXCore::X86State::X87FLAG_IE_LOC;
using FEXCore::X86State::X87FLAG_TOP_LOC;

namespace darling_fex {
namespace {

uint32_t trap_number(int sig, const siginfo_t* info) {
  if (sig == SIGSEGV && (info->si_code == SEGV_MAPERR || info->si_code == SEGV_ACCERR)) {
    return FEXCore::X86State::X86_TRAPNO_PF;
  }
  return sig;
}

uint32_t page_fault_error(void* host_ucontext, int sig, const siginfo_t* info) {
  if (host_ucontext && sig == SIGSEGV && (info->si_code == SEGV_MAPERR || info->si_code == SEGV_ACCERR)) {
    return HostCtx::GetProtectFlags(host_ucontext);
  }
  return 0;
}

void set_xstate_info(X64::xstate* xstate, bool avx) {
  auto& sw = xstate->fpstate.sw_reserved;
  sw.magic1 = avx ? X64::fpx_sw_bytes::FP_XSTATE_MAGIC_1 : 0;
  sw.extended_size = avx ? sizeof(X64::xstate) : 0;
  sw.xfeatures |= X64::fpx_sw_bytes::FEATURE_FP | X64::fpx_sw_bytes::FEATURE_SSE;
  if (avx) {
    sw.xfeatures |= X64::fpx_sw_bytes::FEATURE_YMM;
  }
  sw.xstate_size = sw.extended_size;
  if (avx) {
    xstate->xstate_hdr.xfeatures = 0;
    xstate->magic2.magic = X64::fpx_sw_bytes::FP_XSTATE_MAGIC_2;
  }
}

} // namespace

uint64_t setup_x64_frame(FEXCore::Context::Context* ctx, FEXCore::Core::InternalThreadState* thread, uint64_t original_rip,
                         bool fault_to_top, const void* backup_address, const FrameRequest& req) {
  auto* frame = thread->CurrentFrame;
  auto& state = frame->State;

  const X64FrameLayout layout = x64_frame_layout(req.guest_sp, req.supports_avx);
  const uint64_t sp = layout.entry_sp;
  auto* xstate = reinterpret_cast<X64::xstate*>(layout.fpstate);
  auto* uc = reinterpret_cast<X64::ucontext_t*>(layout.ucontext);
  auto* info = reinterpret_cast<siginfo_t*>(layout.siginfo);
  memset(xstate, 0, req.supports_avx ? sizeof(X64::xstate) : sizeof(X64::_libc_fpstate));
  memset(reinterpret_cast<void*>(sp), 0, kFrameSize);
  *reinterpret_cast<uint64_t*>(sp) = req.restorer;
  *reinterpret_cast<uint64_t*>(layout.backup) = reinterpret_cast<uint64_t>(backup_address);

  uc->uc_flags = X64::UC_FP_XSTATE | X64::UC_SIGCONTEXT_SS | X64::UC_STRICT_RESTORE_SS;
  uc->uc_mcontext.fpregs = &xstate->fpstate;
  set_xstate_info(xstate, req.supports_avx);

  auto* gregs = uc->uc_mcontext.gregs;
  gregs[X64::FEX_REG_RIP] = original_rip;
  gregs[X64::FEX_REG_EFL] = req.eflags;
  // CS, GS, FS selectors from the bottom up, and SS in the top 16 bits.
  gregs[X64::FEX_REG_CSGSFS] = (uint64_t(state.ss_idx) << 48) | (uint64_t(state.fs_idx) << 32) | (uint64_t(state.gs_idx) << 16) |
                               uint64_t(state.cs_idx);

  // aarch64 and x86_64 Linux share the siginfo_t layout.
  int sig = req.sig;
  *info = *req.info;
  if (fault_to_top) {
    gregs[X64::FEX_REG_TRAPNO] = frame->SynchronousFaultData.TrapNo;
    gregs[X64::FEX_REG_ERR] = frame->SynchronousFaultData.err_code;
  } else {
    gregs[X64::FEX_REG_TRAPNO] = trap_number(sig, req.info);
    gregs[X64::FEX_REG_ERR] = page_fault_error(req.host_ucontext, sig, req.info);
  }

#define COPY_REG(x) gregs[X64::FEX_REG_##x] = state.gregs[FEXCore::X86State::REG_##x];
  COPY_REG(R8);
  COPY_REG(R9);
  COPY_REG(R10);
  COPY_REG(R11);
  COPY_REG(R12);
  COPY_REG(R13);
  COPY_REG(R14);
  COPY_REG(R15);
  COPY_REG(RDI);
  COPY_REG(RSI);
  COPY_REG(RBP);
  COPY_REG(RBX);
  COPY_REG(RDX);
  COPY_REG(RAX);
  COPY_REG(RCX);
  COPY_REG(RSP);
#undef COPY_REG

  auto* fpstate = &xstate->fpstate;
  const uint16_t top = state.flags[X87FLAG_TOP_LOC];
  for (size_t i = 0; i < FEXCore::Core::CPUState::NUM_MMS; ++i) {
    memcpy(&fpstate->_st[i], &state.mm[(i + top) % 8], sizeof(state.mm[0]));
  }
  ctx->ReconstructXMMRegisters(thread, fpstate->_xmm, req.supports_avx ? xstate->ymmh.ymmh_space : nullptr);
  fpstate->mxcsr = state.mxcsr;
  fpstate->mxcsr_mask = 0xFFC0;
  fpstate->fcw = state.FCW;
  fpstate->ftw = state.AbridgedFTW;
  fpstate->fsw = (state.flags[X87FLAG_TOP_LOC] << 11) | (state.flags[X87FLAG_C0_LOC] << 8) | (state.flags[X87FLAG_C1_LOC] << 9) |
                 (state.flags[X87FLAG_C2_LOC] << 10) | (state.flags[X87FLAG_C3_LOC] << 14) | state.flags[X87FLAG_IE_LOC];

  uc->uc_stack.ss_sp = reinterpret_cast<void*>(req.altstack->sp);
  uc->uc_stack.ss_flags = req.stack_flags;
  uc->uc_stack.ss_size = req.altstack->size;
  uc->uc_sigmask.val[0] = req.old_mask;

  // RAX is zeroed for variadic handlers that read it as a vector register count.
  state.gregs[FEXCore::X86State::REG_RAX] = 0;
  state.gregs[FEXCore::X86State::REG_RDI] = sig;
  state.gregs[FEXCore::X86State::REG_RSI] = reinterpret_cast<uint64_t>(info);
  state.gregs[FEXCore::X86State::REG_RDX] = reinterpret_cast<uint64_t>(uc);
  return sp;
}

void restore_x64_registers(FEXCore::Context::Context* ctx, FEXCore::Core::InternalThreadState* thread, const X64::ucontext_t* guest_uc,
                           bool supports_avx) {
  auto& state = thread->CurrentFrame->State;
  const auto* gregs = guest_uc->uc_mcontext.gregs;

  state.rip = gregs[X64::FEX_REG_RIP];
  // The selectors stay those the handler ran with (the 64-bit user ones): a selector from the frame indexes
  // the descriptor table, and a handler can write any value there.
  ctx->SetFlagsFromCompactedEFLAGS(thread, gregs[X64::FEX_REG_EFL]);

#define COPY_REG(x) state.gregs[FEXCore::X86State::REG_##x] = gregs[X64::FEX_REG_##x];
  COPY_REG(R8);
  COPY_REG(R9);
  COPY_REG(R10);
  COPY_REG(R11);
  COPY_REG(R12);
  COPY_REG(R13);
  COPY_REG(R14);
  COPY_REG(R15);
  COPY_REG(RDI);
  COPY_REG(RSI);
  COPY_REG(RBP);
  COPY_REG(RBX);
  COPY_REG(RDX);
  COPY_REG(RAX);
  COPY_REG(RCX);
  COPY_REG(RSP);
#undef COPY_REG

  // Linux leaves the fp state alone when the handler cleared the pointer.
  const auto* xstate = reinterpret_cast<const X64::xstate*>(guest_uc->uc_mcontext.fpregs);
  if (!xstate) {
    return;
  }
  const auto* fpstate = &xstate->fpstate;
  ctx->SetXMMRegistersFromState(thread, fpstate->_xmm, supports_avx ? xstate->ymmh.ymmh_space : nullptr);
  state.mxcsr = fpstate->mxcsr & 0xFFC0;
  state.FCW = fpstate->fcw;
  state.AbridgedFTW = fpstate->ftw;
  state.flags[X87FLAG_IE_LOC] = fpstate->fsw & 1;
  state.flags[X87FLAG_C0_LOC] = (fpstate->fsw >> 8) & 1;
  state.flags[X87FLAG_C1_LOC] = (fpstate->fsw >> 9) & 1;
  state.flags[X87FLAG_C2_LOC] = (fpstate->fsw >> 10) & 1;
  state.flags[X87FLAG_C3_LOC] = (fpstate->fsw >> 14) & 1;
  state.flags[X87FLAG_TOP_LOC] = (fpstate->fsw >> 11) & 0b111;
  const uint16_t top = state.flags[X87FLAG_TOP_LOC];
  for (size_t i = 0; i < FEXCore::Core::CPUState::NUM_MMS; ++i) {
    memcpy(&state.mm[(i + top) % 8], &fpstate->_st[i], sizeof(state.mm[0]));
  }
}

} // namespace darling_fex
